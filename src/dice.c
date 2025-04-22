/*
 * Copyright (C) 1999-2003 Gary Wong <gtw@gnu.org>
 * Copyright (C) 1999-2023 the AUTHORS
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

#include "backgammon.h"
#include "config.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "dice.h"
#include "glib_shim.h"
#include "isaac.h"
#include "md5.h"

rng rngCurrent = RNG_ISAAC;
rngcontext *rngctxCurrent = NULL;

struct rngcontext {
    /* RNG_ISAAC */
    randctx rc;

    /* RNG_MD5 */
    md5_uint32 nMD5; /* the current MD5 seed */

    /* common */
    unsigned long c; /* counter */
    unsigned int n;  /* seed */
};

extern void
PrintRNGCounter(const rng rngx, rngcontext *rngctx)
{
    switch (rngx) {
    case RNG_ISAAC:
    case RNG_MD5:
        g_print(_("Number of calls since last seed: %lu."), rngctx->c);
        g_print("\n");
        break;
    }
}

static void
PrintRNGSeedNormal(unsigned int n)
{
    g_print(_("The current seed is %u."), n);
    g_print("\n");
}

extern void
PrintRNGSeed(const rng rngx, rngcontext *rngctx)
{
    switch (rngx) {
    case RNG_MD5:
        g_print(_("The current seed is %u."), rngctx->nMD5);
        g_print("\n");
        return;
    case RNG_ISAAC:
        PrintRNGSeedNormal(rngctx->n);
        return;
    }
    g_printerr(_("You cannot show the seed with this random number generator."));
    g_printerr("\n");
}

extern void
InitRNGSeed(unsigned int n, const rng rngx, rngcontext *rngctx)
{
    rngctx->n = n;
    rngctx->c = 0;

    switch (rngx) {
    case RNG_ISAAC: {
        int i;

        for (i = 0; i < RANDSIZ; i++)
            rngctx->rc.randrsl[i] = (ub4)n;

        irandinit(&rngctx->rc, TRUE);

        break;
    }
    case RNG_MD5:
        rngctx->nMD5 = n;
        break;
    }
}

extern void
CloseRNG(const rng rngx, rngcontext *rngctx)
{
}

extern int
RNGSystemSeed(const rng rngx, void *p, unsigned long *pnSeed)
{
    int f = FALSE;
    rngcontext *rngctx = (rngcontext *)p;
    unsigned int n = 0;

    if (!f) {
        gint64 tv;
        tv = g_get_real_time();
        n = (unsigned int)(((guint64)tv >> 32) ^ ((guint64)tv & 0xFFFFFFFF));
    }

    InitRNGSeed(n, rngx, rngctx);

    if (pnSeed)
        *pnSeed = (unsigned long)n;

    return f;
}

extern void
free_rngctx(rngcontext *rngctx)
{
    g_free(rngctx);
}

extern void *
InitRNG(unsigned long *pnSeed, int *pfInitFrom, const int fSet, const rng rngx)
{
    int f = FALSE;
    rngcontext *rngctx = g_try_new0(rngcontext, 1);

    if (rngctx == NULL)
        return NULL;

    rngctx->c = 0;

    if (fSet)
        f = RNGSystemSeed(rngx, rngctx, pnSeed);

    if (pfInitFrom)
        *pfInitFrom = f;

    return rngctx;
}

extern int
RollDice(unsigned int anDice[2], rng *prng, rngcontext *rngctx)
{
    unsigned long tmprnd;
    const unsigned long exp232_q = 715827882;
    const unsigned long exp232_l = 4294967292U;

    anDice[0] = anDice[1] = 0;

    switch (*prng) {

    case RNG_ISAAC:
        while ((tmprnd = irand(&rngctx->rc)) >= exp232_l)
            ; /* Try again */
        anDice[0] = 1 + (unsigned int)(tmprnd / exp232_q);
        while ((tmprnd = irand(&rngctx->rc)) >= exp232_l)
            ;
        anDice[1] = 1 + (unsigned int)(tmprnd / exp232_q);
        rngctx->c += 2;
        break;

    case RNG_MD5: {
        union _hash {
            char ach[16];
            md5_uint32 an[2];
        } h;

        md5_buffer((char *)&rngctx->nMD5, sizeof rngctx->nMD5, &h);
        while (h.an[0] >= exp232_l || h.an[1] >= exp232_l) {
            md5_buffer((char *)&rngctx->nMD5, sizeof rngctx->nMD5, &h);
            rngctx->nMD5++; /* useful ? indispensable ? */
        }

        anDice[0] = (unsigned int)(h.an[0] / exp232_q + 1);
        anDice[1] = (unsigned int)(h.an[1] / exp232_q + 1);

        rngctx->nMD5++;
        rngctx->c += 2;

        break;
    }
    }
    return 0;
}

rngcontext *
CopyRNGContext(rngcontext *rngctx)
{
    rngcontext *newCtx = (rngcontext *)g_malloc(sizeof(rngcontext));
    *newCtx = *rngctx;
    return newCtx;
}
