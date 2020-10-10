// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
 
#include "curve_intersector_virtual_point.h"

namespace embree
{
  namespace isa
  {
#if defined(__AVX__)
    void AddVirtualCurveLinearCurveInterector8i(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_LINEAR_CURVE ] = LinearConeNiIntersectors<8>();
      prim.vtbl[Geometry::GTY_FLAT_LINEAR_CURVE ] = LinearRibbonNiIntersectors<8>();
    }
    void AddVirtualCurveLinearCurveInterector8v(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_LINEAR_CURVE ] = LinearConeNiIntersectors<8>();
      prim.vtbl[Geometry::GTY_FLAT_LINEAR_CURVE ] = LinearRibbonNiIntersectors<8>();
    }

    void AddVirtualCurveLinearCurveInterector8iMB(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_LINEAR_CURVE ] = LinearConeNiMBIntersectors<8>();
      prim.vtbl[Geometry::GTY_FLAT_LINEAR_CURVE ] = LinearRibbonNiMBIntersectors<8>();
    }
#endif
  }
}
