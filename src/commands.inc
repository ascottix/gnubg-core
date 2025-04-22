/* All the commands are in this file */

#define fake(name) command name [] = { { NULL, NULL, NULL, NULL, NULL } };

fake(acSetPlayer)
fake(acSetRNG)
fake(acSetRollout)
fake(acSetRolloutJsd)
fake(acSetRolloutLate)
fake(acSetRolloutLatePlayer)
fake(acSetRolloutLimit)
fake(acSetRolloutPlayer)
fake(acSetTruncation)
fake(acTop)
fake(acAnnotateMove)
fake(acSetAnalysisPlayer)
fake(acSetCheatPlayer)
fake(acSetEvalParam)
fake(acSetExportParameters)
fake(acSetEvaluation)

static command cTop = { NULL, NULL, NULL, NULL, acTop };
