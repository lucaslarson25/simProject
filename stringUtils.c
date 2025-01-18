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