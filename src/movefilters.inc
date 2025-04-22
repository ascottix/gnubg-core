/*
 * Copyright (C) 2013 Philippe Michel <philippe.michel7@free.fr>
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

/* Default movefilters */

#define MOVEFILTER_TINY							\
  { { { 0,  5, 0.08f }, {  0, 0, 0 }, { 0, 0, 0     }, {  0, 0, 0 } },	\
    { { 0,  5, 0.08f }, { -1, 0, 0 }, { 0, 0, 0     }, {  0, 0, 0 } },	\
    { { 0,  5, 0.08f }, { -1, 0, 0 }, { 0, 2, 0.02f }, {  0, 0, 0 } },  \
    { { 0,  5, 0.08f }, { -1, 0, 0 }, { 0, 2, 0.02f }, { -1, 0, 0 } } }

#define MOVEFILTER_NARROW					        \
  { { { 0,  8, 0.12f }, {  0, 0, 0 }, { 0, 0, 0     }, {  0, 0, 0 } },  \
    { { 0,  8, 0.12f }, { -1, 0, 0 }, { 0, 0, 0     }, {  0, 0, 0 } },  \
    { { 0,  8, 0.12f }, { -1, 0, 0 }, { 0, 2, 0.03f }, {  0, 0, 0 } },  \
    { { 0,  8, 0.12f }, { -1, 0, 0 }, { 0, 2, 0.03f }, { -1, 0, 0 } } }

#define MOVEFILTER_NORMAL					        \
  { { { 0,  8, 0.16f }, {  0, 0, 0 }, { 0, 0, 0     }, {  0, 0, 0 } },  \
    { { 0,  8, 0.16f }, { -1, 0, 0 }, { 0, 0, 0     }, {  0, 0, 0 } },  \
    { { 0,  8, 0.16f }, { -1, 0, 0 }, { 0, 2, 0.04f }, {  0, 0, 0 } },  \
    { { 0,  8, 0.16f }, { -1, 0, 0 }, { 0, 2, 0.04f }, { -1, 0, 0 } } }

#define MOVEFILTER_LARGE					        \
  { { { 0, 16, 0.32f }, {  0, 0, 0 }, { 0, 0, 0     }, {  0, 0, 0 } },  \
    { { 0, 16, 0.32f }, { -1, 0, 0 }, { 0, 0, 0     }, {  0, 0, 0 } },  \
    { { 0, 16, 0.32f }, { -1, 0, 0 }, { 0, 4, 0.08f }, {  0, 0, 0 } },  \
    { { 0, 16, 0.32f }, { -1, 0, 0 }, { 0, 4, 0.08f }, { -1, 0, 0 } } }

#define MOVEFILTER_HUGE							\
  { { { 0, 20, 0.44f }, {  0, 0, 0 }, { 0, 0, 0     }, {  0, 0, 0 } },  \
    { { 0, 20, 0.44f }, { -1, 0, 0 }, { 0, 0, 0     }, {  0, 0, 0 } },  \
    { { 0, 20, 0.44f }, { -1, 0, 0 }, { 0, 6, 0.11f }, {  0, 0, 0 } },  \
    { { 0, 20, 0.44f }, { -1, 0, 0 }, { 0, 6, 0.11f }, { -1, 0, 0 } } }

