#ifndef __MAIN__
#define __MAIN__

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma warning(disable: 4996)

#undef min
#undef max
#endif

#include <math.h>

#include <list>
#include <vector>
#include <map>
#include <string>

#include <string.h>

#include <SharedUtil.h>
#include <CFileSystemInterface.h>
#include <CFileSystem.h>

#include "csv.h"
#include "ipl.h"
#include "ide.h"
#include "ini.h"
#pragma warning(disable: 4996)

#include "blue/main_blue.h"
#include "green/main_green.h"
#include "eirfork/main_eirfork.h"

typedef std::map <unsigned short, CInstance*> lodMap_t;

// Utilities from main file.
CFileTranslator* AcquireResourceRoot( void );
CFileTranslator* AcquireOutputRoot( void );
CObject* GetObjectByModel( const char *model );
const char* GetGenericScriptHeader( void );
const char* GetCompilatorName( void );

// Gay quats
typedef struct quat_s
{
	double w;
	double x;
	double y;
	double z;
} quat_t;

// Global variables
extern bool doCompile;
extern bool debug;
extern bool lodSupport;
extern bool forceLODInherit;
extern bool preserveMainWorldIntegrity;

extern unsigned short modelIDs[65536];
extern const char *names[65536];
extern unsigned short usNames;

extern CIPL *ipls[256];
extern unsigned int numIPL;
extern CIDE *ides[256];
extern unsigned int numIDE;
extern bool modelLOD[65536];
extern CObject *avalID[65536];

extern instanceList_t instances;
extern objectList_t objects;
extern objectList_t lod;
extern lodMap_t lodMap;
extern std::map <unsigned short, CObject*> backLodMap;
extern unsigned int tableCount;
extern unsigned int numAvailable;

extern short usYoffset;
extern short usXoffset;
extern short usZoffset;

// Compile-Time macros for convenience
#define MAP_LUA 1
#define MAP_XML 2
#define MAP_METHOD MAP_XML
#define MAP_MAXLOD TRUE

#define MODEL_TABLE 1
#define MODEL_STATIC 2
#define MODEL_METHOD MODEL_TABLE

#ifdef __linux__
#define _snprintf snprintf
#endif

#endif //__MAIN__
