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

#include "../../tutorials/common/core/ray.h"
//#include "../../kernels/bvh/bvh.h"

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
// interp scene based on Embree's interpolation tutorial
//

using namespace embree;

/* vertex and triangle layout */
struct Vertex   { float x,y,z,r;  }; // FIXME: rename to Vertex4f
struct Triangle { int v0, v1, v2; };


//#define FORCE_FIXED_EDGE_TESSELLATION
#define FIXED_EDGE_TESSELLATION_VALUE 16

#define MAX_EDGE_LEVEL 64.0f
#define MIN_EDGE_LEVEL  4.0f
#define LEVEL_FACTOR  128.0f

/* scene data */
RTCDevice g_device = nullptr;
Vec3fa* vertex_colors = nullptr;
unsigned int triCubeID, quadCubeID;

#define NUM_VERTICES 8

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

__aligned(16) float cube_vertex_colors[8][4] =
{
  {  0.0f,  0.0f,  0.0f, 0.0f },
  {  1.0f,  0.0f,  0.0f, 0.0f },
  {  1.0f,  0.0f,  1.0f, 0.0f },
  {  0.0f,  0.0f,  1.0f, 0.0f },
  {  0.0f,  1.0f,  0.0f, 0.0f },
  {  1.0f,  1.0f,  0.0f, 0.0f },
  {  1.0f,  1.0f,  1.0f, 0.0f },
  {  0.0f,  1.0f,  1.0f, 0.0f }
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

#define NUM_QUAD_INDICES 24
#define NUM_QUAD_FACES 6

unsigned int cube_quad_indices[24] = {
  0, 4, 5, 1,
  1, 5, 6, 2,
  2, 6, 7, 3,
  0, 3, 7, 4,
  4, 7, 6, 5,
  0, 1, 2, 3,
};

unsigned int cube_quad_faces[6] = {
  4, 4, 4, 4, 4, 4
};

#define NUM_TRI_INDICES 36
#define NUM_TRI_FACES 12

unsigned int cube_tri_indices[36] = {
  1, 4, 5,  0, 4, 1,
  2, 5, 6,  1, 5, 2,
  3, 6, 7,  2, 6, 3,
  4, 3, 7,  0, 3, 4,
  5, 7, 6,  4, 7, 5,
  3, 1, 2,  0, 1, 3
};

unsigned int cube_tri_faces[12] = {
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

#define NUM_HAIR_VERTICES 4

__aligned(16) float hair_vertices[4][4] =
{
  { 0.0f, 0.0f, 0.0f, 0.1f },
  { 0.5f, 1.0f, 0.0f, 0.1f },
  { 0.0f, 2.0f, -0.5f, 0.1f },
  { 0.0f, 3.0f, 0.0f, 0.1f }
};

__aligned(16) float hair_vertex_colors[4][4] =
{
  {  1.0f,  0.0f,  0.0f, 0.0f },
  {  1.0f,  1.0f,  0.0f, 0.0f },
  {  0.0f,  0.0f,  1.0f, 0.0f },
  {  1.0f,  1.0f,  1.0f, 0.0f },
};

unsigned int hair_indices[1] = {
  0
};

inline float updateEdgeLevel(const Vec3fa& cam_pos, Vec3fa* vtx, unsigned int* indices, const unsigned int e0, const unsigned int e1)
{
  const Vec3fa v0 = vtx[indices[e0]];
  const Vec3fa v1 = vtx[indices[e1]];
  const Vec3fa edge = v1-v0;
  const Vec3fa P = 0.5f*(v1+v0);
  const Vec3fa dist = cam_pos - P;
  const float level = max(min(LEVEL_FACTOR*(0.5f*length(edge)/length(dist)),MAX_EDGE_LEVEL),MIN_EDGE_LEVEL);
  return level;
}

/* adds a subdiv cube to the scene */
unsigned int addTriangleSubdivCube (RTCScene scene_i, const Vec3fa& pos)
{
  RTCGeometry geom = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_SUBDIVISION);

  //rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, cube_vertices, 0, sizeof(Vec3fa  ), NUM_VERTICES);
  Vec3fa* vtx = (Vec3fa*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vec3fa), NUM_VERTICES);
  for (unsigned int i=0; i<NUM_VERTICES; i++) vtx[i] = Vec3fa(cube_vertices[i][0]+pos.x,cube_vertices[i][1]+pos.y,cube_vertices[i][2]+pos.z);

  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT, cube_tri_indices, 0, sizeof(unsigned int), NUM_TRI_INDICES);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_FACE,  0, RTC_FORMAT_UINT, cube_tri_faces,   0, sizeof(unsigned int), NUM_TRI_FACES);

  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_EDGE_CREASE_INDEX,  0, RTC_FORMAT_UINT2, cube_edge_crease_indices, 0, 2*sizeof(unsigned int), 0);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_EDGE_CREASE_WEIGHT, 0, RTC_FORMAT_FLOAT, cube_edge_crease_weights, 0, sizeof(float),          0);

  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_CREASE_INDEX,  0, RTC_FORMAT_UINT,  cube_vertex_crease_indices, 0, sizeof(unsigned int), 0);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_CREASE_WEIGHT, 0, RTC_FORMAT_FLOAT, cube_vertex_crease_weights, 0, sizeof(float),        0);

  rtcSetGeometryVertexAttributeCount(geom,1);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3, cube_vertex_colors, 0, sizeof(Vec3fa), NUM_VERTICES);

  float* level = (float*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_LEVEL, 0, RTC_FORMAT_FLOAT, sizeof(float), NUM_TRI_INDICES);
  for (unsigned int i=0; i<NUM_TRI_INDICES; i++) level[i] = FIXED_EDGE_TESSELLATION_VALUE;

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene_i, geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

void setTriangleSubdivCubeLevels (RTCGeometry geom, const Vec3fa& cam_pos)
{
  Vec3fa* vtx = (Vec3fa*) rtcGetGeometryBufferData(geom, RTC_BUFFER_TYPE_VERTEX, 0);
  if (vtx == nullptr) return;
  float* level = (float*) rtcGetGeometryBufferData(geom, RTC_BUFFER_TYPE_LEVEL, 0);
  if (level == nullptr) return;

  for (unsigned int i=0; i<NUM_TRI_INDICES; i+=3)
  {
    level[i+0] = updateEdgeLevel(cam_pos, vtx, cube_tri_indices, i+0, i+1);
    level[i+1] = updateEdgeLevel(cam_pos, vtx, cube_tri_indices, i+1, i+2);
    level[i+2] = updateEdgeLevel(cam_pos, vtx, cube_tri_indices, i+2, i+0);
  }

  rtcUpdateGeometryBuffer(geom, RTC_BUFFER_TYPE_LEVEL, 0);
  rtcCommitGeometry(geom);
}

/* adds a subdiv cube to the scene */
unsigned int addQuadSubdivCube (RTCScene scene_i, const Vec3fa& pos)
{
  RTCGeometry geom = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_SUBDIVISION);

  //rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, cube_vertices, 0, sizeof(Vec3fa  ), NUM_VERTICES);
  Vec3fa* vtx = (Vec3fa*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vec3fa), NUM_VERTICES);
  for (unsigned int i=0; i<NUM_VERTICES; i++) vtx[i] = Vec3fa(cube_vertices[i][0]+pos.x,cube_vertices[i][1]+pos.y,cube_vertices[i][2]+pos.z);

  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT, cube_quad_indices, 0, sizeof(unsigned int), NUM_QUAD_INDICES);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_FACE,  0, RTC_FORMAT_UINT, cube_quad_faces,   0, sizeof(unsigned int), NUM_QUAD_FACES);

  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_EDGE_CREASE_INDEX,  0, RTC_FORMAT_UINT2, cube_edge_crease_indices, 0, 2*sizeof(unsigned int), 0);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_EDGE_CREASE_WEIGHT, 0, RTC_FORMAT_FLOAT, cube_edge_crease_weights, 0, sizeof(float),          0);

  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_CREASE_INDEX,  0, RTC_FORMAT_UINT,  cube_vertex_crease_indices, 0, sizeof(unsigned int), 0);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_CREASE_WEIGHT, 0, RTC_FORMAT_FLOAT, cube_vertex_crease_weights, 0, sizeof(float),        0);

  rtcSetGeometryVertexAttributeCount(geom,1);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3, cube_vertex_colors, 0, sizeof(Vec3fa), NUM_VERTICES);

  float* level = (float*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_LEVEL, 0, RTC_FORMAT_FLOAT, sizeof(float), NUM_QUAD_INDICES);
  for (unsigned int i=0; i<NUM_QUAD_INDICES; i++) level[i] = FIXED_EDGE_TESSELLATION_VALUE;

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene_i, geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

void setQuadSubdivCubeLevels (RTCGeometry geom, const Vec3fa& cam_pos)
{
  Vec3fa* vtx = (Vec3fa*) rtcGetGeometryBufferData(geom, RTC_BUFFER_TYPE_VERTEX, 0);
  if (vtx == nullptr) return;
  float* level = (float*) rtcGetGeometryBufferData(geom, RTC_BUFFER_TYPE_LEVEL, 0);
  if (level == nullptr) return;

  for (unsigned int i=0; i<NUM_QUAD_INDICES; i+=4)
  {
    level[i+0] = updateEdgeLevel(cam_pos, vtx, cube_quad_indices, i+0, i+1);
    level[i+1] = updateEdgeLevel(cam_pos, vtx, cube_quad_indices, i+1, i+2);
    level[i+2] = updateEdgeLevel(cam_pos, vtx, cube_quad_indices, i+2, i+3);
    level[i+3] = updateEdgeLevel(cam_pos, vtx, cube_quad_indices, i+3, i+0);
  }

  rtcUpdateGeometryBuffer(geom, RTC_BUFFER_TYPE_LEVEL, 0);
  rtcCommitGeometry(geom);
}

/* adds a triangle cube to the scene */
unsigned int addTriangleCube (RTCScene scene_i, const Vec3fa& pos)
{
  RTCGeometry geom = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_TRIANGLE);

  //rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, cube_vertices, 0, sizeof(Vec3fa  ), NUM_VERTICES);
  Vec3fa* vtx = (Vec3fa*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vec3fa), NUM_VERTICES);
  for (unsigned int i=0; i<NUM_VERTICES; i++) vtx[i] = Vec3fa(cube_vertices[i][0]+pos.x,cube_vertices[i][1]+pos.y,cube_vertices[i][2]+pos.z);

  rtcSetGeometryVertexAttributeCount(geom,1);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX,            0, RTC_FORMAT_UINT3,  cube_tri_indices,   0, 3*sizeof(unsigned int), NUM_TRI_INDICES/3);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3, cube_vertex_colors, 0, sizeof(Vec3fa),         NUM_VERTICES);

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene_i, geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

/* adds a quad cube to the scene */
unsigned int addQuadCube (RTCScene scene_i, const Vec3fa& pos)
{
  RTCGeometry geom = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_QUAD);

  //rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, cube_vertices, 0, sizeof(Vec3fa  ), NUM_VERTICES);
  Vec3fa* vtx = (Vec3fa*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vec3fa), NUM_VERTICES);
  for (unsigned int i=0; i<NUM_VERTICES; i++) vtx[i] = Vec3fa(cube_vertices[i][0]+pos.x,cube_vertices[i][1]+pos.y,cube_vertices[i][2]+pos.z);

  rtcSetGeometryVertexAttributeCount(geom,1);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX,            0, RTC_FORMAT_UINT4,  cube_quad_indices,  0, 4*sizeof(unsigned int), NUM_QUAD_INDICES/4);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3, cube_vertex_colors, 0, sizeof(Vec3fa),         NUM_VERTICES);

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene_i, geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

/* add curve geometry */
unsigned int addCurve (RTCScene scene, const Vec3fa& pos)
{
  RTCGeometry geom = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_ROUND_BEZIER_CURVE);

  //rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, hair_vertices, 0, sizeof(Vec3fa), NUM_HAIR_VERTICES);
  Vec3ff* vtx = (Vec3ff*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT4, sizeof(Vec3ff), NUM_HAIR_VERTICES);
  for (unsigned int i=0; i<NUM_HAIR_VERTICES; i++) {
    vtx[i].x = hair_vertices[i][0]+pos.x;
    vtx[i].y = hair_vertices[i][1]+pos.y;
    vtx[i].z = hair_vertices[i][2]+pos.z;
    vtx[i].w = hair_vertices[i][3];
  }

  rtcSetGeometryVertexAttributeCount(geom,1);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX,            0, RTC_FORMAT_UINT,   hair_indices,       0, sizeof(unsigned int), 1);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3, hair_vertex_colors, 0, sizeof(Vec3fa),       NUM_HAIR_VERTICES);

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene, geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

/* adds a ground plane to the scene */
unsigned int addGroundPlane (RTCScene scene_i)
{
  /* create a triangulated plane with 2 triangles and 4 vertices */
  RTCGeometry geom = rtcNewGeometry (g_device, RTC_GEOMETRY_TYPE_TRIANGLE);

  /* set vertices */
  Vertex* vertices = (Vertex*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex), 4);
  vertices[0].x = -10; vertices[0].y = -2; vertices[0].z = -10;
  vertices[1].x = -10; vertices[1].y = -2; vertices[1].z = +10;
  vertices[2].x = +10; vertices[2].y = -2; vertices[2].z = -10;
  vertices[3].x = +10; vertices[3].y = -2; vertices[3].z = +10;

  /* set triangles */
  Triangle* triangles = (Triangle*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), 2);
  triangles[0].v0 = 0; triangles[0].v1 = 1; triangles[0].v2 = 2;
  triangles[1].v0 = 1; triangles[1].v1 = 3; triangles[1].v2 = 2;

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene_i, geom);
  rtcReleaseGeometry(geom);
  return geomID;
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

static void bora() {
  std::cout << "bora" << std::endl;
}
#endif

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  g_device = rtcNewDevice(nullptr);
  error_handler(nullptr,rtcGetDeviceError(g_device));

  /* set error handler */
  rtcSetDeviceErrorFunction(g_device,error_handler,nullptr);
  RTCScene scene = rtcNewScene(g_device);

  rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);
  rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);

  addGroundPlane(scene);

  /* add cubes */
  addCurve(scene,Vec3fa(4.0f,-1.0f,-3.5f));
  quadCubeID = addQuadSubdivCube(scene,Vec3fa(4.0f,0.0f,0.0f));
  triCubeID  = addTriangleSubdivCube(scene,Vec3fa(4.0f,0.0f,3.5f));
  addTriangleCube(scene,Vec3fa(0.0f,0.0f,-3.0f));
  addQuadCube(scene,Vec3fa(0.0f,0.0f,3.0f));


  rtcCommitScene(scene);

  //DumpScene(scene);

  int width = 512;
  int height = 512;

  std::vector<float> rgb(width * height * 3, 0.0f);

  auto t_start = std::chrono::high_resolution_clock::now();
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // Simple camera. change eye pos and direction fit to .obj model.
      std::cout << "x, y = " << x << ", " << y << std::endl;

      RTCRayHit ray;
      ray.ray.flags = 0;
      ray.ray.org_x = 20.0f;
      ray.ray.org_y = 3.0f;
      ray.ray.org_z = 1.0f;

      float dir[3], ndir[3];
      dir[2] = (x / float(width)) - 0.5f;
      dir[1] = (y / float(height)) - 0.5f;
      dir[0] = -1.0f;
      vnormalize(ndir, dir);
      ray.ray.dir_x = ndir[0];
      ray.ray.dir_y = ndir[1];
      ray.ray.dir_z = ndir[2];

      float kFar = 1.0e+14f;
      ray.ray.tnear = 0.0f;
      ray.ray.tfar = kFar;
      ray.ray.mask = 0xffffffff;
      ray.ray.flags = 0;

      ray.hit.primID = RTC_INVALID_GEOMETRY_ID;
      ray.hit.geomID = RTC_INVALID_GEOMETRY_ID;
      ray.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

      {
        RTCIntersectContext context;
        rtcInitIntersectContext(&context);
        rtcIntersect1(scene,&context,&ray);
      }
      if ((ray.ray.tfar < kFar) && (ray.hit.geomID != RTC_INVALID_GEOMETRY_ID) &&
          (ray.hit.primID != RTC_INVALID_GEOMETRY_ID)) {

        Vec3fa color = Vec3fa(1.0f, 0.0f, 0.0f);;
        /* interpolate diffuse color */
        if (ray.hit.geomID > 0)
        {
          Vec3fa diffuse = Vec3fa(1.0f,0.0f,0.0f);
          unsigned int geomID = ray.hit.geomID; {
            rtcInterpolate0(rtcGetGeometry(scene,geomID),ray.hit.primID,ray.hit.u,ray.hit.v,RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,0,&diffuse.x,3);
          }
          //return diffuse;
          color = 0.5f*diffuse;
        }

        /* calculate smooth shading normal */
        Vec3fa Ng;
        Ng[0] = ray.hit.Ng_x;
        Ng[1] = ray.hit.Ng_y;
        Ng[2] = ray.hit.Ng_z;
        if (ray.hit.geomID == 2 || ray.hit.geomID == 3) {
          //std::cout << "hit 2" << std::endl;
          Vec3fa dPdu,dPdv;
          unsigned int geomID = ray.hit.geomID; {
            rtcInterpolate1(rtcGetGeometry(scene,geomID),ray.hit.primID,ray.hit.u,ray.hit.v,RTC_BUFFER_TYPE_VERTEX,0,nullptr,&dPdu.x,&dPdv.x,3);
          }
          //return dPdu;
          Ng = normalize(cross(dPdu,dPdv));

          color = 0.5f * Ng + Vec3fa(0.5f);
        }

        // Flip Y
        rgb[3 * size_t((height - y - 1) * width + x) + 0] = color[0];
        rgb[3 * size_t((height - y - 1) * width + x) + 1] = color[1];
        rgb[3 * size_t((height - y - 1) * width + x) + 2] = color[2];
      }
    }
  }
  auto t_end = std::chrono::high_resolution_clock::now();
  double elaspedTimeMs = std::chrono::duration<double, std::milli>(t_end-t_start).count();
  std::cout <<  std::fixed << std::setprecision(2)
            << "Elapsed time " << elaspedTimeMs << " ms" << std::endl << std::flush;

  // Save image.
  SaveImagePNG("render.png", &rgb.at(0), width, height);

  rtcReleaseScene(scene);
  rtcReleaseDevice(g_device);

  return EXIT_SUCCESS;
}
