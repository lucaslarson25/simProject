#ifndef SIM_MAIN_H
#define SIM_MAIN_H

#include "stringUtils.h"
#include "datatypes.h"
#include "configops.h"
#include "metadataops.h"
#include "simulator.h"

typedef enum { MIN_NUM_ARGS = 3, LAST_FOUR_LETTERS = 4 } PRGM_CONSTANTS;
//MORE EX DATA TYPES IN SIM STEPS ONE TWO VID

typedef struct CmdLineDataStruct
   {
    bool programRunFlag;
    bool configDisplayFlag;
    bool mdDisplayFlag;
    bool runSimFlag;

    char fileName[ STD_STR_LEN ];
   } CmdLineData;

bool proccessCmdLine( int numArgs, char **strVector, CmdLineData *clDataPtr );

void showCommandLineFormat();

void clearCmdLineStruct( CmdLineData *clDataPtr );

#endif