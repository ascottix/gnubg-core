/*
 * Copyright (C) 1999-2003 Gary Wong <gtw@gnu.org>
 * Copyright (C) 1999-2023 the AUTHORS
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

#ifndef BACKGAMMON_H
#define BACKGAMMON_H

#include "common.h"
#include "glib_shim.h"

#include "eval.h"
#include "rollout.h"

#define STRINGIZEAUX(num) #num
#define STRINGIZE(num) STRINGIZEAUX(num)

#define MAX_CUBE (1 << 12)
#define MAX_NAME_LEN 32

#define BUILD_DATE_STR STRINGIZE(BUILD_DATE)

#define VERSION_STRING "GNU Backgammon " VERSION " " BUILD_DATE_STR

typedef void (*AsyncFun)(void *);

// Stubs
extern int fOutputMWC;

extern int fAutoCrawford;
extern int fAutoSaveRollout;
extern int fShowProgress;
extern matchstate ms;
extern ConstTanBoard msBoard(void);
extern rolloutcontext rcRollout;
extern rngcontext *rngctxRollout;

extern void ProcessEvents(void);
extern void CallbackProgress(void);
extern void SetRNG(rng *prng, rngcontext *rngctx, rng rngNew, char *szSeed);
extern int GetManualDice(unsigned int anDice[2]);

#endif /* BACKGAMMON_H */
