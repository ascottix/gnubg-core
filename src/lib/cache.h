/*
 * Copyright (C) 1997-2000 Gary Wong <gtw@gnu.org>
 * Copyright (C) 2002-2018 the AUTHORS
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

#ifndef CACHE_H
#define CACHE_H

#include "config.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
typedef unsigned int uint32_t;
#endif

#include "gnubg-types.h"

/* Set to calculate simple cache stats */
#define CACHE_STATS 0

typedef struct {
    positionkey key;	/* size = 28 */
    int nEvalContext;
    float ar[6];
} cacheNodeDetail;

typedef struct {
    cacheNodeDetail nd_primary;
    cacheNodeDetail nd_secondary;
} cacheNode;

/* name used in eval.c */
typedef cacheNodeDetail evalcache;

typedef struct {
    cacheNode *entries;

    unsigned int size;
    uint32_t hashMask;
} evalCache;

/* Cache size will be adjusted to a power of 2 */
int CacheCreate(evalCache * pc, unsigned int size);
int CacheResize(evalCache * pc, unsigned int cNew);

#define CACHEHIT ((uint32_t)-1)

/* returns a value which is passed to CacheAdd (if a miss) */
unsigned int CacheLookupWithLocking(evalCache * pc, const cacheNodeDetail * e, float *arOut, float *arCubeful);
unsigned int CacheLookupNoLocking(evalCache * pc, const cacheNodeDetail * e, float *arOut, float *arCubeful);

void CacheAddWithLocking(evalCache * pc, const cacheNodeDetail * e, uint32_t l);

static inline void
CacheAddNoLocking(evalCache * pc, const cacheNodeDetail * e, const uint32_t l)
{
    pc->entries[l].nd_secondary = pc->entries[l].nd_primary;
    pc->entries[l].nd_primary = *e;
}

void CacheFlush(const evalCache * pc);
void CacheDestroy(const evalCache * pc);

#if defined(HAVE_FUNC_ATTRIBUTE_PURE)
uint32_t GetHashKey(uint32_t hashMask, const cacheNodeDetail * e) __attribute((pure));
#else
uint32_t GetHashKey(uint32_t hashMask, const cacheNodeDetail * e);
#endif

#endif
