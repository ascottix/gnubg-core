/*
 * Copyright (C) 2007-2008 Jon Kinsey <jonkinsey@gmail.com>
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

#ifndef MULTITHREAD_H
#define MULTITHREAD_H

#include "config.h"

#include "backgammon.h"

#define multi_debug(x)

typedef struct Task {
    AsyncFun fun;
    void *data;
    struct Task *pLinkedTask;
} Task;

typedef struct {
    int id;
    move *aMoves;
    NNState *pnnState;
} ThreadLocalData;

typedef struct {
    GCond cond;
    int signalled;
} * ManualEvent;	/* a ManualEvent is a pointer to this struct */

typedef GMutex Mutex;

typedef struct {
    GList *tasks;
    int doneTasks;
    int result;
    ThreadLocalData *tld;
} ThreadData;

extern int MT_GetDoneTasks(void);
extern void MT_AbortTasks(void);
extern void MT_AddTask(Task * pt, gboolean lock);
extern void mt_add_tasks(unsigned int num_tasks, AsyncFun pFun, void *taskData, gpointer linked);
extern int MT_WaitForTasks(gboolean(*pCallback) (gpointer), int callbackTime, int autosave);
extern void MT_InitThreads(void);
extern void MT_Close(void);
extern void MT_CloseThreads(void);
extern void CloseThread(void *unused);
extern ThreadLocalData *MT_CreateThreadLocalData(int id);

extern ThreadData td;

#if !defined(MAX_NUMTHREADS)
#define MAX_NUMTHREADS 1
#endif

extern int asyncRet;
#define MT_Exclusive() {}
#define MT_Release() {}
#define MT_GetNumThreads() 1
#define MT_SetResultFailed() asyncRet = -1
#define MT_SafeInc(x) (++(*x))
#define MT_SafeIncValue(x) (++(*x))
#define MT_SafeIncCheck(x) ((*x)++)
#define MT_SafeAdd(x, y) ((*x) += y)
#define MT_SafeDec(x) (--(*x))
#define MT_SafeDecCheck(x) ((--(*x)) == 0)
#define MT_SafeGet(x) (*x)
#define MT_SafeSet(x, y) ((*x) = y)
#define MT_SafeCompare(x, y) ((*x) == y)
#define MT_GetThreadID() 0
#define MT_Get_nnState() td.tld->pnnState
#define MT_Get_aMoves() td.tld->aMoves
#define MT_GetTLD() td.tld

#endif
