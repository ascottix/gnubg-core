/*
 * Copyright (C) 1999-2004 Joern Thyssen <jth@gnubg.org>
 * Copyright (C) 2000-2016 the AUTHORS
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

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define GAMMONRATE 0.25

#include "backgammon.h"
#include "eval.h"
#include "matchequity.h"

// Built-in MET
#include "Kazaross-XG2-MET.inc"

/*
 * A1 (A2) is the match equity of player 1 (2)
 * Btilde is the post-crawford match equities.
 */

float aafMET[MAXSCORE][MAXSCORE];
float aafMETPostCrawford[2][MAXSCORE];

/* gammon prices (calculated once for efficiency) */

float aaaafGammonPrices[MAXCUBELEVEL][MAXSCORE][MAXSCORE][4];
float aaaafGammonPricesPostCrawford[MAXCUBELEVEL][MAXSCORE][2][4];

/*
 * Calculate area under normal distribution curve (with mean rMu and
 * std.dev rSigma) from rMax to rMin.
 *
 * Input:
 *   rMin, rMax: upper and lower integral limits
 *   rMu, rSigma: normal distribution parameters.
 *
 * Returns:
 *  area
 *
 */

static float
NormalDistArea(float rMin, float rMax, float rMu, float rSigma)
{
    float rtMin, rtMax;
    float rInt1, rInt2;

    rtMin = (rMin - rMu) / rSigma;
    rtMax = (rMax - rMu) / rSigma;

    rInt1 = (erff(rtMin / sqrtf(2)) + 1.0f) / 2.0f;
    rInt2 = (erff(rtMax / sqrtf(2)) + 1.0f) / 2.0f;

    return rInt2 - rInt1;
}

static int
GetCubePrimeValue(int i, int j, int nCubeValue)
{
    if ((i < 2 * nCubeValue) && (j >= 2 * nCubeValue))
        /* automatic double */
        return 2 * nCubeValue;
    else
        return nCubeValue;
}

/*
 * Calculate post-Crawford match equity table
 *
 * Formula taken from Zadeh, Management Science 23, 986 (1977)
 *
 * Input:
 *    rG: gammon rate for trailer
 *    rFD2: value of free drop at 1-away, 2-away
 *    rFD4: value of free drop at 1-away, 4-away
 *    iStart: score where met starts, e.g.
 *       iStart = 0 : 1-away, 1-away
 *       iStart = 15: 1-away, 16-away
 *
 *
 * Output:
 *    afMETPostCrawford: post-Crawford match equity table
 *
 */

static void
initPostCrawfordMET(float afMETPostCrawford[MAXSCORE],
                    const int iStart, const float rG, const float rFD2, const float rFD4)
{
    int i;

    /*
     * Calculate post-crawford match equities
     */
    for (i = iStart; i < MAXSCORE; i++) {
        afMETPostCrawford[i] = rG * 0.5f * ((i - 4 >= 0) ? afMETPostCrawford[i - 4] : 1.0f) + (1.0f - rG) * 0.5f * ((i - 2 >= 0) ? afMETPostCrawford[i - 2] : 1.0f);

        /*"insane post crawford equity" */
        g_assert(afMETPostCrawford[i] >= 0.0f && afMETPostCrawford[i] <= 1.0f);
        /*
         * add 1.5% at 1-away, 2-away for the free drop
         * add 0.4% at 1-away, 4-away for the free drop
         */

        if (i == 1)
            afMETPostCrawford[i] -= rFD2;
        /*"insane post crawford equity(1)" */
        g_assert(afMETPostCrawford[i] >= 0.0f && afMETPostCrawford[i] <= 1.0f);

        if (i == 3)
            afMETPostCrawford[i] -= rFD4;
        /*"insane post crawford equity(2)" */
        g_assert(afMETPostCrawford[i] >= 0.0f && afMETPostCrawford[i] <= 1.0f);
    }
}

extern int
GetPoints(float arOutput[5], const cubeinfo *pci, float arCP[2])
{
    /*
     * Input:
     * - arOutput: we need the gammon and backgammon ratios
     *   (we assume arOutput is evaluate for pci -> fMove)
     * - anScore: the current score.
     * - nMatchTo: matchlength
     * - pci: value of cube, who's turn is it
     *
     *
     * Output:
     * - arCP : cash points with live cube
     * These points are necessary for the linear
     * interpolation used in cubeless -> cubeful equity
     * transformation.
     */

    /* Match play */

    /* normalize score */

    int i = pci->nMatchTo - pci->anScore[0] - 1;
    int j = pci->nMatchTo - pci->anScore[1] - 1;

    int nCube = pci->nCube;

    float arCPLive[2][MAXCUBELEVEL];
    float arCPDead[2][MAXCUBELEVEL];
    float arG[2], arBG[2];

    float rDP, rRDP, rDTW, rDTL;

    int nDead, n, nMax, nCubeValue, k;

    float aarMETResults[2][DTLBP1 + 1];

    /* Gammon and backgammon ratio's.
     * Avoid division by zero in extreme cases. */

    if (!pci->fMove) {
        /* arOutput evaluated for player 0 */

        if (arOutput[OUTPUT_WIN] > 0.0f) {
            arG[0] = (arOutput[OUTPUT_WINGAMMON] - arOutput[OUTPUT_WINBACKGAMMON]) / arOutput[OUTPUT_WIN];
            arBG[0] = arOutput[OUTPUT_WINBACKGAMMON] / arOutput[OUTPUT_WIN];
        } else {
            arG[0] = 0.0;
            arBG[0] = 0.0;
        }

        if (arOutput[OUTPUT_WIN] < 1.0f) {
            arG[1] = (arOutput[OUTPUT_LOSEGAMMON] - arOutput[OUTPUT_LOSEBACKGAMMON]) / (1.0f - arOutput[OUTPUT_WIN]);
            arBG[1] = arOutput[OUTPUT_LOSEBACKGAMMON] / (1.0f - arOutput[OUTPUT_WIN]);
        } else {
            arG[1] = 0.0;
            arBG[1] = 0.0;
        }
    } else {
        /* arOutput evaluated for player 1 */

        if (arOutput[OUTPUT_WIN] > 0.0f) {
            arG[1] = (arOutput[OUTPUT_WINGAMMON] - arOutput[OUTPUT_WINBACKGAMMON]) / arOutput[OUTPUT_WIN];
            arBG[1] = arOutput[OUTPUT_WINBACKGAMMON] / arOutput[OUTPUT_WIN];
        } else {
            arG[1] = 0.0;
            arBG[1] = 0.0;
        }

        if (arOutput[OUTPUT_WIN] < 1.0f) {
            arG[0] = (arOutput[OUTPUT_LOSEGAMMON] - arOutput[OUTPUT_LOSEBACKGAMMON]) / (1.0f - arOutput[OUTPUT_WIN]);
            arBG[0] = arOutput[OUTPUT_LOSEBACKGAMMON] / (1.0f - arOutput[OUTPUT_WIN]);
        } else {
            arG[0] = 0.0;
            arBG[0] = 0.0;
        }
    }

    /* Find out what value the cube has when you or your
     * opponent give a dead cube. */

    nDead = nCube;
    nMax = 0;

    while ((i >= 2 * nDead) && (j >= 2 * nDead)) {
        nMax++;
        nDead *= 2;
    }

    for (nCubeValue = nDead, n = nMax; n >= 0; nCubeValue >>= 1, n--) {

        /* Calculate dead and live cube cash points.
         * See notes by me (Joern Thyssen) available from the
         * 'doc' directory.  (FIXME: write notes :-) ) */

        /* Even though it's a dead cube we take account of the opponents
         * automatic redouble. */

        /* Dead cube cash point for player 0 */

        getMEMultiple(pci->anScore[0], pci->anScore[1], pci->nMatchTo, nCubeValue, GetCubePrimeValue(i, j, nCubeValue), /* 0 */
                      GetCubePrimeValue(j, i, nCubeValue),                                                              /* 1 */
                      pci->fCrawford, aafMET, aafMETPostCrawford, aarMETResults[0], aarMETResults[1]);

        for (k = 0; k < 2; k++) {

            /* Live cube cash point for player */

            if ((i < 2 * nCubeValue) || (j < 2 * nCubeValue)) {

                rDTL = (1.0f - arG[!k] - arBG[!k]) * aarMETResults[k][k ? DTLP1 : DTLP0] + arG[!k] * aarMETResults[k][k ? DTLGP1 : DTLGP0] + arBG[!k] * aarMETResults[k][k ? DTLBP1 : DTLBP0];

                rDP = aarMETResults[k][DP];

                rDTW = (1.0f - arG[k] - arBG[k]) * aarMETResults[k][k ? DTWP1 : DTWP0] + arG[k] * aarMETResults[k][k ? DTWGP1 : DTWGP0] + arBG[k] * aarMETResults[k][k ? DTWBP1 : DTWBP0];

                arCPDead[k][n] = (rDTL - rDP) / (rDTL - rDTW);

                /* The doubled cube is going to be dead */
                arCPLive[k][n] = arCPDead[k][n];

            } else {

                /* Doubled cube is alive */

                /* redouble, pass */
                rRDP = aarMETResults[k][DTL];

                /* double, pass */
                rDP = aarMETResults[k][DP];

                /* double, take win */

                rDTW = (1.0f - arG[k] - arBG[k]) * aarMETResults[k][DTW] + arG[k] * aarMETResults[k][DTWG] + arBG[k] * aarMETResults[k][DTWB];

                arCPLive[k][n] = 1.0f - arCPLive[!k][n + 1] * (rDP - rDTW) / (rRDP - rDTW);
            }

        } /* loop k */
    }

    /* return cash point for current cube level */

    arCP[0] = arCPLive[0][0];
    arCP[1] = arCPLive[1][0];

    return 0;
}

extern float
GetDoublePointDeadCube(float arOutput[5], cubeinfo *pci)
{
    /*
     * Calculate double point for dead cubes
     */

    int player = pci->fMove;

    if (!pci->nMatchTo) {
        /* Use Rick Janowski's formulas
         * [insert proper reference here] */

        float rW, rL;

        if (arOutput[0] > 0.0f)
            rW = 1.0f + (arOutput[1] + arOutput[2]) / arOutput[0];
        else
            rW = 1.0f;

        if (arOutput[0] < 1.0f)
            rL = 1.0f + (arOutput[3] + arOutput[4]) / (1.0f - arOutput[0]);
        else
            rL = 1.0f;

        if (pci->fCubeOwner == -1 && pci->fJacoby) {

            /* centered cube */

            if (pci->fBeavers) {
                return (rL - 0.25f) / (rW + rL - 0.5f);
            } else {
                return (rL - 0.5f) / (rW + rL - 1.0f);
            }
        } else {

            /* redoubles or Jacoby rule not in effect */

            return rL / (rL + rW);
        }

    } else {
        /* Match play */

        float aarMETResults[2][DTLBP1 + 1];

        /* normalize score */

        float rG1, rBG1, rG2, rBG2, rDTW, rNDW, rDTL, rNDL;
        float rRisk, rGain;

        /* FIXME: avoid division by zero */
        if (arOutput[OUTPUT_WIN] > 0.0f) {
            rG1 = (arOutput[OUTPUT_WINGAMMON] - arOutput[OUTPUT_WINBACKGAMMON]) / arOutput[OUTPUT_WIN];
            rBG1 = arOutput[OUTPUT_WINBACKGAMMON] / arOutput[OUTPUT_WIN];
        } else {
            rG1 = 0.0;
            rBG1 = 0.0;
        }

        if (arOutput[OUTPUT_WIN] < 1.0f) {
            rG2 = (arOutput[OUTPUT_LOSEGAMMON] - arOutput[OUTPUT_LOSEBACKGAMMON]) / (1.0f - arOutput[OUTPUT_WIN]);
            rBG2 = arOutput[OUTPUT_LOSEBACKGAMMON] / (1.0f - arOutput[OUTPUT_WIN]);
        } else {
            rG2 = 0.0;
            rBG2 = 0.0;
        }

        getMEMultiple(pci->anScore[0], pci->anScore[1], pci->nMatchTo,
                      pci->nCube, -1, -1,
                      pci->fCrawford, aafMET, aafMETPostCrawford, aarMETResults[0], aarMETResults[1]);

        /* double point */

        /* double point */

        rDTW = (1.0f - rG1 - rBG1) * aarMETResults[player][DTW] + rG1 * aarMETResults[player][DTWG] + rBG1 * aarMETResults[player][DTWB];

        rNDW = (1.0f - rG1 - rBG1) * aarMETResults[player][NDW] + rG1 * aarMETResults[player][NDWG] + rBG1 * aarMETResults[player][NDWB];

        rDTL = (1.0f - rG2 - rBG2) * aarMETResults[player][DTL] + rG2 * aarMETResults[player][DTLG] + rBG2 * aarMETResults[player][DTLB];

        rNDL = (1.0f - rG2 - rBG2) * aarMETResults[player][NDL] + rG2 * aarMETResults[player][NDLG] + rBG2 * aarMETResults[player][NDLB];

        /* risk & gain */

        rRisk = rNDL - rDTL;
        rGain = rDTW - rNDW;

        return rRisk / (rRisk + rGain);
    }
}

/*
 * Extend match equity table to MAXSCORE using
 * David Montgomery's extension algorithm. The formula
 * is independent of the values for score < nMaxScore.
 *
 * Input:
 *    nMaxScore: the length of the native met
 *    aarMET: the match equity table for scores below nMaxScore
 *
 * Output:
 *    aarMET: the match equity table for scores up to MAXSCORE.
 *
 */

static void
ExtendMET(float aarMET[MAXSCORE][MAXSCORE], const int nMaxScore)
{
    static const float arStddevTable[] =
        {0, 1.24f, 1.27f, 1.47f, 1.50f, 1.60f, 1.61f, 1.66f, 1.68f, 1.70f, 1.72f, 1.77f};

    float rStddev0, rStddev1, rGames, rSigma;
    int i, j;

    /* Extend match equity table */
    for (i = nMaxScore; i < MAXSCORE; i++) {

        int nScore0 = i + 1;

        if (nScore0 > 10)
            rStddev0 = 1.77f;
        else
            rStddev0 = arStddevTable[nScore0];

        for (j = 0; j <= i; j++) {

            int nScore1 = j + 1;

            rGames = (float)(nScore0 + nScore1) / 2.0f;

            if (nScore1 > 10)
                rStddev1 = 1.77f;
            else
                rStddev1 = arStddevTable[nScore1];

            rSigma = sqrtf(rStddev0 * rStddev0 + rStddev1 * rStddev1) * sqrtf(rGames);

            if (6.0f * rSigma > (float)(nScore0 - nScore1))
                aarMET[i][j] = NormalDistArea((float)(nScore0 - nScore1), 6.0f * rSigma, 0.0f, rSigma);
            else
                aarMET[i][j] = 0.0f;
        }
    }

    /* Generate j > i part of MET */

    for (i = 0; i < MAXSCORE; i++)
        for (j = ((i < nMaxScore) ? nMaxScore : i + 1); j < MAXSCORE; j++)
            aarMET[i][j] = 1.0f - aarMET[j][i];
}

/*
 * Calculate gammon and backgammon price at the specified score
 * with specified cube.
 *
 * Input:
 *    aafMET, aafMETPostCrawford: match equity tables.
 *    nCube: value of cube
 *    fCrawford: is this the Crawford game
 *    nScore0, nScore1, nMatchTo: current score and match length.
 *
 * Output:
 *   arGammonPrice: gammon and backgammon prices.
 *
 */

static void
getGammonPrice(float arGammonPrice[4],
               const int nScore0, const int nScore1, const int nMatchTo,
               const int nCube, const int fCrawford,
               float aafMET[MAXSCORE][MAXSCORE], float aafMETPostCrawford[2][MAXSCORE])
{
    const float epsilon = 1.0E-7f;

    float rWin = getME(nScore0, nScore1, nMatchTo,
                       0, nCube, 0, fCrawford,
                       aafMET, aafMETPostCrawford);

    float rWinGammon = getME(nScore0, nScore1, nMatchTo,
                             0, 2 * nCube, 0, fCrawford,
                             aafMET, aafMETPostCrawford);

    float rWinBG = getME(nScore0, nScore1, nMatchTo,
                         0, 3 * nCube, 0, fCrawford,
                         aafMET, aafMETPostCrawford);

    float rLose = getME(nScore0, nScore1, nMatchTo,
                        0, nCube, 1, fCrawford,
                        aafMET, aafMETPostCrawford);

    float rLoseGammon = getME(nScore0, nScore1, nMatchTo,
                              0, 2 * nCube, 1, fCrawford,
                              aafMET, aafMETPostCrawford);

    float rLoseBG = getME(nScore0, nScore1, nMatchTo,
                          0, 3 * nCube, 1, fCrawford,
                          aafMET, aafMETPostCrawford);

    float rCenter = (rWin + rLose) / 2.0f;

    /* FIXME: correct numerical problems in a better way, than done
     * below. If cube is dead gammon or backgammon price might be a
     * small negative number. For example, at -2,-3 with cube on 2
     * the current code gives: 0.9090..., 0, -2.7e-8, 0 instead
     * of the correct 0.9090..., 0, 0, 0. */

    /* avoid division by zero */

    if (fabsf(rWin - rCenter) > epsilon) {

        /* this expression can be reduced to:
         * 2 * ( rWinGammon - rWin ) / ( rWin - rLose )
         * which is twice the "usual" gammon value */

        arGammonPrice[0] = (rWinGammon - rCenter) / (rWin - rCenter) - 1.0f;

        /* this expression can be reduced to:
         * 2 * ( rLose - rLoseGammon ) / ( rWin - rLose )
         * which is twice the "usual" gammon value */

        arGammonPrice[1] = (rCenter - rLoseGammon) / (rWin - rCenter) - 1.0f;

        arGammonPrice[2] = (rWinBG - rCenter) / (rWin - rCenter) - (arGammonPrice[0] + 1.0f);
        arGammonPrice[3] = (rCenter - rLoseBG) / (rWin - rCenter) - (arGammonPrice[1] + 1.0f);

    } else
        arGammonPrice[0] = arGammonPrice[1] = arGammonPrice[2] = arGammonPrice[3] = 0.0f;

    /* Correct numerical problems */
    if (arGammonPrice[0] < 0.0f)
        arGammonPrice[0] = 0.0f;
    if (arGammonPrice[1] < 0.0f)
        arGammonPrice[1] = 0.0f;
    if (arGammonPrice[2] < 0.0f)
        arGammonPrice[2] = 0.0f;
    if (arGammonPrice[3] < 0.0f)
        arGammonPrice[3] = 0.0f;
}

/*
 * Calculate all gammon and backgammon prices
 *
 * Input:
 *   aafMET, aafMETPostCrawford: match equity tables
 *
 * Output:
 *   aaaafGammonPrices: all gammon prices
 *
 */

static void
calcGammonPrices(float aafMET[MAXSCORE][MAXSCORE],
                 float aafMETPostCrawford[2][MAXSCORE], float aaaafGammonPrices[MAXCUBELEVEL][MAXSCORE][MAXSCORE][4], float aaaafGammonPricesPostCrawford[MAXCUBELEVEL][MAXSCORE][2][4])
{
    int i, j, k;
    int nCube;

    for (i = 0, nCube = 1; i < MAXCUBELEVEL; i++, nCube *= 2)
        for (j = 0; j < MAXSCORE; j++)
            for (k = 0; k < MAXSCORE; k++)
                getGammonPrice(aaaafGammonPrices[i][j][k],
                               MAXSCORE - j - 1, MAXSCORE - k - 1, MAXSCORE,
                               nCube, (MAXSCORE == j) || (MAXSCORE == k), aafMET, aafMETPostCrawford);

    for (i = 0, nCube = 1; i < MAXCUBELEVEL; i++, nCube *= 2)
        for (j = 0; j < MAXSCORE; j++) {
            getGammonPrice(aaaafGammonPricesPostCrawford[i][j][0],
                           MAXSCORE - 1, MAXSCORE - j - 1, MAXSCORE, nCube, FALSE, aafMET, aafMETPostCrawford);
            getGammonPrice(aaaafGammonPricesPostCrawford[i][j][1],
                           MAXSCORE - j - 1, MAXSCORE - 1, MAXSCORE, nCube, FALSE, aafMET, aafMETPostCrawford);
        }
}

void initMatchEquityFromTable(int nLength, const float pre[nLength][nLength], const float post[nLength])
{
    int i, j;

    /* Copy met to current met, extend met (if needed) */

    /* Note that the post Crawford table is extended from
     * n - 1 as the  post Crawford table of a n match equity table
     * might not include the post Crawford equity at n-away, since
     * the first "legal" post Crawford score is n-1. */
    for (j = 0; j < 2; j++) {
        for (i = 0; i < nLength - 1; i++) {
            aafMETPostCrawford[j][i] = post[i];
        }

        initPostCrawfordMET(aafMETPostCrawford[j], nLength - 1, GAMMONRATE, 0.015f, 0.004f);
    }

    /* pre-Crawford table */
    for (i = 0; i < nLength; i++) {
        for (j = 0; j < nLength; j++) {
            aafMET[i][j] = pre[i][j];
        }
    }

    /* Extend match equity table */
    ExtendMET(aafMET, nLength);

    /* initialise gammon prices */
    calcGammonPrices(aafMET, aafMETPostCrawford, aaaafGammonPrices, aaaafGammonPricesPostCrawford);
}

extern void
InitMatchEquity(const char *szFileName)
{
    initMatchEquityFromTable(KazarossXG2_Length, KazarossXG2_pre_crawford, KazarossXG2_post_crawford);
}

/*
 * Return match equity (mwc) assuming player fWhoWins wins nPoints points.
 *
 * If fCrawford then afMETPostCrawford is used, otherwise
 * aafMET is used.
 *
 * Input:
 *    nAway0: points player 0 needs to win
 *    nAway1: points player 1 needs to win
 *    fPlayer: get mwc for this player
 *    fCrawford: is this the Crawford game
 *    aafMET: match equity table for player 0
 *    afMETPostCrawford: post-Crawford match equity table for player 0
 *
 */

extern float
getME(const int nScore0, const int nScore1, const int nMatchTo,
      const int fPlayer,
      const int nPoints, const int fWhoWins,
      const int fCrawford, float aafMET[MAXSCORE][MAXSCORE], float aafMETPostCrawford[2][MAXSCORE])
{
    int n0 = nMatchTo - (nScore0 + (!fWhoWins) * nPoints) - 1;
    int n1 = nMatchTo - (nScore1 + fWhoWins * nPoints) - 1;

    /* check if any player has won the match */

    if (unlikely(n0 < 0))
        /* player 0 has won the game */
        return (fPlayer) ? 0.0f : 1.0f;
    else if (unlikely(n1 < 0))
        /* player 1 has won the game */
        return (fPlayer) ? 1.0f : 0.0f;

    /* the match is not finished */

    if (fCrawford || (nMatchTo - nScore0 == 1) || (nMatchTo - nScore1 == 1)) {

        /* the next game will be post-Crawford */

        if (!n0)
            /* player 0 is leading match */
            /* FIXME: use pc-MET for player 0 */
            return (fPlayer) ? aafMETPostCrawford[1][n1] : 1.0f - aafMETPostCrawford[1][n1];
        else
            /* player 1 is leading the match */
            return (fPlayer) ? 1.0f - aafMETPostCrawford[0][n0] : aafMETPostCrawford[0][n0];

    } else
        /* non-post-Crawford games */
        return (fPlayer) ? 1.0f - aafMET[n0][n1] : aafMET[n0][n1];
}

extern float
getMEAtScore(const int nScore0, const int nScore1, const int nMatchTo,
             const int fPlayer, const int fCrawford,
             float aafMET[MAXSCORE][MAXSCORE], float aafMETPostCrawford[2][MAXSCORE])
{
    int n0 = nMatchTo - nScore0 - 1;
    int n1 = nMatchTo - nScore1 - 1;

    /* check if any player has won the match */

    if (n0 < 0)
        /* player 0 has won the game */
        return (fPlayer) ? 0.0f : 1.0f;
    else if (n1 < 0)
        /* player 1 has won the game */
        return (fPlayer) ? 1.0f : 0.0f;

    /* the match is not finished */

    if (!fCrawford && ((nMatchTo - nScore0 == 1) || (nMatchTo - nScore1 == 1))) {

        /* this game is post-Crawford */

        if (!n0)
            /* player 0 is leading match */
            /* FIXME: use pc-MET for player 0 */
            return (fPlayer) ? aafMETPostCrawford[1][n1] : 1.0f - aafMETPostCrawford[1][n1];
        else
            /* player 1 is leading the match */
            return (fPlayer) ? 1.0f - aafMETPostCrawford[0][n0] : aafMETPostCrawford[0][n0];

    } else
        /* non-post-Crawford games */
        return (fPlayer) ? 1.0f - aafMET[n0][n1] : aafMET[n0][n1];
}

extern void
invertMET(void)
{
    int i, j;

    for (i = 0; i < MAXSCORE; i++) {

        /* post crawford entries */

        float r = aafMETPostCrawford[0][i];
        aafMETPostCrawford[0][i] = aafMETPostCrawford[1][i];
        aafMETPostCrawford[1][i] = r;

        /* diagonal */

        aafMET[i][i] = 1.0f - aafMET[i][i];

        /* off diagonal entries */

        for (j = 0; j < i; j++) {

            r = aafMET[i][j];
            aafMET[i][j] = 1.0f - aafMET[j][i];
            aafMET[j][i] = 1.0f - r;
        }
    }

    calcGammonPrices(aafMET, aafMETPostCrawford, aaaafGammonPrices, aaaafGammonPricesPostCrawford);
}

/* given a match score, return a pair of arrays with the METs for
 * player0 and player 1 winning/losing including gammons & backgammons
 *
 * if nCubePrime0 < 0, then we're only interested in the first
 * values in each array, using nCube.
 *
 * Otherwise, if nCubePrime0 >= 0, we do another set of METs with
 * both sides using nCubePrime0
 *
 * if nCubePrime1 >= 0, we do a third set using nCubePrime1
 *
 * FIXME ? It looks like if nCubePrime0 >= 0, nCubePrime1 is as well
 *         That could simplify the code below a little
 *
 * This reduces the *huge* number of calls to get equity table entries
 * when analyzing matches by something like 40 times
 */

extern void
getMEMultiple(const int nScore0, const int nScore1, const int nMatchTo,
              const int nCube, const int nCubePrime0, const int nCubePrime1,
              const int fCrawford, float aafMET[MAXSCORE][MAXSCORE],
              float aafMETPostCrawford[2][MAXSCORE], float *player0, float *player1)
{
    int scores[2][DTLBP1 + 1]; /* the resulting match scores */
    int i, max_res;
    int *score0, *score1;
    int mult[] = {1, 2, 3, 4, 6};
    float *p0, *p1, f;
    int away0, away1;
    int fCrawf = fCrawford;

    /* figure out how many results we'll be returning */
    max_res = (nCubePrime0 < 0) ? DTLB + 1 : (nCubePrime1 < 0) ? DTLBP0 + 1
                                                               : DTLBP1 + 1;

    /* set up a table of resulting match scores for all
     * the results we're calculating */
    score0 = scores[0];
    score1 = scores[1];
    away0 = nMatchTo - nScore0 - 1;
    away1 = nMatchTo - nScore1 - 1;
    fCrawf |= (nMatchTo - nScore0 == 1) || (nMatchTo - nScore1 == 1);

    /* player 0 wins normal, doubled, gammon, backgammon */
    for (i = 0; i < NDL; ++i) {
        *score0++ = away0 - mult[i] * nCube;
        *score1++ = away1;
    }
    /* player 1 wins normal, doubled, etc. */
    for (i = 0; i < NDL; ++i) {
        *score0++ = away0;
        *score1++ = away1 - mult[i] * nCube;
    }
    if (max_res > DPP0) {
        /* same using the second cube value */
        for (i = 0; i < NDL; ++i) {
            *score0++ = away0 - mult[i] * nCubePrime0;
            *score1++ = away1;
        }
        for (i = 0; i < NDL; ++i) {
            *score0++ = away0;
            *score1++ = away1 - mult[i] * nCubePrime0;
        }
        if (max_res > DPP1) {
            /* same using the third cube value */
            for (i = 0; i < NDL; ++i) {
                *score0++ = away0 - mult[i] * nCubePrime1;
                *score1++ = away1;
            }
            for (i = 0; i < NDL; ++i) {
                *score0++ = away0;
                *score1++ = away1 - mult[i] * nCubePrime1;
            }
        }
    }

    score0 = scores[0];
    score1 = scores[1];
    p0 = player0;
    p1 = player1;

    /* now go through the resulting scores, looking up the equities */
    for (i = 0; i < max_res; ++i) {
        int s0 = *score0++;
        int s1 = *score1++;

        if (unlikely(s0 < 0)) {
            /* player 0 wins */
            *p0++ = 1.0f;
            *p1++ = 0.0f;
        } else if (unlikely(s1 < 0)) {
            *p0++ = 0.0f;
            *p1++ = 1.0f;
        } else if (unlikely(fCrawf)) {
            if (s0 == 0) { /* player 0 is leading */
                *p0++ = 1.0f - aafMETPostCrawford[1][s1];
                *p1++ = aafMETPostCrawford[1][s1];
            } else {
                *p0++ = aafMETPostCrawford[0][s0];
                *p1++ = 1.0f - aafMETPostCrawford[0][s0];
            }
        } else { /* non-post-Crawford */
            *p0++ = aafMET[s0][s1];
            *p1++ = 1.0f - aafMET[s0][s1];
        }
    }

    /* results for player 0 are done, results for player 1 have the
     *  losses in cols 0-4 and 8-12, but we want them to be in the same
     *  order as results0 - e.g wins in cols 0-4, and 8-12
     */
    p0 = player1;
    p1 = player1 + NDL;
    for (i = 0; i < NDL; ++i) {
        f = *p0;
        *p0++ = *p1;
        *p1++ = f;
    }

    if (max_res > DTLBP0) {
        p0 += NDL;
        p1 += NDL;
        for (i = 0; i < NDL; ++i) {
            f = *p0;
            *p0++ = *p1;
            *p1++ = f;
        }
    }

    if (max_res > DTLBP1) {
        p0 += NDL;
        p1 += NDL;
        for (i = 0; i < NDL; ++i) {
            f = *p0;
            *p0++ = *p1;
            *p1++ = f;
        }
    }
}
