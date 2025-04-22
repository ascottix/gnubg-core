/*
 * Copyright (C) 2006 Oystein Johansen <oystein@gnubg.org>
 * Copyright (C) 2011-2018 the AUTHORS
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "gnubg-types.h"
#include "simd.h"
#include "eval.h"

#if defined(USE_SIMD_INSTRUCTIONS)
#if defined(USE_NEON)
#include <arm_neon.h>
#elif defined(USE_AVX)
#include <immintrin.h>
#elif defined(USE_SSE2)
#include <emmintrin.h>
#else
#include <xmmintrin.h>
#endif
#else
typedef float float_vector[4];
#endif /* USE_SIMD_INSTRUCTIONS */

typedef SSE_ALIGN(float float_vec_aligned[sizeof(float_vector)/sizeof(float)]);

SSE_ALIGN (static float_vec_aligned inpvec[16]) = {
    /*  0 */  { 0.0, 0.0, 0.0, 0.0},
    /*  1 */  { 1.0, 0.0, 0.0, 0.0},
    /*  2 */  { 0.0, 1.0, 0.0, 0.0},
    /*  3 */  { 0.0, 0.0, 1.0, 0.0},
    /*  4 */  { 0.0, 0.0, 1.0, 0.5},
    /*  5 */  { 0.0, 0.0, 1.0, 1.0},
    /*  6 */  { 0.0, 0.0, 1.0, 1.5},
    /*  7 */  { 0.0, 0.0, 1.0, 2.0},
    /*  8 */  { 0.0, 0.0, 1.0, 2.5},
    /*  9 */  { 0.0, 0.0, 1.0, 3.0},
    /* 10 */  { 0.0, 0.0, 1.0, 3.5},
    /* 11 */  { 0.0, 0.0, 1.0, 4.0},
    /* 12 */  { 0.0, 0.0, 1.0, 4.5},
    /* 13 */  { 0.0, 0.0, 1.0, 5.0},
    /* 14 */  { 0.0, 0.0, 1.0, 5.5},
    /* 15 */  { 0.0, 0.0, 1.0, 6.0}};

SSE_ALIGN(static float_vec_aligned inpvecb[16]) = {
    /*  0 */  { 0.0, 0.0, 0.0, 0.0},
    /*  1 */  { 1.0, 0.0, 0.0, 0.0},
    /*  2 */  { 1.0, 1.0, 0.0, 0.0},
    /*  3 */  { 1.0, 1.0, 1.0, 0.0},
    /*  4 */  { 1.0, 1.0, 1.0, 0.5},
    /*  5 */  { 1.0, 1.0, 1.0, 1.0},
    /*  6 */  { 1.0, 1.0, 1.0, 1.5},
    /*  7 */  { 1.0, 1.0, 1.0, 2.0},
    /*  8 */  { 1.0, 1.0, 1.0, 2.5},
    /*  9 */  { 1.0, 1.0, 1.0, 3.0},
    /* 10 */  { 1.0, 1.0, 1.0, 3.5},
    /* 11 */  { 1.0, 1.0, 1.0, 4.0},
    /* 12 */  { 1.0, 1.0, 1.0, 4.5},
    /* 13 */  { 1.0, 1.0, 1.0, 5.0},
    /* 14 */  { 1.0, 1.0, 1.0, 5.5},
    /* 15 */  { 1.0, 1.0, 1.0, 6.0}};

extern void
baseInputs(const TanBoard anBoard, float arInput[])
{
    int j, i;

    for (j = 0; j < 2; ++j) {
        float *afInput = arInput + j * 25 * 4;
        const unsigned int *board = anBoard[j];

        /* Points */
        for (i = 0; i < 24; i++) {
            const unsigned int nc = board[i];

            afInput[i * 4 + 0] = inpvec[nc][0];
            afInput[i * 4 + 1] = inpvec[nc][1];
            afInput[i * 4 + 2] = inpvec[nc][2];
            afInput[i * 4 + 3] = inpvec[nc][3];
        }

        /* Bar */
        {
            const unsigned int nc = board[24];

            afInput[24 * 4 + 0] = inpvecb[nc][0];
            afInput[24 * 4 + 1] = inpvecb[nc][1];
            afInput[24 * 4 + 2] = inpvecb[nc][2];
            afInput[24 * 4 + 3] = inpvecb[nc][3];
        }
    }
}
