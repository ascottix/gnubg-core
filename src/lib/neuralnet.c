/*
 * Copyright (C) 1998-2002 Gary Wong <gtw@gnu.org>
 * Copyright (C) 2000-2019 the AUTHORS
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
#include "backgammon.h"
#include "common.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "neuralnet.h"
#include "simd.h"
#include "sigmoid.h"

static int
NeuralNetCreate(neuralnet * pnn, unsigned int cInput, unsigned int cHidden,
                unsigned int cOutput, float rBetaHidden, float rBetaOutput)
{
    pnn->cInput = cInput;
    pnn->cHidden = cHidden;
    pnn->cOutput = cOutput;
    pnn->rBetaHidden = rBetaHidden;
    pnn->rBetaOutput = rBetaOutput;
    pnn->nTrained = 0;

    if ((pnn->arHiddenWeight = sse_malloc(cHidden * cInput * sizeof(float))) == NULL)
        return -1;

    if ((pnn->arOutputWeight = sse_malloc(cOutput * cHidden * sizeof(float))) == NULL) {
        sse_free(pnn->arHiddenWeight);
        return -1;
    }

    if ((pnn->arHiddenThreshold = sse_malloc(cHidden * sizeof(float))) == NULL) {
        sse_free(pnn->arOutputWeight);
        sse_free(pnn->arHiddenWeight);
        return -1;
    }

    if ((pnn->arOutputThreshold = sse_malloc(cOutput * sizeof(float))) == NULL) {
        sse_free(pnn->arHiddenThreshold);
        sse_free(pnn->arOutputWeight);
        sse_free(pnn->arHiddenWeight);
        return -1;
    }

    return 0;
}

extern void
NeuralNetDestroy(neuralnet * pnn)
{
    sse_free(pnn->arHiddenWeight);
    pnn->arHiddenWeight = 0;
    sse_free(pnn->arOutputWeight);
    pnn->arOutputWeight = 0;
    sse_free(pnn->arHiddenThreshold);
    pnn->arHiddenThreshold = 0;
    sse_free(pnn->arOutputThreshold);
    pnn->arOutputThreshold = 0;
}

#if !defined(USE_SIMD_INSTRUCTIONS)

/* separate context for race, crashed, contact
 * -1: regular eval
 * 0: save base
 * 1: from base
 */

static inline NNEvalType
NNevalAction(NNState * pnState)
{
    if (!pnState)
        return NNEVAL_NONE;

    switch (pnState->state) {
    case NNSTATE_NONE:
        {
            /* incremental evaluation not useful */
            return NNEVAL_NONE;
        }
    case NNSTATE_INCREMENTAL:
        {
            /* next call should return FROMBASE */
            pnState->state = NNSTATE_DONE;

            /* starting a new context; save base in the hope it will be useful */
            return NNEVAL_SAVE;
        }
    case NNSTATE_DONE:
        {
            /* context hit!  use the previously computed base */
            return NNEVAL_FROMBASE;
        }
    }
    /* never reached */
    return NNEVAL_NONE;         /* for the picky compiler */
}

static void
Evaluate(const neuralnet * pnn, const float arInput[], float ar[], float arOutput[], float *saveAr)
{
    const unsigned int cHidden = pnn->cHidden;
    unsigned int i, j;
    float *prWeight;

    /* Calculate activity at hidden nodes */
    for (i = 0; i < cHidden; i++)
        ar[i] = pnn->arHiddenThreshold[i];

    prWeight = pnn->arHiddenWeight;

    for (i = 0; i < pnn->cInput; i++) {
        float const ari = arInput[i];

        if (ari == 0.0f)
            prWeight += cHidden;
        else {
            float *pr = ar;

            if (ari == 1.0f)
                for (j = cHidden; j; j--)
                    *pr++ += *prWeight++;
            else
                for (j = cHidden; j; j--)
                    *pr++ += *prWeight++ * ari;
        }
    }

    if (saveAr)
        memcpy(saveAr, ar, cHidden * sizeof(*saveAr));

    for (i = 0; i < cHidden; i++)
        ar[i] = sigmoid(-pnn->rBetaHidden * ar[i]);

    /* Calculate activity at output nodes */
    prWeight = pnn->arOutputWeight;

    for (i = 0; i < pnn->cOutput; i++) {
        float r = pnn->arOutputThreshold[i];

        for (j = 0; j < cHidden; j++)
            r += ar[j] * *prWeight++;

        arOutput[i] = sigmoid(-pnn->rBetaOutput * r);
    }
}

static void
EvaluateFromBase(const neuralnet * pnn, const float arInputDif[], float ar[], float arOutput[])
{
    unsigned int i, j;
    float *prWeight;

    /* Calculate activity at hidden nodes */
    /*    for( i = 0; i < pnn->cHidden; i++ )
     * ar[ i ] = pnn->arHiddenThreshold[ i ]; */

    prWeight = pnn->arHiddenWeight;

    for (i = 0; i < pnn->cInput; ++i) {
        float const ari = arInputDif[i];

        if (ari == 0.0f)
            prWeight += pnn->cHidden;
        else {
            float *pr = ar;

            if (ari == 1.0f)
                for (j = pnn->cHidden; j; j--)
                    *pr++ += *prWeight++;
            else if (ari == -1.0f)
                for (j = pnn->cHidden; j; j--)
                    *pr++ -= *prWeight++;
            else
                for (j = pnn->cHidden; j; j--)
                    *pr++ += *prWeight++ * ari;
        }
    }

    for (i = 0; i < pnn->cHidden; i++)
        ar[i] = sigmoid(-pnn->rBetaHidden * ar[i]);

    /* Calculate activity at output nodes */
    prWeight = pnn->arOutputWeight;

    for (i = 0; i < pnn->cOutput; i++) {
        float r = pnn->arOutputThreshold[i];

        for (j = 0; j < pnn->cHidden; j++)
            r += ar[j] * *prWeight++;

        arOutput[i] = sigmoid(-pnn->rBetaOutput * r);
    }
}

extern int
NeuralNetEvaluate(const neuralnet * pnn, float arInput[], float arOutput[], NNState * pnState)
{
    float *ar = (float *) g_alloca(pnn->cHidden * sizeof(float));
    switch (NNevalAction(pnState)) {
    case NNEVAL_NONE:
        {
            Evaluate(pnn, arInput, ar, arOutput, 0);
            break;
        }
    case NNEVAL_SAVE:
        {
            pnState->cSavedIBase = pnn->cInput;
            memcpy(pnState->savedIBase, arInput, pnn->cInput * sizeof(*ar));
            Evaluate(pnn, arInput, ar, arOutput, pnState->savedBase);
            break;
        }
    case NNEVAL_FROMBASE:
        {
            if (pnState->cSavedIBase != pnn->cInput) {
                Evaluate(pnn, arInput, ar, arOutput, 0);
                break;
            }
            memcpy(ar, pnState->savedBase, pnn->cHidden * sizeof(*ar));

            {
                float *r = arInput;
                float *s = pnState->savedIBase;
                unsigned int i;

                for (i = 0; i < pnn->cInput; ++i, ++r, ++s) {
                    if (*r != *s /*lint --e(777) */ ) {
                        *r -= *s;
                    } else {
                        *r = 0.0;
                    }
                }
            }
            EvaluateFromBase(pnn, arInput, ar, arOutput);
            break;
        }
    }
    return 0;
}
#endif

extern int
NeuralNetLoad(neuralnet * pnn, FILE * pf)
{

    unsigned int i;
    float *pr;
    char dummy[16];

    if (fscanf(pf, "%u %u %u %15s %f %f\n", &pnn->cInput, &pnn->cHidden,
               &pnn->cOutput, dummy, &pnn->rBetaHidden,
               &pnn->rBetaOutput) < 5 || pnn->cInput < 1 ||
        pnn->cHidden < 1 || pnn->cOutput < 1 || pnn->rBetaHidden <= 0.0f || pnn->rBetaOutput <= 0.0f) {
        errno = EINVAL;
        return -1;
    }

    if (NeuralNetCreate(pnn, pnn->cInput, pnn->cHidden, pnn->cOutput, pnn->rBetaHidden, pnn->rBetaOutput))
        return -1;

    pnn->nTrained = 1;

    for (i = pnn->cInput * pnn->cHidden, pr = pnn->arHiddenWeight; i; i--)
        if (fscanf(pf, "%f\n", pr++) < 1)
            return -1;

    for (i = pnn->cHidden * pnn->cOutput, pr = pnn->arOutputWeight; i; i--)
        if (fscanf(pf, "%f\n", pr++) < 1)
            return -1;

    for (i = pnn->cHidden, pr = pnn->arHiddenThreshold; i; i--)
        if (fscanf(pf, "%f\n", pr++) < 1)
            return -1;

    for (i = pnn->cOutput, pr = pnn->arOutputThreshold; i; i--)
        if (fscanf(pf, "%f\n", pr++) < 1)
            return -1;

    return 0;
}

extern int
NeuralNetLoadBinary(neuralnet * pnn, FILE * pf)
{

    int dummy;

#define FREAD( p, c ) \
    if ( fread( (p), sizeof( *(p) ), (c), pf ) < (unsigned int)(c) ) return -1

    FREAD(&pnn->cInput, 1);
    FREAD(&pnn->cHidden, 1);
    FREAD(&pnn->cOutput, 1);
    FREAD(&dummy, 1);
    FREAD(&pnn->rBetaHidden, 1);
    FREAD(&pnn->rBetaOutput, 1);

    if (pnn->cInput < 1 || pnn->cHidden < 1 || pnn->cOutput < 1 || pnn->rBetaHidden <= 0.0f || pnn->rBetaOutput <= 0.0f) {
        errno = EINVAL;
        return -1;
    }

    if (NeuralNetCreate(pnn, pnn->cInput, pnn->cHidden, pnn->cOutput, pnn->rBetaHidden, pnn->rBetaOutput))
        return -1;

    pnn->nTrained = 1;

    FREAD(pnn->arHiddenWeight, pnn->cInput * pnn->cHidden);
    FREAD(pnn->arOutputWeight, pnn->cHidden * pnn->cOutput);
    FREAD(pnn->arHiddenThreshold, pnn->cHidden);
    FREAD(pnn->arOutputThreshold, pnn->cOutput);
#undef FREAD

    return 0;
}

extern int
NeuralNetSaveBinary(const neuralnet * pnn, FILE * pf)
{

#define FWRITE( p, c ) \
    if ( fwrite( (p), sizeof( *(p) ), (c), pf ) < (unsigned int)(c) ) return -1

    FWRITE(&pnn->cInput, 1);
    FWRITE(&pnn->cHidden, 1);
    FWRITE(&pnn->cOutput, 1);
    FWRITE(&pnn->nTrained, 1);
    FWRITE(&pnn->rBetaHidden, 1);
    FWRITE(&pnn->rBetaOutput, 1);

    FWRITE(pnn->arHiddenWeight, pnn->cInput * pnn->cHidden);
    FWRITE(pnn->arOutputWeight, pnn->cHidden * pnn->cOutput);
    FWRITE(pnn->arHiddenThreshold, pnn->cHidden);
    FWRITE(pnn->arOutputThreshold, pnn->cOutput);
#undef FWRITE

    return 0;
}
