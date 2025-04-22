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
#ifndef STRINGBUFFER_H
#define STRINGBUFFER_H

#include <stdlib.h>

typedef struct {
    char *buf;
    size_t len;
    size_t cap;
} StringBuffer;

extern void sbInit(StringBuffer *jb);
extern void sbAppend(StringBuffer *jb, const char *s);
extern void sbAppendf(StringBuffer *jb, const char *fmt, ...);
extern const char *sbFinalize(StringBuffer *jb);
extern void sbFree(StringBuffer *jb);

#endif // STRINGBUFFER_H
