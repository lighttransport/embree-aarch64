// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
 
#include "curve_intersector_virtual_point.h"

namespace embree
{
  namespace isa
  {
    void AddVirtualCurvePointInterector4i(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_SPHERE_POINT] = SphereNiIntersectors<4>();
      prim.vtbl[Geometry::GTY_DISC_POINT] = DiscNiIntersectors<4>();
      prim.vtbl[Geometry::GTY_ORIENTED_DISC_POINT] = OrientedDiscNiIntersectors<4>();
    }
    void AddVirtualCurvePointInterector4v(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_SPHERE_POINT] = SphereNiIntersectors<4>();
      prim.vtbl[Geometry::GTY_DISC_POINT] = DiscNiIntersectors<4>();
      prim.vtbl[Geometry::GTY_ORIENTED_DISC_POINT] = OrientedDiscNiIntersectors<4>();
    }

    void AddVirtualCurvePointInterector4iMB(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_SPHERE_POINT] = SphereNiMBIntersectors<4>();
      prim.vtbl[Geometry::GTY_DISC_POINT] = DiscNiMBIntersectors<4>();
      prim.vtbl[Geometry::GTY_ORIENTED_DISC_POINT] = OrientedDiscNiMBIntersectors<4>();
    }
  }
}
