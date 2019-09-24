// ======================================================================== //
// Copyright 2009-2018 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "../common/math/random_sampler.h"
#include "../common/core/differential_geometry.h"
#include "../common/tutorial/tutorial_device.h"
#include "../common/tutorial/scene_device.h"
#include <algorithm>
#include "../common/tutorial/tutorial.h"

namespace embree {

extern "C" ISPCScene* g_ispc_scene;
extern "C" int g_instancing_mode;
extern "C" bool g_all_hits;
extern "C" int g_max_next_hits;
extern "C" int g_max_total_hits;
extern "C" bool g_verify;
extern "C" bool g_visualize_errors;
extern "C" float g_curve_opacity;

#define MAX_TOTAL_HITS 16*1024

/* extended ray structure that gathers all hits along the ray */
struct HitList
{
  HitList ()
    : begin(0), end(0) {}

  /* return number of gathered hits */
  unsigned int size() const {
    return end-begin;
  }

  /* Hit structure that defines complete order over hits */
  struct Hit
  {
    Hit() {}

    Hit (bool opaque, float t, unsigned int primID = 0xFFFFFFFF, unsigned int geomID = 0xFFFFFFFF, unsigned int instID = 0xFFFFFFFF)
      : opaque(opaque), t(t), primID(primID), geomID(geomID), instID(instID) {}

    /* lexicographical order (t,instID,geomID,primID) */
    __forceinline friend bool operator < (const Hit& a, const Hit& b)
    {
      if (a.t == b.t) {
        if (a.instID == b.instID) {
          if (a.geomID == b.geomID) return a.primID < b.primID;
          else                      return a.geomID < b.geomID;
        }
        else return a.instID < b.instID;
      }
      return a.t < b.t;
    }

    __forceinline friend bool operator == (const Hit& a, const Hit& b) {
      return a.t == b.t && a.primID == b.primID && a.geomID == b.geomID && a.instID == b.instID;
    }

    __forceinline friend bool operator <= (const Hit& a, const Hit& b)
    {
      if (a == b) return true;
      else return a < b;
    }

    __forceinline friend bool operator != (const Hit& a, const Hit& b) {
      return !(a == b);
    }

    friend std::ostream& operator<<(std::ostream& cout, const Hit& hit) {
      return cout << "Hit { opaque = " << hit.opaque << ", t = " << hit.t << ", instID = " << hit.instID << ", geomID = " << hit.geomID << ", primID = " << hit.primID << " }";
    }

  public:
    bool opaque;
    float t;
    unsigned int primID;
    unsigned int geomID;
    unsigned int instID;
  };

public:
  unsigned int begin;   // begin of hit list
  unsigned int end;     // end of hit list
  Hit hits[MAX_TOTAL_HITS];   // array to store all found hits to
};

/* we store the Hit list inside the intersection context to access it from the filter functions */
struct IntersectContext
{
  IntersectContext(HitList& hits)
    : max_next_hits(g_max_next_hits), hits(hits) {}

  RTCIntersectContext context;
  HitList& hits;
  int max_next_hits;
};

/* scene data */
RTCScene g_scene = nullptr;

RTCScene convertScene(ISPCScene* scene_in)
{
  RTCScene scene_out = ConvertScene(g_device, g_ispc_scene, RTC_BUILD_QUALITY_MEDIUM);
  rtcSetSceneFlags(scene_out, rtcGetSceneFlags(scene_out) | RTC_SCENE_FLAG_CONTEXT_FILTER_FUNCTION);

  /* commit individual objects in case of instancing */
  if (g_instancing_mode != ISPC_INSTANCING_NONE)
  {
    for (unsigned int i=0; i<scene_in->numGeometries; i++) {
      ISPCGeometry* geometry = g_ispc_scene->geometries[i];
      if (geometry->type == GROUP) {
        rtcSetSceneFlags(geometry->scene, rtcGetSceneFlags(geometry->scene) | RTC_SCENE_FLAG_CONTEXT_FILTER_FUNCTION);
        rtcCommitScene(geometry->scene);
      }
    }
  }

  /* commit changes to scene */
  return scene_out;
}

/* Filter callback function that gathers all hits */
void gatherAllHits(const struct RTCFilterFunctionNArguments* args)
{
  assert(*args->valid == -1);
  IntersectContext* context = (IntersectContext*) args->context;
  HitList& hits = context->hits;
  RTCRay* ray = (RTCRay*) args->ray;
  RTCHit* hit = (RTCHit*) args->hit;
  assert(args->N == 1);
  args->valid[0] = 0; // ignore all hits
    
  if (hits.end > g_max_total_hits) return;

  /* check if geometry is opaque */
  ISPCGeometry* geometry = (ISPCGeometry*) args->geometryUserPtr;
  bool opaque = geometry->type != CURVES;
  
  HitList::Hit h(opaque,ray->tfar,hit->primID,hit->geomID,hit->instID[0]);

  /* add hit to list */
  hits.hits[hits.end++] = h;
}

/* Filter callback function that gathers first N hits */
void gatherNHits(const struct RTCFilterFunctionNArguments* args)
{
  assert(*args->valid == -1);
  IntersectContext* context = (IntersectContext*) args->context;
  HitList& hits = context->hits;
  RTCRay* ray = (RTCRay*) args->ray;
  RTCHit* hit = (RTCHit*) args->hit;
  assert(args->N == 1);
  args->valid[0] = 0; // ignore all hits
    
  if (hits.end > g_max_total_hits) return;

   /* check if geometry is opaque */
  ISPCGeometry* geometry = (ISPCGeometry*) args->geometryUserPtr;
  bool opaque = geometry->type != CURVES;

  HitList::Hit nhit(opaque,ray->tfar,hit->primID,hit->geomID,hit->instID[0]);

  /* ignore already found hits */
  if (hits.begin > 0 && nhit <= hits.hits[hits.begin-1])
    return;

  /* insert new hit at proper location */
  for (size_t i=hits.begin; i<hits.end; i++)
    if (nhit < hits.hits[i])
      std::swap(nhit,hits.hits[i]);

  /* store furthest hit if place left */
  if (hits.size() < context->max_next_hits)
    hits.hits[hits.end++] = nhit;

  if (hits.size() == context->max_next_hits)
  {
    ray->tfar = hits.hits[hits.end-1].t;
    args->valid[0] = -1; // accept hit
  }
}

/* Filter callback function that gathers first N hits */
void gatherNNonOpaqueHits(const struct RTCFilterFunctionNArguments* args)
{
  assert(*args->valid == -1);
  IntersectContext* context = (IntersectContext*) args->context;
  HitList& hits = context->hits;
  RTCRay* ray = (RTCRay*) args->ray;
  RTCHit* hit = (RTCHit*) args->hit;
  assert(args->N == 1);
  args->valid[0] = 0; // ignore all hits
    
  if (hits.end > g_max_total_hits) return;

  /* check if geometry is opaque */
  ISPCGeometry* geometry = (ISPCGeometry*) args->geometryUserPtr;
  bool opaque = geometry->type != CURVES;
  
  HitList::Hit nhit(opaque, ray->tfar,hit->primID,hit->geomID,hit->instID[0]);

  /* ignore already found hits */
  if (hits.begin > 0 && nhit <= hits.hits[hits.begin-1])
    return;

  /* insert new hit at proper location */
  for (size_t i=hits.begin; i<hits.end; i++)
  {
    if (nhit < hits.hits[i]) {
      std::swap(nhit,hits.hits[i]);
      if (hits.hits[i].opaque) {
        hits.end = i+1;
        break;
      }
    }
  }

  /* store farthest hit if place left and last is not opaque */
  if (hits.size() < context->max_next_hits)
  {
    if (!hits.size() || hits.size() && !hits.hits[hits.end-1].opaque)
      hits.hits[hits.end++] = nhit;
  }

  if (hits.size() == context->max_next_hits || (hits.size() && hits.hits[hits.end-1].opaque))
  {
    ray->tfar = hits.hits[hits.end-1].t;
    args->valid[0] = -1; // accept hit
  }
}

void single_pass(Ray ray, HitList& hits_o, RayStats& stats)
{
  IntersectContext context(hits_o);
  rtcInitIntersectContext(&context.context);
  
  context.context.filter = gatherAllHits;
  rtcIntersect1(g_scene,&context.context,RTCRayHit_(ray));
  RayStats_addRay(stats);
  std::sort(&context.hits.hits[context.hits.begin],&context.hits.hits[context.hits.end]);

  /* ignore duplicated hits that can occur for tesselated primitives */
  if (hits_o.size())
  {
    size_t i=0, j=1;
    for (; j<hits_o.size(); j++) {
      if (hits_o.hits[i] == hits_o.hits[j]) continue;
      hits_o.hits[++i] = hits_o.hits[j];
    }
    hits_o.end = i+1;
  }
}

void multi_pass(Ray ray, HitList& hits_o, int max_next_hits, RayStats& stats)
{
  IntersectContext context(hits_o);
  rtcInitIntersectContext(&context.context);

  context.max_next_hits = max_next_hits;
  context.context.filter = gatherNHits;
  //context.context.filter = gatherNNonOpaqueHits;
  
  int iter = 0;
  do {
    
    if (context.hits.end)
      ray.tnear() = context.hits.hits[context.hits.end-1].t;
    
    ray.tfar = inf;
    ray.geomID = RTC_INVALID_GEOMETRY_ID;
    ray.instID[0] = RTC_INVALID_GEOMETRY_ID;
    context.hits.begin = context.hits.end;
    
    for (size_t i=0; i<context.max_next_hits; i++)
      if (context.hits.begin+i < g_max_total_hits)
        context.hits.hits[context.hits.begin+i] = HitList::Hit(false,neg_inf);

    rtcIntersect1(g_scene,&context.context,RTCRayHit_(ray));
    RayStats_addRay(stats);
    iter++;

    //if (context.hits.size() && context.hits.hits[context.hits.end-1].opaque)
    //  break;
    
  } while (context.hits.size() != 0);
  
  context.hits.begin = 0;
}

/* number of hits found in previous pass */
int* g_num_prev_hits = nullptr;
unsigned int g_num_prev_hits_width = 0;
unsigned int g_num_prev_hits_height = 0;

void hair_pass(Ray ray, HitList& hits_o, int max_next_hits, RandomSampler& sampler, RayStats& stats)
{
  IntersectContext context(hits_o);
  rtcInitIntersectContext(&context.context);

  context.max_next_hits = max_next_hits;
  context.context.filter = gatherNNonOpaqueHits;
  
  int iter = 0;
  do {
    
    if (context.hits.end) 
      ray.tnear() = context.hits.hits[context.hits.end-1].t;
    
    ray.tfar = inf;
    ray.geomID = RTC_INVALID_GEOMETRY_ID;
    ray.instID[0] = RTC_INVALID_GEOMETRY_ID;
    context.hits.begin = context.hits.end;
    
    for (size_t i=0; i<context.max_next_hits; i++)
      if (context.hits.begin+i < g_max_total_hits)
        context.hits.hits[context.hits.begin+i] = HitList::Hit(false,neg_inf);

    rtcIntersect1(g_scene,&context.context,RTCRayHit_(ray));
    RayStats_addRay(stats);
    iter++;

    /* shade all hits */
    for (size_t i=context.hits.begin; i<context.hits.end; i++)
    {
      HitList::Hit& hit = context.hits.hits[i];

      bool opaque = hit.opaque;
      if (RandomSampler_get1D(sampler) < g_curve_opacity)
        opaque = true;
        
      if (opaque) {
        context.hits.begin = 0;
        context.hits.end = i+1;
        return;
      }
    }
    
  } while (context.hits.size() != 0);
  
  context.hits.begin = 0;
}

/* task that renders a single screen tile */
Vec3fa renderPixelStandard(float x, float y, const ISPCCamera& camera, RayStats& stats)
{
  int ix = (int)x;
  int iy = (int)y;
  
  /* initialize sampler */
  RandomSampler mysampler;
  RandomSampler_init(mysampler, ix, iy, 0);
  
  bool has_error = false;
  HitList hits;

  int max_next_hits = g_max_next_hits;
  if (max_next_hits == 0)
    max_next_hits = max(1,g_num_prev_hits[iy*TutorialApplication::instance->width+ix]);
  
  /* initialize ray */
  Ray ray(Vec3fa(camera.xfm.p), Vec3fa(normalize(x*camera.xfm.l.vx + y*camera.xfm.l.vy + camera.xfm.l.vz)), 0.0f, inf, 0.0f);

  /* either gather hits in single pass or using multiple passes */
  if (g_all_hits)
    single_pass(ray,hits,stats);
  else if (g_curve_opacity >= 0.0f) 
    hair_pass(ray,hits,max_next_hits,mysampler,stats);
  else
    multi_pass (ray,hits,max_next_hits,stats);

  /* verify result with gathering all hits */
  if (g_verify || g_visualize_errors)
  {
    HitList verify_hits;
    single_pass(ray,verify_hits,stats);

    //std::cout << std::hexfloat;
    //for (size_t i=0; i<hits.size(); i++)
    //  PRINT2(i,hits.hits[i]);
     
    //for (size_t i=0; i<verify_hits.size(); i++)
    //  PRINT2(i,verify_hits.hits[i]);
    
    if (verify_hits.size() != hits.size())
      has_error = true;
    
    for (size_t i=verify_hits.begin; i<verify_hits.end; i++)
    {
      if (verify_hits.hits[i] != hits.hits[i])
        has_error = true;
    }

    if (!g_visualize_errors && has_error)
      throw std::runtime_error("hits differ");
  }

  /* calculate random sequence based on hit geomIDs and primIDs */
  RandomSampler sampler = { 0 };
  for (size_t i=hits.begin; i<hits.end; i++) {
    sampler.s = MurmurHash3_mix(sampler.s, hits.hits[i].instID);
    sampler.s = MurmurHash3_mix(sampler.s, hits.hits[i].geomID);
    sampler.s = MurmurHash3_mix(sampler.s, hits.hits[i].primID);
  }
  sampler.s = MurmurHash3_finalize(sampler.s);

  /* map geomID/primID sequence to color */
  Vec3fa color;
  color.x = RandomSampler_getFloat(sampler);
  color.y = RandomSampler_getFloat(sampler);
  color.z = RandomSampler_getFloat(sampler);

  /* mark errors red */
  if (g_visualize_errors)
  {
    color.x = color.y = color.z;
    if (has_error)
      color = Vec3fa(1,0,0);
  }
  
  color.w = hits.size();
  return color;
}

/* renders a single screen tile */
void renderTileStandard(int taskIndex,
                        int threadIndex,
                        int* pixels,
                        const unsigned int width,
                        const unsigned int height,
                        const float time,
                        const ISPCCamera& camera,
                        const int numTilesX,
                        const int numTilesY)
{
  const int t = taskIndex;
  const unsigned int tileY = t / numTilesX;
  const unsigned int tileX = t - tileY * numTilesX;
  const unsigned int x0 = tileX * TILE_SIZE_X;
  const unsigned int x1 = min(x0+TILE_SIZE_X,width);
  const unsigned int y0 = tileY * TILE_SIZE_Y;
  const unsigned int y1 = min(y0+TILE_SIZE_Y,height);

  for (unsigned int y=y0; y<y1; y++) for (unsigned int x=x0; x<x1; x++)
  {
    Vec3fa color = renderPixelStandard((float)x,(float)y,camera,g_stats[threadIndex]);

    /* write color to framebuffer */
    unsigned int r = (unsigned int) (255.0f * clamp(color.x,0.0f,1.0f));
    unsigned int g = (unsigned int) (255.0f * clamp(color.y,0.0f,1.0f));
    unsigned int b = (unsigned int) (255.0f * clamp(color.z,0.0f,1.0f));
    pixels[y*width+x] = (b << 16) + (g << 8) + r;
    g_num_prev_hits[y*width+x] = (int) color.w;
  }
}

/* task that renders a single screen tile */
void renderTileTask (int taskIndex, int threadIndex, int* pixels,
                         const unsigned int width,
                         const unsigned int height,
                         const float time,
                         const ISPCCamera& camera,
                         const int numTilesX,
                         const int numTilesY)
{
  renderTile(taskIndex,threadIndex,pixels,width,height,time,camera,numTilesX,numTilesY);
}

/* called by the C++ code for initialization */
extern "C" void device_init (const char* cfg)
{
  /* set start render mode */
  renderTile = renderTileStandard;
  key_pressed_handler = device_key_pressed_default;
}

/* called by the C++ code to render */
extern "C" void device_render (unsigned* pixels,
                           const unsigned int width,
                           const unsigned int height,
                           const float time,
                           const ISPCCamera& camera)
{
  /* create scene */
  if (g_scene == nullptr) {
    g_scene = convertScene(g_ispc_scene);
    rtcCommitScene (g_scene);
  }

  /* create buffer to remember previous number of hits found */
  if (!g_num_prev_hits || g_num_prev_hits_width != width || g_num_prev_hits_height != height)
  {
    delete[] g_num_prev_hits;
    g_num_prev_hits = new int[width*height];
    g_num_prev_hits_width = width;
    g_num_prev_hits_height = height;
    for (unsigned int i=0; i<width*height; i++)
      g_num_prev_hits[i] = 1;
  }

  /* render image */
  const int numTilesX = (width +TILE_SIZE_X-1)/TILE_SIZE_X;
  const int numTilesY = (height+TILE_SIZE_Y-1)/TILE_SIZE_Y;
  parallel_for(size_t(0),size_t(numTilesX*numTilesY),[&](const range<size_t>& range) {
    const int threadIndex = (int)TaskScheduler::threadIndex();
    for (size_t i=range.begin(); i<range.end(); i++)
      renderTileTask((int)i,threadIndex,(int*)pixels,width,height,time,camera,numTilesX,numTilesY);
  });
  //rtcDebug();
}

/* called by the C++ code for cleanup */
extern "C" void device_cleanup ()
{
  rtcReleaseScene (g_scene); g_scene = nullptr;
  delete[] g_num_prev_hits; g_num_prev_hits = nullptr;
}

} // namespace embree
