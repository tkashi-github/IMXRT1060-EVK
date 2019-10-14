
#pragma once
#include "UserTypedef.h"

extern void *pvlibSYSMalloc( size_t xWantedSize );
extern void vlibSYSPortFree( void *pv );
extern size_t xlibSYSPortGetFreeHeapSize( void );
