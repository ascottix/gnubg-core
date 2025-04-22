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
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

char *BuildFilename(const char *file)
{
    char *dir = getPkgDataDir();
    int dirlen = strlen(dir);
    int filelen = strlen(file);
    char *ret = g_malloc(dirlen + filelen + 2);
    strcpy(ret, dir);
    strcat(ret, "/");
    strcat(ret, file);
    return ret;
}

void PrintError(const char *message)
{
    printf("*** Error: %s\n", message);
    fflush(stdout);
}

char *getPkgDataDir(void)
{
    return "./data";
}

const char *cubeDecisionToString(cubedecision cd)
{
    switch (cd) {
    case DOUBLE_TAKE:
        return "double, take";
    case DOUBLE_PASS:
        return "double, pass";
    case NODOUBLE_TAKE:
        return "no double, take";
    case TOOGOOD_TAKE:
        return "too good, take";
    case TOOGOOD_PASS:
        return "too good, pass";
    case DOUBLE_BEAVER:
        return "double, beaver";
    case NODOUBLE_BEAVER:
        return "no double, beaver";
    case REDOUBLE_TAKE:
        return "redouble, take";
    case REDOUBLE_PASS:
        return "redouble, pass";
    case NO_REDOUBLE_TAKE:
        return "no redouble, take";
    case TOOGOODRE_TAKE:
        return "too good redouble, take";
    case TOOGOODRE_PASS:
        return "too good redouble, pass";
    case NO_REDOUBLE_BEAVER:
        return "no redouble, beaver";
    case NODOUBLE_DEADCUBE:
        return "no double, dead cube";
    case NO_REDOUBLE_DEADCUBE:
        return "no redouble, dead cube";
    case NOT_AVAILABLE:
        return "not available";
    case OPTIONAL_DOUBLE_TAKE:
        return "optional double, take";
    case OPTIONAL_REDOUBLE_TAKE:
        return "optional redouble, take";
    case OPTIONAL_DOUBLE_BEAVER:
        return "optional double, beaver";
    case OPTIONAL_DOUBLE_PASS:
        return "optional double, pass";
    case OPTIONAL_REDOUBLE_PASS:
        return "optional redouble, pass";
    default:
        return "unknown";
    }
}
