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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stringbuffer.h"

void sbInit(StringBuffer *jb)
{
    jb->cap = 256;
    jb->len = 0;
    jb->buf = malloc(jb->cap);
    if (jb->buf) {
        jb->buf[0] = '\0';
    }
}

static void jsonbEnsureCapacity(StringBuffer *jb, size_t extra)
{
    size_t required = jb->len + extra + 1;
    if (required > jb->cap) {
        while (jb->cap < required) {
            jb->cap *= 2;
        }
        jb->buf = realloc(jb->buf, jb->cap);
    }
}

void sbAppend(StringBuffer *jb, const char *s)
{
    size_t slen = strlen(s);
    jsonbEnsureCapacity(jb, slen);
    memcpy(jb->buf + jb->len, s, slen + 1); // include \0
    jb->len += slen;
}

void sbAppendf(StringBuffer *jb, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed < 0) {
        va_end(args_copy);
        return;
    }

    jsonbEnsureCapacity(jb, needed);
    vsnprintf(jb->buf + jb->len, needed + 1, fmt, args_copy);
    jb->len += needed;
    va_end(args_copy);
}

const char *sbFinalize(StringBuffer *jb)
{
    return jb->buf;
}

void sbFree(StringBuffer *jb)
{
    free(jb->buf);
    jb->buf = NULL;
    jb->len = jb->cap = 0;
}
