// ======================================================================== //
// Copyright 2009-2020 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#if defined(__aarch64__)
#include <arm_neon.h>
#endif

#include "constants.h"

namespace embree
{
  TrueTy True;
  FalseTy False;
  ZeroTy zero;
  OneTy one;
  NegInfTy neg_inf;
  PosInfTy inf;
  PosInfTy pos_inf;
  NaNTy nan;
  UlpTy ulp;
  PiTy pi;
  OneOverPiTy one_over_pi;
  TwoPiTy two_pi;
  OneOverTwoPiTy one_over_two_pi;
  FourPiTy four_pi;
  OneOverFourPiTy one_over_four_pi;
  StepTy step;
  ReverseStepTy reverse_step;
  EmptyTy empty;
  UndefinedTy undefined;

#if defined(__aarch64__)
const uint32x4_t movemask_mask = { 1, 2, 4, 8 };
const uint32x4_t vzero = { 0, 0, 0, 0 };
const uint32x4_t v0x80000000 = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
const uint32x4_t v0x7fffffff = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
const uint32x4_t v000F = { 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF };
const uint32x4_t v00F0 = { 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000 };
const uint32x4_t v00FF = { 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF };
const uint32x4_t v0F00 = { 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 };
const uint32x4_t v0F0F = { 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF };
const uint32x4_t v0FF0 = { 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };
const uint32x4_t v0FFF = { 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
const uint32x4_t vF000 = { 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 };
const uint32x4_t vF00F = { 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF };
const uint32x4_t vF0F0 = { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000 };
const uint32x4_t vF0FF = { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF };
const uint32x4_t vFF00 = { 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000 };
const uint32x4_t vFF0F = { 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF };
const uint32x4_t vFFF0 = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };
const uint32x4_t vFFFF = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
const uint8x16_t v0022 = {0,1,2,3, 0,1,2,3, 8,9,10,11, 8,9,10,11};
const uint8x16_t v1133 = {4,5,6,7, 4,5,6,7, 12,13,14,15, 12,13,14,15};
const uint8x16_t v0101 = {0,1,2,3, 4,5,6,7, 0,1,2,3, 4,5,6,7};
const float32x4_t vOne = { 1.0f, 1.0f, 1.0f, 1.0f };
const float32x4_t vmOne = { -1.0f, -1.0f, -1.0f, -1.0f };
const float32x4_t vInf = { INFINITY, INFINITY, INFINITY, INFINITY };
const float32x4_t vmInf = { -INFINITY, -INFINITY, -INFINITY, -INFINITY };
#endif

}
