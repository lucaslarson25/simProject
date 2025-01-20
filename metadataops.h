#ifndef METADATAOPS_H
#define METADATAOPS_H

#include "datatypes.h"
#include "sim_main.h"

bool getMetaData( const char *fileName, OpCodeType **opCodeDataHead,
    char *endStateMsg );

void displayMetaData( const OpCodeType *localPtr );

#endif