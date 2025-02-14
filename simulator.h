#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "datatypes.h"
#include "sim_main.h"
#include "simtimer.h"

typedef struct PCBtype 
   {
    int pid;
    double burstT;
    ProcessState pState;
    OpCodeType *firstOp;
    struct PCBtype *nextPCB;
   } PCBtype;


void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr );

PCBtype *createPCB(PCBtype *newNode, OpCodeType **headPtr, int pid, 
                                                    ConfigDataType *configPtr);

PCBtype *addPCB( PCBtype *localPtr, PCBtype *newNode );

void displayPCBs( PCBtype *localPtr );

PCBtype *freePCBs( PCBtype *localPtr );

PCBtype *getPCB(PCBtype *readyQueueHead, ConfigDataType *configPtr );

PCBtype *runPCB( PCBtype **readyQueueHead, int pid, ConfigDataType *configPtr );

PCBtype *executePCB( PCBtype *pcb, ConfigDataType *configPtr );


#endif