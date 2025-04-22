/*
 * Copyright (C) 1999-2002 Gary Wong <gtw@gnu.org>
 * Copyright (C) 2000-2020 the AUTHORS
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
#include "drawboard.h"

#include "glib_shim.h"

static char *
FormatPoint(char *pch, int n)
{

    g_assert(n >= 0);

    /*don't translate 'off' and 'bar' as these may be used in UserCommand at a later
     * point */
    if (!n) {
        strcpy(pch, ("off"));
        return pch + strlen(("off"));
    } else if (n == 25) {
        strcpy(pch, ("bar"));
        return pch + strlen(("bar"));
    } else if (n > 9)
        *pch++ = (char) ((n / 10) + '0');

    *pch++ = (char) ((n % 10) + '0');

    return pch;
}

static char *
FormatPointPlain(char *pch, int n)
{

    g_assert(n >= 0);

    if (n > 9)
        *pch++ = (char) ((n / 10) + '0');

    *pch++ = (char) ((n % 10) + '0');

    return pch;
}

extern char *
FormatMovePlain(char *sz, const TanBoard anBoard, const int anMove[8])
{

    char *pch = sz;
    int i, j;

    for (i = 0; i < 8 && anMove[i] >= 0; i += 2) {
        pch = FormatPointPlain(pch, anMove[i] + 1);
        *pch++ = '/';
        pch = FormatPointPlain(pch, anMove[i + 1] + 1);

        if (anBoard && anMove[i + 1] >= 0 && anBoard[0][23 - anMove[i + 1]]) {
            for (j = 1;; j += 2)
                if (j > i) {
                    *pch++ = '*';
                    break;
                } else if (anMove[i + 1] == anMove[j])
                    break;
        }

        if (i < 6)
            *pch++ = ' ';
    }

    *pch = 0;

    return sz;
}

static int
CompareMovesSimple(const void *p0, const void *p1)
{

    int n0 = *((const int *) p0), n1 = *((const int *) p1);

    if (n0 != n1)
        return n1 - n0;
    else
        return *((const int *) p1 + 1) - *((const int *) p0 + 1);
}

extern void
CanonicalMoveOrder(int an[])
{

    int i;

    for (i = 0; i < 4 && an[2 * i] > -1; i++);

    qsort(an, i, sizeof(int) << 1, CompareMovesSimple);
}

extern char *
FormatMove(char *sz, const TanBoard anBoard, const int anMove[8])
{

    char *pch = sz;
    int aanMove[4][4], *pnSource[4], *pnDest[4], i, j;
    int fl = 0;
    int anCount[4], nMoves, nDuplicate, k;

    /* Re-order moves into 2-dimensional array. */
    for (i = 0; i < 4 && anMove[i << 1] >= 0; i++) {
        aanMove[i][0] = anMove[i << 1] + 1;
        aanMove[i][1] = anMove[(i << 1) | 1] + 1;
        pnSource[i] = aanMove[i];
        pnDest[i] = aanMove[i] + 1;
    }

    while (i < 4) {
        aanMove[i][0] = aanMove[i][1] = -1;
        pnSource[i++] = NULL;
    }

    /* Order the moves in decreasing order of source point. */
    qsort(aanMove, 4, 4 * sizeof(int), CompareMovesSimple);

    /* Combine moves of a single chequer. */
    for (i = 0; i < 4; i++)
        for (j = i; j < 4; j++)
            if (pnSource[i] && pnSource[j] && *pnDest[i] == *pnSource[j]) {
                if (anBoard[0][24 - *pnDest[i]])
                    /* Hitting blot; record intermediate point. */
                    *++pnDest[i] = *pnDest[j];
                else
                    /* Non-hit; elide intermediate point. */
                    *pnDest[i] = *pnDest[j];

                pnSource[j] = NULL;
            }

    /* Compact array. */
    i = 0;

    for (j = 0; j < 4; j++)
        if (pnSource[j]) {
            if (j > i) {
                pnSource[i] = pnSource[j];
                pnDest[i] = pnDest[j];
            }

            i++;
        }

    while (i < 4)
        pnSource[i++] = NULL;

    for (i = 0; i < 4; i++)
        anCount[i] = pnSource[i] ? 1 : 0;

    for (i = 0; i < 3; i++) {
        if (pnSource[i]) {
            nMoves = (int) (pnDest[i] - pnSource[i]);
            for (j = i + 1; j < 4; j++) {
                if (pnSource[j]) {
                    nDuplicate = 1;

                    if (pnDest[j] - pnSource[j] != nMoves)
                        nDuplicate = 0;
                    else
                        for (k = 0; k <= nMoves && nDuplicate; k++) {
                            if (pnSource[i][k] != pnSource[j][k])
                                nDuplicate = 0;
                        }
                    if (nDuplicate) {
                        anCount[i]++;
                        pnSource[j] = NULL;
                    }
                }
            }
        }
    }

    /* Compact array. */
    i = 0;

    for (j = 0; j < 4; j++)
        if (pnSource[j]) {
            if (j > i) {
                pnSource[i] = pnSource[j];
                pnDest[i] = pnDest[j];
                anCount[i] = anCount[j];
            }

            i++;
        }

    if (i < 4)
        pnSource[i] = NULL;

    for (i = 0; i < 4 && pnSource[i]; i++) {
        if (i)
            *pch++ = ' ';

        pch = FormatPoint(pch, *pnSource[i]);

        for (j = 1; pnSource[i] + j < pnDest[i]; j++) {
            *pch = '/';
            pch = FormatPoint(pch + 1, pnSource[i][j]);
            *pch++ = '*';
            fl |= 1 << pnSource[i][j];
        }

        *pch = '/';
        pch = FormatPoint(pch + 1, *pnDest[i]);

        if (*pnDest[i] && anBoard[0][24 - *pnDest[i]] && !(fl & (1 << *pnDest[i]))) {
            *pch++ = '*';
            fl |= 1 << *pnDest[i];
        }

        if (anCount[i] > 1) {
            *pch++ = '(';
            *pch++ = (char) ('0' + anCount[i]);
            *pch++ = ')';
        }
    }

    *pch = 0;

    return sz;
}
