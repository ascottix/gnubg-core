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

void test(const char *xgid)
{
    const char *json = hint(xgid, 2);
    printf("%s\n", json);
    free((void *)json);
}

int main()
{
    init();

    printf("Hello from GnuBG Core\n");

    // evaluatePositionXgid("XGID=-b----E-C---eE---c-e----B-:0:0:-1:52:0:0:0:5:10:24/22, 13/8", 0);
    // evaluatePositionXgid("XGID=-b----E-C---eE---c-e----B-:0:0:-1:52:0:0:0:5:10:24/22, 13/8", 1);
    // printBestMoves("XGID=-B-BabC-B---bDb--c-eA--A--:0:0:1:55:1:0:0:0:10");
    // test();
    // test("XGID=aa--BBBB----dE---d-e----B-:0:0:1:00:0:0:0:0:10:=== double, pass");
    // test("XGID=aa--BBBB----dE---d-e----B-:0:0:1:D:0:0:0:0:10:=== pass (opponent has doubled)");
    // test("XGID=aBaB--C-A---dE--ac-e----B-:0:0:1:00:0:0:0:0:10:=== no double, take");
    // test("XGID=aB-B-aC-A---dE--ac-e----B-:0:0:1:00:0:0:0:0:10:=== double, take");
    // test("XGID=aa--BBBB----dE---d-e----B-:0:0:1:00:0:0:0:0:10:=== double, pass");
    // test("XGID=aBaB--C-A---dE--ac-e----B-:0:0:1:00:0:0:0:0:10:=== roll");
    // test("XGID=aBaB--C-A---dE--ac-e----B-:0:0:1:42:0:0:0:0:10:=== play");
    // test("XGID=-b----E-C---eE---c-e----B-:0:0:-1:52:0:0:0:5:10:24/22, 13/8");
    // test("XGID=-b----E-C---eE---cad----B-:0:0:1:65:0:0:0:0:10:=== play 24/18* 18/13");
    // test("XGID=--------abcba------CBABC--:0:0:1:55:0:0:0:0:10");
    test("XGID=---B-bD-C--AcC--bb-db---B-:0:0:1:22:0:4:0:7:10");

    shutdown();

    return 0;
}
