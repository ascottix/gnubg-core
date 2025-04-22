/*
 * Copyright (C) 1999-2001 Gary Wong <gtw@gnu.org>
 * Copyright (C) 2000-2015 the AUTHORS
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

#ifndef DRAWBOARD_H
#define DRAWBOARD_H

#include "gnubg-types.h"

#define FORMATEDMOVESIZE 29

extern char *FormatMove(char *pch, const TanBoard anBoard, const int anMove[8]);
extern char *FormatMovePlain(char *pch, const TanBoard anBoard, const int anMove[8]);

#endif
