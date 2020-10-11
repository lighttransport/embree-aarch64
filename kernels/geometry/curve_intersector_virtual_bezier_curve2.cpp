// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
 
#include "curve_intersector_virtual_bezier_curve.h"

namespace embree
{
  namespace isa
  {
#if defined(__AVX__)
    void AddVirtualCurveBezierCurveInterector8i(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_BEZIER_CURVE] = CurveNiIntersectors <BezierCurveT,8>();
      prim.vtbl[Geometry::GTY_FLAT_BEZIER_CURVE ] = RibbonNiIntersectors<BezierCurveT,8>();
      prim.vtbl[Geometry::GTY_ORIENTED_BEZIER_CURVE] = OrientedCurveNiIntersectors<BezierCurveT,8>();
    }
    void AddVirtualCurveBezierCurveInterector8v(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_BEZIER_CURVE] = CurveNvIntersectors <BezierCurveT,8>();
      prim.vtbl[Geometry::GTY_FLAT_BEZIER_CURVE ] = RibbonNvIntersectors<BezierCurveT,8>();
      prim.vtbl[Geometry::GTY_ORIENTED_BEZIER_CURVE] = OrientedCurveNiIntersectors<BezierCurveT,8>();
    }

    void AddVirtualCurveBezierCurveInterector8iMB(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_BEZIER_CURVE] = CurveNiMBIntersectors <BezierCurveT,8>();
      prim.vtbl[Geometry::GTY_FLAT_BEZIER_CURVE ] = RibbonNiMBIntersectors<BezierCurveT,8>();
      prim.vtbl[Geometry::GTY_ORIENTED_BEZIER_CURVE] = OrientedCurveNiMBIntersectors<BezierCurveT,8>();
    }
#endif
  }
}
