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

#include "../../include/embree3/rtcore.h"

#include "../../kernels/bvh/bvh.h"

#include "../../tutorials/common/tutorial/noise.h"

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
// Disp genrator based on Embree's displacement_geometry tutorial
//
using namespace embree;

/* configuration */
#define EDGE_LEVEL 256.0f
#define ENABLE_SMOOTH_NORMALS 0

/* scene data */
RTCScene g_scene = nullptr;

/* previous camera position */
Vec3fa old_p;

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


#define NUM_INDICES 24
#define NUM_FACES 6
#define FACE_SIZE 4

unsigned int cube_indices[24] = {
  0, 4, 5, 1,
  1, 5, 6, 2,
  2, 6, 7, 3,
  0, 3, 7, 4,
  4, 7, 6, 5,
  0, 1, 2, 3,
};

unsigned int cube_faces[6] = {
  4, 4, 4, 4, 4, 4
};

float displacement(const Vec3fa& P)
{
  float dN = 0.0f;
  for (float freq = 1.0f; freq<40.0f; freq*= 2) {
    float n = fabs(noise(freq*P));
    dN += 1.4f*n*n/freq;
  }
  return dN;
}

float displacement_du(const Vec3fa& P, const Vec3fa& dPdu)
{
  const float du = 0.001f;
  return (displacement(P+du*dPdu)-displacement(P))/du;
}

float displacement_dv(const Vec3fa& P, const Vec3fa& dPdv)
{
  const float dv = 0.001f;
  return (displacement(P+dv*dPdv)-displacement(P))/dv;
}

void displacementFunction(const struct RTCDisplacementFunctionNArguments* args)
{
  const float* nx = args->Ng_x;
  const float* ny = args->Ng_y;
  const float* nz = args->Ng_z;
  float* px = args->P_x;
  float* py = args->P_y;
  float* pz = args->P_z;
  unsigned int N = args->N;

  for (unsigned int i=0; i<N; i++) {
    const Vec3fa P = Vec3fa(px[i],py[i],pz[i]);
    const Vec3fa Ng = Vec3fa(nx[i],ny[i],nz[i]);
    const Vec3fa dP = displacement(P)*Ng;
    px[i] += dP.x; py[i] += dP.y; pz[i] += dP.z;
  }
}

/* adds a cube to the scene */
unsigned int addCube (RTCDevice device, RTCScene scene_i)
{
  /* create a triangulated cube with 6 quads and 8 vertices */
  RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_SUBDIVISION);

  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, cube_vertices, 0, sizeof(Vec3fa),       8);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX,  0, RTC_FORMAT_UINT,   cube_indices,  0, sizeof(unsigned int), NUM_INDICES);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_FACE,   0, RTC_FORMAT_UINT,   cube_faces,    0, sizeof(unsigned int), NUM_FACES);

  float* level = (float*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_LEVEL, 0, RTC_FORMAT_FLOAT, sizeof(float), NUM_INDICES);
  for (size_t i=0; i<NUM_INDICES; i++) level[i] = EDGE_LEVEL;

  rtcSetGeometryDisplacementFunction(geom,displacementFunction);

  rtcCommitGeometry(geom);
  unsigned int geomID = rtcAttachGeometry(scene_i,geom);
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

  addCube(device, scene);

  rtcCommitScene(scene);

  DumpScene(scene);

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
      ray.ray.org_y = 0.0f;
      ray.ray.org_z = 3.0f;

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

        Vec3fa Ng = normalize(Vec3fa(ray.hit.Ng_x, ray.hit.Ng_y, ray.hit.Ng_z));
#if 1
        
        Vec3fa P;
        P.x = ray.ray.org_x + ray.ray.tfar*ray.ray.dir_x;
        P.y = ray.ray.org_y + ray.ray.tfar*ray.ray.dir_y;
        P.z = ray.ray.org_z + ray.ray.tfar*ray.ray.dir_z;

        if (ray.hit.geomID > 0) {
          Vec3fa dPdu,dPdv;
          unsigned int geomID = ray.hit.geomID; {
            rtcInterpolate1(rtcGetGeometry(scene,geomID),ray.hit.primID,ray.hit.u,ray.hit.v,RTC_BUFFER_TYPE_VERTEX,0,nullptr,&dPdu.x,&dPdv.x,3);
          }
          Ng = normalize(cross(dPdu,dPdv));
          dPdu = dPdu + Ng*displacement_du(P,dPdu);
          dPdv = dPdv + Ng*displacement_dv(P,dPdv);
          Ng = normalize(cross(dPdu,dPdv));
        }
#endif
        // Flip Y
        rgb[3 * size_t((height - y - 1) * width + x) + 0] =
            0.5f * Ng.x + 0.5f;
        rgb[3 * size_t((height - y - 1) * width + x) + 1] =
            0.5f * Ng.y + 0.5f;
        rgb[3 * size_t((height - y - 1) * width + x) + 2] =
            0.5f * Ng.z + 0.5f;
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
