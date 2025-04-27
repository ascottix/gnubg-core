/*
 * Copyright (C) 2002-2003 Joern Thyssen <jthyssen@dk.ibm.com>
 * Copyright (C) 2004-2013 the AUTHORS
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
#include "lib/gnubg-types.h" // For gamestate
#include "matchid.h"
#include "matchequity.h" // For MAXSCORE
#include "positionid.h" // For Base64

/*
 * Calculate log2 of Cube value.
 *
 * Input:
 *   n: cube value
 *
 * Returns:
 *   log(n)/log(2)
 *
 */

extern int
LogCube(int n)
{
    int i = 0;

    while (n >>= 1)
        i++;

    return i;
}

static void
SetBit(unsigned char *pc, unsigned int bitPos, int test, unsigned int iBit)
{

    const unsigned int k = bitPos / 8;
    const unsigned char rbit = (unsigned char) (0x1 << (bitPos % 8));

    unsigned char c;

    if (test & (0x1 << iBit))
        c = rbit;
    else
        c = 0;
    pc[k] = (pc[k] & (0xFF ^ rbit)) | c;

}

static void
GetBits(const unsigned char *pc, const unsigned int bitPos, const unsigned int nBits, int *piContent)
{
    unsigned int i, j;
    unsigned char c[2];

    c[0] = 0;
    c[1] = 0;

    for (i = 0, j = bitPos; i < nBits; i++, j++) {

        unsigned int k, r;

        k = j / 8;
        r = j % 8;

        SetBit(c, i, pc[k], r);

    }

    *piContent = c[0] | (c[1] << 8);
}

static int
MatchFromKey(int anDice[2],
             int *pfTurn,
             int *pfResigned,
             int *pfDoubled,
             int *pfMove, int *pfCubeOwner, int *pfCrawford, int *pnMatchTo, int anScore[2], int *pnCube,
             int *pfJacoby, gamestate * pgs, const unsigned char *auchKey)
{
    int temp;
    GetBits(auchKey, 0, 4, pnCube);
    *pnCube = 0x1 << *pnCube;

    GetBits(auchKey, 4, 2, pfCubeOwner);
    if (*pfCubeOwner && *pfCubeOwner != 1)
        *pfCubeOwner = -1;

    GetBits(auchKey, 6, 1, pfMove);
    GetBits(auchKey, 7, 1, pfCrawford);
    GetBits(auchKey, 8, 3, &temp);
    *pgs = (gamestate) temp;
    GetBits(auchKey, 11, 1, pfTurn);
    GetBits(auchKey, 12, 1, pfDoubled);
    GetBits(auchKey, 13, 2, pfResigned);
    GetBits(auchKey, 15, 3, &anDice[0]);
    GetBits(auchKey, 18, 3, &anDice[1]);
    GetBits(auchKey, 21, 15, pnMatchTo);
    GetBits(auchKey, 36, 15, &anScore[0]);
    GetBits(auchKey, 51, 15, &anScore[1]);
    GetBits(auchKey, 66, 1, pfJacoby);
    *pfJacoby = !(*pfJacoby);

    /* FIXME: implement a consistency check */

    if (anDice[0] < 0 || anDice[0] > 6)
        return -1;
    if (anDice[1] < 0 || anDice[1] > 6)
        return -1;
    if (*pnMatchTo < 0 || *pnMatchTo > MAXSCORE)
        return -1;

    if (*pnMatchTo) {
        if (anScore[0] < 0 || anScore[0] > *pnMatchTo)
            return -1;
        if (anScore[1] < 0 || anScore[1] > *pnMatchTo)
            return -1;
    } else {
        /* money game */
        if (*pfCrawford)
            /* no Crawford game in money play */
            return -1;
    }

    return 0;

}

extern int
MatchFromID(unsigned int anDice[2],
            int *pfTurn,
            int *pfResigned,
            int *pfDoubled, int *pfMove, int *pfCubeOwner, int *pfCrawford, int *pnMatchTo, int anScore[2], int *pnCube,
            int *pfJacoby, gamestate * pgs, const char *szMatchID)
{

    unsigned char auchKey[9];
    unsigned char *puch = auchKey;
    unsigned char ach[L_MATCHID + 1];
    unsigned char *pch = ach;
    int i;

    memset(ach, 0, sizeof(ach));
    /* decode base64 into key */
    for (i = 0; i < L_MATCHID && szMatchID[i]; i++)
        pch[i] = Base64((unsigned char) szMatchID[i]);

    for (i = 0; i < 3; i++) {
        *puch++ = (unsigned char) (pch[0] << 2) | (pch[1] >> 4);
        *puch++ = (unsigned char) (pch[1] << 4) | (pch[2] >> 2);
        *puch++ = (unsigned char) (pch[2] << 6) | pch[3];

        pch += 4;
    }

    /* get matchstate info from the key */

    return MatchFromKey((int *) anDice, pfTurn, pfResigned, pfDoubled, pfMove, pfCubeOwner, pfCrawford, pnMatchTo,
                        (int *) anScore, pnCube, pfJacoby, pgs, auchKey);
}
