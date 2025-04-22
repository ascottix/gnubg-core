/*
 * Copyright (C) 2007-2009 Jon Kinsey <jonkinsey@gmail.com>
 * Copyright (C) 2007-2018 the AUTHORS
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

/*
 * Multithreaded support functions, moved out of multithread.c
 */

#include "config.h"
#include "multithread.h"

#include <stdlib.h>
#include <string.h>

#include "lib/simd.h"

SSE_ALIGN(ThreadData td);

extern ThreadLocalData *
MT_CreateThreadLocalData(int id)
{
    ThreadLocalData *tld = (ThreadLocalData *) g_malloc(sizeof(ThreadLocalData));
    tld->id = id;
    tld->pnnState = (NNState *) g_malloc(sizeof(NNState) * 3);
    // cppcheck-suppress duplicateExpression
    tld->pnnState[CLASS_RACE - CLASS_RACE].savedBase = g_malloc0(nnRace.cHidden * sizeof(float));
    // cppcheck-suppress duplicateExpression
    tld->pnnState[CLASS_RACE - CLASS_RACE].savedIBase = g_malloc0(nnRace.cInput * sizeof(float));
    tld->pnnState[CLASS_CRASHED - CLASS_RACE].savedBase = g_malloc0(nnCrashed.cHidden * sizeof(float));
    tld->pnnState[CLASS_CRASHED - CLASS_RACE].savedIBase = g_malloc0(nnCrashed.cInput * sizeof(float));
    tld->pnnState[CLASS_CONTACT - CLASS_RACE].savedBase = g_malloc0(nnContact.cHidden * sizeof(float));
    tld->pnnState[CLASS_CONTACT - CLASS_RACE].savedIBase = g_malloc0(nnContact.cInput * sizeof(float));

    tld->aMoves = (move *) g_malloc0(sizeof(move) * MAX_INCOMPLETE_MOVES);
    return tld;
}

extern void
MT_InitThreads(void)
{
    td.tld = MT_CreateThreadLocalData(-1);
}

extern void
MT_Close(void)
{
    int i;
    NNState *pnnState;

    if (!td.tld)
        return;

    g_free(td.tld->aMoves);
    pnnState = td.tld->pnnState;
    for (i = 0; i < 3; i++) {
        g_free(pnnState[i].savedBase);
        g_free(pnnState[i].savedIBase);
    }
    g_free(pnnState);
    g_free(td.tld);
}
