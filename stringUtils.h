#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdbool.h>
#include <stdio.h>
#include "StandardConstants.h"
#include <stdlib.h>

int getStringLength( const char *testStr );

int compareString( const char *oneStr, const char *otherString );

void concatenateString( char *destStr, const char *sourceStr );

void copyString( char *destStr, const char *sourceStr );

int findSubString( const char *testStr, const char *searchSubStr );

#endif //STRING_UTILS_H