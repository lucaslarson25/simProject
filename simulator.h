#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "datatypes.h"
#include "sim_main.h"
#include "simtimer.h"
#include <pthread.h>

typedef struct PCBtype 
   {
    int pid;
    double burstT;
    ProcessState pState;
    OpCodeType *firstOp;
    struct PCBtype *nextPCB;
   } PCBtype;

typedef struct OutputString
   {
    char outStr[ MAX_STR_LEN ];
    struct OutputString *nextOutputString;
   } OutputString;

void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr );

PCBtype *createPCB(PCBtype *newNode, OpCodeType **headPtr, int pid, 
                                                    ConfigDataType *configPtr);

PCBtype *addPCB( PCBtype *localPtr, PCBtype *newNode );

PCBtype *freePCBs( PCBtype *localPtr );

PCBtype *getPCB(PCBtype *readyQueueHead, ConfigDataType *configPtr );

PCBtype *runPCB( PCBtype **readyQueueHead, int pid, ConfigDataType *configPtr );

PCBtype *executePCB( PCBtype *pcb, ConfigDataType *configPtr,
                                                OutputString **outputBuffer );

void *waitIO( void *ptr );

OutputString *bufferOutput( OutputString *headPtr, const char *outStr,
                                                    ConfigDataType *configPtr );

OutputString *printBuffer( OutputString *headPtr, ConfigDataType *configPtr );

#endif