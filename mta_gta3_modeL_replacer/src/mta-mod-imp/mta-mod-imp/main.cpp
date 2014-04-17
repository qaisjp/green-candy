/*
	GTA model importer creator for mtasa dm.
	This executable creates files which are needed due to limitations of mtasa dm.
	Created by The_GTA
*/
#include "StdInc.h"

const char *compilator = NULL;

bool doCompile = false;
bool debug = false;
bool lodSupport = true;
bool forceLODInherit = false;
bool preserveMainWorldIntegrity = true;

unsigned short modelIDs[65536];
NameRegistry g_usedModelNames;

CIPL *ipls[256];
unsigned int numIPL = 0;
CIDE *ides[256];
unsigned int numIDE = 0;
bool modelLOD[65536];
CObject *avalID[65536];

instanceList_t instances;
objectList_t objects;
objectList_t lod;
lodMap_t lodMap;
std::map <unsigned short, CObject*> backLodMap;
unsigned int tableCount = 0;
unsigned int numAvailable = 0;

int mapXoffset=0;
int mapYoffset=0;
int mapZoffset=500;

eMappingType mappingType = MAPPING_XML;
eMappingFormat mappingFormat = MAPPING_SANANDREAS;

static bool zipOutput = false;
const char *zipName = NULL;
static bool zipResources = false;

static bool namesCaseInsensitive = true;

// Collision registry for dynamic lookup.
CCollisionRegistry *colRegistry = NULL;

CFileTranslator *g_resourceRoot = NULL;
CFileTranslator *g_outputRoot = NULL;


bool CompareNames( const char *prim, const char *sec )
{
    if ( namesCaseInsensitive )
    {
        return ( stricmp( prim, sec ) == 0 );
    }
    else
    {
        return ( strcmp( prim, sec ) == 0 );
    }
}

CObject*	GetObjectByModel( const char *model )
{
	for ( objectList_t::iterator iter = objects.begin(); iter != objects.end(); iter++ )
    {
		if ( CompareNames( (*iter)->m_modelName, model ) )
        {
			return *iter;
        }
    }

	return NULL;
}

// Process data
void	LoadTargetIPL(const char *filename)
{
	unsigned int numInst = 0;
	CIPL *ipl;
	instanceMap_t::iterator i_iter;

	ipl = LoadIPL(filename);

	for (i_iter = ipl->m_instances.begin(); i_iter != ipl->m_instances.end(); i_iter++, numInst++)
	{
        CInstance *inst = i_iter->second;

		if (!lodSupport)
		{
			if (ipl->IsLOD(numInst))
				continue;
		}
		else if ( CInstance *lod = ipl->GetLod( inst ) )
			lodMap[lod->m_modelID] = inst;

		instances.push_back(inst);
	}

	ipls[numIPL++] = ipl;
}

void	LoadTargetIDE(const char *name)
{
	CIDE *ide = LoadIDE(name);
	objectList_t::iterator iter;

	if ( !ide )
	{
		throw converter_runtime_error( "could not load '" + std::string( name ) + "' file!" );
	}

	for (iter = ide->m_objects.begin(); iter != ide->m_objects.end(); iter++)
	{
		unsigned int m;

		// Assign the ID
		for ( m = 0; m < 65534; m++ )
		{
			if ( !avalID[m] )
				continue;

			avalID[m] = NULL;
			break;
		}
		if ( m == 65534 )
		{
			throw converter_runtime_error( "requiring more valid model ids" );
		}

		(*iter)->m_realModelID = m;
		modelIDs[(*iter)->m_modelID] = m;

		numAvailable--;

		if ( lodSupport )
		{
			//TODO: One lod to multiple objects!
			lodMap_t::const_iterator lodIter = lodMap.find((*iter)->m_modelID);

			if ( lodIter != lodMap.end() )
			{
				lod.push_back(*iter);
				backLodMap[(*lodIter).second->m_modelID] = *iter;
			}
		}

		objects.push_back(*iter);
	}
}

void	LoadReplaceIPL(const char *filename)
{
	CIPL *ipl = LoadIPL(filename);
	instanceMap_t::iterator iter;
	unsigned int numInst = 0;

	for ( iter = ipl->m_instances.begin(); iter != ipl->m_instances.end(); iter++, numInst++ )
    {
		modelLOD[ iter->second->m_modelID ] = ( ipl->IsLOD( numInst ) || iter->second->m_lod != -1 );
    }

	delete ipl;
}

void	LoadReplaceIDE(const char *filename)
{
	CIDE *ide = LoadIDE(filename);
	objectList_t::iterator iter;

	// Marks all ids as available
	for (iter = ide->m_objects.begin(); iter != ide->m_objects.end(); iter++)
	{
		if ((*iter)->m_flags & (OBJECT_GRAFFITI | OBJECT_STATUE | OBJECT_UNKNOWN | OBJECT_UNKNOWN_2 | OBJECT_ALPHA1 | OBJECT_ALPHA2 | OBJECT_BREAKGLASS | OBJECT_BREAKGLASS_CRACK | OBJECT_GARAGE | OBJECT_MULTICLUMP | OBJECT_EXPLOSIVE | OBJECT_VEGETATION | OBJECT_BIG_VEGETATION | OBJECT_UNKNOWN_HIGH))
		//if ((*iter)->m_flags != 0 && !((*iter)->m_flags & OBJECT_INTERIOR))
			continue;

		if (preserveMainWorldIntegrity && modelLOD[(*iter)->m_modelID])
			continue;

		avalID[(*iter)->m_modelID] = *iter;
		numAvailable++;
	}
}

static void LoadReplaceFileIPL( const filePath& path, void* )
{
    LoadReplaceIPL( path.c_str() );
}

static void LoadReplaceFileIDE( const filePath& path, void* )
{
    LoadReplaceIDE( path.c_str() );
}

static void LoadTargetFileIPL( const filePath& path, void* )
{
    LoadTargetIPL( path.c_str() );
}

static void LoadTargetFileIDE( const filePath& path, void* )
{
    LoadTargetIDE( path.c_str() );
}

static void LoadCOLContainer( const filePath& path, void* )
{
    colRegistry->AddContainer( g_resourceRoot, path );
}

// Entry
int		main (int argc, char *argv[])
{
	CINI *config = LoadINI("config.ini");
	CINI::Entry *mainEntry;
	const char *mode;

    bool success = false;

	new CFileSystem;

	if (config && (mainEntry = config->GetEntry("General")))
	{
		// Apply configuration
        compilator = mainEntry->Get("compilator");
		doCompile = mainEntry->GetBool("compile", true);
		debug = mainEntry->GetBool("debug", false);
		mode = mainEntry->Get("mode");
		lodSupport = mainEntry->GetBool("lodSupport", true);
		preserveMainWorldIntegrity = mainEntry->GetBool("preserveMainWorldIntegrity", false);
		mapXoffset = mainEntry->GetInt("xOffset", 0);
		mapYoffset = mainEntry->GetInt("yOffset", 0);
		mapZoffset = mainEntry->GetInt("zOffset", 0);
        zipOutput = mainEntry->GetBool("zipOutput", false);
        zipName = mainEntry->Get("zipName");
        zipResources = mainEntry->GetBool("zipResources", false);
        namesCaseInsensitive = mainEntry->GetBool("namesCaseInsensitive", true);

        const char *mapType = mainEntry->Get("mappingType");

        if ( mapType != NULL && stricmp( mapType, "xml" ) == 0 )
        {
            mappingType = MAPPING_XML;
        }
        else if ( mapType != NULL && stricmp( mapType, "lua" ) == 0 )
        {
            mappingType = MAPPING_LUA;
        }
        else
        {
            mappingType = MAPPING_XML;
        }

        const char *formatType = mainEntry->Get("mapFormat");

        if ( formatType != NULL && ( stricmp( formatType, "iii" ) == 0 || stricmp( formatType, "3" ) == 0 ) )
        {
            mappingFormat = MAPPING_III;
        }
        else if ( formatType != NULL && ( stricmp( formatType, "vc" ) == 0 || stricmp( formatType, "vicecity" ) == 0 ) )
        {
            mappingFormat = MAPPING_VICECITY;
        }
        else if ( formatType != NULL && ( stricmp( formatType, "sa" ) == 0 || stricmp( formatType, "sanandreas" ) == 0 ) )
        {
            mappingFormat = MAPPING_SANANDREAS;
        }
        else
        {
            mappingFormat = MAPPING_SANANDREAS;
        }
	}
	else
	{
		// Defaults
        compilator = NULL;
		doCompile = true;
		debug = false;
		mode = "green";
		lodSupport = true;
		preserveMainWorldIntegrity = false;
        zipOutput = false;
        zipName = NULL;
        zipResources = false;
        namesCaseInsensitive = true;
        mappingType = MAPPING_XML;
        mappingFormat = MAPPING_SANANDREAS;
	}

    // Protect the runtime block.
    try
    {
        if ( mappingFormat != MAPPING_SANANDREAS )
            throw converter_runtime_error( "this converter version only supports San Andreas map format" );

	    if ( preserveMainWorldIntegrity )
		    printf( "Compiling with main world integrity...\n" );

	    // Reset the IDs
	    for ( unsigned int n = 0; n < 65536; n++ )
        {
		    avalID[n] = NULL;
        }

        // Allocate a collision registry.
        colRegistry = new CCollisionRegistry;

	    numIPL = 0;
	    numIDE = 0;

	    if ( preserveMainWorldIntegrity )
	    {
            // Initialize the main model array
		    memset( modelLOD, 0, sizeof(modelLOD) );

		    // Load all GTA:SA static scene objects (hack)
            CFileTranslator *replaceRoot = fileSystem->CreateTranslator( "rplipl/" );

            if ( replaceRoot )
            {
                replaceRoot->ScanDirectory( "", "*.ipl", true, NULL, LoadReplaceFileIPL, NULL );

                delete replaceRoot;
            }
            else
            {
                throw converter_runtime_error( "Could not find any GTA:SA item placement information (rplipl/*.ipl)" );
            }
	    }

        // To calculate LOD model relevance, we should load all replace instances
        // of the original GTA:SA map.
	    if ( CFileTranslator *replaceRoot = fileSystem->CreateTranslator( "rplide/" ) )
        {
            replaceRoot->ScanDirectory( "", "*.ide", true, NULL, LoadReplaceFileIDE, NULL );

            delete replaceRoot;
        }
        else
        {
            throw converter_runtime_error( "Could not find GTA:SA model definitions" );
        }

	    // We scan through all ipl files and load em
        if ( CFileTranslator *iplRoot = fileSystem->CreateTranslator( "ipl/" ) )
        {
            iplRoot->ScanDirectory( "", "*.ipl", true, NULL, LoadTargetFileIPL, NULL );

            delete iplRoot;
        }
        else
        {
		    throw converter_runtime_error( "Could not find any item placement files (ipl/*.ipl)" );
        }

	    // Now proceed through all .ide files
	    if ( CFileTranslator *ideRoot = fileSystem->CreateTranslator( "ipl/" ) )
	    {
            ideRoot->ScanDirectory( "", "*.ide", true, NULL, LoadTargetFileIDE, NULL );

            delete ideRoot;
	    }
        else
	    {
		    throw converter_runtime_error( "Could not find any model definitions (ipl/*.ide)" );
	    }

        // Acquire access.
        CFile *archiveOutputStream = NULL;
        CArchiveTranslator *archiveOutputRoot = NULL;
        CFileTranslator *outputRoot = NULL;

        // Attempt to create a .zip archive.
        if ( zipOutput )
        {
            const char *outputZipName = zipName;

            if ( !outputZipName )
                outputZipName = "output.zip";

            archiveOutputStream = fileRoot->Open( outputZipName, "rb" );

            if ( archiveOutputStream )
            {
                archiveOutputRoot = fileSystem->OpenArchive( *archiveOutputStream );

                if ( !archiveOutputRoot )
                {
                    delete archiveOutputStream;

                    archiveOutputStream = NULL;
                }
            }

            if ( !archiveOutputStream )
            {
                archiveOutputStream = fileRoot->Open( outputZipName, "wb" );

                if ( archiveOutputStream )
                {
                    archiveOutputRoot = fileSystem->CreateZIPArchive( *archiveOutputStream );
                }
            }

            // Attempt to set the output root.
            outputRoot = archiveOutputRoot;
        }

        // If we do not have an output root already.
        if ( !outputRoot )
        {
	        // Set up the directory scheme
            fileRoot->CreateDir( "output/" );

            outputRoot = fileSystem->CreateTranslator( "output/" );
        }

        CFile *archiveResourceStream = NULL;
        CArchiveTranslator *archiveResourceRoot = NULL;
        CFileTranslator *resourceRoot = NULL;

        // Try to find zipped resources (if the config allows their loading).
        if ( zipResources )
        {
            archiveResourceStream = fileRoot->Open( "resources.zip", "rb" );

            if ( archiveResourceStream )
            {
                archiveResourceRoot = fileSystem->OpenArchive( *archiveResourceStream );

                if ( !archiveResourceRoot )
                {
                    delete archiveResourceStream;

                    archiveResourceStream = NULL;
                }
            }

            resourceRoot = archiveResourceRoot;
        }

        // Default to a directory on the root filesystem.
        if ( !resourceRoot )
        {
            resourceRoot = fileSystem->CreateTranslator( "resources/" );
        }

        g_resourceRoot = resourceRoot;
	    g_outputRoot = outputRoot;

        if ( !g_resourceRoot )
        {
            printf( "warning: resource root could not be found\n" );
        }

        // Load all collision containers.
        if ( resourceRoot )
        {
            printf( "Initializing collision containers...\n" );

            resourceRoot->ScanDirectory( "@", "*.col", false, NULL, LoadCOLContainer, NULL );

            printf( "done!\n" );
        }

        if ( g_outputRoot )
        {
            g_outputRoot->CreateDir( "models/" );
            g_outputRoot->CreateDir( "textures/" );
            g_outputRoot->CreateDir( "coll/" );

	        // Branch to the handler
	        if ( !mode || stricmp( mode, "green" ) == 0 )
            {
		        success = bundleForGREEN( config );
            }
            else if ( stricmp( mode, "eirfork" ) == 0 )
            {
                success = bundleForEIRFORK( config );
            }
            else
            {
                success = bundleForBLUE( config );
            }
        }
        else
        {
            throw converter_runtime_error( "failed to acquire output folder link" );
        }

        // Save output data (if necessary)
        if ( archiveOutputRoot )
        {
            printf( "writing to .zip archive (do not close)...\n" );

            archiveOutputRoot->Save();

            printf( "finished!\n" );
        }

        // Clean up FileSystem links.
        if ( g_resourceRoot )
        {
            delete g_resourceRoot;

            g_resourceRoot = NULL;
        }

        if ( g_outputRoot )
        {
            delete g_outputRoot;

            g_outputRoot = NULL;
        }

        // Clean up special links.
        if ( archiveResourceStream )
        {
            delete archiveResourceStream;

            archiveResourceStream = NULL;
        }

        if ( archiveOutputStream )
        {
            delete archiveOutputStream;

            archiveOutputStream = NULL;
        }

        // Delete the collision registry.
        delete colRegistry;
    }
    catch( converter_runtime_error& error )
    {
        printf( "ERROR: %s\n", error.getMessage() );

#ifdef _WIN32
        getchar();
#endif //_WIN32

        success = false;
    }

    // Clean up the fileSystem module activity.
    delete fileSystem;

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

CFileTranslator* AcquireResourceRoot( void )
{
    return g_resourceRoot;
}

CFileTranslator* AcquireOutputRoot( void )
{
    return g_outputRoot;
}

const char* GetGenericScriptHeader( void )
{
    return "-- Compiled by Scene2Res, a MTA:SA GTA III engine map importer";
}

const char* GetCompilatorName( void )
{
    if ( compilator == NULL )
        return "The_GTA";

    return compilator;
}
