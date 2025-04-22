/*
 * Copyright (C) 1997-2000 Gary Wong <gtw@gnu.org>
 * Copyright (C) 2002-2022 the AUTHORS
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

#include <stdlib.h>
#include <string.h>

#include "cache.h"
#include "positionid.h"

int CacheCreate(evalCache *pc, unsigned int s)
{
    if (s > 1u << 31)
        return -1;

    pc->size = s;
    /* adjust size to smallest power of 2 GE to s */
    while ((s & (s - 1)) != 0)
        s &= (s - 1);

    pc->size = (s < pc->size) ? 2 * s : s;
    pc->hashMask = (pc->size >> 1) - 1;

    void *mem = malloc((pc->size / 2) * sizeof(*pc->entries));

    pc->entries = (cacheNode *)mem;

    if (pc->entries == NULL)
        return -1;

    CacheFlush(pc);
    return 0;
}

/* MurmurHash3  https://code.google.com/p/smhasher/wiki/MurmurHash */

extern uint32_t
GetHashKey(uint32_t hashMask, const cacheNodeDetail *restrict e)
{
    uint32_t hash = (uint32_t)e->nEvalContext;
    int i;

    hash *= 0xcc9e2d51;
    hash = (hash << 15) | (hash >> (32 - 15));
    hash *= 0x1b873593;

    hash = (hash << 13) | (hash >> (32 - 13));
    hash = hash * 5 + 0xe6546b64;

    for (i = 0; i < 7; i++) {
        uint32_t k = e->key.data[i];

        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> (32 - 15));
        k *= 0x1b873593;

        hash ^= k;
        hash = (hash << 13) | (hash >> (32 - 13));
        hash = hash * 5 + 0xe6546b64;
    }

    /* Real MurmurHash3 has a "hash ^= len" here,
     * but for us len is constant. Skip it */

    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;

    return (hash & hashMask);
}

uint32_t
CacheLookupWithLocking(evalCache *restrict pc, const cacheNodeDetail *restrict e, float *restrict arOut, float *restrict arCubeful)
{
    uint32_t const l = GetHashKey(pc->hashMask, e);

    if (!EqualKeys(pc->entries[l].nd_primary.key, e->key) || pc->entries[l].nd_primary.nEvalContext != e->nEvalContext) {         /* Not in primary slot */
        if (!EqualKeys(pc->entries[l].nd_secondary.key, e->key) || pc->entries[l].nd_secondary.nEvalContext != e->nEvalContext) { /* Cache miss */
            return l;
        } else { /* Found in second slot, promote "hot" entry */
            cacheNodeDetail tmp = pc->entries[l].nd_primary;

            pc->entries[l].nd_primary = pc->entries[l].nd_secondary;
            pc->entries[l].nd_secondary = tmp;
        }
    }
    /* Cache hit */
    memcpy(arOut, pc->entries[l].nd_primary.ar, sizeof(float) * 5 /*NUM_OUTPUTS */);
    if (arCubeful)
        *arCubeful = pc->entries[l].nd_primary.ar[5]; /* Cubeful equity stored in slot 5 */

    return CACHEHIT;
}

uint32_t
CacheLookupNoLocking(evalCache *restrict pc, const cacheNodeDetail *restrict e, float *restrict arOut, float *restrict arCubeful)
{
    uint32_t const l = GetHashKey(pc->hashMask, e);

    if (!EqualKeys(pc->entries[l].nd_primary.key, e->key) || pc->entries[l].nd_primary.nEvalContext != e->nEvalContext) {         /* Not in primary slot */
        if (!EqualKeys(pc->entries[l].nd_secondary.key, e->key) || pc->entries[l].nd_secondary.nEvalContext != e->nEvalContext) { /* Cache miss */
            return l;
        } else { /* Found in second slot, promote "hot" entry */
            cacheNodeDetail tmp = pc->entries[l].nd_primary;

            pc->entries[l].nd_primary = pc->entries[l].nd_secondary;
            pc->entries[l].nd_secondary = tmp;
        }
    }

    /* Cache hit */
    memcpy(arOut, pc->entries[l].nd_primary.ar, sizeof(float) * 5 /*NUM_OUTPUTS */);
    if (arCubeful)
        *arCubeful = pc->entries[l].nd_primary.ar[5]; /* Cubeful equity stored in slot 5 */

    return CACHEHIT;
}

void CacheAddWithLocking(evalCache *restrict pc, const cacheNodeDetail *restrict e, uint32_t l)
{
    pc->entries[l].nd_secondary = pc->entries[l].nd_primary;
    pc->entries[l].nd_primary = *e;
}

/* CacheAddNoLocking() is inlined and in cache.h */

void CacheDestroy(const evalCache *pc)
{
    free(pc->entries);
}

void CacheFlush(const evalCache *pc)
{
    unsigned int k;
    for (k = 0; k < pc->size / 2; ++k) {
        pc->entries[k].nd_primary.key.data[0] = (unsigned int)-1;
        pc->entries[k].nd_secondary.key.data[0] = (unsigned int)-1;
    }
}

int CacheResize(evalCache *pc, unsigned int cNew)
{
    if (cNew != pc->size) {
        CacheDestroy(pc);
        if (CacheCreate(pc, cNew) != 0)
            return -1;
    }

    return (int)pc->size;
}
