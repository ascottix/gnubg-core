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
#ifndef XGID_H
#define XGID_H

#include "backgammon.h"

typedef enum {
    ActionUnknown,
    ActionMove,
    ActionRoll,
    ActionDouble,
    ActionTake,
    ActionDrop,
    ActionAcceptResignation,
    ActionRejectResignation,
    ActionBeaver
} PlayerAction;

typedef struct {
    int anMove[8];
    float rEquity;
    float rCubelessEquity;
    float arEval[NUM_ROLLOUT_OUTPUTS];
    float arEvalStdDev[NUM_ROLLOUT_OUTPUTS];
} PlayerMove;

#define MaxPlayerMoves 40

typedef struct {
    cubedecision cd;
    float arEquity[NUM_CUBEFUL_OUTPUTS];
    float aarOutput[2][NUM_ROLLOUT_OUTPUTS];
} PlayerActionDataCube;

typedef struct {
    unsigned cMoves;
    PlayerMove list[MaxPlayerMoves];
} PlayerActionDataMove;

typedef struct {
    PlayerAction action;
    union {
        PlayerActionDataCube cube;
        PlayerActionDataMove move;
    } data;
} PlayerActionInfo;

extern int parseXgid(matchstate *pms, const char *xgid);
extern int getCubeInfoFromMatchState(cubeinfo *pci, const matchstate *pms);
extern int getCubeInfoFromMatchStateWithBeavers(cubeinfo *pci, const matchstate *pms, int nBeavers);
extern int evaluatePosition(const matchstate *pms, int nPlies);
extern int evaluatePositionXgid(const char *xgid, int nPlies);
extern int findBestMoves(movelist *pml, const matchstate *pms, int nPlies);
extern int findCubeDecision(cubedecision *pcd, float arEquity[NUM_CUBEFUL_OUTPUTS], float aarOutput[2][NUM_ROLLOUT_OUTPUTS], const matchstate *pms, int nPlies);

extern PlayerAction getActionFromCubeDecision(cubedecision cd, const matchstate *pms);

int findBestAction(PlayerActionInfo *ppai, const char *xgid, int nPlies);

#endif // XGID_H