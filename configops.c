#include "configops.h"

bool getConfigData( const char *fileName, ConfigDataType **configData,
    char *endStateMsg)
   {
    // Initalize variables
    const int NUM_DATA_LINES = 10;
    const char READ_ONLY_FLAG[] = "r";
    ConfigDataType *tempData;
    FILE *fileAccessPtr;
    char dataBuffer[ MAX_STR_LEN ], lowerCaseDataBuffer[ MAX_STR_LEN ];
    int intData, dataLineCode, lineCtr = 0;
    double doubleData;

    // Set endStateMsg to success
    char testMsg[] = "Configuration file upload successful";
    copyString( endStateMsg, testMsg );

    // Initalize config data pointer
    *configData = NULL;

    // Open the file
    fileAccessPtr = fopen( fileName, READ_ONLY_FLAG );

    // Check for file open failure
    if( fileAccessPtr == NULL )
       {
        // Set end state msg to config file access error
        char testMsg[] = "Configuration file access error";
        copyString( endStateMsg, testMsg );

        // Return failure
        return false;
       }

    // Get the first line, check for failure
    if( !getStringToDelimiter( fileAccessPtr, COLON, dataBuffer ) ||
    compareString(dataBuffer, "Start Simulator Configuration File") != STR_EQ )
       {
        // Close the file
        fclose( fileAccessPtr );

        // Copy end state msg
        copyString( endStateMsg, "Corrupt configuration leader line error" );

        // Return failure
        return false;
       }
    
    // Create temp pointer to config data structure
    tempData = (ConfigDataType *)malloc( sizeof( ConfigDataType ) );

    // Loop to the end of config data items
    while( lineCtr < NUM_DATA_LINES )
       {
        // Get line leader, check for failure
        if( !getStringToDelimiter( fileAccessPtr, COLON, dataBuffer ) )
           {
            // Free temp struct
            free( tempData );

            // Close file
            fclose( fileAccessPtr );
            
            // Set end msg and return failure
            copyString(endStateMsg,
                "Configuration data leader line capture error" );
            return false;
           }
        
        // Strip the trailing spaces
        stripTrailingSpaces( dataBuffer );

        // Find correct data line code number from string
        dataLineCode = getDataLineCode( dataBuffer );

        // Check for data line found
        if( dataLineCode != CFG_CORRUPT_PROMPT_ERR )
           {
            // Get data value, check for version number (double)
            if( dataLineCode == CFG_VERSION_CODE )
               {
                // Get version number
                fscanf( fileAccessPtr, "%lf", &doubleData );
               }
            
            // Otherwise, check for metaData, filenames, or other strings
            else if( dataLineCode == CFG_MD_FILE_NAME_CODE
                        || dataLineCode == CFG_LOG_FILE_NAME_CODE
                            || dataLineCode == CFG_CPU_SCHED_CODE
                                || dataLineCode == CFG_LOG_TO_CODE
                                    || dataLineCode ==  CFG_MEM_DISPLAY_CODE )
               {
                // Get string input
                fscanf( fileAccessPtr, "%s", dataBuffer);

                // Set string to lower case
                setStrToLowerCase( lowerCaseDataBuffer, dataBuffer );
               }
            
            // Otherwise, assume integer data
            else
               {
                // Get integer input
                fscanf( fileAccessPtr, "%d", &intData );
               }
            
            // Check for data value in range
            if( valueInRange( dataLineCode, intData, doubleData,
                lowerCaseDataBuffer ) )
               {
                // Assign to data pointer
                switch( dataLineCode )
                   {
                    case CFG_VERSION_CODE:
                        tempData->version = doubleData;
                        break;
                    
                    case CFG_MD_FILE_NAME_CODE:
                        copyString( tempData->metaDataFileName, dataBuffer );
                        break;
                    
                    case CFG_CPU_SCHED_CODE:
                        tempData->cpuSchedCode = 
                            getCpuSchedCode( lowerCaseDataBuffer );
                        break;

                    case CFG_QUANT_CYCLES_CODE:
                        tempData->quantumCycles = intData;
                        break;

                    case CFG_MEM_DISPLAY_CODE:
                        tempData->memDisplay = 
                            compareString( lowerCaseDataBuffer, "on" ) == 0;
                    
                    case CFG_MEM_AVAILABLE_CODE:
                        tempData->memAvailable = intData;
                        break;

                    case CFG_PROC_CYCLES_CODE:
                        tempData->procCycleRate = intData;
                        break;

                    case CFG_IO_CYCLES_CODE:
                        tempData->ioCycleRate = intData;
                        break;
                    
                    case CFG_LOG_TO_CODE:
                        tempData->logToCode =
                            getLogToCode( lowerCaseDataBuffer );
                        break;

                    case CFG_LOG_FILE_NAME_CODE:
                        copyString( tempData->logToFileName, dataBuffer );
                        break;
                   }
               }
            
            // Otherwise, assume data value not in range
            else
               {
                // Free the memory, close file, set end state msg and return
                free( tempData );
                fclose( fileAccessPtr );
                copyString( endStateMsg, "Configuration item out of range" );
                return false;
               }
           }
        // Otherwise, assume data line not found
        else
           {
            // Free the memory, close file, set end state msg and return failure
            free( tempData );
            fclose( fileAccessPtr );
            return false;
           }
        
        // Increment line count
        lineCtr++;
       }

    // Acquire end of sim config string
    if( !getStringToDelimiter( fileAccessPtr, PERIOD, dataBuffer )
        || compareString( dataBuffer, "End Simulator Configuration File")
            != STR_EQ )
       {
        // Free temp data, close file, set end state msg and return failure
        free( tempData );
        fclose( fileAccessPtr );
        return false;
       }
    
    // Test for "file only" output
    tempData->memDisplay = tempData->memDisplay
        && tempData->logToCode != LOGTO_FILE_CODE;

    // Assign temp pointer to parameter return pointer
    *configData = tempData;

    // Close file access
    fclose( fileAccessPtr );

    // Return no errors
    return true;
   }

void displayConfigData( ConfigDataType *configData )
   {
    // Initalize variables
    char displayString[ STD_STR_LEN ];

    // Print lines of display for all values
    printf( "Config File Display\n" );
    printf( "-------------------\n" );
    printf( "Version                : %3.2f\n", configData->version );
    printf( "Program file name      : %s\n", configData->metaDataFileName );
    configCodeToString( configData->cpuSchedCode, displayString );
    printf( "CPU schedule selection : %s\n", displayString );
    printf( "Quantum time           : %d\n", configData->quantumCycles );
    printf( "Memory Available       : %d\n", configData->memAvailable );
    printf( "Memory Display         : " );
    if( configData->memDisplay )
       {
        printf( "On\n" );
       }
    else
       {
        printf( "Off\n" );
       }
    printf( "Process cycle rate     : %d\n", configData->procCycleRate );
    printf( "I/O cycle rate         : %d\n", configData->ioCycleRate );
    configCodeToString( configData->logToCode, displayString );
    printf( "Log to selection       : %s\n", displayString );
    printf( "Log file name          : %s\n\n", configData->logToFileName );
    }

ConfigDataType *clearConfigData( ConfigDataType *configData )
   {
    // Check if the data is not null
    if( configData != NULL )
       {
        // Free existing data and set to NULL
        free( configData );
        configData = NULL;
       }

    // Return null
    return NULL;
   }

void configCodeToString( int code, char *outString )
   {
    // Define array of strings
    char displayStrings[8][10] = { "SJF-N", "SRTF-P", "FCFS-P", "RR-P", 
        "FCFS-N", "Monitor", "File", "Both" };

    // Copy string to return parameter
    copyString( outString, displayStrings[ code ] );
   }

ConfigCodeMessages getDataLineCode( const char *dataBuffer )
   {
    // Return appropriate code
    if( compareString( dataBuffer, "Version/Phase" ) == STR_EQ )
       {
        return CFG_VERSION_CODE;
       }

    if( compareString( dataBuffer, "File Path" ) == STR_EQ )
       {
        return CFG_MD_FILE_NAME_CODE;
       }
       
    if( compareString( dataBuffer, "CPU Scheduling Code" ) == STR_EQ )
       {
        return CFG_CPU_SCHED_CODE;
       }
    
    if( compareString( dataBuffer, "Quantum Time (cycles)" ) == STR_EQ )
       {
        return CFG_QUANT_CYCLES_CODE;
       }
    
    if( compareString( dataBuffer, "Memory Display (On/Off)" ) == STR_EQ )
       {
        return CFG_MEM_DISPLAY_CODE;
       }
       
    if( compareString( dataBuffer, "Memory Available (KB)" ) == STR_EQ )
       {
        return CFG_MEM_AVAILABLE_CODE;
       }
       
    if( compareString( dataBuffer, "Processor Cycle Time (msec)" ) == STR_EQ )
       {
        return CFG_PROC_CYCLES_CODE;
       }

    if( compareString( dataBuffer, "I/O Cycle Time (msec)" ) == STR_EQ )
       {
        return CFG_IO_CYCLES_CODE;
       }
       
    if( compareString( dataBuffer, "Log To" ) == STR_EQ )
       {
        return CFG_LOG_TO_CODE;
       }
    
    if( compareString( dataBuffer, "Log File Path" ) == STR_EQ )
       {
        return CFG_LOG_FILE_NAME_CODE;
       }

    // Code was not found, return corrupt code error
    return CFG_CORRUPT_PROMPT_ERR;
   }

ConfigDataCodes getLogToCode( const char *lowerCaseLogToStr )
   {
    // Initialze variables
    ConfigDataCodes returnVal = LOGTO_MONITOR_CODE;

    // Check for BOTH
    if( compareString( lowerCaseLogToStr, "both" ) == STR_EQ )
       {
        returnVal = LOGTO_BOTH_CODE;
       }
    
    if( compareString( lowerCaseLogToStr, "file" ) == STR_EQ )
       {
        returnVal = LOGTO_FILE_CODE;
       }

    // Return value
    return returnVal;
   }

void stripTrailingSpaces( char *str )
   {
    // Initialize variables
    int index = getStringLength( str ) - 1;

    // Loop while space is found at end of string
    while( str[ index ] == SPACE )
       {
        // Set element to NULL_CHAR
        str[ index ] = NULL_CHAR;

        // Decrement the index
        index--;
       }
   }

bool valueInRange( int lineCode, int intVal, double doubleVal,
    const char *lowerCaseStringVal )
   {
    // Initialize variables
    bool result = true;

    // Use line code to indentify prompt line
    switch( lineCode )
       {
        case CFG_VERSION_CODE:
            if( doubleVal < 0.00 || doubleVal > 10.00 )
               {
                result = false;
               }
            break;
        
        case CFG_CPU_SCHED_CODE:
            if( compareString( lowerCaseStringVal, "fcfs-n" ) != STR_EQ
                && compareString( lowerCaseStringVal, "sjf-n" ) != STR_EQ
                && compareString( lowerCaseStringVal, "srtf-p" ) != STR_EQ
                && compareString( lowerCaseStringVal, "fcfs-p" ) != STR_EQ
                && compareString( lowerCaseStringVal, "rr-p" ) != STR_EQ )
               {
                result = false;
               }
            break;
        
        case CFG_QUANT_CYCLES_CODE:
            if( intVal < 0 || intVal > 100 )
               {
                result = false;
               }
            break;

        case CFG_MEM_DISPLAY_CODE:
            if( compareString( lowerCaseStringVal, "on" ) != STR_EQ
                && compareString( lowerCaseStringVal, "off" ) != STR_EQ )
               {
                result = false;
               }
            break;
        
        case CFG_MEM_AVAILABLE_CODE:
            if( intVal < 1024 || intVal > 102400 )
               {
                result = false;
               }
            break;

        case CFG_PROC_CYCLES_CODE:
            if( intVal < 1 || intVal > 100 )
               {
                result = false;
               }
            break;

        case CFG_IO_CYCLES_CODE:
            if( intVal < 1 || intVal > 1000 )
               {
                result = false;
               }
            break;

        case CFG_LOG_TO_CODE:
            if( compareString( lowerCaseStringVal, "both") != STR_EQ
                && compareString( lowerCaseStringVal, "monitor" ) != STR_EQ
                && compareString( lowerCaseStringVal, "file" ) != STR_EQ )
               {
                result = false;
               }
            break;
       }
    
    // Return result
    return result;
   }

ConfigDataCodes getCpuSchedCode( const char *lowerCaseCodeStr )
   {
    // Initalize variables
    ConfigDataCodes returnVal = CPU_SCHED_FCFS_N_CODE;

    // Check for SJF-N
    if( compareString( lowerCaseCodeStr, "sjf-n" ) == STR_EQ )
       {
        // Set SJF-N code
        returnVal = CPU_SCHED_SJF_N_CODE;
       }

    // Check for SRTF-P
    if( compareString( lowerCaseCodeStr, "srtf-p" ) == STR_EQ )
       {
        // Set SRTF-P
        returnVal = CPU_SCHED_SRTF_P_CODE;
       }

    // Check for FCFS-P
    if( compareString( lowerCaseCodeStr, "fcfs-p" ) == STR_EQ )
       {
        // Set FCFS-P code
        returnVal = CPU_SCHED_FCFS_P_CODE;
       }

    // Check for RR-P
    if( compareString( lowerCaseCodeStr, "rr-p" ) == STR_EQ )
       {
        // Set RR-P code
        returnVal = CPU_SCHED_RR_P_CODE;
       }

    // Return the selected value
    return returnVal;

   }