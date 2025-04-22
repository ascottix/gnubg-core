/*
 * Copyright (C) 2025 Alessandro Scotti
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
#include "movefilters.inc"
#include "glib_shim.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

int fOutputMWC = FALSE;
int fAutoCrawford = 1;
int fAutoSaveRollout = FALSE;
int nAutoSaveTime = 15;
int fShowProgress = 0;

matchstate ms = {
    {{0}, {0}},         /* anBoard */
    {0},                /* anDice */
    -1,                 /* fTurn */
    0,                  /* fResigned */
    0,                  /* fResignationDeclined */
    FALSE,              /* fDoubled */
    0,                  /* cGames */
    -1,                 /* fMove */
    -1,                 /* fCubeOwner */
    FALSE,              /* fCrawford */
    FALSE,              /* fPostCrawford */
    0,                  /* nMatchTo */
    {0, 0},             /* anScore */
    1,                  /* nCube */
    0,                  /* cBeavers */
    VARIATION_STANDARD, /* bgv */
    TRUE,               /* fCubeUse */
    TRUE,               /* fJacoby */
    GAME_NONE           /* gs */
};

rngcontext *rngctxRollout = NULL;

// Global used by rollout.c instead of the passed parameter
rolloutcontext rcRollout = {
    {/* player 0/1 cube decision */
     {TRUE, 2, TRUE, TRUE, 0.0},
     {TRUE, 2, TRUE, TRUE, 0.0}},
    {/* player 0/1 chequerplay */
     {TRUE, 0, TRUE, TRUE, 0.0},
     {TRUE, 0, TRUE, TRUE, 0.0}},

    {/* player 0/1 late cube decision */
     {TRUE, 2, TRUE, TRUE, 0.0},
     {TRUE, 2, TRUE, TRUE, 0.0}},
    {/* player 0/1 late chequerplay */
     {TRUE, 0, TRUE, TRUE, 0.0},
     {TRUE, 0, TRUE, TRUE, 0.0}},
    /* truncation point cube and chequerplay */
    {TRUE, 2, TRUE, TRUE, 0.0},
    {TRUE, 2, TRUE, TRUE, 0.0},

    /* move filters */
    {MOVEFILTER_NORMAL, MOVEFILTER_NORMAL},
    {MOVEFILTER_NORMAL, MOVEFILTER_NORMAL},

    TRUE,         /* cubeful */
    TRUE,         /* variance reduction */
    FALSE,        /* initial position */
    TRUE,         /* rotate */
    TRUE,         /* truncate at BEAROFF2 for cubeless rollouts */
    TRUE,         /* truncate at BEAROFF2_OS for cubeless rollouts */
    FALSE,        /* late evaluations */
    FALSE,        /* Truncation enabled */
    FALSE,        /* no stop on STD */
    FALSE,        /* no stop on JSD */
    FALSE,        /* no move stop on JSD */
    10,           /* truncation */
    1296,         /* number of trials */
    5,            /* late evals start here */
    RNG_ISAAC,    /* RNG */
    0,            /* seed */
    324,          /* minimum games  */
    0.01f,        /* stop when std's are lower than 0.01 */
    324,          /* minimum games  */
    2.33f,        /* stop when best has j.s.d. for 99% confidence */
    0,            /* nGamesDone */
    0.0,          /* rStoppedOnJSD */
    0,            /* nSkip */
};

extern gboolean save_autosave(gpointer UNUSED(unused))
{
    return 0;
}

extern int GetManualDice(unsigned int anDice[2])
{
    printf("STUB! GetManualDice\n");

    anDice[0] = 1;
    anDice[1] = 1;
    return 0;
}

extern void ProcessEvents(void)
{
    printf("STUB! ProcessEvents\n");
}

extern void SetRNG(rng *prng, rngcontext *rngctx, rng rngNew, char *szSeed)
{
    printf("STUB! SetRNG\n");
}

GMappedFile *g_mapped_file_new(const gchar *filename, gboolean writable, GError **error)
{
    (void)writable;

#define ret_error(e)              \
    *error = malloc(sizeof(int)); \
    *(*error) = e;                \
    return NULL;

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        ret_error(1);
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        ret_error(2);
    }

    long len = ftell(fp);
    if (len < 0) {
        fclose(fp);
        ret_error(3);
    }

    rewind(fp);

    char *buffer = (char *)malloc(len);
    if (!buffer) {
        fclose(fp);
        ret_error(4);
    }

    size_t read = fread(buffer, 1, len, fp);
    fclose(fp);

    if (read != (size_t)len) {
        free(buffer);
        ret_error(5);
    }

    GMappedFile *mapped = (GMappedFile *)malloc(sizeof(GMappedFile));
    if (!mapped) {
        free(buffer);
        ret_error(6);
    }

    mapped->data = buffer;
    mapped->size = (size_t)len;

#undef ret_error

    return mapped;
}

gchar *g_mapped_file_get_contents(GMappedFile *file)
{
    return file ? file->data : NULL;
}

void g_mapped_file_unref(GMappedFile *file)
{
    if (!file) return;
    free(file->data);
    free(file);
}

const char *gettext(const char *msgid)
{
    return msgid;
}

char *g_strdup_printf_impl(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int size = vsnprintf(NULL, 0, fmt, ap) + 1;
    va_end(ap);
    char *buf = malloc(size);
    va_start(ap, fmt);
    vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return buf;
}

GList *g_list_first(GList *list)
{
    GList *node = list;
    while (node && node->prev)
        node = node->prev;
    return node;
}

GList *g_list_append(GList *list, void *data)
{
    GList *new_node = malloc(sizeof(GList));
    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (!list)
        return new_node;

    GList *last = list;
    while (last->next)
        last = last->next;

    last->next = new_node;
    new_node->prev = last;
    return list;
}

GList *g_list_prepend(GList *list, void *data)
{
    GList *new_node = malloc(sizeof(GList));
    new_node->data = data;
    new_node->next = list;
    new_node->prev = NULL;

    if (list)
        list->prev = new_node;

    return new_node;
}

GList *g_list_nth(GList *list, unsigned int n)
{
    for (unsigned int i = 0; list && i < n; i++)
        list = list->next;
    return list;
}

void *g_list_nth_data(GList *list, unsigned int n)
{
    GList *node = g_list_nth(list, n);
    return node ? node->data : NULL;
}

int g_list_length(GList *list)
{
    int len = 0;
    while (list) {
        len++;
        list = list->next;
    }
    return len;
}

void g_list_free(GList *list)
{
    while (list) {
        GList *next = list->next;
        free(list);
        list = next;
    }
}
