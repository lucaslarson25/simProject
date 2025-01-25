#ifndef METADATAOPS_H
#define METADATAOPS_H

#include "datatypes.h"
#include "sim_main.h"

typedef enum {MSG} OpCodeMessages; //FIXME THIS IS JUST A PLACEHOLDER

bool getMetaData( const char *fileName, OpCodeType **opCodeDataHead,
    char *endStateMsg );

void displayMetaData( const OpCodeType *localPtr );

OpCodeType *clearMetaDataList( OpCodeType *localPtr );

OpCodeType *addNode( OpCodeType *localPtr, OpCodeType *newNode );

int getCommand( char *cmd, const char *inputStr, int index );

OpCodeMessages getOpCommand( FILE *filePtr, OpCodeType *inData );

int getNumberArg( int *number, const char *inputStr, int index );

int getStringArg( char *strArg, const char *inputStr, int index );

bool isDigit( char testChar );

int updateEndCount( int count, const char *opString );

int updateStartCount( int count, const char *opString);

bool verifyFirstStringArg( const char *strArg );

bool verifyValidCommand( char *testCmd );

#endif