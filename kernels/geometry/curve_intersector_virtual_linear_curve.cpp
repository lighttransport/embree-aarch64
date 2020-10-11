// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
 
#include "curve_intersector_virtual_point.h"

namespace embree
{
  namespace isa
  {
    void AddVirtualCurveLinearCurveInterector4i(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_LINEAR_CURVE ] = LinearConeNiIntersectors<4>();
      prim.vtbl[Geometry::GTY_FLAT_LINEAR_CURVE ] = LinearRibbonNiIntersectors<4>();
    }
    void AddVirtualCurveLinearCurveInterector4v(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_LINEAR_CURVE ] = LinearConeNiIntersectors<4>();
      prim.vtbl[Geometry::GTY_FLAT_LINEAR_CURVE ] = LinearRibbonNiIntersectors<4>();
    }

    void AddVirtualCurveLinearCurveInterector4iMB(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_LINEAR_CURVE ] = LinearConeNiMBIntersectors<4>();
      prim.vtbl[Geometry::GTY_FLAT_LINEAR_CURVE ] = LinearRibbonNiMBIntersectors<4>();
    }
  }
}
