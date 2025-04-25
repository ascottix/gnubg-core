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
#include "api.h"
#include "backgammon.h"
#include "drawboard.h"
#include "eval.h"
#include "matchequity.h"
#include "movefilters.inc"
#include "multithread.h"
#include "positionid.h"
#include "stringbuffer.h"
#include "xgid.h"
#include <stdio.h>

int init()
{
    // Init RNG: this is necessary because rollout.c uses global rngctxRollout and rcRollout
    if (!(rngctxRollout = InitRNG(&rcRollout.nSeed, NULL, TRUE, rcRollout.rngRollout))) {
        printf("%s\n", _("Failure setting up RNG for rollout."));
        return -1;
    }

    // WARNING: the initialization order is important
    char *met = "./data/met/Kazaross-XG2.xml";
    InitMatchEquity(met);

    char *gnubg_weights = "./data/gnubg.weights";
    char *gnubg_weights_binary = "./data/gnubg.wd";
    int fNoBearoff = FALSE;
    EvalInitialise(gnubg_weights, gnubg_weights_binary, fNoBearoff, NULL);

    MT_InitThreads();

    return 0;
}

int shutdown()
{
    MT_Close();
    EvalShutdown();
    free_rngctx(rngctxRollout);

    return 0;
}

void appendAction(StringBuffer *jb, const char *action)
{
    sbAppendf(jb, "\"action\": \"%s\"", action);
}

void appendCubeDecisionData(StringBuffer *jb, const char *action, const PlayerActionDataCube *ppadc)
{
    appendAction(jb, action);
    sbAppend(jb, ", \"data\": {");
    sbAppendf(jb, "\"cd\": %d,", ppadc->cd);
    sbAppendf(jb, "\"equity\": [%.4f, %.4f, %.4f, %.4f]}",
              ppadc->arEquity[OUTPUT_NODOUBLE],
              ppadc->arEquity[OUTPUT_TAKE],
              ppadc->arEquity[OUTPUT_DROP],
              ppadc->arEquity[OUTPUT_OPTIMAL]);
}

const char *hint(const char *xgid, int nPlies)
{
    StringBuffer jb;
    sbInit(&jb);
    sbAppend(&jb, "{");

    PlayerActionInfo pai;

    int res = findBestAction(&pai, xgid, nPlies);

    if (res < 0) {
        sbAppendf(&jb, "\"error\": %d}", res);
        return sbFinalize(&jb);
    }

    TanBoard anBoard; // Required for FormatMove

    PositionFromXG(anBoard, xgid + 5);

    switch (pai.action) {
    case ActionRoll:
        appendCubeDecisionData(&jb, "roll", &pai.data.cube);
        break;
    case ActionDouble:
        appendCubeDecisionData(&jb, "double", &pai.data.cube);
        break;
    case ActionTake:
        appendCubeDecisionData(&jb, "take", &pai.data.cube);
        break;
    case ActionDrop:
        appendCubeDecisionData(&jb, "drop", &pai.data.cube);
        break;
    case ActionAcceptResignation:
        appendAction(&jb, "accept resignation");
        break;
    case ActionRejectResignation:
        appendAction(&jb, "reject resignation");
        break;
    case ActionBeaver:
        appendAction(&jb, "beaver");
        break;
    case ActionMove:
        appendAction(&jb, "play");
        sbAppend(&jb, ", \"data\": [");
        for (unsigned i = 0; i < pai.data.move.cMoves; i++) {
            if (i > 0) sbAppend(&jb, ",");
            sbAppend(&jb, "{");

            PlayerMove *pm = pai.data.move.list + i;

            char szMove[FORMATEDMOVESIZE];
            FormatMovePlain(szMove, anBoard, pm->anMove);
            int len = strlen(szMove);
            if(len > 0 && szMove[len-1] == ' ') szMove[len-1] = 0;

            sbAppendf(&jb, "\"move\": \"%s\",", szMove);
            sbAppendf(&jb, "\"equity\": [%.4f, %.4f],", pm->rEquity, pm->rCubelessEquity);
            sbAppendf(&jb, "\"eval\": [%.4f, %.4f, %.4f, %.4f, %.4f]}",
                      pm->arEval[OUTPUT_WIN],
                      pm->arEval[OUTPUT_WINGAMMON],
                      pm->arEval[OUTPUT_WINBACKGAMMON],
                      pm->arEval[OUTPUT_LOSEGAMMON],
                      pm->arEval[OUTPUT_LOSEBACKGAMMON]);
        }
        sbAppend(&jb, "]");
        break;
    default:
        printf("unknown");
        break;
    }

    sbAppend(&jb, "}");

    return sbFinalize(&jb);
}
