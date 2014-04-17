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
#include "col.h"
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

bool CompareNames( const char *prim, const char *sec );

class ImmutableString
{
public:
    inline ImmutableString( const char *data )
    {
        dataPtr = NULL;

        if ( data )
        {
            dataPtr = strdup( data );
        }
    }

    inline ImmutableString( const ImmutableString& right )
    {
        dataPtr = NULL;

        if ( right.dataPtr )
        {
            dataPtr = strdup( right.dataPtr );
        }
    }

    inline ~ImmutableString( void )
    {
        if ( dataPtr )
        {
            free( (void*)dataPtr );
        }
    }

    bool operator ==( const char *right ) const
    {
        if ( dataPtr == NULL )
        {
            return ( right == NULL || *right == '\0' );
        }

        return CompareNames( dataPtr, right );
    }

    operator const char*( void ) const
    {
        if ( dataPtr == NULL )
            return "";

        return dataPtr;
    }

private:
    const char *dataPtr;
};

class NameRegistry
{
public:
    void Add( const char *name )
    {
        if ( !Exists( name ) )
        {
            names.push_back( name );
        }
    }

    bool Exists( const char *name ) const
    {
        return ( std::find( names.begin(), names.end(), name ) != names.end() );
    }

private:
    typedef std::list <ImmutableString> nameList_t;

    nameList_t names;
};

// Global variables
extern bool doCompile;
extern bool debug;
extern bool lodSupport;
extern bool forceLODInherit;
extern bool preserveMainWorldIntegrity;

extern unsigned short modelIDs[65536];
extern NameRegistry g_usedModelNames;

extern CCollisionRegistry *colRegistry;

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

extern int mapYoffset;
extern int mapXoffset;
extern int mapZoffset;

enum eMappingType
{
    MAPPING_LUA,
    MAPPING_XML
};
extern eMappingType mappingType;

enum eMappingFormat
{
    MAPPING_III,
    MAPPING_VICECITY,
    MAPPING_SANANDREAS
};
extern eMappingFormat mappingFormat;

struct converter_runtime_error
{
    inline converter_runtime_error( const std::string& msg ) : message( msg.c_str() )
    {
    }

    const char* getMessage( void ) const
    {
        return message;
    }

    ImmutableString message;
};

#ifdef __linux__
#define _snprintf snprintf
#endif

#endif //__MAIN__
