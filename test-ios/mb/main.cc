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

#include <embree3/rtcore.h>

//#include "../../kernels/bvh/bvh.h"
//#include "../../tutorials/common/tutorial/tutorial.h"
#include "../../tutorials/common/core/ray.h"

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
// Motion blur test scene based on Embree's subduvision_geometry tutorial
//

/* size of screen tiles */
// We observe a few % performance gain from 8 -> 16 on aarch64
#if defined(__aarch64__)
#define TILE_SIZE_X 16
#define TILE_SIZE_Y 16  
#else
#define TILE_SIZE_X 8
#define TILE_SIZE_Y 8
#endif

/* vertex and triangle layout */
struct Vertex   { float x,y,z,r;  }; // FIXME: rename to Vertex4f
struct Triangle { int v0, v1, v2; };


using namespace embree;

/* scene data */
RTCDevice g_device = nullptr;
RTCScene g_scene = nullptr;
Vec3fa face_colors[12];

/* accumulation buffer */
Vec3fa* g_accu = nullptr;
unsigned int g_accu_width = 0;
unsigned int g_accu_height = 0;
unsigned int g_accu_count = 0;
Vec3fa g_accu_vx;
Vec3fa g_accu_vy;
Vec3fa g_accu_vz;
Vec3fa g_accu_p;

float g_time = -1.0f;
unsigned int g_num_time_steps = 8;
unsigned int g_num_time_steps2 = 30;


__aligned(16) float cube_vertices[8][4] =
{
  { -1.0f, -1.0f, -1.0f, 0.0f },
  {  1.0f, -1.0f, -1.0f, 0.0f },
  {  1.0f, -1.0f,  1.0f, 0.0f },
  { -1.0f, -1.0f,  1.0f, 0.0f },
  { -1.0f,  1.0f, -1.0f, 0.0f },
  {  1.0f,  1.0f, -1.0f, 0.0f },
  {  1.0f,  1.0f,  1.0f, 0.0f },
  { -1.0f,  1.0f,  1.0f, 0.0f }
};

unsigned int cube_triangle_indices[36] = {
  1, 4, 5,  0, 4, 1,
  2, 5, 6,  1, 5, 2,
  3, 6, 7,  2, 6, 3,
  4, 3, 7,  0, 3, 4,
  5, 7, 6,  4, 7, 5,
  3, 1, 2,  0, 1, 3
};

unsigned int cube_quad_indices[24] = {
  0, 4, 5, 1,
  1, 5, 6, 2,
  2, 6, 7, 3,
  0, 3, 7, 4,
  4, 7, 6, 5,
  0, 1, 2, 3,
};

__aligned(16) float cube_vertex_crease_weights[8] = {
  inf, inf,inf, inf, inf, inf, inf, inf
};

__aligned(16) unsigned int cube_vertex_crease_indices[8] = {
  0,1,2,3,4,5,6,7
};

__aligned(16) float cube_edge_crease_weights[12] = {
  inf, inf, inf, inf, inf, inf, inf, inf, inf, inf, inf, inf
};

__aligned(16) unsigned int cube_edge_crease_indices[24] =
{
  0,1, 1,2, 2,3, 3,0,
  4,5, 5,6, 6,7, 7,4,
  0,4, 1,5, 2,6, 3,7,
};

#define NUM_INDICES 24
#define NUM_FACES 6
#define FACE_SIZE 4

unsigned int cube_quad_faces[6] = {
  4, 4, 4, 4, 4, 4
};

/* adds a cube to the scene */
unsigned int addTriangleCube (RTCScene scene, const Vec3fa& pos, unsigned int num_time_steps)
{
  /* create a triangulated cube with 12 triangles and 8 vertices */
  RTCGeometry geom = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_TRIANGLE);
  rtcSetGeometryTimeStepCount(geom,num_time_steps);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, cube_triangle_indices, 0, 3*sizeof(unsigned int), 12);

  for (unsigned int t=0; t<num_time_steps; t++)
  {
    RTCBufferType bufType = RTC_BUFFER_TYPE_VERTEX;
    Vec3fa* vertices = (Vec3fa*) rtcSetNewGeometryBuffer(geom,bufType,t,RTC_FORMAT_FLOAT3,sizeof(Vec3fa), 8);

    AffineSpace3fa rotation = AffineSpace3fa::rotate(Vec3fa(0,0,0),Vec3fa(0,1,0),2.0f*float(pi)*(float)t/(float)(num_time_steps-1));
    AffineSpace3fa scale = AffineSpace3fa::scale(Vec3fa(2.0f,1.0f,1.0f));

    for (int i=0; i<8; i++) {
      Vec3fa v = Vec3fa(cube_vertices[i][0],cube_vertices[i][1],cube_vertices[i][2]);
      vertices[i] = Vec3fa(xfmPoint(rotation*scale,v)+pos);
    }
  }

  /* create face color array */
  face_colors[0] = Vec3fa(1,0,0);
  face_colors[1] = Vec3fa(1,0,0);
  face_colors[2] = Vec3fa(0,1,0);
  face_colors[3] = Vec3fa(0,1,0);
  face_colors[4] = Vec3fa(0.5f);
  face_colors[5] = Vec3fa(0.5f);
  face_colors[6] = Vec3fa(1.0f);
  face_colors[7] = Vec3fa(1.0f);
  face_colors[8] = Vec3fa(0,0,1);
  face_colors[9] = Vec3fa(0,0,1);
  face_colors[10] = Vec3fa(1,1,0);
  face_colors[11] = Vec3fa(1,1,0);

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene,geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

/* adds a cube to the scene */
unsigned int addQuadCube (RTCScene scene, const Vec3fa& pos, unsigned int num_time_steps)
{
  /* create a quad cube with 6 quads and 8 vertices */
  RTCGeometry geom = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_QUAD);
  rtcSetGeometryTimeStepCount(geom,num_time_steps);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT4, cube_quad_indices, 0, 4*sizeof(unsigned int), 6);

  for (unsigned int t=0; t<num_time_steps; t++)
  {
    RTCBufferType bufType = RTC_BUFFER_TYPE_VERTEX;
    Vec3fa* vertices = (Vec3fa*) rtcSetNewGeometryBuffer(geom,bufType,t,RTC_FORMAT_FLOAT3,sizeof(Vec3fa), 8);

    AffineSpace3fa rotation = AffineSpace3fa::rotate(Vec3fa(0,0,0),Vec3fa(0,1,0),2.0f*float(pi)*(float)t/(float)(num_time_steps-1));
    AffineSpace3fa scale = AffineSpace3fa::scale(Vec3fa(2.0f,1.0f,1.0f));

    for (int i=0; i<8; i++) {
      Vec3fa v = Vec3fa(cube_vertices[i][0],cube_vertices[i][1],cube_vertices[i][2]);
      vertices[i] = Vec3fa(xfmPoint(rotation*scale,v)+pos);
    }
  }

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene,geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

/* adds a subdivision cube to the scene */
unsigned int addSubdivCube (RTCScene scene, const Vec3fa& pos, unsigned int num_time_steps)
{
  /* create a triangulated cube with 6 quads and 8 vertices */
  RTCGeometry geom = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_SUBDIVISION);
  rtcSetGeometryTimeStepCount(geom,num_time_steps);

  //rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, cube_vertices, 0, sizeof(Vec3fa), 8);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT, cube_quad_indices, 0, sizeof(unsigned int), NUM_INDICES);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_FACE,  0, RTC_FORMAT_UINT, cube_quad_faces,   0, sizeof(unsigned int), NUM_FACES);

  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_EDGE_CREASE_INDEX,  0, RTC_FORMAT_UINT2, cube_edge_crease_indices,  0, 2*sizeof(unsigned int), 0);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_EDGE_CREASE_WEIGHT, 0, RTC_FORMAT_FLOAT, cube_edge_crease_weights,  0, sizeof(float),          0);

  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_CREASE_INDEX,  0, RTC_FORMAT_UINT,  cube_vertex_crease_indices,0, sizeof(unsigned int), 0);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_CREASE_WEIGHT, 0, RTC_FORMAT_FLOAT, cube_vertex_crease_weights,0, sizeof(float),        0);

  float* level = (float*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_LEVEL, 0, RTC_FORMAT_FLOAT, sizeof(float), NUM_INDICES);
  for (unsigned int i=0; i<NUM_INDICES; i++) level[i] = 16.0f;

  for (unsigned int t=0; t<num_time_steps; t++)
  {
    RTCBufferType bufType = RTC_BUFFER_TYPE_VERTEX;
    Vec3fa* vertices = (Vec3fa*) rtcSetNewGeometryBuffer(geom,bufType,t,RTC_FORMAT_FLOAT3,sizeof(Vec3fa),8);

    AffineSpace3fa rotation = AffineSpace3fa::rotate(Vec3fa(0,0,0),Vec3fa(0,1,0),2.0f*float(pi)*(float)t/(float)(num_time_steps-1));
    AffineSpace3fa scale = AffineSpace3fa::scale(Vec3fa(2.0f,1.0f,1.0f));

    for (int i=0; i<8; i++) {
      Vec3fa v = Vec3fa(cube_vertices[i][0],cube_vertices[i][1],cube_vertices[i][2]);
      vertices[i] = Vec3fa(xfmPoint(rotation*scale,v)+pos);
    }
  }

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene,geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

/* add hair geometry */
unsigned int addCurve (RTCScene scene, const Vec3fa& pos, RTCGeometryType type, unsigned int num_time_steps)
{
  RTCGeometry geom = rtcNewGeometry(g_device, type);
  rtcSetGeometryTimeStepCount(geom,num_time_steps);
  rtcSetGeometryTessellationRate (geom,16.0f);

  Vec3fa* bspline = (Vec3fa*) malloc(16*sizeof(Vec3fa));
  for (int i=0; i<16; i++) {
    float f = (float)(i)/16.0f;
    bspline[i] = Vec3fa(2.0f*f-1.0f,sin(12.0f*f),cos(12.0f*f));
  }

  for (unsigned int t=0; t<num_time_steps; t++)
  {
    RTCBufferType bufType = RTC_BUFFER_TYPE_VERTEX;
    Vec3fa* vertices = (Vec3fa*) rtcSetNewGeometryBuffer(geom,bufType,t,RTC_FORMAT_FLOAT4,sizeof(Vec3fa),16);

    AffineSpace3fa rotation = AffineSpace3fa::rotate(Vec3fa(0,0,0),Vec3fa(0,1,0),2.0f*float(pi)*(float)t/(float)(num_time_steps-1));
    AffineSpace3fa scale = AffineSpace3fa::scale(Vec3fa(2.0f,1.0f,1.0f));

    for (int i=0; i<16; i++)
      vertices[i] = Vec3fa(xfmPoint(rotation*scale,bspline[i])+pos,0.2f);
  }

  int* indices = (int*) rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT,sizeof(int),13);
  for (int i=0; i<13; i++) indices[i] = i;

  free(bspline);

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene,geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

/* add line geometry */
unsigned int addLines (RTCScene scene, const Vec3fa& pos, unsigned int num_time_steps)
{
  RTCGeometry geom = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_FLAT_LINEAR_CURVE);
  rtcSetGeometryTimeStepCount(geom,num_time_steps);

  Vec3fa* bspline = (Vec3fa*) malloc(16*sizeof(Vec3fa));
  for (int i=0; i<16; i++) {
    float f = (float)(i)/16.0f;
    bspline[i] = Vec3fa(2.0f*f-1.0f,sin(12.0f*f),cos(12.0f*f));
  }

  for (unsigned int t=0; t<num_time_steps; t++)
  {
    RTCBufferType bufType = RTC_BUFFER_TYPE_VERTEX;
    Vec3fa* vertices = (Vec3fa*) rtcSetNewGeometryBuffer(geom,bufType,t,RTC_FORMAT_FLOAT4,sizeof(Vec3fa),16);

    AffineSpace3fa rotation = AffineSpace3fa::rotate(Vec3fa(0,0,0),Vec3fa(0,1,0),2.0f*float(pi)*(float)t/(float)(num_time_steps-1));
    AffineSpace3fa scale = AffineSpace3fa::scale(Vec3fa(2.0f,1.0f,1.0f));

    for (int i=0; i<16; i++)
      vertices[i] = Vec3fa(xfmPoint(rotation*scale,bspline[i])+pos,0.2f);
  }

  int* indices = (int*) rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT,sizeof(int),15);
  for (int i=0; i<15; i++) indices[i] = i;

  free(bspline);

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene,geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

/* adds an instanced triangle cube to the scene, rotate instance */
RTCScene addInstancedTriangleCube (RTCScene global_scene, const Vec3fa& pos, unsigned int num_time_steps)
{
  RTCScene scene = rtcNewScene(g_device);
  RTCGeometry geom = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_TRIANGLE);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX,  0, RTC_FORMAT_UINT3,  cube_triangle_indices, 0, 3*sizeof(unsigned int), 12);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, cube_vertices, 0, 4*sizeof(float), 8);
  rtcCommitGeometry(geom);
  rtcAttachGeometry(scene,geom);
  rtcReleaseGeometry(geom);
  rtcCommitScene(scene);

  RTCGeometry inst = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_INSTANCE);
   rtcSetGeometryInstancedScene(inst,scene);
   rtcSetGeometryTimeStepCount(inst,num_time_steps);
  
  for (unsigned int t=0; t<num_time_steps; t++)
  {
    AffineSpace3fa rotation = AffineSpace3fa::rotate(Vec3fa(0,0,0),Vec3fa(0,1,0),2.0f*float(pi)*(float)t/(float)(num_time_steps-1));
    AffineSpace3fa scale = AffineSpace3fa::scale(Vec3fa(2.0f,1.0f,1.0f));
    AffineSpace3fa translation = AffineSpace3fa::translate(pos);
    AffineSpace3fa xfm = translation*rotation*scale;
    rtcSetGeometryTransform(inst,t,RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR,(float*)&xfm);
  }

  rtcCommitGeometry(inst);
  rtcAttachGeometry(global_scene,inst);
  rtcReleaseGeometry(inst);
  return scene;
}

/* adds an instanced quad cube to the scene, rotate instance and geometry */
RTCScene addInstancedQuadCube (RTCScene global_scene, const Vec3fa& pos, unsigned int num_time_steps)
{
  RTCScene scene = rtcNewScene(g_device);
  RTCGeometry geom = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_QUAD);
  rtcSetGeometryTimeStepCount(geom,num_time_steps);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT4, cube_quad_indices, 0, 4*sizeof(unsigned int), 6);

  for (unsigned int t=0; t<num_time_steps; t++)
  {
    RTCBufferType bufType = RTC_BUFFER_TYPE_VERTEX;
    Vec3fa* vertices = (Vec3fa*) rtcSetNewGeometryBuffer(geom,bufType,t,RTC_FORMAT_FLOAT3,sizeof(Vec3fa),8);

    AffineSpace3fa rotation = AffineSpace3fa::rotate(Vec3fa(0,0,0),Vec3fa(0,1,0),0.5f*2.0f*float(pi)*(float)t/(float)(num_time_steps-1));
    AffineSpace3fa scale = AffineSpace3fa::scale(Vec3fa(2.0f,1.0f,1.0f));

    for (int i=0; i<8; i++) {
      Vec3fa v = Vec3fa(cube_vertices[i][0],cube_vertices[i][1],cube_vertices[i][2]);
      vertices[i] = Vec3fa(xfmPoint(rotation*scale,v));
    }
  }

  rtcCommitGeometry(geom);
  rtcAttachGeometry(scene,geom);
  rtcReleaseGeometry(geom);  
  rtcCommitScene(scene);

  RTCGeometry inst = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_INSTANCE);
   rtcSetGeometryInstancedScene(inst,scene);
   rtcSetGeometryTimeStepCount(inst,num_time_steps);

  for (unsigned int t=0; t<num_time_steps; t++)
  {
    AffineSpace3fa rotation = AffineSpace3fa::rotate(Vec3fa(0,0,0),Vec3fa(0,1,0),0.5f*2.0f*float(pi)*(float)t/(float)(num_time_steps-1));
    AffineSpace3fa translation = AffineSpace3fa::translate(pos);
    AffineSpace3fa xfm = translation*rotation;
    rtcSetGeometryTransform(inst,t,RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR,(float*)&xfm);
  }

  rtcCommitGeometry(inst);
  rtcAttachGeometry(global_scene,inst);
  rtcReleaseGeometry(inst);
  return scene;
}

// ======================================================================== //
//                     User defined sphere geometry                         //
// ======================================================================== //

struct Sphere
{
  ALIGNED_STRUCT_(16)
  Vec3fa p;                      //!< position of the sphere
  float r;                      //!< radius of the sphere
  unsigned int geomID;
  unsigned int num_time_steps;
};

void sphereBoundsFunc(const struct RTCBoundsFunctionArguments* args)
{
  const Sphere* spheres = (const Sphere*) args->geometryUserPtr;
  RTCBounds* bounds_o = args->bounds_o;
  const unsigned int time = args->timeStep;
  const Sphere& sphere = spheres[args->primID];
  float ft = 2.0f*float(pi) * (float) time / (float) (sphere.num_time_steps-1);
  Vec3fa p = sphere.p + Vec3fa(cos(ft),0.0f,sin(ft));
  bounds_o->lower_x = p.x-sphere.r;
  bounds_o->lower_y = p.y-sphere.r;
  bounds_o->lower_z = p.z-sphere.r;
  bounds_o->upper_x = p.x+sphere.r;
  bounds_o->upper_y = p.y+sphere.r;
  bounds_o->upper_z = p.z+sphere.r;
}

void sphereIntersectFuncN(const RTCIntersectFunctionNArguments* args)
{
  const int* valid = args->valid;
  void* ptr  = args->geometryUserPtr;
  RTCRayHitN* rays = (RTCRayHitN*)args->rayhit;
  unsigned int primID = args->primID;
  assert(args->N == 1);
  const Sphere* spheres = (const Sphere*)ptr;
  const Sphere& sphere = spheres[primID];

  if (!valid[0])
    return;
  
  Ray *ray = (Ray *)rays;
  
  const int time_segments = sphere.num_time_steps-1;
  const float time = ray->time()*(float)(time_segments);
  const int itime = clamp((int)(floorf(time)),(int)0,time_segments-1);
  const float ftime = time - (float)(itime);
  const float ft0 = 2.0f*float(pi) * (float) (itime+0) / (float) (sphere.num_time_steps-1);
  const float ft1 = 2.0f*float(pi) * (float) (itime+1) / (float) (sphere.num_time_steps-1);
  const Vec3fa p0 = sphere.p + Vec3fa(cos(ft0),0.0f,sin(ft0));
  const Vec3fa p1 = sphere.p + Vec3fa(cos(ft1),0.0f,sin(ft1));
  const Vec3fa sphere_p = (1.0f-ftime)*p0 + ftime*p1;
  
  const Vec3fa v = ray->org-sphere_p;
  const float A = dot(ray->dir,ray->dir);
  const float B = 2.0f*dot(v,ray->dir);
  const float C = dot(v,v) - sqr(sphere.r);
  const float D = B*B - 4.0f*A*C;
  if (D < 0.0f) return;
  const float Q = sqrtf(D);
  const float rcpA = rcp(A);
  const float t0 = 0.5f*rcpA*(-B-Q);
  const float t1 = 0.5f*rcpA*(-B+Q);
  if ((ray->tnear() < t0) & (t0 < ray->tfar)) {
    ray->u = 0.0f;
    ray->v = 0.0f;
    ray->tfar = t0;
    ray->geomID = sphere.geomID;
    ray->primID = (unsigned int) primID;
    ray->Ng = ray->org+t0*ray->dir-sphere_p;
  }
  if ((ray->tnear() < t1) & (t1 < ray->tfar)) {
    ray->u = 0.0f;
    ray->v = 0.0f;
    ray->tfar = t1;
    ray->geomID = sphere.geomID;
    ray->primID = (unsigned int) primID;
    ray->Ng = ray->org+t1*ray->dir-sphere_p;
  }
}

void sphereOccludedFuncN(const RTCOccludedFunctionNArguments* args)
{
  const int* valid = args->valid;
  void* ptr  = args->geometryUserPtr;
  RTCRayHitN* rays = (RTCRayHitN*)args->ray;
  unsigned int primID = args->primID;
  assert(args->N == 1);
  const Sphere* spheres = (const Sphere*)ptr;
  const Sphere& sphere = spheres[primID];

  if (!valid[0])
    return;
  
  Ray *ray = (Ray *)rays;
  const int time_segments = sphere.num_time_steps-1;
  const float time = ray->time()*(float)(time_segments);
  const int itime = clamp((int)(floorf(time)),(int)0,time_segments-1);
  const float ftime = time - (float)(itime);
  const float ft0 = 2.0f*float(pi) * (float) (itime+0) / (float) (sphere.num_time_steps-1);
  const float ft1 = 2.0f*float(pi) * (float) (itime+1) / (float) (sphere.num_time_steps-1);
  const Vec3fa p0 = sphere.p + Vec3fa(cos(ft0),0.0f,sin(ft0));
  const Vec3fa p1 = sphere.p + Vec3fa(cos(ft1),0.0f,sin(ft1));
  const Vec3fa sphere_p = (1.0f-ftime)*p0 + ftime*p1;
  
  const Vec3fa v = ray->org-sphere_p;
  const float A = dot(ray->dir,ray->dir);
  const float B = 2.0f*dot(v,ray->dir);
  const float C = dot(v,v) - sqr(sphere.r);
  const float D = B*B - 4.0f*A*C;
  if (D < 0.0f) return;
  const float Q = sqrtf(D);
  const float rcpA = rcp(A);
  const float t0 = 0.5f*rcpA*(-B-Q);
  const float t1 = 0.5f*rcpA*(-B+Q);
  if ((ray->tnear() < t0) & (t0 < ray->tfar)) {
    ray->tfar = neg_inf;
  }
  if ((ray->tnear() < t1) & (t1 < ray->tfar)) {
    ray->tfar = neg_inf;
  }
}

Sphere* addUserGeometrySphere (RTCScene scene, const Vec3fa& p, float r, unsigned int num_time_steps)
{
  RTCGeometry geom = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_USER);
  Sphere* sphere = (Sphere*) malloc(sizeof(Sphere));
  sphere->p = p;
  sphere->r = r;
  sphere->geomID = rtcAttachGeometry(scene,geom);
  sphere->num_time_steps = num_time_steps;
  rtcSetGeometryUserPrimitiveCount(geom,1);
  rtcSetGeometryTimeStepCount(geom,num_time_steps);
  rtcSetGeometryUserData(geom,sphere);
  rtcSetGeometryBoundsFunction(geom,sphereBoundsFunc,nullptr);
  rtcSetGeometryIntersectFunction(geom,sphereIntersectFuncN);
  rtcSetGeometryOccludedFunction (geom,sphereOccludedFuncN);
  rtcCommitGeometry(geom);
  rtcReleaseGeometry(geom);
  return sphere;
}

/* adds a ground plane to the scene */
unsigned int addGroundPlane (RTCScene scene)
{
  /* create a triangulated plane with 2 triangles and 4 vertices */
  RTCGeometry geom = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_TRIANGLE);

  /* set vertices */
  Vertex* vertices = (Vertex*) rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(Vertex),4);
  vertices[0].x = -10; vertices[0].y = -2; vertices[0].z = -10;
  vertices[1].x = -10; vertices[1].y = -2; vertices[1].z = +10;
  vertices[2].x = +10; vertices[2].y = -2; vertices[2].z = -10;
  vertices[3].x = +10; vertices[3].y = -2; vertices[3].z = +10;

  /* set triangles */
  Triangle* triangles = (Triangle*) rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,sizeof(Triangle),2);
  triangles[0].v0 = 0; triangles[0].v1 = 1; triangles[0].v2 = 2;
  triangles[1].v0 = 1; triangles[1].v1 = 3; triangles[1].v2 = 2;

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene,geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

RTCScene scene0 = nullptr;
RTCScene scene1 = nullptr;
RTCScene scene2 = nullptr;
RTCScene scene3 = nullptr;
Sphere* sphere0 = nullptr;
Sphere* sphere1 = nullptr;

static void device_init (char* cfg)
{
  /* initialize last seen camera */
  g_accu_vx = Vec3fa(0.0f);
  g_accu_vy = Vec3fa(0.0f);
  g_accu_vz = Vec3fa(0.0f);
  g_accu_p  = Vec3fa(0.0f);

  /* create scene */
  g_scene = rtcNewScene(g_device);

  /* add geometry to the scene */
  addTriangleCube(g_scene,Vec3fa(-5,1,-5),g_num_time_steps);
  addTriangleCube(g_scene,Vec3fa(-5,5,-5),g_num_time_steps2);

  addQuadCube    (g_scene,Vec3fa( 0,1,-5),g_num_time_steps);
  addQuadCube    (g_scene,Vec3fa( 0,5,-5),g_num_time_steps2);

  addSubdivCube  (g_scene,Vec3fa(+5,1,-5),g_num_time_steps);
  addSubdivCube  (g_scene,Vec3fa(+5,5,-5),g_num_time_steps2);

  addLines       (g_scene,Vec3fa(-5,1, 0),g_num_time_steps);
  addLines       (g_scene,Vec3fa(-5,5, 0),g_num_time_steps2);

  addCurve (g_scene,Vec3fa( 0,1, 0),RTC_GEOMETRY_TYPE_FLAT_BSPLINE_CURVE,g_num_time_steps);
  addCurve (g_scene,Vec3fa( 0,5, 0),RTC_GEOMETRY_TYPE_FLAT_BSPLINE_CURVE,g_num_time_steps2);

  addCurve (g_scene,Vec3fa(+5,1, 0),RTC_GEOMETRY_TYPE_ROUND_BSPLINE_CURVE,g_num_time_steps);
  addCurve (g_scene,Vec3fa(+5,5, 0),RTC_GEOMETRY_TYPE_ROUND_BSPLINE_CURVE,g_num_time_steps2);

  scene0 = addInstancedTriangleCube(g_scene,Vec3fa(-5,1,+5),g_num_time_steps);
  scene1 = addInstancedTriangleCube(g_scene,Vec3fa(-5,5,+5),g_num_time_steps2);

  scene2 = addInstancedQuadCube    (g_scene,Vec3fa( 0,1,+5),g_num_time_steps);
  scene3 = addInstancedQuadCube    (g_scene,Vec3fa( 0,5,+5),g_num_time_steps2);

  sphere0 = addUserGeometrySphere   (g_scene,Vec3fa(+5,1,+5),1.0f,g_num_time_steps);
  sphere1 = addUserGeometrySphere   (g_scene,Vec3fa(+5,5,+5),1.0f,g_num_time_steps2);

  addGroundPlane(g_scene);

  /* commit changes to scene */
  rtcCommitScene (g_scene);

  /* set start render mode */
  //renderTile = renderTileStandard;
  //key_pressed_handler = device_key_pressed_default;
}

int frameID = 50;

/* task that renders a single screen tile */
Vec3fa renderPixelStandard(float x, float y, const Vec3fa &camorg, const Vec3fa &camdir)
{
  RTCIntersectContext context;
  rtcInitIntersectContext(&context);
  
  float time = fabs((int)(0.01f*frameID) - 0.01f*frameID);
  if (g_time != -1) time = g_time;

  /* initialize ray */
  Ray ray(camorg, camdir, 0.0f, inf, time);

  /* intersect ray with scene */
  rtcIntersect1(g_scene,&context,RTCRayHit_(ray));
  //RayStats_addRay(stats);

  /* shade pixels */
  Vec3fa color = Vec3fa(0.0f);
  if (ray.geomID != RTC_INVALID_GEOMETRY_ID)
  {
    Vec3fa diffuse = Vec3fa(0.5f,0.5f,0.5f);
    if (ray.instID == RTC_INVALID_GEOMETRY_ID)
      ray.instID = ray.geomID;
    switch (ray.instID / 2) {
    case 0: diffuse = face_colors[ray.primID]; break;
    case 1: diffuse = face_colors[2*ray.primID]; break;
    case 2: diffuse = face_colors[2*ray.primID]; break;

    case 3: diffuse = Vec3fa(0.5f,0.0f,0.0f); break;
    case 4: diffuse = Vec3fa(0.0f,0.5f,0.0f); break;
    case 5: diffuse = Vec3fa(0.0f,0.0f,0.5f); break;

    case 6: diffuse = face_colors[ray.primID]; break;
    case 7: diffuse = face_colors[2*ray.primID]; break;
    case 8: diffuse = Vec3fa(0.5f,0.5f,0.0f); break;
    default: diffuse = Vec3fa(0.5f,0.5f,0.5f); break;
    }
    color = color + diffuse*0.5f;
    Vec3fa lightDir = normalize(Vec3fa(-1,-4,-1));

    /* initialize shadow ray */
    Ray shadow(ray.org + ray.tfar*ray.dir, -lightDir, 0.001f, inf, time);

    /* trace shadow ray */
    rtcOccluded1(g_scene,&context,RTCRay_(shadow));
    //RayStats_addShadowRay(stats);

    /* add light contribution */
    if (shadow.tfar >= 0.0f)
      color = color + diffuse*clamp(-dot(lightDir,normalize(ray.Ng)),0.0f,1.0f);
  }
  return color;
}


#if 0
  for (unsigned int y=y0; y<y1; y++) for (unsigned int x=x0; x<x1; x++)
  {
    /* calculate pixel color */
    Vec3fa color = renderPixelStandard((float)x,(float)y,camera,g_stats[threadIndex]);

    /* write color to framebuffer */
    Vec3fa accu_color = g_accu[y*width+x] + Vec3fa(color.x,color.y,color.z,1.0f); g_accu[y*width+x] = accu_color;
    float f = rcp(max(0.001f,accu_color.w));
    unsigned int r = (unsigned int) (255.0f * clamp(accu_color.x*f,0.0f,1.0f));
    unsigned int g = (unsigned int) (255.0f * clamp(accu_color.y*f,0.0f,1.0f));
    unsigned int b = (unsigned int) (255.0f * clamp(accu_color.z*f,0.0f,1.0f));
    pixels[y*width+x] = (b << 16) + (g << 8) + r;
  }
}
#endif


static void device_cleanup ()
{
  free(sphere0); sphere0 = nullptr;
  free(sphere1); sphere1 = nullptr;
  rtcReleaseScene(scene0); scene0 = nullptr;
  rtcReleaseScene(scene1); scene1 = nullptr;
  rtcReleaseScene(scene2); scene2 = nullptr;
  rtcReleaseScene(scene3); scene3 = nullptr;
  rtcReleaseScene (g_scene); g_scene = nullptr;
  free(g_accu); g_accu = nullptr;
  g_accu_width = 0;
  g_accu_height = 0;
  g_accu_count = 0;
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

static void error_handler(void* userPtr, const RTCError code, const char* str = nullptr)
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


int main(int argc, char **argv) {
  (void)argc;
  (void)argv;


  g_device = rtcNewDevice(nullptr);
  error_handler(nullptr,rtcGetDeviceError(g_device));

  /* set error handler */
  rtcSetDeviceErrorFunction(g_device,error_handler,nullptr);

  device_init(nullptr);

#if 0
  RTCScene scene = rtcNewScene(device);

  rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);
  rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);

  addCube(device, scene);

  rtcCommitScene(scene);

  DumpScene(scene);
#endif

  int width = 512;
  int height = 512;

  std::vector<float> rgb(width * height * 3, 0.0f);

  g_accu = (Vec3fa*) malloc(width*height*sizeof(Vec3fa));
  g_accu_width = width;
  g_accu_height = height;
  for (unsigned int i=0; i<width*height; i++) {
    g_accu[i] = Vec3fa(0.0f);
  }


  int max_frames = 10;

  auto t_start = std::chrono::high_resolution_clock::now();
  for (int frame = 0; frame < max_frames; frame++) {
    frameID++;
    for (int y = 0; y < height; y++) {
      std::cout << "frame = " << frame << ", y = " << y << std::endl;
      for (int x = 0; x < width; x++) {

        float dir[3], ndir[3];
        dir[0] = (x / float(width)) - 0.5f;
        dir[1] = (y / float(height)) - 0.5f;
        dir[2] = -1.0f;
        vnormalize(ndir, dir);


        Vec3fa camorg(0.0f, 5.0f, 20.0f);
        Vec3fa camdir(ndir[0], ndir[1], ndir[2]);

        Vec3fa color = renderPixelStandard((float)x,(float)y, camorg, camdir);

        {
          // Flip Y
          rgb[3 * size_t((height - y - 1) * width + x) + 0] += color[0] / float(max_frames);
          rgb[3 * size_t((height - y - 1) * width + x) + 1] += color[1] / float(max_frames);
          rgb[3 * size_t((height - y - 1) * width + x) + 2] += color[2] / float(max_frames);
        }
      }
    }
  }
  auto t_end = std::chrono::high_resolution_clock::now();
  double elaspedTimeMs = std::chrono::duration<double, std::milli>(t_end-t_start).count();
  std::cout <<  std::fixed << std::setprecision(2)
            << "Elapsed time " << elaspedTimeMs << " ms" << std::endl << std::flush;

  // Save image.
  SaveImagePNG("render.png", &rgb.at(0), width, height);

  device_cleanup();

  return EXIT_SUCCESS;
}
