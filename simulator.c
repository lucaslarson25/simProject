#include "simulator.h"

void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr )
   {
    printf( "Simulator Run\n-------------\n\n" );

    // Initialize variables
    OpCodeType *mdPtr = metaDataMstrPtr;
    PCBtype *tempNode = NULL;
    PCBtype *readyQueue = NULL;
    int pid = 0;
    char timer [ MIN_STR_LEN ];

    
    // Set up output log

    // Start OS Sim timer
    accessTimer( ZERO_TIMER, timer);
    printf( "%s, OS: Simulator start\n", timer );

    // Create all PCBs (loop over MD LL)
    while( mdPtr != NULL )
       {
        if( compareString( mdPtr->command, "app") == STR_EQ &&
            compareString( mdPtr->strArg1, "start") == STR_EQ)
           {
            tempNode = createPCB(tempNode, &mdPtr, pid, configPtr);
            pid++;
            readyQueue = addPCB(readyQueue, tempNode);
            accessTimer( LAP_TIMER, timer);
            printf( "%s, OS: Process %i set to READY state from NEW state\n",
                                                        timer, tempNode->pid );
           }
        mdPtr = mdPtr->nextNode;
        // Calculate total time for process (app start to app end)
       }
    // Reset metadata wrkptr to the start of LL
    mdPtr = metaDataMstrPtr;
    // Set up master loop
    while( readyQueue != NULL )
       {
        // Get the next scheduled PCB
        tempNode = getPCB( readyQueue, configPtr );
        accessTimer( LAP_TIMER, timer );
        printf( "%s, OS: Process %i selected with %.0lf ms remaining\n", timer,
                                    tempNode->pid, tempNode->burstT );

        // Move from ready queue to runnning state
        tempNode = runPCB( &readyQueue, tempNode->pid, configPtr );
        accessTimer( LAP_TIMER, timer );
        printf( "%s, OS: Process %i set from READY to RUNNING\n\n",
                                                        timer, tempNode->pid );

        tempNode = executePCB( tempNode, configPtr );
        // Free the node after it is complete (terminate)
        free(tempNode);
       }

    accessTimer( LAP_TIMER, timer );
    printf( "%s, OS: System stop\n", timer );

    readyQueue = freePCBs(readyQueue);

    accessTimer( LAP_TIMER, timer );
    printf( "%s, OS: Simulation end\n", timer );
   }


// allocates and creates new PCB node with given pid
// Burst time is calculated from metadata pointer and headPtr will be updated to
// next app end
PCBtype *createPCB(PCBtype *newNode, OpCodeType **headPtr, int pid, 
                                                    ConfigDataType *configPtr)
   {
    // Allocate memory for new node
    newNode = (PCBtype*)malloc(sizeof(PCBtype));

    if( newNode == NULL )
       {
        return NULL;
       }

    // Initalize its data
    newNode->pid = pid;
    newNode->firstOp = *headPtr;
    newNode->pState = NEW_STATE;
    newNode->nextPCB = NULL;

    // Go through op codes and calculate total burst time
    int totalTime = 0;
    OpCodeType *wkgPtr = *headPtr;
    
    // Loop through metadata linked list until null ptr or "app end" opcode
    while( wkgPtr != NULL && !( compareString( wkgPtr->command, "app") == STR_EQ
        && compareString( wkgPtr->strArg1, "end" ) == STR_EQ ) )
       {

        // Check for dev command
        if( compareString( wkgPtr->command, "dev" ) == STR_EQ )
           {
            // Add cycles * ioCycleRate
            totalTime += wkgPtr->intArg2 * configPtr->ioCycleRate;
           }
        // Check for cpu command
        else if( compareString( wkgPtr->command, "cpu" ) == STR_EQ )
           {
            // Add cycles * procCycleRate
            totalTime += wkgPtr->intArg2 * configPtr->procCycleRate;
           }
        
        // Increment working pointer
        wkgPtr = wkgPtr->nextNode;
       }

    // Set new node's burst time to calculated total time
    newNode->burstT = totalTime;

    // Update headPtr to be the wkgPtr
    *headPtr = wkgPtr;

    // Return newly created PCB node
    return newNode;
   }

PCBtype *addPCB( PCBtype *localPtr, PCBtype *newNode )
   {
    PCBtype *wkgPtr = localPtr;
    newNode->pState = READY_STATE;

    if( wkgPtr == NULL )
       {
        return newNode;
       }

    while( wkgPtr->nextPCB != NULL )
       {
        wkgPtr = wkgPtr->nextPCB;
       }
    
    wkgPtr->nextPCB = newNode;
    return localPtr;
   }

void displayPCBs( PCBtype *localPtr )
   {
    while( localPtr != NULL )
       {
        printf( "PCB %i\n", localPtr->pid );
        localPtr = localPtr->nextPCB;
       }
   }

PCBtype *freePCBs( PCBtype *localPtr )
   {
    PCBtype *tempPtr;
    while( localPtr != NULL )
       {
        tempPtr = localPtr->nextPCB;
        printf("freeing %i\n", localPtr->pid );
        free( localPtr );
        localPtr = tempPtr;
       }
    return localPtr;
   }

PCBtype *getPCB(PCBtype *readyQueueHead, ConfigDataType *configPtr )
   {
    if( configPtr->cpuSchedCode == CPU_SCHED_FCFS_N_CODE )
       {
        return readyQueueHead;
       }

    configPtr->cpuSchedCode = CPU_SCHED_FCFS_N_CODE;
    return getPCB( readyQueueHead, configPtr );
   }

PCBtype *runPCB( PCBtype **readyQueueHead, int pid, ConfigDataType *configPtr )
   {
    PCBtype *foundPCB = NULL, *prev = NULL, *curr = *readyQueueHead;

    while( curr != NULL && curr->pid != pid )
       {
        prev = curr;
        curr = curr->nextPCB;
       }
    
    curr->pState = RUNNING_STATE;
    if( prev == NULL )
       {
        *readyQueueHead = curr->nextPCB;
        return curr;
       }

    foundPCB = curr;
    prev->nextPCB = curr->nextPCB;
    return foundPCB;
   }

PCBtype *executePCB( PCBtype *pcb, ConfigDataType *configPtr )
   {
    char timer [ MIN_STR_LEN ];
    int time;
    pthread_t waitThread;
    OpCodeType *opCode = pcb->firstOp->nextNode;
    while( opCode != NULL && !( compareString( opCode->command, "app") == STR_EQ
                                                                            ) )
       {

        if( compareString( opCode->command, "dev" ) == STR_EQ )
           {

            accessTimer( LAP_TIMER, timer );
            printf( "%s, Process: %i, %s %sput operation start\n",
                        timer, pcb->pid, opCode->strArg1, opCode->inOutArg );
            //THREAD
            time = opCode->intArg2 * configPtr->ioCycleRate;
            pthread_create( &waitThread, NULL, waitIO, (void*) &time);
            pthread_join( waitThread, NULL );

            pcb->burstT -= time;

            accessTimer( LAP_TIMER, timer );
            printf( "%s, Process: %i, %s %sput operation end\n",
                        timer, pcb->pid, opCode->strArg1, opCode->inOutArg );
           }

        else if( compareString( opCode->command, "cpu" ) == STR_EQ )
           {
            accessTimer( LAP_TIMER, timer );
            printf( "%s, Process: %i, cpu process operation start\n",
                                                            timer, pcb->pid );

            time = opCode->intArg2 * configPtr->procCycleRate;
            runTimer(time);
            pcb->burstT -= time;

            accessTimer( LAP_TIMER, timer );
            printf( "%s, Process: %i, cpu process operation end\n",
                                                            timer, pcb->pid );
           }
        // ELSE for mem statements

        opCode = opCode->nextNode;
       }

    accessTimer( LAP_TIMER, timer );
    printf( "\n%s, OS: Process %i ended\n", timer, pcb->pid );
    
    pcb->pState = EXIT_STATE;

    accessTimer( LAP_TIMER, timer );
    printf( "%s, OS: Process %i set to EXIT\n", timer, pcb->pid );

    return pcb;
   }

void *waitIO( void *ptr )
   {
    int *ms = (int *)(ptr);
    runTimer( *ms );
    return NULL;
   }