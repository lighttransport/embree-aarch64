// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
 
#include "curve_intersector_virtual_point.h"

namespace embree
{
  namespace isa
  {
#if defined (__AVX__)
    void AddVirtualCurvePointInterector8i(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_SPHERE_POINT] = SphereNiIntersectors<8>();
      prim.vtbl[Geometry::GTY_DISC_POINT] = DiscNiIntersectors<8>();
      prim.vtbl[Geometry::GTY_ORIENTED_DISC_POINT] = OrientedDiscNiIntersectors<8>();
    }
    void AddVirtualCurvePointInterector8v(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_SPHERE_POINT] = SphereNiIntersectors<8>();
      prim.vtbl[Geometry::GTY_DISC_POINT] = DiscNiIntersectors<8>();
      prim.vtbl[Geometry::GTY_ORIENTED_DISC_POINT] = OrientedDiscNiIntersectors<8>();
    }

    void AddVirtualCurvePointInterector8iMB(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_SPHERE_POINT] = SphereNiMBIntersectors<8>();
      prim.vtbl[Geometry::GTY_DISC_POINT] = DiscNiMBIntersectors<8>();
      prim.vtbl[Geometry::GTY_ORIENTED_DISC_POINT] = OrientedDiscNiMBIntersectors<8>();
    }
#endif
  }
}
