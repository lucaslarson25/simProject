#include "metadataops.h"

bool getMetaData( const char *fileName, OpCodeType **opCodeDataHead,
    char *endStateMsg )
   {
    // Initialize variables
    const char READ_ONLY_FLAG[] = "r";
    int accessResult, startCount = 0, endCount = 0;
    char dataBuffer[ MAX_STR_LEN ];
    bool returnState = true;
    OpCodeType *newNodePtr;
    OpCodeType *localHeadPtr = NULL;
    FILE *fileAccessPtr;

    // Initialize op code data pointer
    *opCodeDataHead = NULL;
    copyString( endStateMsg, "Metadata file upload successful" );

    // Open file for reading
    fileAccessPtr = fopen( fileName, READ_ONLY_FLAG );

    // Check for file open failure
    if( fileAccessPtr == NULL )
       {
        // Set end state msg to failure
        copyString( endStateMsg, "Metadata file access error" );
        return false;
       }

    // Check first line for correct leader
    if( !getStringToDelimiter( fileAccessPtr, COLON, dataBuffer )
        || compareString(dataBuffer, "Start Program Meta-Data Code") != STR_EQ )
       {
        // Close file, set end msg and return error
        fclose( fileAccessPtr );
        copyString( endStateMsg, "Corrupt metadata leader line error" );
        return false;
       }
    
    // Allocate memory for the temporary data structure
    newNodePtr = ( OpCodeType *)malloc( sizeof( OpCodeType ) );


    // Get the first op command
    accessResult = getOpCommand( fileAccessPtr, newNodePtr );

    // Get start and end counts
    startCount = updateStartCount( startCount, newNodePtr->strArg1 );
    endCount = updateEndCount( endCount, newNodePtr->strArg1 );

    // Check for failure of first complete op command
    if( accessResult != COMPLETE_OPCMD_FOUND_MSG )
       {
        // Close the file, free the data, and return failure
        fclose( fileAccessPtr );
        *opCodeDataHead = clearMetaDataList( localHeadPtr );
        free( newNodePtr );
        copyString( endStateMsg, "Metadata incomplete first op command found" );
        return false;
       }

    // Loop across all remaining op commands
    while( accessResult == COMPLETE_OPCMD_FOUND_MSG )
       {
        // Add the new op command to the linked list
        localHeadPtr = addNode( localHeadPtr, newNodePtr );

        // Get a new op command
        accessResult = getOpCommand( fileAccessPtr, newNodePtr );

        // Update start and endcounts
        startCount = updateStartCount( startCount, newNodePtr->strArg1 );
        endCount = updateEndCount( endCount, newNodePtr->strArg1 );
       }
    
    // After loop completion, check for last op commmand found
    if( accessResult == LAST_OPCMD_FOUND_MSG )
       {
        if( startCount == endCount )
           {
            // Add the last node to the linked list
            localHeadPtr = addNode( localHeadPtr, newNodePtr );

            // Set access result to no error
            accessResult = NO_ACCESS_ERR;

            // Check the last line for incorrect end descriptor
            if( !getStringToDelimiter( fileAccessPtr, PERIOD, dataBuffer )
                || compareString( dataBuffer, "End Program Meta-Data Code" )
                != STR_EQ )
               {
                // Set access result to corrupted descriptor error
                accessResult = MD_CORRUPT_DESCRIPTOR_ERR;

                // Set end state message
                copyString( endStateMsg, "Metadata corrupted descriptor error");

               }
           }
        
        // Otherwise, assume start/end counts are not equal
        else
           {
            // Close file, free data, set msg, and return failure
            fclose( fileAccessPtr );
            *opCodeDataHead = clearMetaDataList( localHeadPtr );
            free( newNodePtr );
            
            accessResult = UNBALANCED_START_END_ERR;
            copyString( endStateMsg,
                "Unbalanced start and end arguments in metadata" );
            return false;
           }
       }

    // Otherwise, assume we didn't find the end
    else
       {
        // Set end state msg and return state
        copyString( endStateMsg, "Corrupted metadata op code" );
        returnState = false;
       }

    // Check for any errors
    if( accessResult != NO_ACCESS_ERR)
       {
        // Clear the op command list
        localHeadPtr = clearMetaDataList( localHeadPtr );
       }
    
    // Close file access
    fclose( fileAccessPtr );
    
    // Free temp data
    free( newNodePtr );

    // Assign temp local head pointer to parameter return pointer
    *opCodeDataHead = localHeadPtr;

    // Return access result
    return returnState;
   }

void displayMetaData( const OpCodeType *localPtr )
   {
    // Print titles
    printf( "Meta-Data File Display\n" );
    printf( "----------------------\n" );

    // Loop to end of linked list
    while( localPtr != NULL )
       {
        // Print leader
        printf( "Op Code: " );

        // Print op code
        printf( "/pid:  %d", localPtr->pid );
        printf( "/cmd: %s", localPtr->command );

        // Check for dev op
        if( compareString( localPtr->command, "dev" ) == STR_EQ )
           {
            // Print in/out param
            printf( "/io: %s", localPtr->inOutArg );
           }
        
        // Otherwise assume it's not dev
        else
           {
            // Print NA
            printf( "/io: NA" );
           }
        
        // Print args
        printf( "\n\t /arg1: %s", localPtr->strArg1 );
        printf( "/arg 2: %d", localPtr->intArg2 );
        printf( "/arg 3: %d", localPtr->intArg3 );

        // Print op end time
        printf( "/op end time: %8.6f", localPtr->opEndTime );

        // Print line end
        printf( "\n\n" );

        // Assign local pointer to the next node
        localPtr = localPtr->nextNode;
       }
   }

OpCodeType *clearMetaDataList( OpCodeType *localPtr )
   {
    // Check for local ptr not null
    if( localPtr != NULL )
       {
        // Call recursive function with next pointer
        clearMetaDataList( localPtr->nextNode );

        // After recursive call, free the node
        free( localPtr );
        localPtr = NULL;
       }
    
    // Return null to calling function
    return NULL;
   }

OpCodeType *addNode( OpCodeType *localPtr, OpCodeType *newNode )
   {
    // Check for null
    if( localPtr == NULL )
       {
        // Allocate new node
        localPtr = (OpCodeType *)malloc(sizeof( OpCodeType ) );

        // Assign values to new node
        localPtr->pid = newNode->pid;
        copyString( localPtr->command, newNode->command );
        copyString( localPtr->inOutArg, newNode->inOutArg );
        copyString( localPtr->strArg1, newNode->strArg1 );
        localPtr->intArg2 = newNode->intArg2;
        localPtr->intArg3 = newNode->intArg3;
        localPtr->opEndTime = newNode->opEndTime;

        localPtr->nextNode = NULL;

        // Return new local pointer
        return localPtr;
       }
    
    // Assume end of list not found yet
    localPtr->nextNode = addNode( localPtr->nextNode, newNode );

    // Return local pointer
    return localPtr;
   }

int getCommand( char *cmd, const char *inputStr, int index )
   {
    // Initialize variables
    int lengthOfCommand = 3;
    
    // Loop across command length
    while( index < lengthOfCommand )
       {
        // Assign character from input string to buffer string
        cmd[ index ] = inputStr[ index ];

        // Increment index
        index++;

        // Set next char to null char
        cmd[ index ] = NULL_CHAR;
       }

    // Return current index
    return index;
   }

OpCodeMessages getOpCommand( FILE *filePtr, OpCodeType *inData )
   {
    // Initialize variables
    const int MAX_CMD_LENGTH = 5;
    const int MAX_ARG_STR_LENGTH = 15;

    int numBuffer = 0;
    char strBuffer[ STD_STR_LEN ];
    char cmdBuffer[ MAX_CMD_LENGTH ];
    char argStrBuffer[ MAX_ARG_STR_LENGTH ];
    int runningStringIndex = 0;
    bool arg2FailureFlag = false;
    bool arg3FailureFlag = false;

    // Get whole op command as string
    if( getStringToDelimiter( filePtr, SEMICOLON, strBuffer ) )
       {
        // Get three letter command
        runningStringIndex = getCommand( cmdBuffer,
            strBuffer, runningStringIndex );
        
        // Assign op command to node
        copyString( inData->command, cmdBuffer );

       }
    
    // Otherwise, assume unsuccessfull access
    else
       {
        // Set pointer to data struct to null
        inData = NULL;

        // Return op command access failure
        return OPCMD_ACCESS_ERR;
       }
    
    // Verify op command
    if( !verifyValidCommand( cmdBuffer ) ) 
       {
        // Return op command error
        return CORRUPT_OPCMD_ERR;
       }
    
    // Set all struct values
    inData->pid = 0;
    inData->inOutArg[ 0 ] = NULL_CHAR;
    inData->intArg2 = 0;
    inData->intArg3 = 0;
    inData->opEndTime = 0.0;
    inData->nextNode = NULL;

    // Check for device command
    if( compareString( cmdBuffer, "dev" ) == STR_EQ )
       {
        // Get in/out argument
        runningStringIndex = getStringArg( argStrBuffer, strBuffer,
            runningStringIndex );

        // Set device in/out argument
        copyString( inData->inOutArg, argStrBuffer );

        // Check correct argument
        if( compareString( argStrBuffer, "in" ) != STR_EQ
            && compareString( argStrBuffer, "out" ) != STR_EQ )
           {
            // Return arguement error
            return CORRUPT_OPCMD_ARG_ERR;
           }
       }
       
    // Get first string arg
    runningStringIndex = getStringArg( argStrBuffer, strBuffer,
        runningStringIndex );

    // Set device in/out arg
    copyString( inData->strArg1, argStrBuffer );

    // Check for legitimate first string arg
    if( !verifyFirstStringArg( argStrBuffer) )
       {
        // Return argument error
        return CORRUPT_OPCMD_ARG_ERR;
       }
    
    // Check for last op command found
    if( compareString( inData->command, "sys" ) == STR_EQ 
        && compareString( inData->strArg1, "end" ) == STR_EQ )
       {
        // Return last op command found message
        return LAST_OPCMD_FOUND_MSG;
       }
    
    // Check for app start seconds argument
    if( compareString( inData->command, "app" ) == STR_EQ
        && compareString( inData->strArg1, "start" ) == STR_EQ )
       {
        // get number arg
        runningStringIndex = getNumberArg( &numBuffer, strBuffer,
            runningStringIndex );
        
        // Check for failed number access
        if( numBuffer <= BAD_ARG_VAL )
           {
            // Set failure flag
            arg2FailureFlag = true;
           }

        // Set first int argument to number
        inData->intArg2 = numBuffer;
       }
    
    // Check for cpu cycle time
    else if( compareString( inData->command, "cpu" ) == STR_EQ )
       {
        // Get number argument
        runningStringIndex = getNumberArg( &numBuffer, strBuffer,
            runningStringIndex );

        // Check for failed number access
        if( numBuffer <= BAD_ARG_VAL )
           {
            // Set failure flag
            arg2FailureFlag = true;
           }
        
        // Set first int argument to number
        inData->intArg2 = numBuffer;
       }

    // Check for device cycle time
    else if( compareString( inData->command, "dev" ) == STR_EQ )
       {
        // Get number argument
        runningStringIndex = getNumberArg( &numBuffer, strBuffer,
            runningStringIndex );
        
        // Check for failed number access
        if( numBuffer <= BAD_ARG_VAL )
           {
            // Set failure flag
            arg2FailureFlag = true;
           }
        
        // Set first int argument to number
        inData->intArg2 = numBuffer;
       }

    // Check for memory base and offset
    else if( compareString( inData->command, "mem" ) == STR_EQ )
       {
        // Get number argument for base
        runningStringIndex = getNumberArg( &numBuffer, strBuffer,
            runningStringIndex );
        
        // Check for failed number access
        if( numBuffer <= BAD_ARG_VAL )
           {
            // Set failure flag
            arg2FailureFlag = true;
           }
        
        // Set first int argument to number
        inData->intArg2 = numBuffer;

        // Get number argument for offset
        runningStringIndex = getNumberArg( &numBuffer, strBuffer,
                runningStringIndex );
        
        // Check for failed number access
        if( numBuffer <= BAD_ARG_VAL )
           {
            // Set failure flag
            arg3FailureFlag = true;
           }
        
        // Set second int arguemnt to number
        inData->intArg3 = numBuffer;
       }
    
    // Check int args for upload failure
    if( arg2FailureFlag || arg3FailureFlag )
       {
        // Return corrupt op command error error
        return CORRUPT_OPCMD_ARG_ERR;
       }

    // Return complete op command found message
    return COMPLETE_OPCMD_FOUND_MSG;
   }

int getNumberArg( int *number, const char *inputStr, int index )
   {
    // Initialize variables
    bool foundDigit = false;
    *number = 0;
    int multiplier = 1;

    // Loop to skip white space
    while( inputStr[ index ] <= SPACE || inputStr[ index ] == COMMA )
       {
        index++;
       }

    // Loop across string length
    while( isDigit( inputStr[ index ] ) && inputStr[ index ] != NULL_CHAR )
       {
        // Set flag
        foundDigit = true;

        // Assign digit to ouput
        *number = (*number) * multiplier + inputStr[ index ] - '0';

        // Increment index and multiplier
        index++;
        multiplier = 10;
       }
    
    if( !foundDigit )
       {
        *number = BAD_ARG_VAL;
       }

    // Return current index
    return index;
   }

int getStringArg( char *strArg, const char *inputStr, int index )
   {
    // Initialize variables
    int localIndex = 0;

    // Loop to skip over white space
    while( inputStr[ index ] <= SPACE || inputStr[ index ] == COMMA )
       {
        index++;
       }
    
    // Loop across string length
    while( inputStr[ index ] != COMMA && inputStr[ index] != NULL_CHAR )
       {
        // Assign character from input string to buffer string
        strArg[ localIndex ] = inputStr[ index ];

        // Increment indicies
        index++;
        localIndex++;

        // Set next character to null character
        strArg[ localIndex ] = NULL_CHAR;
       }

    // Return index
    return index;
   }

bool isDigit( char testChar )
   {
    // Test character
    return ( testChar >= '0' && testChar <= '9' );
   }

int updateEndCount( int count, const char *opString )
   {
    // Check for end in op string
    if( compareString( opString, "end" ) == STR_EQ )
       {
        // Return incremented end count
        return count + 1;
       }

    // Return unchanged end count
    return count;
   }

int updateStartCount( int count, const char *opString)
   {
    // Check for start in op string
    if( compareString( opString, "start" ) == STR_EQ )
       {
        // Return incremented start count
        return count + 1;
       }

    // Return unchanged start count
    return count;
   }

bool verifyFirstStringArg( const char *strArg )
   {
    // Check for string holding correct first argument
    return compareString( strArg, "access" ) == STR_EQ ||
        compareString( strArg, "allocate" ) == STR_EQ ||
        compareString( strArg, "end" ) == STR_EQ ||
        compareString( strArg, "ethernet" ) == STR_EQ ||
        compareString( strArg, "hard drive" ) == STR_EQ ||
        compareString( strArg, "keyboard" ) == STR_EQ ||
        compareString( strArg, "monitor" ) == STR_EQ ||
        compareString( strArg, "printer" ) == STR_EQ ||
        compareString( strArg, "process" ) == STR_EQ ||
        compareString( strArg, "serial" ) == STR_EQ ||
        compareString( strArg, "sound signal" ) == STR_EQ ||
        compareString( strArg, "start" ) == STR_EQ ||
        compareString( strArg, "usb" ) == STR_EQ ||
        compareString( strArg, "video signal" ) == STR_EQ;

   }

bool verifyValidCommand( char *testCmd )
   {
    return compareString( testCmd, "sys" ) == STR_EQ ||
        compareString( testCmd, "app" ) == STR_EQ ||
        compareString( testCmd, "cpu" ) == STR_EQ ||
        compareString( testCmd, "mem" ) == STR_EQ ||
        compareString( testCmd, "dev" ) == STR_EQ;
   }