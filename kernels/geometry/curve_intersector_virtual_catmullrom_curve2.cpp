// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
 
#include "curve_intersector_virtual_catmullrom_curve.h"

namespace embree
{
  namespace isa
  {
#if defined(__AVX__)
    void AddVirtualCurveCatmullRomCurveInterector8i(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_CATMULL_ROM_CURVE] = CurveNiIntersectors <CatmullRomCurveT,8>();
      prim.vtbl[Geometry::GTY_FLAT_CATMULL_ROM_CURVE ] = RibbonNiIntersectors<CatmullRomCurveT,8>();
      prim.vtbl[Geometry::GTY_ORIENTED_CATMULL_ROM_CURVE] = OrientedCurveNiIntersectors<CatmullRomCurveT,8>();
    }
    void AddVirtualCurveCatmullRomCurveInterector8v(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_CATMULL_ROM_CURVE] = CurveNiIntersectors <CatmullRomCurveT,8>();
      prim.vtbl[Geometry::GTY_FLAT_CATMULL_ROM_CURVE ] = RibbonNiIntersectors<CatmullRomCurveT,8>();
      prim.vtbl[Geometry::GTY_ORIENTED_CATMULL_ROM_CURVE] = OrientedCurveNiIntersectors<CatmullRomCurveT,8>();
    }

    void AddVirtualCurveCatmullRomCurveInterector8iMB(VirtualCurveIntersector &prim) {
      prim.vtbl[Geometry::GTY_ROUND_CATMULL_ROM_CURVE] = CurveNiMBIntersectors <CatmullRomCurveT,8>();
      prim.vtbl[Geometry::GTY_FLAT_CATMULL_ROM_CURVE ] = RibbonNiMBIntersectors<CatmullRomCurveT,8>();
      prim.vtbl[Geometry::GTY_ORIENTED_CATMULL_ROM_CURVE] = OrientedCurveNiMBIntersectors<CatmullRomCurveT,8>();
    }
#endif
  }
}
