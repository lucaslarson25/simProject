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
    tempString = (char *)malloc( ( sourceStrLen + 1 ) * sizeof(char) );
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

int findSubString( const char *testStr, const char *searchSubStr )
   {
    // Initialize variables
    int testStrLen = getStringLength( testStr );
    int masterIndex = 0;
    int searchIndex, internalIndex;

    // Loop across testStr
    while( masterIndex < testStrLen )
       {
        // Set internal loop index to current test string index
        internalIndex = masterIndex;

        // Set internal search index to zero
        searchIndex = 0;

        // Loop to end of test string
        while( internalIndex <= testStrLen
            && testStr[ internalIndex ] == searchSubStr[ searchIndex ] )
           {
            // Increment test string and substring indicies
            internalIndex++;
            searchIndex++;

            // Check for end of the substring
            if( searchSubStr[ searchIndex ] == NULL_CHAR )
               {
                // Return beginning location of the sub string
                return masterIndex;
               }
           }
        // Increment master index
        masterIndex++;
       }
    // Assume test has failed, return SUBSTRING_NOT_FOUND
    return SUBSTRING_NOT_FOUND;
   }

bool getStringConstrained( FILE *inStream, bool clearLeadingNonPrintable,
    bool clearLeadingSpace, bool stopAtNonPrintable, char delimiter,
    char *capturedString )
   {
    // Initalize variables
    int intChar = EOF;
    int index = 0;

    // Initialze output string
    capturedString[ index ] = NULL_CHAR;

    // Capture first value in stream
    intChar = fgetc( inStream );

    // Loop to clear non printable or space, if indicated
    while( ( intChar != EOF) && ( ( clearLeadingNonPrintable
        && (int)SPACE ) || ( clearLeadingSpace && intChar == (int)SPACE ) ) )
       {
        // Get next character
        intChar = fgetc( inStream );
       }
    
    // Check if file is empty
    if( intChar == EOF )
       {
        // String was not found, return failure
        return false;
       }

    // Loop across input
        // ( Not at EOF and not reach max string length )
        // AND ( (if stopAtNonPrintable flag is set AND intChar is nonPrintable)
        // OR stopAtNonPrintable is false ) AND intChar is not the delimiter
    while( ( intChar != EOF && index < MAX_STR_LEN - 1 )
        && ( ( stopAtNonPrintable && intChar >= (int)SPACE )
        || ( !stopAtNonPrintable ) ) && ( intChar != (char)delimiter ) )
       {
        // Place character in array
        capturedString[ index ] = (char)intChar;

        // Increment array index
        index++;

        // Set next element to null character (end of c string)
        capturedString[ index ] = NULL_CHAR;

        // Get next character (reprime)
        intChar = fgetc( inStream );
       }
    
    // Return success
    return true;
   }

bool getStringToDelimiter(FILE *inStream, char delimiter, char *capturedString)
   {
    // Call engine with delimiter
    return getStringConstrained(inStream, true, true, true, delimiter,
        capturedString);
   }

bool getStringToLineEnd( FILE *inStream, char *capturedString )
   {
    // Call engine with delimiter
    return getStringConstrained(inStream, true, true, true, NON_PRINTABLE_CHAR,
        capturedString);
   }

void getSubString( char *destStr, const char *sourceStr, int startIndex,
    int endIndex )
   {
    // Initialize variables
    int sourceStrLen = getStringLength( sourceStr );
    int destIndex = 0;
    int sourceIndex = startIndex;

    // Create char pointer for temporary string
    char *tempStr;

    // Check if indicies are valid
    if( startIndex >= 0 && startIndex <= endIndex && endIndex < sourceStrLen)
       {
        // Allocate memory for temporary string and copy from source string
        tempStr = (char *)malloc( sizeof(char) * ( sourceStrLen + 1 ) );
        copyString( tempStr, sourceStr );

        // Loop over start to end index
        while( sourceIndex <= endIndex )
           {
            // Copy character from temp to dest strings
            destStr[ destIndex ] = tempStr[ sourceIndex ];

            // Increment indicies
            destIndex++;
            sourceIndex++;

            // Add null character for c style string
            destStr[ destIndex ] = NULL_CHAR;
           }

        // Free temporary string
        free( tempStr );
       }
   }   

void setStrToLowerCase( char *destStr, const char *sourceStr )
   {
    // Initialize variables
    int sourceStrLen = getStringLength( sourceStr );
    int index = 0;

    // Create temporary string pointer and copy source string
    char *tempStr = (char *)malloc( sizeof(char) * ( sourceStrLen + 1 ) );
    copyString( tempStr, sourceStr );

    // Loop over temp string
    while( tempStr[ index ] != NULL_CHAR && index < MAX_STR_LEN )
       {
        // Copy lower case character
        destStr[ index ] = toLowerCase( tempStr[ index ] );

        // Increment index
        index++;

        // Terminate c style string
        destStr[ index ] = NULL_CHAR;
       }
    // Free temp string
    free( tempStr );

   }

char toLowerCase( char inChar )
   {
    // Check for lowercase character
    if( inChar >= 'A' && inChar <= 'Z' )
       {
        // Return lowercase
        return inChar - 'A' + 'a';
       }

    // Otherwise, character is lowercase
    return inChar;
   }