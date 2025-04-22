/*
 * Copyright (C) 1999-2002 Gary Wong <gtw@gnu.org>
 * Copyright (C) 1999-2016 the AUTHORS
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

#ifndef DICE_H
#define DICE_H

#include <stdio.h>

typedef enum {
    RNG_ISAAC, RNG_MD5
} rng;

typedef struct rngcontext rngcontext;

extern rng rngCurrent;
extern rngcontext *rngctxCurrent;

rngcontext *CopyRNGContext(rngcontext * rngctx);

extern void free_rngctx(rngcontext * rngctx);
extern void *InitRNG(unsigned long *pnSeed, int *pfInitFrom, const int fSet, const rng rngx);
extern void CloseRNG(const rng rngx, rngcontext * rngctx);
extern void DestroyRNG(const rng rngx, rngcontext ** rngctx);
extern void PrintRNGSeed(const rng rngx, rngcontext * rngctx);
extern void PrintRNGCounter(const rng rngx, rngcontext * rngctx);
extern void InitRNGSeed(unsigned int n, const rng rngx, rngcontext * rngctx);
extern int RNGSystemSeed(const rng rngx, void *p, unsigned long *pnSeed);

extern int RollDice(unsigned int anDice[2], rng * prng, rngcontext * rngctx);

#endif
