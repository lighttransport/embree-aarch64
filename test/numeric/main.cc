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

#include "../../kernels/geometry/curve_intersector_virtual.h"

#include <cstdio>
#include <cstdlib>
#include <limits>
#include <cmath>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>


void issue_24()
{
  typedef embree::vfloat4 vfloatx;

  vfloatx a(-std::numeric_limits<float>::signaling_NaN());

  vfloatx b = clamp(a, vfloatx(0.0f), vfloatx(1.0f));

  printf("a.x = %f\n", a[0]);
  printf("b.x = %f\n", b[0]); // expects 1.0

  vfloatx c(-std::numeric_limits<float>::quiet_NaN());

  vfloatx d = clamp(c, vfloatx(0.0f), vfloatx(1.0f));

  printf("c.x = %f\n", c[0]);
  printf("d.x = %f\n", d[0]); // expects 1.0
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  issue_24();


  return EXIT_SUCCESS;
}
