#include "simulator.h"

// Main simulator function
void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr )
   {
    printf( "Simulator Run\n-------------\n\n" );

    // Initialize variables
    OpCodeType *mdPtr = metaDataMstrPtr;
    PCBtype *tempNode = NULL;
    PCBtype *readyQueue = NULL;
    OutputString *outputBuffer = NULL;
    VmemType *memory = NULL;
    SysErrCode sysErr = NO_ERR;
    int pid = 0;
    char timer [ MIN_STR_LEN ];
    char outStr [ MAX_STR_LEN ];

    
    // If logging output to file only, display message
    if( configPtr->logToCode == LOGTO_FILE_CODE )
       {
        printf( "Simulator running for output to file only\n" );
       }

    // Start OS Sim timer
    accessTimer( ZERO_TIMER, timer);
    sprintf(outStr, "  %s, OS: Simulator start\n", timer );
    outputBuffer = bufferOutput( outputBuffer, outStr, configPtr );

    // Create all PCBs (loop over MD LL)
    while( mdPtr != NULL )
       {
        // Check for app start command
        if( compareString( mdPtr->command, "app") == STR_EQ &&
            compareString( mdPtr->strArg1, "start") == STR_EQ)
           {
            // Create new PCB and add it to ready queue
            tempNode = createPCB(tempNode, &mdPtr, pid, configPtr);
            pid++;
            readyQueue = addPCB(readyQueue, tempNode);
            accessTimer( LAP_TIMER, timer);
            sprintf(outStr, 
                    "  %s, OS: Process %i set to READY state from NEW state\n",
                                                        timer, tempNode->pid );
            outputBuffer = bufferOutput( outputBuffer, outStr, configPtr );
           }
        // Increment mdPtr into LL
        mdPtr = mdPtr->nextNode;
       }

    // Initialize Virtual Memory
    memory = initMem( configPtr, &outputBuffer );

    // Reset metadata wrkptr to the start of LL
    mdPtr = metaDataMstrPtr;

    // Set up master loop (loop while readyQueue is not empty)
    while( readyQueue != NULL )
       {
        // Reset sysErr flag
        sysErr = NO_ERR;

        // Get the next scheduled PCB
        tempNode = getPCB( readyQueue, configPtr );
        accessTimer( LAP_TIMER, timer );
        sprintf( outStr, "  %s, OS: Process %i selected with %.0lf ms remaining\n", 
                                timer, tempNode->pid, tempNode->burstT );
        outputBuffer = bufferOutput( outputBuffer, outStr, configPtr );

        // Move from ready queue to runnning state
        tempNode = runPCB( &readyQueue, tempNode->pid, configPtr );
        accessTimer( LAP_TIMER, timer );
        sprintf( outStr, "  %s, OS: Process %i set from READY to RUNNING\n\n",
                                                        timer, tempNode->pid );
        outputBuffer = bufferOutput( outputBuffer, outStr, configPtr );

        // Execute PCB's instructions
        tempNode = executePCB( tempNode, configPtr, &outputBuffer, &sysErr,
                                                                    memory );

        // Free the node after it is complete (terminate)
        free(tempNode);
       }

    // All processes are terminated, declare system stop
    accessTimer( LAP_TIMER, timer );
    sprintf( outStr, "  %s, OS: System stop\n", timer );
    outputBuffer = bufferOutput( outputBuffer, outStr, configPtr );

    // Ensure all PCB structs are freed
    readyQueue = freePCBs( readyQueue );

    // Free all VMem
    memory = freeMem( FREE_ALL_MEM, memory, configPtr, &outputBuffer );

    // Declare end of simulation
    accessTimer( LAP_TIMER, timer );
    sprintf( outStr, "  %s, OS: Simulation end\n", timer );
    outputBuffer = bufferOutput( outputBuffer, outStr, configPtr );

    // Print string buffer LL to log file
    outputBuffer = printBuffer( outputBuffer, configPtr );
   }


// Allocates and creates new PCB node with given pid
// Burst time is calculated from metadata pointer and headPtr will be updated to
// next app end
PCBtype *createPCB(PCBtype *newNode, OpCodeType **headPtr, int pid, 
                                                    ConfigDataType *configPtr)
   {
    // Allocate memory for new node
    newNode = (PCBtype*)malloc(sizeof(PCBtype));

    // Check for allocation failure
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

// Add newly created PCB to ready queue and return new ready queue
PCBtype *addPCB( PCBtype *localPtr, PCBtype *newNode )
   {
    // Create wkgPtr and set PCB state to ready
    PCBtype *wkgPtr = localPtr;
    newNode->pState = READY_STATE;

    // If linked list is empty, return node
    if( wkgPtr == NULL )
       {
        return newNode;
       }

    // Loop to the end of ready queue linked list
    while( wkgPtr->nextPCB != NULL )
       {
        wkgPtr = wkgPtr->nextPCB;
       }
    
    // Add new node to the end of the linked list
    wkgPtr->nextPCB = newNode;

    // Return head pointer
    return localPtr;
   }

// Free all PCB structs from LL
PCBtype *freePCBs( PCBtype *localPtr )
   {
    // Create temp pointer
    PCBtype *tempPtr;

    // Loop to end of linked list
    while( localPtr != NULL )
       {
        // Save next PCB
        tempPtr = localPtr->nextPCB;

        // Free current PCB
        free( localPtr );

        // Increment to next PCB
        localPtr = tempPtr;
       }
    
    // Return NULL LL
    return localPtr;
   }

// Return next PCB to run based on CPU schedule type
PCBtype *getPCB(PCBtype *readyQueueHead, ConfigDataType *configPtr )
   {
    PCBtype *pcbToReturn, *wkgPtr;

    // Get the next PCB based on CPU schedule type
    if( configPtr->cpuSchedCode == CPU_SCHED_FCFS_N_CODE )
       {
        // Return first PCB
        return readyQueueHead;
       }
    
    else if( configPtr->cpuSchedCode == CPU_SCHED_SJF_N_CODE )
       {
        // Find the next shortest job based off of calculated burst time
        pcbToReturn = readyQueueHead;
        wkgPtr = readyQueueHead->nextPCB;

        // Iterate over metadata LL to find PCB with shortest burst time
        while( wkgPtr != NULL)
           {
            if(wkgPtr->burstT < pcbToReturn->burstT)
               {
                pcbToReturn = wkgPtr;
               }

            wkgPtr = wkgPtr->nextPCB;
           }

        // Return found PCB
        return pcbToReturn;

       }

    // Sim3: All other codes will default to FCFS-N
    configPtr->cpuSchedCode = CPU_SCHED_FCFS_N_CODE;
    return getPCB( readyQueueHead, configPtr );
   }

// Find and remove PCB with given pid from ready queue and return
PCBtype *runPCB( PCBtype **readyQueueHead, int pid, ConfigDataType *configPtr )
   {
    // Initialize variables
    PCBtype *foundPCB = NULL, *prev = NULL, *curr = *readyQueueHead;

    // Search for PCB with given pid in ready queue LL
    while( curr != NULL && curr->pid != pid )
       {
        prev = curr;
        curr = curr->nextPCB;
       }
    
    // Once found, set to running state
    curr->pState = RUNNING_STATE;

    // If found PCB was the first
    if( prev == NULL )
       {
        // Set head ptr to next PCB and return current
        *readyQueueHead = curr->nextPCB;
        return curr;
       }

    // Otherwise, found PCB was not the first so remove it from LL and return
    foundPCB = curr;
    prev->nextPCB = curr->nextPCB;
    return foundPCB;
   }

// Execute PCB opcodes and terminate
PCBtype *executePCB( PCBtype *pcb, ConfigDataType *configPtr,
            OutputString **outputBuffer, SysErrCode* sysErr, VmemType *memHeadPtr )
   {
    // Initialize variables
    char timer [ MIN_STR_LEN ];
    char outStr [ MAX_STR_LEN ];
    int time;
    pthread_t waitThread;

    // First opcode is "app start" so increment to next
    OpCodeType *opCode = pcb->firstOp->nextNode;

    // Loop over op code LL until NULL or next "app" command
    while( opCode != NULL && !( compareString( opCode->command, "app") == STR_EQ
                                                      ) && *sysErr == NO_ERR )
       {
        // Check for dev command
        if( compareString( opCode->command, "dev" ) == STR_EQ )
           {
            // Time and print operation start
            accessTimer( LAP_TIMER, timer );
            sprintf( outStr, "  %s, Process: %i, %s %sput operation start\n",
                        timer, pcb->pid, opCode->strArg1, opCode->inOutArg );
            *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr );
            
            // Calculate time for operation
            time = opCode->intArg2 * configPtr->ioCycleRate;

            // Create IO thread to sleep for opCode burst time
            pthread_create( &waitThread, NULL, waitIO, (void*) &time);
            pthread_join( waitThread, NULL );

            // Decrement PCB burst time by time slept
            pcb->burstT -= time;

            // Time and print operation end
            accessTimer( LAP_TIMER, timer );
            sprintf( outStr, "  %s, Process: %i, %s %sput operation end\n",
                        timer, pcb->pid, opCode->strArg1, opCode->inOutArg );
            *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr );
           }

        // Otherwise, check for cpu command
        else if( compareString( opCode->command, "cpu" ) == STR_EQ )
           {
            // Time and print operation start
            accessTimer( LAP_TIMER, timer );
            sprintf( outStr, "  %s, Process: %i, cpu process operation start\n",
                                                            timer, pcb->pid );
            *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr );

            // Calculate op code burst time, sleep, and decrement PCB burst time
            time = opCode->intArg2 * configPtr->procCycleRate;
            runTimer(time);
            pcb->burstT -= time;

            // Time and print operation end
            accessTimer( LAP_TIMER, timer );
            sprintf( outStr, "  %s, Process: %i, cpu process operation end\n",
                                                            timer, pcb->pid );
            *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr );
           }
        // Lastly, op command must be mem
        else
           {
            // Check for allocation
            if( compareString( opCode->strArg1, "allocate" ) == STR_EQ )
               {
               // Time and print operation start
                accessTimer( LAP_TIMER, timer );
                sprintf( outStr, 
                            "  %s, Process: %i, mem allocate request (%i, %i)\n",
                            timer, pcb->pid, opCode->intArg2, opCode->intArg3 );

                *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr);

                // Allocate memory
                *sysErr = allocateMem( pcb->pid, opCode->intArg2,
                    opCode->intArg3, &memHeadPtr, configPtr, outputBuffer );

               }
            // Otherwise, must be access
            else
               {
                // Time and print operation start
                accessTimer( LAP_TIMER, timer );
                sprintf( outStr, 
                            "  %s, Process: %i, mem access request (%i, %i)\n",
                            timer, pcb->pid, opCode->intArg2, opCode->intArg3 );
                            
                *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr);

                // Access memory
                *sysErr = accessMem( pcb->pid, opCode->intArg2, opCode->intArg3,
                                        memHeadPtr, configPtr, outputBuffer );
               }

            // Check for allocation error
            if( *sysErr == ALL_ERR )
               {
                // Time and print operation failure
                accessTimer( LAP_TIMER, timer );
                sprintf( outStr, 
                            "  %s, Process: %i, failed mem allocate request\n",
                                                            timer, pcb->pid );
                            
                *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr);
               }

            // Check for access error
            else if( *sysErr == ACC_ERR )
               {
                // Time and print operation failure
                accessTimer( LAP_TIMER, timer );
                sprintf( outStr, 
                                "  %s, Process: %i, failed mem access request\n",
                                                            timer, pcb->pid );
                            
                *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr);
               }

            // No memory errors
            else
               {
                // Time and print operation success
                accessTimer( LAP_TIMER, timer );
                sprintf( outStr, 
                                "  %s, Process: %i, successful mem %s request\n",
                                            timer, pcb->pid, opCode->strArg1 );
                            
                *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr);
               }
           }

        // Increment opCode pointer
        opCode = opCode->nextNode;
       }

    // Check for no memory errors
    if( *sysErr == NO_ERR )
       {
        // Time and print process end
        accessTimer( LAP_TIMER, timer );
        sprintf( outStr, "\n  %s, OS: Process %i ended\n", timer, pcb->pid );
        *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr );
       }

    // Memory errors have occured
    else
       {
        // Time and print process end
        accessTimer( LAP_TIMER, timer );
        sprintf( outStr, "\n  %s, OS: Segmentation fault, Process %i ended\n", 
                                                            timer, pcb->pid );
        *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr );
       }

    // After process has ended free its memory blocks
    memHeadPtr = freeMem( pcb->pid, memHeadPtr, configPtr, outputBuffer );
    
    // Set PCB state to exit
    pcb->pState = EXIT_STATE;

    // Time and print process termination
    accessTimer( LAP_TIMER, timer );
    sprintf( outStr, "  %s, OS: Process %i set to EXIT\n", timer, pcb->pid );
    *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr );

    // Return terminated PCB
    return pcb;
   }

// Sleep function for IO thread opeartions
void *waitIO( void *ptr )
   {
    // Void pointer magic for pthreading

    // Create pointer to milliseconds which casts the void parameter ptr
    int *ms = (int *)(ptr);

    // Sleep for the value at ms pointer
    runTimer( *ms );

    // Return thread
    return NULL;
   }

// Create and add output string node to buffer LL
OutputString *bufferOutput( OutputString *headPtr, const char *outStr,
                                                    ConfigDataType *configPtr )
   {
    // Create new node for output, wkgPtr, and copy given string to new node
    OutputString *newNode = ( OutputString * )malloc( sizeof( OutputString ) );
    OutputString *wkgPtr = headPtr;
    copyString( newNode->outStr, outStr );
    newNode->nextOutputString = NULL;

    // If monitor logging is selected
    if( configPtr->logToCode == LOGTO_MONITOR_CODE ||
        configPtr->logToCode == LOGTO_BOTH_CODE )
       {
        // Print string in real time
        printf( "%s", outStr );
       }

    // If output buffer LL is empty
    if( wkgPtr == NULL )
       {
        // Return new node
        return newNode;
       }

    // Loop to end of buffer LL
    while( wkgPtr->nextOutputString != NULL)
       {
        wkgPtr = wkgPtr->nextOutputString;
       }
    
    // Add new node to end of LL
    wkgPtr->nextOutputString = newNode;
    
    // Return head ptr
    return headPtr;
   }

// Print and free output buffer to file
OutputString *printBuffer( OutputString *headPtr, ConfigDataType *configPtr )
   {
    // Initialize variables
    OutputString *wkgPtr = headPtr, *prev = NULL;
    bool fileOut = configPtr->logToCode == LOGTO_FILE_CODE || 
                        configPtr->logToCode == LOGTO_BOTH_CODE;
    FILE *outFile = NULL;

    // If file output is selected
    if( fileOut )
       {
        // Open given filename to write
        outFile = fopen( configPtr->logToFileName, FILE_WRITE );
       }

    // Loop over buffer LL
    while( wkgPtr != NULL )
       {
        // If file output is selected
        if( fileOut )
           {
            // Write string to file
            fprintf( outFile, "%s", wkgPtr->outStr );
           }
        // Increment pointers
        prev = wkgPtr;
        wkgPtr = wkgPtr->nextOutputString;

        // Free the printed node
        free( prev );
       }
    
    // If file output is selected
    if( fileOut )
       {
        // Close output file
        fclose( outFile );
       }

    // Return empty buffer LL
    return headPtr;
   }

SysErrCode allocateMem( int pid, int base, int offset, VmemType** memHeadPtr,
                        ConfigDataType *configPtr, OutputString **outputBuffer )
   {
    // Initialize variables
    VmemType *wkgPtr = *memHeadPtr;
    VmemType *tempPtr;
    bool memSuccess = false;

    // Iterate over memory LL while we haven't allocated yet
    while( wkgPtr != NULL && !memSuccess)
       {
        // Check for not empty block that the base or base + offset falls in
        if( wkgPtr->pid != X_STATE && ((wkgPtr->startVAdr <= base && wkgPtr->endVAdr > base) ||
            (wkgPtr->startVAdr < base + offset && wkgPtr->endVAdr > base + offset)) )
           {
            // Allocate overlap error
            printMem( *memHeadPtr, configPtr, outputBuffer, "After allocate overlap failure\n");
            return ALL_ERR;
           }

        // Otherwise, check for free block that can hold requested memory
        else if( wkgPtr->pid == X_STATE && wkgPtr->endAdr - wkgPtr->startAdr >= offset )
           {
            // Set success to true and allocate new block struct
            memSuccess = true;
            tempPtr = ( VmemType* )malloc( sizeof( VmemType ) );
            tempPtr->startAdr = wkgPtr->startAdr + offset;
            tempPtr->endAdr = wkgPtr->endAdr;
            tempPtr->pid = X_STATE;
            tempPtr->startVAdr = ZERO;
            tempPtr->endVAdr = ZERO;
            tempPtr->nextBlock = NULL;
            
            wkgPtr->endAdr = wkgPtr->startAdr + offset - ONE;
            wkgPtr->startVAdr = base;
            wkgPtr->endVAdr = base + offset - ONE;
            wkgPtr->pid = pid;
            wkgPtr->nextBlock = tempPtr;
           }
        
        // Iterate linked list
        wkgPtr = wkgPtr->nextBlock;
       }

    // Check for success
    if( memSuccess )
       {
        // Return no errors
        printMem( *memHeadPtr, configPtr, outputBuffer, "After allocate success\n");
        return NO_ERR;
       }

    // Not enough room for allocation, return allocation error 
    printMem( *memHeadPtr, configPtr, outputBuffer, "After allocate out of memory failure\n");
    return ALL_ERR;
   }

SysErrCode accessMem(int pid, int base, int offset, VmemType *memHeadPtr,
                        ConfigDataType *configPtr, OutputString **outputBuffer )
   {
    // Initialize variables
    VmemType *wkgPtr = memHeadPtr;

    // Iterate over memory LL
    while( wkgPtr != NULL)
       {
        // Check if current block matches pid and access range fits inside
        if( wkgPtr->pid == pid && wkgPtr->startVAdr <= base && wkgPtr->endVAdr >= base + offset - ONE )
           {
            // Print mem display and return no errors
            printMem( memHeadPtr, configPtr, outputBuffer, "After access success\n");
            return NO_ERR;
           }

        // Iterate linked list
        wkgPtr = wkgPtr->nextBlock;
       }

    // If no block was found, print mem display and return access error
    printMem( memHeadPtr, configPtr, outputBuffer, "After access failure\n");
    return ACC_ERR;
   }

VmemType *freeMem( int pid, VmemType *memHeadPtr, ConfigDataType *configPtr,
                                                 OutputString **outputBuffer )
   {
    // Initialize variables
    VmemType *wkgPtr = memHeadPtr;
    VmemType *nextPtr;
    bool merged = false;
    char outStr [MAX_STR_LEN];

    // Check for free all memory flag
    if( pid == FREE_ALL_MEM )
       {
        // Iterate over memory linked list
        while( wkgPtr != NULL )
           {
            // Save next block, free current, and iterate
            nextPtr = wkgPtr->nextBlock;
            free(wkgPtr);
            wkgPtr = nextPtr;
           }

        // Print memory display and return NULL pointer
        printMem( NULL, configPtr, outputBuffer, "After clear all process success\nNo memory configured\n");
        return NULL;
       }

    // Iterate over LL searching for matching pid blocks
    while( wkgPtr != NULL )
       {
        nextPtr = wkgPtr->nextBlock;

        // Check if the block matches the given pid
        if( wkgPtr->pid == pid )
           {
            // Mark as free
            wkgPtr->pid = X_STATE;
            wkgPtr->startVAdr = ZERO;
            wkgPtr->endVAdr = ZERO;
           }

        // Iterate linked list
        wkgPtr = nextPtr;
       }

    // After marking blocks, combine adjacent free blocks
    wkgPtr = memHeadPtr;

    // Iterate over linked list
    while( wkgPtr != NULL )
       {
        // Reset merged flag
        merged = false;
        nextPtr = wkgPtr->nextBlock;

        // Check if both current and next block are free
        if( wkgPtr->pid == X_STATE && nextPtr != NULL && nextPtr->pid == X_STATE )
           {
            // Merge next block into current block
            wkgPtr->endAdr = nextPtr->endAdr;
            wkgPtr->nextBlock = nextPtr->nextBlock;

            // Free the merged block and relink
            free(nextPtr);
            nextPtr = wkgPtr->nextBlock; 

            // Set merged flag
            merged = true;
           }

        // If not merged, iterate; otherwise check merged block next iteration
        if( !merged )
           {
            wkgPtr = nextPtr;
           }
       }

    // Print memory display and return new head pointer
    sprintf( outStr, "After clear process %i success\n", pid );
    printMem(memHeadPtr, configPtr, outputBuffer, outStr);
    return memHeadPtr;
   }
   
VmemType *initMem( ConfigDataType *configPtr, OutputString **outputBuffer )
   {
    // Allocate struct memory
    VmemType *memory = ( VmemType * )malloc( sizeof( VmemType ) );

    // Set up struct
    memory->startAdr = ZERO;
    memory->endAdr = configPtr->memAvailable - ONE;
    memory->startVAdr = ZERO;
    memory->endVAdr = ZERO;
    memory->pid = X_STATE;
    memory->nextBlock = NULL;

    // Print memory display and return head pointer
    printMem( memory, configPtr, outputBuffer, "After memory initialization\n");
    return memory;
   }

void printMem( VmemType *memHeadPtr, ConfigDataType *configPtr, OutputString **outputBuffer,
                                                            const char *outStr )
   {
    // Initalize variables
    char bufferStr [ MAX_STR_LEN ];
    VmemType *wkgPtr;

    // Check if memory display is configured
    if( configPtr->memDisplay )
       {
        // Print and buffer beginining line and given string
        sprintf( bufferStr, "--------------------------------------------------\n");
        *outputBuffer = bufferOutput( *outputBuffer, bufferStr, configPtr );
        *outputBuffer = bufferOutput( *outputBuffer, outStr, configPtr );

        // Iterate over linked list
        wkgPtr = memHeadPtr;
        while( wkgPtr != NULL)
           {
            // Check for free block
            if( wkgPtr->pid == X_STATE )
               {
                // Print free block
                sprintf( bufferStr, "%i [ Open, P#: x, 0-0 ] %i\n",
                                                wkgPtr->startAdr, wkgPtr->endAdr);
                *outputBuffer = bufferOutput( *outputBuffer, bufferStr, configPtr );
               }
            // Otherwise, it is a used block
            else
               {
                // Print used block
                sprintf( bufferStr, "%i [ Used, P#: %i, %i-%i ] %i\n",
                                wkgPtr->startAdr, wkgPtr->pid, wkgPtr->startVAdr,
                                                wkgPtr->endVAdr, wkgPtr->endAdr );
                *outputBuffer = bufferOutput( *outputBuffer, bufferStr, configPtr );
               }
            
            // Iterate working pointer
            wkgPtr = wkgPtr->nextBlock;
           }

        // Print closing line
        sprintf( bufferStr, "--------------------------------------------------\n");
        *outputBuffer = bufferOutput( *outputBuffer, bufferStr, configPtr );
       }
   }