// Modified by Light Transport Entertainment Inc.
// Split implementation to make compilation faster.
//
// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
 
#include "curve_intersector_virtual.h"

#include "curve_intersector_virtual_point.h"
#include "curve_intersector_virtual_linear_curve.h"
#include "curve_intersector_virtual_bezier_curve.h"
#include "curve_intersector_virtual_bspline_curve.h"
#include "curve_intersector_virtual_hermite_curve.h"
#include "curve_intersector_virtual_catmullrom_curve.h"

namespace embree
{
  namespace isa
  {
    VirtualCurveIntersector* VirtualCurveIntersector4i()
    {
      static VirtualCurveIntersector function_local_static_prim;
      AddVirtualCurvePointInterector4i(function_local_static_prim);
      AddVirtualCurveLinearCurveInterector4i(function_local_static_prim);
      AddVirtualCurveBezierCurveInterector4i(function_local_static_prim);
      AddVirtualCurveBSplineCurveInterector4i(function_local_static_prim);
      AddVirtualCurveHermiteCurveInterector4i(function_local_static_prim);
      AddVirtualCurveCatmullRomCurveInterector4i(function_local_static_prim);
      //function_local_static_prim.vtbl[Geometry::GTY_SPHERE_POINT] = SphereNiIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_DISC_POINT] = DiscNiIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_DISC_POINT] = OrientedDiscNiIntersectors<4>();

      //function_local_static_prim.vtbl[Geometry::GTY_CONE_LINEAR_CURVE ] = LinearConeNiIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_LINEAR_CURVE ] = LinearRoundConeNiIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_LINEAR_CURVE ] = LinearRibbonNiIntersectors<4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_BEZIER_CURVE] = CurveNiIntersectors <BezierCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_BEZIER_CURVE ] = RibbonNiIntersectors<BezierCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_BEZIER_CURVE] = OrientedCurveNiIntersectors<BezierCurveT,4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_BSPLINE_CURVE] = CurveNiIntersectors <BSplineCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_BSPLINE_CURVE ] = RibbonNiIntersectors<BSplineCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_BSPLINE_CURVE] = OrientedCurveNiIntersectors<BSplineCurveT,4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_HERMITE_CURVE] = HermiteCurveNiIntersectors <HermiteCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_HERMITE_CURVE ] = HermiteRibbonNiIntersectors<HermiteCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_HERMITE_CURVE] = HermiteOrientedCurveNiIntersectors<HermiteCurveT,4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_CATMULL_ROM_CURVE] = CurveNiIntersectors <CatmullRomCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_CATMULL_ROM_CURVE ] = RibbonNiIntersectors<CatmullRomCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_CATMULL_ROM_CURVE] = OrientedCurveNiIntersectors<CatmullRomCurveT,4>();
      return &function_local_static_prim;
    }

    VirtualCurveIntersector* VirtualCurveIntersector4v()
    {
      static VirtualCurveIntersector function_local_static_prim;
      AddVirtualCurvePointInterector4v(function_local_static_prim);
      AddVirtualCurveLinearCurveInterector4v(function_local_static_prim);
      AddVirtualCurveBezierCurveInterector4v(function_local_static_prim);
      AddVirtualCurveBSplineCurveInterector4v(function_local_static_prim);
      AddVirtualCurveHermiteCurveInterector4v(function_local_static_prim);
      AddVirtualCurveCatmullRomCurveInterector4v(function_local_static_prim);
      //function_local_static_prim.vtbl[Geometry::GTY_SPHERE_POINT] = SphereNiIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_DISC_POINT] = DiscNiIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_DISC_POINT] = OrientedDiscNiIntersectors<4>();

      //function_local_static_prim.vtbl[Geometry::GTY_CONE_LINEAR_CURVE ] = LinearConeNiIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_LINEAR_CURVE ] = LinearRoundConeNiIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_LINEAR_CURVE ] = LinearRibbonNiIntersectors<4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_BEZIER_CURVE] = CurveNvIntersectors <BezierCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_BEZIER_CURVE ] = RibbonNvIntersectors<BezierCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_BEZIER_CURVE] = OrientedCurveNiIntersectors<BezierCurveT,4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_BSPLINE_CURVE] = CurveNvIntersectors <BSplineCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_BSPLINE_CURVE ] = RibbonNvIntersectors<BSplineCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_BSPLINE_CURVE] = OrientedCurveNiIntersectors<BSplineCurveT,4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_HERMITE_CURVE] = HermiteCurveNiIntersectors <HermiteCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_HERMITE_CURVE ] = HermiteRibbonNiIntersectors<HermiteCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_HERMITE_CURVE] = HermiteOrientedCurveNiIntersectors<HermiteCurveT,4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_CATMULL_ROM_CURVE] = CurveNiIntersectors <CatmullRomCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_CATMULL_ROM_CURVE ] = RibbonNiIntersectors<CatmullRomCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_CATMULL_ROM_CURVE] = OrientedCurveNiIntersectors<CatmullRomCurveT,4>();
      return &function_local_static_prim;
    }

    VirtualCurveIntersector* VirtualCurveIntersector4iMB()
    {
      static VirtualCurveIntersector function_local_static_prim;
      AddVirtualCurvePointInterector4iMB(function_local_static_prim);
      AddVirtualCurveLinearCurveInterector4iMB(function_local_static_prim);
      AddVirtualCurveBezierCurveInterector4iMB(function_local_static_prim);
      AddVirtualCurveBSplineCurveInterector4iMB(function_local_static_prim);
      AddVirtualCurveHermiteCurveInterector4iMB(function_local_static_prim);
      AddVirtualCurveCatmullRomCurveInterector4iMB(function_local_static_prim);
      //function_local_static_prim.vtbl[Geometry::GTY_SPHERE_POINT] = SphereNiMBIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_DISC_POINT] = DiscNiMBIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_DISC_POINT] = OrientedDiscNiMBIntersectors<4>();

      //function_local_static_prim.vtbl[Geometry::GTY_CONE_LINEAR_CURVE ] = LinearConeNiMBIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_LINEAR_CURVE ] = LinearRoundConeNiMBIntersectors<4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_LINEAR_CURVE ] = LinearRibbonNiMBIntersectors<4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_BEZIER_CURVE] = CurveNiMBIntersectors <BezierCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_BEZIER_CURVE ] = RibbonNiMBIntersectors<BezierCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_BEZIER_CURVE] = OrientedCurveNiMBIntersectors<BezierCurveT,4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_BSPLINE_CURVE] = CurveNiMBIntersectors <BSplineCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_BSPLINE_CURVE ] = RibbonNiMBIntersectors<BSplineCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_BSPLINE_CURVE] = OrientedCurveNiMBIntersectors<BSplineCurveT,4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_HERMITE_CURVE] = HermiteCurveNiMBIntersectors <HermiteCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_HERMITE_CURVE ] = HermiteRibbonNiMBIntersectors<HermiteCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_HERMITE_CURVE] = HermiteOrientedCurveNiMBIntersectors<HermiteCurveT,4>();

      //function_local_static_prim.vtbl[Geometry::GTY_ROUND_CATMULL_ROM_CURVE] = CurveNiMBIntersectors <CatmullRomCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_FLAT_CATMULL_ROM_CURVE ] = RibbonNiMBIntersectors<CatmullRomCurveT,4>();
      //function_local_static_prim.vtbl[Geometry::GTY_ORIENTED_CATMULL_ROM_CURVE] = OrientedCurveNiMBIntersectors<CatmullRomCurveT,4>();
      return &function_local_static_prim;
    }
  }
}
