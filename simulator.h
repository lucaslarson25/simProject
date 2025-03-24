#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "datatypes.h"
#include "sim_main.h"
#include "simtimer.h"
#include <pthread.h>

typedef enum { NO_ERR, ALL_ERR, ACC_ERR } SysErrCode;

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

typedef struct VmemType
   {
    int pid;
    int startAdr;
    int endAdr;
    int startVAdr;
    int endVAdr;
    struct VmemType *nextBlock;
   } VmemType;

void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr );

PCBtype *createPCB(PCBtype *newNode, OpCodeType **headPtr, int pid, 
                                                    ConfigDataType *configPtr);

PCBtype *addPCB( PCBtype *localPtr, PCBtype *newNode );

PCBtype *freePCBs( PCBtype *localPtr );

PCBtype *getPCB(PCBtype *readyQueueHead, ConfigDataType *configPtr );

PCBtype *runPCB( PCBtype **readyQueueHead, int pid, ConfigDataType *configPtr );

PCBtype *executePCB( PCBtype *pcb, ConfigDataType *configPtr,
            OutputString **outputBuffer, SysErrCode *sysErr, VmemType *memHeadPtr );

void *waitIO( void *ptr );

OutputString *bufferOutput( OutputString *headPtr, const char *outStr,
                                                    ConfigDataType *configPtr );

OutputString *printBuffer( OutputString *headPtr, ConfigDataType *configPtr );

SysErrCode allocateMem( int pid, int base, int offset, VmemType** memHeadPtr,
                     ConfigDataType *configPtr, OutputString **outputBuffer );

SysErrCode accessMem(int pid, int base, int offset, VmemType *memHeadPtr,
                     ConfigDataType *configPtr, OutputString **outputBuffer );

VmemType *freeMem( int pid, VmemType *memHeadPtr, ConfigDataType *configPtr,
                                                OutputString **outputBuffer );

VmemType *initMem( ConfigDataType *configPtr, OutputString **outputBuffer );

void printMem( VmemType *memHeadPtr, ConfigDataType *configPtr, 
                              OutputString **outputBuffer, const char *outStr );

#endif