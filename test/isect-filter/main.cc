#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4324)
#pragma warning(disable : 4457)
#pragma warning(disable : 4456)
#endif

//#include "embree3/rtcore.h"

//#include "../../kernels/bvh/bvh.h"

/* include embree API */
#include "../../include/embree3/rtcore.h"

/* include optional vector library */
#include "../../tutorials/common/math/math.h"
#include "../../tutorials/common/math/vec.h"
#include "../../tutorials/common/math/affinespace.h"
#include "../../tutorials/common/core/ray.h"
//#include "camera.h"
//#include "scene_device.h"
//#include "noise.h"
//#include "../../tutorials/common/core/ray.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <cstdio>
#include <cstdlib>
#include <limits>
#include <cmath>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>


template <typename T = float>
class real3 {
 public:
  real3() {}
  real3(T x) {
    v[0] = x;
    v[1] = x;
    v[2] = x;
  }
  real3(T xx, T yy, T zz) {
    v[0] = xx;
    v[1] = yy;
    v[2] = zz;
  }
  explicit real3(const T *p) {
    v[0] = p[0];
    v[1] = p[1];
    v[2] = p[2];
  }

  inline T x() const { return v[0]; }
  inline T y() const { return v[1]; }
  inline T z() const { return v[2]; }

  real3 operator*(T f) const { return real3(x() * f, y() * f, z() * f); }
  real3 operator-(const real3 &f2) const {
    return real3(x() - f2.x(), y() - f2.y(), z() - f2.z());
  }
  real3 operator*(const real3 &f2) const {
    return real3(x() * f2.x(), y() * f2.y(), z() * f2.z());
  }
  real3 operator+(const real3 &f2) const {
    return real3(x() + f2.x(), y() + f2.y(), z() + f2.z());
  }
  real3 &operator+=(const real3 &f2) {
    v[0] += f2.x();
    v[1] += f2.y();
    v[2] += f2.z();
    return (*this);
  }
  real3 operator/(const real3 &f2) const {
    return real3(x() / f2.x(), y() / f2.y(), z() / f2.z());
  }
  real3 operator-() const { return real3(-x(), -y(), -z()); }
  T operator[](int i) const { return v[i]; }
  T &operator[](int i) { return v[i]; }

  T v[3];
  // T pad;  // for alignment(when T = float)
};

template <typename T>
inline real3<T> operator*(T f, const real3<T> &v) {
  return real3<T>(v.x() * f, v.y() * f, v.z() * f);
}

template <typename T>
inline real3<T> vneg(const real3<T> &rhs) {
  return real3<T>(-rhs.x(), -rhs.y(), -rhs.z());
}

template <typename T>
inline T vlength(const real3<T> &rhs) {
  return std::sqrt(rhs.x() * rhs.x() + rhs.y() * rhs.y() + rhs.z() * rhs.z());
}

template <typename T>
inline real3<T> vnormalize(const real3<T> &rhs) {
  real3<T> v = rhs;
  T len = vlength(rhs);
  if (std::fabs(len) > std::numeric_limits<T>::epsilon()) {
    T inv_len = static_cast<T>(1.0) / len;
    v.v[0] *= inv_len;
    v.v[1] *= inv_len;
    v.v[2] *= inv_len;
  }
  return v;
}

template <typename T>
inline real3<T> vcross(const real3<T> a, const real3<T> b) {
  real3<T> c;
  c[0] = a[1] * b[2] - a[2] * b[1];
  c[1] = a[2] * b[0] - a[0] * b[2];
  c[2] = a[0] * b[1] - a[1] * b[0];
  return c;
}

template <typename T>
inline T vdot(const real3<T> a, const real3<T> b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

template <typename T>
inline real3<T> vsafe_inverse(const real3<T> v) {
  real3<T> r;

  if (std::fabs(v[0]) < std::numeric_limits<T>::epsilon()) {
    r[0] = std::numeric_limits<T>::infinity() * std::copysign(static_cast<T>(1), v[0]);
  } else {
    r[0] = static_cast<T>(1.0) / v[0];
  }

  if (std::fabs(v[1]) < std::numeric_limits<T>::epsilon()) {
    r[1] = std::numeric_limits<T>::infinity() * std::copysign(static_cast<T>(1), v[1]);
  } else {
    r[1] = static_cast<T>(1.0) / v[1];
  }

  if (std::fabs(v[2]) < std::numeric_limits<T>::epsilon()) {
    r[2] = std::numeric_limits<T>::infinity() * std::copysign(static_cast<T>(1), v[2]);
  } else {
    r[2] = static_cast<T>(1.0) / v[2];
  }

  return r;
}

typedef real3<float> float3;


// ---------------------------------------------------------------------------
// Intersection filter based on Embree's intersection_filter tutorial
//


void InitRay(Ray &ray,
                            const embree::Vec3fa& org,
                            const embree::Vec3fa& dir,
                            float tnear = 0.0f,
                            float tfar = 1.0e+14f,
                            float time = 0.0f,
                            int mask = -1,
                            unsigned int geomID = RTC_INVALID_GEOMETRY_ID,
                            unsigned int primID = RTC_INVALID_GEOMETRY_ID)
{
  ray = Ray(org,dir,tnear,tfar,time,mask,geomID,primID);
}


/*! intersection context passed to intersect/occluded calls */
#if 0
struct IntersectContextIF
{
  RTCIntersectContext context;
  void* userRayExt;               //!< can be used to pass extended ray data to callbacks
};

__forceinline void InitIntersectionContextIF(struct IntersectContextIF* context)
{
  rtcInitIntersectContext(&context->context);
  context->userRayExt = NULL;
}
#endif

using namespace embree;

Vec3fa* colors = nullptr;

/******************************************************************************************/
/*                             Standard Mode                                              */
/******************************************************************************************/

#define HIT_LIST_LENGTH (16)

/* extended ray structure that includes total transparency along the ray */
struct Ray2
{
  Ray ray;

  // ray extensions
  float transparency; //!< accumulated transparency value

  // we remember up to 16 hits to ignore duplicate hits
  unsigned int firstHit, lastHit;
  unsigned int hit_geomIDs[HIT_LIST_LENGTH];
  unsigned int hit_primIDs[HIT_LIST_LENGTH];
};

inline RTCRayHit* RTCRayHit_(Ray2& ray)
{
  RTCRayHit* ray_ptr = (RTCRayHit*)&ray;
  return ray_ptr;
}

inline RTCRay* RTCRay_(Ray2& ray)
{
  RTCRay* ray_ptr = (RTCRay*)&ray;
  return ray_ptr;
}

/* 3D procedural transparency */
inline float transparencyFunction(Vec3fa& h)
{
  float v = std::abs(std::sin(4.0f*h.x)*std::cos(4.0f*h.y)*std::sin(4.0f*h.z));
  float T = clamp((v-0.1f)*3.0f,0.0f,1.0f);
  return T;
  //return 0.5f;
}


#if 0
/* task that renders a single screen tile */
Vec3fa renderPixelStandard(float x, float y, const ISPCCamera& camera, RayStats& stats)
{
  float weight = 1.0f;
  Vec3fa color = Vec3fa(0.0f);

  IntersectContext context;
  InitIntersectionContext(&context);
  
  /* initialize ray */
  Ray2 primary;
  init_Ray(primary.ray,Vec3fa(camera.xfm.p), Vec3fa(normalize(x*camera.xfm.l.vx + y*camera.xfm.l.vy + camera.xfm.l.vz)), 0.0f, inf);
  primary.ray.mask = 0; // needs to encode rayID for filter
  primary.transparency = 0.0f;


  while (true)
  {
    context.userRayExt = &primary;

    /* intersect ray with scene */
    rtcIntersect1(g_scene,&context.context,RTCRayHit_(primary));
    RayStats_addRay(stats);

    /* shade pixels */
    if (primary.ray.geomID == RTC_INVALID_GEOMETRY_ID)
      break;

    float opacity = 1.0f-primary.transparency;
    Vec3fa diffuse = colors[primary.ray.primID];
    Vec3fa La = diffuse*0.5f;
    color = color + weight*opacity*La;
    Vec3fa lightDir = normalize(Vec3fa(-1,-1,-1));

    /* initialize shadow ray */
    Ray2 shadow;
    init_Ray(shadow.ray, primary.ray.org + primary.ray.tfar*primary.ray.dir, neg(lightDir), 0.001f, inf);
    shadow.ray.mask = 0; // needs to encode rayID for filter
    shadow.transparency = 1.0f;
    shadow.firstHit = 0;
    shadow.lastHit = 0;
    context.userRayExt = &shadow;

    /* trace shadow ray */
    rtcOccluded1(g_scene,&context.context,RTCRay_(shadow));
    RayStats_addShadowRay(stats);

    /* add light contribution */
    if (shadow.ray.tfar >= 0.0f) {
      Vec3fa Ll = diffuse*shadow.transparency*clamp(-dot(lightDir,normalize(primary.ray.Ng)),0.0f,1.0f);
      color = color + weight*opacity*Ll;
    }

    /* shoot transmission ray */
    weight *= primary.transparency;
    primary.ray.tnear() = 1.001f*primary.ray.tfar;
    primary.ray.tfar = (float)(inf);
    primary.ray.geomID = RTC_INVALID_GEOMETRY_ID;
    primary.ray.primID = RTC_INVALID_GEOMETRY_ID;
    primary.transparency = 0.0f;
  }
  return color;
}
#endif

/* intersection filter function for single rays and packets */
void intersectionFilter(const RTCFilterFunctionNArguments* args)
{
  /* avoid crashing when debug visualizations are used */
  if (args->context == nullptr) return;

  assert(args->N == 1);
  int* valid = args->valid;
  const IntersectContext* context = (const IntersectContext*) args->context;
  Ray* ray = (Ray*)args->ray;
  //RTCHit* hit = (RTCHit*)args->hit;

  /* ignore inactive rays */
  if (valid[0] != -1) return;

  /* calculate transparency */
  Vec3fa h = ray->org + ray->dir  * ray->tfar;
  float T = transparencyFunction(h);

  /* ignore hit if completely transparent */
  if (T >= 1.0f) 
    valid[0] = 0;
  /* otherwise accept hit and remember transparency */
  else
  {
    Ray2* eray = (Ray2*) context->userRayExt;
    eray->transparency = T;
  }
}

/* occlusion filter function for single rays and packets */
void occlusionFilter(const RTCFilterFunctionNArguments* args)
{
  /* avoid crashing when debug visualizations are used */
  if (args->context == nullptr) return;

  assert(args->N == 1);
  int* valid = args->valid;
  const IntersectContext* context = (const IntersectContext*) args->context;
  Ray* ray = (Ray*)args->ray;
  RTCHit* hit = (RTCHit*)args->hit;

  /* ignore inactive rays */
  if (valid[0] != -1) return;

  Ray2* ray2 = (Ray2*) context->userRayExt;
  assert(ray2);

  for (unsigned int i=ray2->firstHit; i<ray2->lastHit; i++) {
    unsigned slot= i%HIT_LIST_LENGTH;
    if (ray2->hit_geomIDs[slot] == hit->geomID && ray2->hit_primIDs[slot] == hit->primID) {
      valid[0] = 0; return; // ignore duplicate intersections
    }
  }
  /* store hit in hit list */
  unsigned int slot = ray2->lastHit%HIT_LIST_LENGTH;
  ray2->hit_geomIDs[slot] = hit->geomID;
  ray2->hit_primIDs[slot] = hit->primID;
  ray2->lastHit++;
  if (ray2->lastHit - ray2->firstHit >= HIT_LIST_LENGTH)
    ray2->firstHit++;

  Vec3fa h = ray->org + ray->dir * ray->tfar;

  /* calculate and accumulate transparency */
  float T = transparencyFunction(h);
  T *= ray2->transparency;
  ray2->transparency = T;
  if (T != 0.0f) 
    valid[0] = 0;
}

/******************************************************************************************/
/*                              Scene Creation                                            */
/******************************************************************************************/

#define NUM_VERTICES 8
#define NUM_QUAD_INDICES 24
#define NUM_TRI_INDICES 36
#define NUM_QUAD_FACES 6
#define NUM_TRI_FACES 12

__aligned(16) float cube_vertices[NUM_VERTICES][4] =
{
  { -1, -1, -1, 0 },
  { -1, -1, +1, 0 },
  { -1, +1, -1, 0 },
  { -1, +1, +1, 0 },
  { +1, -1, -1, 0 },
  { +1, -1, +1, 0 },
  { +1, +1, -1, 0 },
  { +1, +1, +1, 0 },
};

unsigned int cube_quad_indices[NUM_QUAD_INDICES] = {
  0, 1, 3, 2,
  5, 4, 6, 7,
  0, 4, 5, 1,
  6, 2, 3, 7,
  0, 2, 6, 4,
  3, 1, 5, 7
};

unsigned int cube_tri_indices[NUM_TRI_INDICES] = {
  0, 1, 2,  2, 1, 3,
  5, 4, 7,  7, 4, 6,
  0, 4, 1,  1, 4, 5,
  6, 2, 7,  7, 2, 3,
  0, 2, 4,  4, 2, 6,
  3, 1, 7,  7, 1, 5
};

unsigned int cube_quad_faces[NUM_QUAD_FACES] = {
  4, 4, 4, 4, 4, 4
};

/* adds a cube to the scene */
unsigned int addCube (RTCDevice device, RTCScene scene_i, const Vec3fa& offset, const Vec3fa& scale, float rotation)
{
  /* create a triangulated cube with 12 triangles and 8 vertices */
  RTCGeometry geom = rtcNewGeometry (device, RTC_GEOMETRY_TYPE_TRIANGLE);
  //rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, cube_vertices,     0, sizeof(Vec3fa  ), NUM_VERTICES);
  Vec3fa* ptr = (Vec3fa*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vec3fa), NUM_VERTICES);
  for (unsigned int i=0; i<NUM_VERTICES; i++) {
    float x = cube_vertices[i][0];
    float y = cube_vertices[i][1];
    float z = cube_vertices[i][2];
    Vec3fa vtx = Vec3fa(x,y,z);
    ptr[i] = Vec3fa(offset+LinearSpace3fa::rotate(Vec3fa(0,1,0),rotation)*LinearSpace3fa::scale(scale)*vtx);
  }
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, cube_tri_indices, 0, 3*sizeof(unsigned int), NUM_TRI_FACES);

  /* create per-triangle color array */
  colors = (Vec3fa*) malloc(12*sizeof(Vec3fa));
  colors[0] = Vec3fa(1,0,0); // left side
  colors[1] = Vec3fa(1,0,0);
  colors[2] = Vec3fa(0,1,0); // right side
  colors[3] = Vec3fa(0,1,0);
  colors[4] = Vec3fa(0.5f);  // bottom side
  colors[5] = Vec3fa(0.5f);
  colors[6] = Vec3fa(1.0f);  // top side
  colors[7] = Vec3fa(1.0f);
  colors[8] = Vec3fa(0,0,1); // front side
  colors[9] = Vec3fa(0,0,1);
  colors[10] = Vec3fa(1,1,0); // back side
  colors[11] = Vec3fa(1,1,0);

  /* set intersection filter for the cube */
  {
    rtcSetGeometryIntersectFilterFunction(geom,intersectionFilter);
    rtcSetGeometryOccludedFilterFunction(geom,occlusionFilter);
  }

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene_i,geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

#if 0
/* adds a cube to the scene */
unsigned int addSubdivCube (RTCScene scene_i)
{
  RTCGeometry geom = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_SUBDIVISION);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, cube_vertices,      0, sizeof(Vec3fa),       NUM_VERTICES);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX,  0, RTC_FORMAT_UINT,   cube_quad_indices,  0, sizeof(unsigned int), NUM_QUAD_INDICES);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_FACE,   0, RTC_FORMAT_UINT,   cube_quad_faces,    0, sizeof(unsigned int), NUM_QUAD_FACES);

  float* level = (float*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_LEVEL, 0, RTC_FORMAT_FLOAT, sizeof(float), NUM_QUAD_INDICES);
  for (unsigned int i=0; i<NUM_QUAD_INDICES; i++) level[i] = 4;

  /* create face color array */
  colors = (Vec3fa*) alignedMalloc(6*sizeof(Vec3fa),16);
  colors[0] = Vec3fa(1,0,0); // left side
  colors[1] = Vec3fa(0,1,0); // right side
  colors[2] = Vec3fa(0.5f);  // bottom side
  colors[3] = Vec3fa(1.0f);  // top side
  colors[4] = Vec3fa(0,0,1); // front side
  colors[5] = Vec3fa(1,1,0); // back side

  /* set intersection filter for the cube */
  if (g_mode == MODE_NORMAL && nativePacketSupported(g_device))
  {
    rtcSetGeometryIntersectFilterFunction(geom,intersectionFilter);
    rtcSetGeometryOccludedFilterFunction(geom,occlusionFilter);
  }
  else
  {
    rtcSetGeometryIntersectFilterFunction(geom,intersectionFilterN);
    rtcSetGeometryOccludedFilterFunction(geom,occlusionFilterN);
  }

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene_i,geom);
  rtcReleaseGeometry(geom);
  return geomID;
}
#endif

/* adds a ground plane to the scene */
unsigned int addGroundPlane (RTCDevice device, RTCScene scene_i)
{
  /* create a triangulated plane with 2 triangles and 4 vertices */
  RTCGeometry geom = rtcNewGeometry (device, RTC_GEOMETRY_TYPE_TRIANGLE);

  /* set vertices */
  float* vertices = (float*) rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(float)*4,4);
  vertices[4 * 0 + 0] = -10; vertices[4 * 0 + 1] = -2; vertices[4 * 0 + 2] = -10;
  vertices[4 * 1 + 0] = -10; vertices[4 * 1 + 1] = -2; vertices[4 * 1 + 2] = +10;
  vertices[4 * 2 + 0] = +10; vertices[4 * 2 + 1] = -2; vertices[4 * 2 + 2] = -10;
  vertices[4 * 3 + 0] = +10; vertices[4 * 3 + 1] = -2; vertices[4 * 3 + 2] = +10;

  /* set triangles */
  uint32_t* triangles = (uint32_t*) rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,sizeof(uint32_t) * 3,2);
  triangles[0] = 0; triangles[1] = 1; triangles[2] = 2;
  triangles[3] = 1; triangles[4] = 3; triangles[5] = 2;

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene_i,geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

Vec3fa renderPixelStandard(RTCScene scene, float x, float y, const Vec3fa &camera_org, const Vec3fa &camera_dir)
{
  float weight = 1.0f;
  Vec3fa color = Vec3fa(0.0f);

  IntersectContext context;
  InitIntersectionContext(&context);
  
  /* initialize ray */
  Ray2 primary;
  InitRay(primary.ray, camera_org, camera_dir, 0.0f, 1.0e+10f);
  primary.ray.mask = 0; // needs to encode rayID for filter
  primary.transparency = 0.0f;


  while (true)
  {
    context.userRayExt = &primary;

    /* intersect ray with scene */
    rtcIntersect1(scene,&context.context,RTCRayHit_(primary));
    //RayStats_addRay(stats);

    /* shade pixels */
    if (primary.ray.geomID == RTC_INVALID_GEOMETRY_ID)
      break;

    float opacity = 1.0f-primary.transparency;
    Vec3fa diffuse = colors[primary.ray.primID];
    Vec3fa La = diffuse*0.5f;
    color = color + weight*opacity*La;
    Vec3fa lightDir = normalize(Vec3fa(-1,-1,-1));

    /* initialize shadow ray */
    Ray2 shadow;
    init_Ray(shadow.ray, primary.ray.org + primary.ray.tfar*primary.ray.dir, neg(lightDir), 0.001f, 1.0e+10f);
    shadow.ray.mask = 0; // needs to encode rayID for filter
    shadow.transparency = 1.0f;
    shadow.firstHit = 0;
    shadow.lastHit = 0;
    context.userRayExt = &shadow;

    /* trace shadow ray */
    rtcOccluded1(scene,&context.context,RTCRay_(shadow));
    //RayStats_addShadowRay(stats);

    /* add light contribution */
    if (shadow.ray.tfar >= 0.0f) {
      Vec3fa Ll = diffuse*shadow.transparency*clamp(-dot(lightDir,normalize(primary.ray.Ng)),0.0f,1.0f);
      color = color + weight*opacity*Ll;
    }

    /* shoot transmission ray */
    weight *= primary.transparency;
    primary.ray.tnear() = 1.001f*primary.ray.tfar;
    primary.ray.tfar = (float)(inf);
    primary.ray.geomID = RTC_INVALID_GEOMETRY_ID;
    primary.ray.primID = RTC_INVALID_GEOMETRY_ID;
    primary.transparency = 0.0f;
  }
  return color;
}
// ---------------------------------------------------------------------------

static inline void vnormalize(float dst[3], const float v[3]) {
  dst[0] = v[0];
  dst[1] = v[1];
  dst[2] = v[2];
  const float len = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
  if (std::fabs(len) > std::numeric_limits<float>::epsilon()) {
    const float inv_len = 1.0f / len;
    dst[0] *= inv_len;
    dst[1] *= inv_len;
    dst[2] *= inv_len;
  }
}

#if 0
static void MultV(float dst[3], const float m[4][4], const float v[3]) {
  dst[0] = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0];
  dst[1] = m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1];
  dst[2] = m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2];
}
#endif

static void SaveImagePNG(const char *filename, const float *rgb, int width,
                         int height) {
  unsigned char *bytes = new unsigned char[size_t(width * height * 3)];
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const int index = y * width + x;
      bytes[index * 3 + 0] = static_cast<unsigned char>(
          std::max(0.0f, std::min(rgb[index * 3 + 0] * 255.0f, 255.0f)));
      bytes[index * 3 + 1] = static_cast<unsigned char>(
          std::max(0.0f, std::min(rgb[index * 3 + 1] * 255.0f, 255.0f)));
      bytes[index * 3 + 2] = static_cast<unsigned char>(
          std::max(0.0f, std::min(rgb[index * 3 + 2] * 255.0f, 255.0f)));
    }
  }
  stbi_write_png(filename, width, height, 3, bytes, width * 3);
  delete[] bytes;
}

void error_handler(void* userPtr, RTCError code, const char* str = nullptr)
{
  if (code == RTC_ERROR_NONE)
    return;
 
  printf("Embree: ");
  switch (code) {
  case RTC_ERROR_UNKNOWN          : printf("RTC_ERROR_UNKNOWN"); break;
  case RTC_ERROR_INVALID_ARGUMENT : printf("RTC_ERROR_INVALID_ARGUMENT"); break;
  case RTC_ERROR_INVALID_OPERATION: printf("RTC_ERROR_INVALID_OPERATION"); break;
  case RTC_ERROR_OUT_OF_MEMORY    : printf("RTC_ERROR_OUT_OF_MEMORY"); break;
  case RTC_ERROR_UNSUPPORTED_CPU  : printf("RTC_ERROR_UNSUPPORTED_CPU"); break;
  case RTC_ERROR_CANCELLED        : printf("RTC_ERROR_CANCELLED"); break;
  default                         : printf("invalid error code"); break;
  }
  if (str) {
    printf(" (");
    while (*str) putchar(*str++);
    printf(")\n");
  }
  exit(1);
}

#if 0
static void print_bvh4(embree::BVH4::NodeRef node, size_t depth)
{
  if (node.isAlignedNode())
  { 
    embree::BVH4::AlignedNode* n = node.alignedNode();
    
    std::cout << "AlignedNode(" << node << ") {" << std::endl;
    for (size_t i=0; i<4; i++)
    { 
      for (size_t k=0; k<depth; k++) std::cout << "  ";
      std::cout << "  bounds" << i << " = " << n->bounds(i) << std::endl;
    }
    
    for (size_t i=0; i<4; i++)
    { 
      if (n->child(i) == embree::BVH4::emptyNode)
        continue;
      
      for (size_t k=0; k<depth; k++) std::cout << "  ";
      std::cout << "  child(" << n->child(i) << ") " << i << " = ";
      print_bvh4(n->child(i),depth+1);
    }
    for (size_t k=0; k<depth; k++) std::cout << "  ";
    std::cout << "}" << std::endl;
  } else {
    // dump leaf.
  }
}

void DumpScene(RTCScene scene)
{
  embree::AccelData *accel = ((embree::Accel*)scene)->intersectors.ptr;
  std::cout << "ty = " << accel->type << std::endl;
  if (accel->type == embree::AccelData::TY_BVH4) {
    std::cout << "got it" << std::endl;
     embree::BVH4 *bvh4 = (embree::BVH4*)accel;
    print_bvh4(bvh4->root, 0);
  }
} 
#endif

static void bora() {
  std::cout << "bora" << std::endl;
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  RTCDevice device = rtcNewDevice(nullptr);
  error_handler(nullptr,rtcGetDeviceError(device));

  /* set error handler */
  rtcSetDeviceErrorFunction(device,error_handler,nullptr);
  RTCScene scene = rtcNewScene(device);

  //rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);
  //rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);

  /* add cube */
  addCube(device, scene,Vec3fa(0.0f,0.0f,0.0f),Vec3fa(10.0f,1.0f,1.0f),45.0f);
  //addSubdivCube(g_scene);

  /* add ground plane */
  addGroundPlane(device, scene);

  rtcCommitScene(scene);

  //DumpScene(scene);

  int width = 512;
  int height = 512;

  std::vector<float> rgb(width * height * 3, 0.0f);

  auto t_start = std::chrono::high_resolution_clock::now();
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // Simple camera. change eye pos and direction fit to .obj model.

      float dir[3], ndir[3];
      dir[0] = (x / float(width)) - 0.5f;
      dir[1] = (y / float(height)) - 0.5f;
      dir[2] = -1.0f;
      vnormalize(ndir, dir);

      Vec3fa camorg(0.0f, 0.0f, 8.0f);
      Vec3fa camdir(ndir[0], ndir[1], ndir[2]);

      Vec3fa col = renderPixelStandard(scene, x, y, camorg, camdir);

      // Flip Y
      rgb[3 * size_t((height - y - 1) * width + x) + 0] = col[0];
      rgb[3 * size_t((height - y - 1) * width + x) + 1] = col[1];
      rgb[3 * size_t((height - y - 1) * width + x) + 2] = col[2];
    }
  }
  auto t_end = std::chrono::high_resolution_clock::now();
  double elaspedTimeMs = std::chrono::duration<double, std::milli>(t_end-t_start).count();
  std::cout <<  std::fixed << std::setprecision(2)
            << "Elapsed time " << elaspedTimeMs << " ms" << std::endl << std::flush;

  // Save image.
  SaveImagePNG("render.png", &rgb.at(0), width, height);

  rtcReleaseScene(scene);
  rtcReleaseDevice(device);

  return EXIT_SUCCESS;
}
