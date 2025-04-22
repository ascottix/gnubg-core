/*
 * Copyright (C) 2007-2009 Jon Kinsey <jonkinsey@gmail.com>
 * Copyright (C) 2007-2023 the AUTHORS
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/simd.h"
#include "multithread.h"
#include "rollout.h"
#include "util.h"

#include "multithread.h"
#include <stdlib.h>

int asyncRet;
void MT_AddTask(Task *pt, gboolean lock)
{
    (void)lock;    /* silence compiler warning */
    td.result = 0; /* Reset result for new tasks */
    td.tasks = g_list_append(td.tasks, pt);
}

void mt_add_tasks(unsigned int num_tasks, AsyncFun pFun, void *taskData, gpointer linked)
{
    unsigned int i;
    for (i = 0; i < num_tasks; i++) {
        Task *pt = (Task *)g_malloc(sizeof(Task));
        pt->fun = pFun;
        pt->data = taskData;
        pt->pLinkedTask = linked;
        MT_AddTask(pt, FALSE);
    }
}

extern int
MT_GetDoneTasks(void)
{
    return MT_SafeGet(&td.doneTasks);
}

int MT_WaitForTasks(gboolean (*pCallback)(gpointer), int callbackTime, int autosave)
{
    GList *member;
    // guint as_source = 0; // TODO

    (void)callbackTime; /* silence compiler warning */
    MT_SafeSet(&td.doneTasks, 0);

    multi_debug("waiting for all tasks");

    if (pCallback)
        pCallback(NULL);

    guint cb_source = g_timeout_add(1000, pCallback, NULL);
    // TODO
    // if (autosave)
    //     as_source = g_timeout_add(nAutoSaveTime * 60000, save_autosave, NULL);
    for (member = g_list_first(td.tasks); member; member = member->next, MT_SafeInc(&td.doneTasks)) {
        Task *task = member->data;
        task->fun(task->data);
        g_free(task->pLinkedTask);
        g_free(task);
        ProcessEvents();
    }
    g_list_free(td.tasks);
    // TODO
    // if (autosave) {
    //     g_source_remove(as_source);
    //     save_autosave(NULL);
    // }

    g_source_remove(cb_source);
    td.tasks = NULL;

    return td.result;
}

extern void
MT_AbortTasks(void)
{
    td.result = -1;
}
