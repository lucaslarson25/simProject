#include "stringUtils.h"

int main(void)
   {
    // Utilities check

    printf( "The string '123' is %d characters long\n", 
        getStringLength( "123" ) );

    printf( "The strings '123' and '1234' have the difference of %d\n",
        compareString( "123", "1234" ) );

    printf( "The strings '1234' and '123' have the difference of %d\n",
        compareString( "1234", "123" ) );

    char testStr[MAX_STR_LEN] = "1234";
    concatenateString( testStr, "567" );

    printf( "The strings '1234' and '567' concatted creates %s\n", testStr);

    char testStrTwo[MAX_STR_LEN] = "Ploop";
    copyString( testStr, testStrTwo );
    printf( "Copied strings: %s %s\n", testStr, testStrTwo );

    copyString( testStr, "Hello World!" );
    copyString( testStrTwo, "World!" );
    printf( "Substring '%s' is found at index %d in '%s'\n", testStrTwo,
        findSubString( testStr, testStrTwo ), testStr );

    //getStringConstrained( somefile , false, false, false, ',', testStr );

    getSubString( testStrTwo, testStr, 6, 11);
    printf( "Substring: '%s'\n", testStrTwo );
    
    return 0;
   }