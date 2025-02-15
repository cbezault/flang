/*
 * Copyright (c) 2017-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "mthdecls.h"

/*
 * Generic implementation of intrinsic sincos.
 *
 * Compiler expects two return values, thus using the complex type to implement
 * the return sequence.
 */

double_complex_t __mth_i_dsincos(double a)
{
  double s, c;
  __mth_dsincos(a, &s, &c);
  double_complex_t r = PGMATH_CMPLX_CONST(s, c);
  return r;
}
