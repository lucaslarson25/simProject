#include "metadataops.h"

bool getMetaData( const char *fileName, OpCodeType **opCodeDataHead,
    char *endStateMsg )
   {
    return false;
   }

void displayMetaData( const OpCodeType *localPtr )
   {

   }

OpCodeType *clearMetaDataList( OpCodeType *localPtr )
   {
    return NULL;
   }

OpCodeType *addNode( OpCodeType *localPtr, OpCodeType *newNode )
   {
    return NULL;
   }

int getCommand( char *cmd, const char *inputStr, int index )
   {
    return 0;
   }

OpCodeMessages getOpCommand( FILE *filePtr, OpCodeType *inData )
   {
    return 0;
   }

int getNumberArg( int *number, const char *inputStr, int index )
   {
    return 0;
   }

int getStringArg( char *strArg, const char *inputStr, int index )
   {
    return 0;
   }

bool isDigit( char testChar )
   {
    return false;
   }

int updateEndCount( int count, const char *opString )
   {
    return 0;
   }

int updateStartCount( int count, const char *opString)
   {
    return 0;
   }

bool verifyFirstStringArg( const char *strArg )
   {
    return false;
   }

bool verifyValidCommand( char *testCmd )
   {
    return false;
   }