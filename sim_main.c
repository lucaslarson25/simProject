#include "stringUtils.h"
#include "sim_main.h"

int main( int argc, char **argv )
   {
    // Initialize variables
    ConfigDataType *configDataPtr = NULL;
    OpCodeType *metaDataPtr = NULL;
    char errorMessage[ MAX_STR_LEN ];
    CmdLineData cmdLineData;
    bool configUploadSuccess = false;

    // Print title
    printf("\nSimulator Program\n");
    printf("=================\n\n");

    // Proccess the command line
    if( proccessCmdLine( argc, argv, &cmdLineData ) )
       {
        // Upload config file and check for success
        if( getConfigData( cmdLineData.fileName,
            &configDataPtr, errorMessage ) )
           {
            // Check config display flag
            if( cmdLineData.configDisplayFlag )
               {
                // Display config data
                displayConfigData( configDataPtr );
               }
            
            // Set config upload success flag
            configUploadSuccess = true;
           }
        
        // Otherwise, assume config file upload failure
        else
           {
            // Show error message and end program
            printf( "\nConfig Upload Error: %s, program aborted\n\n",
                errorMessage);
           }

        // Check for config success and need for meta data
        if( configUploadSuccess && 
            ( cmdLineData.mdDisplayFlag || cmdLineData.runSimFlag ) )
           {
            // Upload meta data file, check for success
            if( getMetaData( configDataPtr->metaDataFileName, &metaDataPtr,
                errorMessage ) )
               {

                // Check meta data display flag
                if( cmdLineData.mdDisplayFlag )
                   {

                    // Display metadata
                    displayMetaData( metaDataPtr );
                   }
                
                // Check run simulator flag
                if( cmdLineData.runSimFlag )
                   {
                    
                    // Run simulator
                    runSim( configDataPtr, metaDataPtr );
                   }
               }
            
            // Otherwise, assume meta data upload failure
            else
               {

                // Print error message, end program
                printf( "\nMetadata Upload Error: %s, program aborted\n",
                    errorMessage );
               }
           }
           
           // Clean up config data as needed
           configDataPtr = clearConfigData( configDataPtr );

           // Clean up meta data as needed
           metaDataPtr = clearMetaDataList( metaDataPtr );
       }
    
    // Otherwise, assume command line failure
    else
       {

        // Show command argument requirements
        showCommandLineFormat();
       }

    // Print program end
    printf( "\nSimulator Program End.\n\n" );

    //return success
    return 0;
   }

bool proccessCmdLine( int numArgs, char **strVector, CmdLineData *clDataPtr )
   {
    // Initialize variables
    int fileStrLen, fileStrSubLoc;

    // Initialize structure to defaults
    clearCmdLineStruct( clDataPtr );

    // Initialize success flags to false
    bool atLeastOneSwitchFlag = false;
    bool correctConfigFileFlag = false;

    // Initialize first arg index to one
    int argIndex = 1;

    // Must have program name, at least one switch, and config file name
    if( numArgs >= MIN_NUM_ARGS )
       {

        // Loop across  args
        while( argIndex < numArgs )
           {
            
            // Check for -dc flag
            if( compareString( strVector[ argIndex ], "-dc" ) == STR_EQ )
               {
                // Set config display flag and one switch flag
                clDataPtr->configDisplayFlag = true;
                atLeastOneSwitchFlag = true;
               }
            
            // Otherwise, check for -dm flag
            else if( compareString( strVector[ argIndex ], "-dm" ) == STR_EQ )
               {
                // Set metadata display flag and one switch flag
                clDataPtr->mdDisplayFlag = true;
                atLeastOneSwitchFlag = true;
               }

            // Otherwise, check for -rs flag
            else if( compareString( strVector[ argIndex ], "-rs" ) == STR_EQ )
               {
                // Set run simulator flag and one switch flag
                clDataPtr->runSimFlag = true;
                atLeastOneSwitchFlag = true;
               }
            
            // Otherwise, check for file name ending in .cfg
            else
               {
                // Find lengths to verify name
                fileStrLen = getStringLength( strVector[ numArgs - 1] );
                fileStrSubLoc = findSubString( strVector[ numArgs - 1 ],
                    ".cfg" );

                // Verify string found and correct format
                if( fileStrSubLoc != SUBSTRING_NOT_FOUND
                    && fileStrSubLoc == fileStrLen - LAST_FOUR_LETTERS )
                   {
                    // Set file name to variable
                    copyString( clDataPtr->fileName, strVector[ numArgs - 1 ] );

                    // Set correct config file flag true
                    correctConfigFileFlag = true;
                   }

                // Otherwise, assume bad config file name
                else
                   {
                    // Reset struct
                    clearCmdLineStruct( clDataPtr );
                   }
               }
            
            // Update arg index
            argIndex++;
           }
       }

    // Return verification
    return atLeastOneSwitchFlag && correctConfigFileFlag;
   }

void showCommandLineFormat()
   {
    // Print command line format
    printf("Command Line Format:\n");
    printf("    sim [-dc] [-dm] [-rs] <config file name>\n");
    printf("    -dc [optional] displays configuration data\n");
    printf("    -dm [optional] displays meta data\n");
    printf("    -rs [optional] runs simulator\n");
    printf("    required config file name\n");
   }

void clearCmdLineStruct( CmdLineData *clDataPtr )
   {
    // Set all struct members to default
    clDataPtr->programRunFlag = false;
    clDataPtr->configDisplayFlag = false;
    clDataPtr->mdDisplayFlag = false;
    clDataPtr->runSimFlag = false;
    clDataPtr->fileName[ 0 ] = NULL_CHAR;
   }