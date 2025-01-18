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

    return 0;
   }