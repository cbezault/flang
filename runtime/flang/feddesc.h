/*
 * Copyright (c) 1995-2019, NVIDIA CORPORATION.  All rights reserved.
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

/** \file
 * \brief Definitions for FORTRAN edit descriptor symbolics
 *
 * If contents are changed, changes must also be applied to the run-time
 * document file (fio.n) and this file must copied to the io source rte
 * directory.
 */

#define FED_END -1
#define FED_LPAREN -2
#define FED_RPAREN -3
#define FED_P -4
#define FED_STR -5
#define FED_T -6
#define FED_TL -7
#define FED_TR -8
#define FED_X -9
#define FED_S -10
#define FED_SP -11
#define FED_SS -12
#define FED_BN -13
#define FED_BZ -14
#define FED_SLASH -15
#define FED_COLON -16
#define FED_Q -17
#define FED_DOLLAR -18
#define FED_Aw -19
#define FED_Lw -20
#define FED_Iw_m -21
#define FED_Fw_d -22
#define FED_Ee -23
#define FED_Ew_d -24
#define FED_ENw_d -25
#define FED_ESw_d -26
#define FED_Gw_d -27
#define FED_Dw_d -28
#define FED_Ow_m -29
#define FED_Zw_m -30
#define FED_A -31
#define FED_L -32
#define FED_I -33
#define FED_F -34
#define FED_E -35
#define FED_G -36
#define FED_D -37
#define FED_O -38
#define FED_Z -39
#define FED_KANJI_STRING -40
#define FED_Nw -41
#define FED_N -42
#define FED_Bw_m -43
#define FED_ERROR -44
#define FED_DC -45
#define FED_DP -46
#define FED_RU -47
#define FED_RD -48
#define FED_RZ -49
#define FED_RN -50
#define FED_RC -51
#define FED_RP -52
#define FED_DT -53
#define FED_G0 -54
#define FED_G0_d -55

void I8(get_vlist_desc)(F90_Desc *sd, __INT_T ubnd);
