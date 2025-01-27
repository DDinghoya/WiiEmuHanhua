/*
Comment out the DEBUG variable in global.h to make the preprocessor remove
all normal instances of dbgprintf.  Do not include debug.h in:
DI.c
ES.c
FST.c
HID.c
Patch.c
SDI.c
or any header files.
*/

#ifndef __DEBUG_H___
#define __DEBUG_H___

#include "global.h"

#ifndef DEBUG
#define dbgprintf(...)
#else
extern int dbgprintf( const char *fmt, ...);
//can track which function calls it via debug.h
//#define dbgprintf(...) dbgprintf( __FUNCTION__ )
#endif

#endif