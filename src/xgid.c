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
#include "xgid.h"
#include "matchequity.h" // For MAXSCORE
#include "movefilters.inc"
#include "positionid.h"
#include <string.h>

/**
 * Parses an XGID string into a matchstate. Does NOT verify that the XGID is valid.
 */
int parseXgid(matchstate *pms, const char *xgid)
{
    memset(pms, 0, sizeof(matchstate));

    // Check the prefix
    if (strstr(xgid, "XGID=") != xgid) {
        return -1;
    }

    // Get the board
    if (PositionFromXG(pms->anBoard, xgid + 5) != 0) {
        return -2;
    }

    // Separate the tokens
    char *s = strdup(xgid);

    if (s == NULL) {
        return -3;
    }

    const char *v[9]; // Tokens

    char *c = s;
    for (int i = 0; i < 9 && (c = strchr(c, ':')); i++) {
        *c++ = '\0';
        v[i] = c;
    }

    // Setup the match state
    pms->nCube = 1 << atoi(v[0]);

    switch (atoi(v[1])) {
    case 1:
        pms->fCubeOwner = 1;
        break;
    case -1:
        pms->fCubeOwner = 0;
        break;
    default:
        pms->fCubeOwner = -1;
        break;
    }

    pms->fMove = atoi(v[2]) > 0 ? 1 : 0;

    switch (v[3][0]) {
    case 'D':
        pms->fTurn = !pms->fMove;
        pms->fDoubled = 1;
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
        pms->fTurn = pms->fMove;
        pms->anDice[0] = v[3][0] - '0';
        pms->anDice[1] = v[3][1] - '0';
        break;
    default:
        free(s);
        return -4; // Unsupported
    }

    pms->anScore[0] = atoi(v[4]);
    pms->anScore[1] = atoi(v[5]);

    pms->nMatchTo = atoi(v[7]);

    int nRules = atoi(v[6]);

    if (pms->nMatchTo > 0) {
        pms->fCrawford = nRules != 0 ? 1 : 0;
    } else {
        pms->fJacoby = nRules & 1;
    }

    pms->fPostCrawford = !pms->fCrawford && (pms->anScore[0] == pms->nMatchTo - 1 || pms->anScore[1] == pms->nMatchTo - 1);
    pms->fCubeUse = atoi(v[8]) > 0 ? TRUE : FALSE;
    pms->bgv = VARIATION_STANDARD;
    pms->gs = GAME_PLAYING;

    free(s);

    // Check boundaries
    if (pms->nCube > MAX_CUBE || pms->nMatchTo > MAXSCORE) {
        return -5;
    }

    // Board is always relative to the player on move
    if (!pms->fMove) {
        SwapSides(pms->anBoard);
    }

    return 0;
}

int getCubeInfoFromMatchStateWithBeavers(cubeinfo *pci, const matchstate *pms, int nBeavers)
{
    return SetCubeInfo(
        pci,
        pms->nCube,
        pms->fCubeOwner,
        pms->fMove,
        pms->nMatchTo,
        pms->anScore,
        pms->fCrawford,
        pms->fJacoby,
        nBeavers,
        pms->bgv);
}

int getCubeInfoFromMatchState(cubeinfo *pci, const matchstate *pms)
{
    return getCubeInfoFromMatchStateWithBeavers(pci, pms, 3);
}

int evaluatePosition(const matchstate *pms, int nPlies)
{
    cubeinfo ci;

    int res = getCubeInfoFromMatchState(&ci, pms);

    if (res < 0) return res;

    evalcontext ec;

    ec.fCubeful = pms->fCubeUse;
    ec.nPlies = (unsigned) nPlies; // 0 to 3 are sensible values
    ec.fUsePrune = FALSE;
    ec.fDeterministic = TRUE;
    ec.rNoise = 0.0f; // No noise

    float arOutput[NUM_OUTPUTS];

    res = EvaluatePosition(NULL, pms->anBoard, arOutput, &ci, &ec);

    if (res < 0) return res;

    positionclass pc = ClassifyPosition(pms->anBoard, ci.bgv);

    printf("Eval plies=%d, cubeful=%d, pos. type=%d\n", nPlies, ec.fCubeful, pc);

    unsigned int anPips[2];

    PipCount(pms->anBoard, anPips);

    printf("Pip counts: %u, %u\n", anPips[0], anPips[1]);

    printf(
        "Win=%f, WinGammon=%f, WinBackGammon=%f, LoseGammon=%f, LoseBackGammon=%f\n",
        arOutput[OUTPUT_WIN],
        arOutput[OUTPUT_WINGAMMON],
        arOutput[OUTPUT_WINBACKGAMMON],
        arOutput[OUTPUT_LOSEGAMMON],
        arOutput[OUTPUT_LOSEBACKGAMMON]);

    return res;
}

int evaluatePositionXgid(const char *xgid, int nPlies)
{
    matchstate ms;

    int res = parseXgid(&ms, "XGID=-b----E-C---eE---c-e----B-:0:0:1:35:3:7:0:0:10");

    if (res < 0) return res;

    return evaluatePosition(&ms, nPlies);
}

int findBestMoves(movelist *pml, const matchstate *pms, int nPlies)
{
    // Get dice values
    int nDice0 = pms->anDice[0];
    int nDice1 = pms->anDice[1];

    if (!nDice0 || !nDice1) {
        return -1;
    }

    // Get the cube information
    cubeinfo ci;

    int res = getCubeInfoFromMatchState(&ci, pms);

    if (res < 0) return res;

    // Prepare the evaluation context
    evalcontext ec;

    ec.fCubeful = pms->fCubeUse;
    ec.nPlies = (unsigned) nPlies;
    ec.fUsePrune = FALSE;
    ec.fDeterministic = TRUE;
    ec.rNoise = 0.0f; // No noise

    movefilter aamf[MAX_FILTER_PLIES][MAX_FILTER_PLIES] = MOVEFILTER_NORMAL;
    // MOVEFILTER_TINY, MOVEFILTER_NARROW, MOVEFILTER_NORMAL, MOVEFILTER_LARGE, MOVEFILTER_HUGE

    // Search for the best moves
    res = FindnSaveBestMoves(pml, nDice0, nDice1, pms->anBoard, NULL, 0.0f, &ci, &ec, aamf);

    return res;
}

int findCubeDecision(cubedecision *pcd, float arEquity[NUM_CUBEFUL_OUTPUTS], const matchstate *pms, int nPlies)
{
    float aarOutput[2][NUM_ROLLOUT_OUTPUTS];
    float aarStdDev[2][NUM_ROLLOUT_OUTPUTS];
    rolloutstat aarsStatistics[2][2];
    cubeinfo ci;
    evalsetup esSupremo = {
        EVAL_EVAL,                  // evaltype
        {TRUE, nPlies, TRUE, TRUE, 0.0}, // evalcontext
        { // rolloutcontext
            {
                {FALSE, 2, TRUE, TRUE, 0.0}, // player 0 cube decision
                {FALSE, 2, TRUE, TRUE, 0.0}  // player 1 cube decision
            },
            {
                {FALSE, 0, TRUE, TRUE, 0.0}, // player 0 chequerplay
                {FALSE, 0, TRUE, TRUE, 0.0}  // player 1 chequerplay
            },
            {
                {FALSE, 2, TRUE, TRUE, 0.0}, // p 0 late cube decision
                {FALSE, 2, TRUE, TRUE, 0.0}  // p 1 late cube decision
            },
            {
                {FALSE, 0, TRUE, TRUE, 0.0}, // p 0 late chequerplay
                {FALSE, 0, TRUE, TRUE, 0.0}  // p 1 late chequerplay
            },
            {FALSE, 2, TRUE, TRUE, 0.0}, // truncate cube decision
            {FALSE, 2, TRUE, TRUE, 0.0}, // truncate chequerplay
            {MOVEFILTER_NORMAL, MOVEFILTER_NORMAL},
            {MOVEFILTER_NORMAL, MOVEFILTER_NORMAL},
            FALSE,        // cubeful
            TRUE,         // variance reduction
            FALSE,        // initial position
            TRUE,         // rotate
            TRUE,         // truncate at BEAROFF2 for cubeless rollouts
            TRUE,         // truncate at BEAROFF2_OS for cubeless rollouts
            FALSE,        // late evaluations
            TRUE,         // Truncation enabled
            FALSE,        // no stop on STD
            FALSE,        // no stop on JSD
            FALSE,        // no move stop on JSD
            10,           // truncation
            1296,         // number of trials
            5,            // late evals start here
            RNG_ISAAC,    // RNG
            0,            // seed
            324,          // minimum games
            0.01f,        // stop when std's are lower than 0.01
            324,          // minimum games
            2.33f,        // stop when best has j.s.d. for 99% confidence
            0,
            0.0,
            0}};

    getCubeInfoFromMatchState(&ci, pms);

    // Offer cube or roll
    int res = GeneralCubeDecision(
        aarOutput,
        aarStdDev,
        aarsStatistics,
        pms->anBoard,
        &ci,
        &esSupremo,
        NULL, NULL);

    // TODO: it seems rollout parameters are happily ignored in favor of using the global rcRollout

    if (res < 0) return res;

    *pcd = FindCubeDecision(arEquity, aarOutput, &ci);

    return 0;
}

int findBestAction(PlayerActionInfo *ppai, const char *xgid, int nPlies)
{
    matchstate ms;

    int res = parseXgid(&ms, xgid);

    if (res < 0) {
        printf("parseXgid() error: %d\n", res);
        return res;
    }

    if (ms.fResigned) {
        // TODO
        ppai->action = ActionRejectResignation;
    } else if (ms.anDice[0] == 0) {
        // No dice, either the opponent has doubled or we must decide whether to double or roll
        res = findCubeDecision(&ppai->data.cube.cd, ppai->data.cube.arEquity, &ms, nPlies);

        if (res < 0) {
            printf("findCubeDecision() error: %d\n", res);
            return res;
        }

        ppai->action = getActionFromCubeDecision(ppai->data.cube.cd, &ms);
    } else {
        // Pick the best move
        movelist ml;

        res = findBestMoves(&ml, &ms, nPlies);

        if (res < 0) {
            printf("findBestMoves() error: %d\n", res);
            return res;
        }

        const movelist *pml = &ml;
        unsigned cMoves = pml->cMoves;

        if(cMoves > MaxPlayerMoves) cMoves = MaxPlayerMoves; // Max moves to output

        ppai->action = ActionMove;
        ppai->data.move.cMoves = cMoves;

        for (unsigned i = 0; i < cMoves; i++) {
            move *m = pml->amMoves + i;
            PlayerMove *pm = ppai->data.move.list + i;

            pm->rEquity = m->rScore;
            pm->rCubelessEquity = m->rScore2;

            memcpy(pm->anMove, m->anMove, sizeof(pm->anMove));
            memcpy(pm->arEval, m->arEvalMove, sizeof(pm->arEval));
            memcpy(pm->arEvalStdDev, m->arEvalStdDev, sizeof(pm->arEvalStdDev));
        }
    }

    return 0;
}

PlayerAction getActionFromCubeDecision(cubedecision cd, const matchstate *pms)
{
    if (pms->fDoubled) {
        // Opponent has doubled
        switch (cd) {
        case DOUBLE_TAKE:
        case NODOUBLE_TAKE:
        case TOOGOOD_TAKE:
        case REDOUBLE_TAKE:
        case NO_REDOUBLE_TAKE:
        case TOOGOODRE_TAKE:
        case NODOUBLE_DEADCUBE:
        case NO_REDOUBLE_DEADCUBE:
        case OPTIONAL_DOUBLE_TAKE:
        case OPTIONAL_REDOUBLE_TAKE:
            return ActionTake;

        case DOUBLE_PASS:
        case TOOGOOD_PASS:
        case REDOUBLE_PASS:
        case TOOGOODRE_PASS:
        case OPTIONAL_DOUBLE_PASS:
        case OPTIONAL_REDOUBLE_PASS:
            return ActionDrop;

        case NODOUBLE_BEAVER:
        case DOUBLE_BEAVER:
        case NO_REDOUBLE_BEAVER:
        case OPTIONAL_DOUBLE_BEAVER:
            return ActionBeaver;

        default:
            return ActionTake;
        }
    } else {
        // Decide if double or not
        switch (cd) {
        case DOUBLE_TAKE:
        case DOUBLE_PASS:
        case DOUBLE_BEAVER:
        case REDOUBLE_TAKE:
        case REDOUBLE_PASS:
            return ActionDouble;

        case NODOUBLE_TAKE:
        case TOOGOOD_TAKE:
        case NO_REDOUBLE_TAKE:
        case TOOGOODRE_TAKE:
        case TOOGOOD_PASS:
        case TOOGOODRE_PASS:
        case NODOUBLE_BEAVER:
        case NO_REDOUBLE_BEAVER:
        case NODOUBLE_DEADCUBE:
        case NO_REDOUBLE_DEADCUBE:
            return ActionRoll;

        case OPTIONAL_DOUBLE_BEAVER:
        case OPTIONAL_DOUBLE_TAKE:
        case OPTIONAL_REDOUBLE_TAKE:
        case OPTIONAL_DOUBLE_PASS:
        case OPTIONAL_REDOUBLE_PASS:
            return ActionRoll;

        default:
            return ActionRoll;
        }
    }

    return ActionUnknown; // Should never reach here
}
