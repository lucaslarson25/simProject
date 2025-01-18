#include "stringUtils.h"

int getStringLength( const char *testStr )
   {
    // Initialize index variable
    int index = 0;

    // Loop over the string until null char is found or index passes STD_STR_LEN
    while( index < STD_STR_LEN && testStr[ index ] != NULL_CHAR )
       {
        // Increment index
        index++;
       }

    // Return string length
    return index;
   }

int compareString( const char *oneStr, const char *otherStr )
   {
    // Initialize variables
    int diff, index = 0;

    // Loop to end of the shorter string
    while( oneStr[ index ] != NULL_CHAR
            && otherStr[ index ] != NULL_CHAR
                && index < MAX_STR_LEN )
       {
        // Calculate the difference in characters
        diff = oneStr[ index ] - otherStr[ index ];

        // Check for a difference between characters
        if( diff != 0 )
           {
            // Return the difference
            return diff;
           }
        
        // Increment the index
        index++;
       }
    
    // If no character difference was found, return difference of lengths
    return getStringLength( oneStr ) - getStringLength( otherStr );
   }

void concatenateString( char *destStr, const char *sourceStr )
   {
    // Initalize variables
    int destIndex = getStringLength( destStr );
    int sourceIndex = 0;
    int sourceStrLen = getStringLength( sourceStr );

    // Create temporary string pointer
    char *tempString;
    
    
    // Copy source string
    tempString = (char *)malloc( sizeof( sourceStrLen + 1 ) );
    copyString( tempString, sourceStr );

    // Loop to the end of the source string
    while( tempString[ sourceIndex ] != NULL_CHAR && destIndex < MAX_STR_LEN )
       {
        // Assign characters to end of destination string
        destStr[ destIndex ] = tempString[ sourceIndex ];

        // Update indicies
        destIndex++;
        sourceIndex++;

        // Set temporary end of destination string
        destStr[ destIndex ] = NULL_CHAR;
       }

    // Free temporary string
    free(tempString);
   }

void copyString( char *destStr, const char *sourceStr )
   {
    // Initalize variables
    int index = 0;

    // Check for source/dest pointer not the same
    if( destStr != sourceStr )
       {
        // Loop across source string
        while( sourceStr[ index ] != NULL_CHAR && index < MAX_STR_LEN )
           {
            // Assign characters to end of dest string
            destStr[ index ] = sourceStr[ index ];

            // Increment index
            index++;

            // Set temporary end of dest string
            destStr[ index ] = NULL_CHAR;
           }
       }
   }