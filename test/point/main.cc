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

#include "../../tutorials/common/tutorial/tutorial_device.h"
#include "../../tutorials/common/core/ray.h"
#include "../../kernels/bvh/bvh.h"
#include "../../tutorials/common/math/random_sampler.h"

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
// Point genrator based on Embree's curve_geometry tutorial
//
using namespace embree;

#define NUM_POINTS 512
Vec3fa point_colors[NUM_POINTS];

/* add point geometry */
static void addPoints (RTCDevice device, RTCScene scene, RTCGeometryType gtype, const Vec3fa& pos)
{
  RandomSampler rng;
  RandomSampler_init(rng, 42);

#define COORD RandomSampler_get1D(rng) * 4.f - 2.f
#define RADIUS RandomSampler_get1D(rng) * 0.13f + 0.02f
#define COLOR RandomSampler_get1D(rng)
#define NORMAL RandomSampler_get1D(rng) * 2.f - 1.f

  RTCGeometry geom = rtcNewGeometry(device, gtype);
  Vec4f *point_vertices = (Vec4f*)rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT4, sizeof(Vec4f), NUM_POINTS);

  for (int i = 0; i < NUM_POINTS; i++) {
    float vtx[4];
    vtx[0] = COORD;
    vtx[1] = COORD;
    vtx[2] = COORD;
    vtx[3] = RADIUS;
    float color[3];
    color[0] = COLOR;
    color[1] = COLOR;
    color[2] = COLOR;
    point_vertices[i] = Vec4f(pos.x, pos.y, pos.z, 0.0f) + Vec4f(vtx[0], vtx[1], vtx[2], vtx[3]);
    point_colors[i] = Vec3fa(color[0], color[1], color[2]);
  }

  if (gtype == RTC_GEOMETRY_TYPE_ORIENTED_DISC_POINT) {
    Vec3fa *point_normals = (Vec3fa*)rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_NORMAL, 0, RTC_FORMAT_FLOAT3, sizeof(Vec3fa), NUM_POINTS);
    for (int i = 0; i < NUM_POINTS; i++) {
      float normal[3];
      normal[0] = NORMAL;
      normal[1] = NORMAL;
      normal[2] = NORMAL;
      point_normals[i] = Vec3fa(normal[0], normal[1], normal[2]);
      point_normals[i] = normalize(point_normals[i]);
    }
  }

  rtcCommitGeometry(geom);
  rtcAttachGeometry(scene,geom);
  rtcReleaseGeometry(geom);
}

/* adds a ground plane to the scene */
static unsigned int addGroundPlane (RTCDevice device, RTCScene scene_i)
{
  /* create a triangulated plane with 2 triangles and 4 vertices */
  RTCGeometry geom = rtcNewGeometry (device, RTC_GEOMETRY_TYPE_TRIANGLE);

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
  unsigned int geomID = rtcAttachGeometry(scene_i,geom);
  rtcReleaseGeometry(geom);
  return geomID;
}

/* called by the C++ code for initialization */
static void SceneInit(RTCDevice device, RTCScene scene)
{
  /* add ground plane */
  addGroundPlane(device, scene);

  /* add curve */
  addPoints(device, scene, RTC_GEOMETRY_TYPE_SPHERE_POINT,        Vec3fa( 0.0f, 0.0f, 0.0f));
  addPoints(device, scene, RTC_GEOMETRY_TYPE_DISC_POINT,          Vec3fa( 5.0f, 0.0f, 0.0f));
  addPoints(device, scene, RTC_GEOMETRY_TYPE_ORIENTED_DISC_POINT, Vec3fa(-5.0f, 0.0f, 0.0f));

  /* commit changes to scene */
  rtcCommitScene (scene);

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

static void my_error_handler(void* userPtr, const RTCError code, const char* str = nullptr)
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
  my_error_handler(nullptr,rtcGetDeviceError(device));

  /* set error handler */
  rtcSetDeviceErrorFunction(device,my_error_handler,nullptr);
  RTCScene scene = rtcNewScene(device);

  rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);
  rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);

  SceneInit(device, scene);

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
      if ((x == 128) && (y == 0)) {
        bora();
      }

      RTCRayHit ray;
      ray.ray.flags = 0;
      ray.ray.org_x = 0.0f;
      ray.ray.org_y = 1.0f;
      ray.ray.org_z = 14.0f;

      float dir[3], ndir[3];
      dir[0] = (x / float(width)) - 0.5f;
      dir[1] = (y / float(height)) - 0.5f;
      dir[2] = -1.0f;
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

        float3 ng;
        ng[0] = ray.hit.Ng_x;
        ng[1] = ray.hit.Ng_y;
        ng[2] = ray.hit.Ng_z;
        ng = vnormalize(ng);

        // Flip Y
        rgb[3 * size_t((height - y - 1) * width + x) + 0] =
            0.5f * ng[0] + 0.5f;
        rgb[3 * size_t((height - y - 1) * width + x) + 1] =
            0.5f * ng[1] + 0.5f;
        rgb[3 * size_t((height - y - 1) * width + x) + 2] =
            0.5f * ng[2] + 0.5f;
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
  rtcReleaseDevice(device);

  return EXIT_SUCCESS;
}
