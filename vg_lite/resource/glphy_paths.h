/****************************************************************************
 * apps/testing/gpu/vg_lite/resource/glphy_paths.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef GLPHY_PATHS_H
#define GLPHY_PATHS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

/* '龍' */

static const int16_t glphy_u9f8d_path_data[] =
  {
    VLC_OP_MOVE, 1839, 1694,
    VLC_OP_QUAD, 2185, 1647, 2381, 1444,
    VLC_OP_QUAD, 2577, 1242, 2577, 934,
    VLC_OP_QUAD, 2577, 486, 2242, 245,
    VLC_OP_QUAD, 1907, 4, 1289, 4,
    VLC_OP_LINE, 341, 4,
    VLC_OP_LINE, 341, 3239,
    VLC_OP_LINE, 1284, 3239,
    VLC_OP_QUAD, 1831, 3239, 2123, 3025,
    VLC_OP_QUAD, 2415, 2812, 2415, 2419,
    VLC_OP_QUAD, 2415, 2159, 2261, 1973,
    VLC_OP_QUAD, 2108, 1788, 1839, 1720,
    VLC_OP_LINE, 1839, 1694,
    VLC_OP_MOVE, 670, 2927,
    VLC_OP_LINE, 670, 1856,
    VLC_OP_LINE, 1267, 1856,
    VLC_OP_QUAD, 1647, 1856, 1858, 2003,
    VLC_OP_QUAD, 2069, 2151, 2069, 2415,
    VLC_OP_QUAD, 2069, 2663, 1854, 2795,
    VLC_OP_QUAD, 1639, 2927, 1242, 2927,
    VLC_OP_LINE, 670, 2927,
    VLC_OP_MOVE, 1242, 316,
    VLC_OP_QUAD, 1720, 316, 1976, 480,
    VLC_OP_QUAD, 2232, 644, 2232, 956,
    VLC_OP_QUAD, 2232, 1237, 1982, 1388,
    VLC_OP_QUAD, 1732, 1540, 1267, 1540,
    VLC_OP_LINE, 670, 1540,
    VLC_OP_LINE, 670, 316,
    VLC_OP_LINE, 1242, 316,
    VLC_OP_END
  };

/* 'A' */

static const int16_t glphy_u0041_path_data[] =
  {
    VLC_OP_MOVE, 2748, 4,
    VLC_OP_LINE, 2398, 4,
    VLC_OP_LINE, 2005, 1109,
    VLC_OP_LINE, 802, 1109,
    VLC_OP_LINE, 410, 4,
    VLC_OP_LINE, 60, 4,
    VLC_OP_LINE, 1229, 3239,
    VLC_OP_LINE, 1575, 3239,
    VLC_OP_LINE, 2748, 4,
    VLC_OP_MOVE, 1391, 2756,
    VLC_OP_LINE, 917, 1425,
    VLC_OP_LINE, 1890, 1425,
    VLC_OP_LINE, 1417, 2756,
    VLC_OP_LINE, 1391, 2756,
    VLC_OP_END
  };

/* '显' */

static const int16_t glphy_u663e_path_data[] =
  {
    VLC_OP_MOVE, 5267, 3424,
    VLC_OP_LINE, 5267, 279,
    VLC_OP_LINE, 7700, 279,
    VLC_OP_LINE, 7700, -287,
    VLC_OP_LINE, 524, -287,
    VLC_OP_LINE, 524, 279,
    VLC_OP_LINE, 2949, 279,
    VLC_OP_LINE, 2949, 3424,
    VLC_OP_LINE, 1327, 3424,
    VLC_OP_LINE, 1327, 6570,
    VLC_OP_LINE, 6881, 6570,
    VLC_OP_LINE, 6881, 3424,
    VLC_OP_LINE, 5267, 3424,
    VLC_OP_MOVE, 6259, 5267,
    VLC_OP_LINE, 6259, 6021,
    VLC_OP_LINE, 1950, 6021,
    VLC_OP_LINE, 1950, 5267,
    VLC_OP_LINE, 6259, 5267,
    VLC_OP_MOVE, 6259, 4743,
    VLC_OP_LINE, 1950, 4743,
    VLC_OP_LINE, 1950, 3973,
    VLC_OP_LINE, 6259, 3973,
    VLC_OP_LINE, 6259, 4743,
    VLC_OP_MOVE, 4653, 3424,
    VLC_OP_LINE, 3564, 3424,
    VLC_OP_LINE, 3564, 279,
    VLC_OP_LINE, 4653, 279,
    VLC_OP_LINE, 4653, 3424,
    VLC_OP_MOVE, 5644, 1163,
    VLC_OP_QUAD, 5923, 1540, 6259, 2113,
    VLC_OP_QUAD, 6595, 2687, 6758, 3072,
    VLC_OP_LINE, 7274, 2777,
    VLC_OP_QUAD, 7086, 2376, 6742, 1794,
    VLC_OP_QUAD, 6398, 1212, 6144, 860,
    VLC_OP_LINE, 5644, 1163,
    VLC_OP_MOVE, 1401, 2966,
    VLC_OP_QUAD, 1638, 2654, 1978, 2138,
    VLC_OP_QUAD, 2318, 1622, 2540, 1229,
    VLC_OP_LINE, 2040, 852,
    VLC_OP_QUAD, 1827, 1262, 1491, 1790,
    VLC_OP_QUAD, 1155, 2318, 909, 2630,
    VLC_OP_LINE, 1401, 2966,
    VLC_OP_END
  };

#endif
