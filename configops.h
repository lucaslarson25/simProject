#ifndef CONFIGOPS_H
#define CONFIGOPS_H

#include "datatypes.h"
#include "sim_main.h"

bool getConfigData( const char *fileName, ConfigDataType **configData,
    char *endStateMsg);

void displayConfigData( ConfigDataType *configData );

ConfigDataType *clearConfigData( ConfigDataType *configData );

OpCodeType *clearMetaDataList( OpCodeType *localPtr );

#endif