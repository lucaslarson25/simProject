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