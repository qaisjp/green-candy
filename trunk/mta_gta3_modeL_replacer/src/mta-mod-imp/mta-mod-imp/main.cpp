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
const char *names[65536];
unsigned short usNames = 0;

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

short usYoffset=0;
short usXoffset=0;
short usZoffset=500;

static bool zipOutput = false;
const char *zipName = NULL;
static bool zipResources = false;

// Collision registry for dynamic lookup.
CCollisionRegistry *colRegistry = NULL;

CFileTranslator *g_resourceRoot = NULL;
CFileTranslator *g_outputRoot = NULL;


CObject*	GetObjectByModel(const char *model)
{
	objectList_t::iterator iter;

	for (iter = objects.begin(); iter != objects.end(); iter++)
		if (strcmp((*iter)->m_modelName, model) == 0)
			return *iter;

	return NULL;
}

// Process data
void	LoadTargetIPL(const char *filename)
{
	unsigned int numInst = 0;
	CIPL *ipl;
	instanceList_t::iterator i_iter;

	ipl = LoadIPL(filename);

	for (i_iter = ipl->m_instances.begin(); i_iter != ipl->m_instances.end(); i_iter++, numInst++)
	{
        CInstance *inst = *i_iter;

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

	if (!ide)
	{
		printf("could not load '%s' file!", name);
		getchar();

		exit(EXIT_FAILURE);
	}

	for (iter = ide->m_objects.begin(); iter != ide->m_objects.end(); iter++)
	{
		unsigned int m;

		// Assign the ID
		for (m=0; m<65534; m++)
		{
			if ( !avalID[m] )
				continue;

			avalID[m] = NULL;
			break;
		}
		if ( m == 65534 )
		{
			printf( "ERROR: requiring more valid model ids\n" );

			getchar();
			exit(EXIT_FAILURE);
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
	instanceList_t::iterator iter;
	unsigned int numInst = 0;

	for ( iter = ipl->m_instances.begin(); iter != ipl->m_instances.end(); iter++, numInst++ )
		modelLOD[(*iter)->m_modelID] = (ipl->IsLOD( numInst ) || (*iter)->m_lod != -1);

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
	unsigned int n;

	new CFileSystem;

	if (config && (mainEntry = config->GetEntry("General")))
	{
		// Apply configuration
        compilator = mainEntry->Get("compilator");
		doCompile = mainEntry->GetBool("compile");
		debug = mainEntry->GetBool("debug");
		mode = mainEntry->Get("mode");
		lodSupport = mainEntry->GetBool("lodSupport");
		preserveMainWorldIntegrity = mainEntry->GetBool("preserveMainWorldIntegrity");
		usXoffset = mainEntry->GetInt("xOffset");
		usYoffset = mainEntry->GetInt("yOffset");
		usZoffset = mainEntry->GetInt("zOffset");
        zipOutput = mainEntry->GetBool("zipOutput");
        zipName = mainEntry->Get("zipName");
        zipResources = mainEntry->GetBool("zipResources");
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
	}

	if ( preserveMainWorldIntegrity )
		printf( "Compiling with main world integrity...\n" );

	// Reset the IDs
	for ( n=0; n < 65536; n++ )
		avalID[n] = NULL;

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
            printf("Error: Could not find any GTA:SA item placement information (rplipl/*.ipl)\n");

#ifdef _WIN32
            getchar();
#endif //_WIN32
            return EXIT_FAILURE;
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
        printf( "ERROR: Could not find GTA:SA model definitions\n" );

#ifdef _WIN32
        getchar();
#endif //_WIN32
        return EXIT_FAILURE;
    }

	// We scan through all ipl files and load em
    if ( CFileTranslator *iplRoot = fileSystem->CreateTranslator( "ipl/" ) )
    {
        iplRoot->ScanDirectory( "", "*.ipl", true, NULL, LoadTargetFileIPL, NULL );

        delete iplRoot;
    }
    else
    {
		printf( "ERROR: Could not find any item placement files (ipl/*.ipl)\n" );

#ifdef _WIN32
		getchar();
#endif //_WIN32
		return EXIT_FAILURE;
    }

	// Now proceed through all .ide files
	if ( CFileTranslator *ideRoot = fileSystem->CreateTranslator( "ipl/" ) )
	{
        ideRoot->ScanDirectory( "", "*.ide", true, NULL, LoadTargetFileIDE, NULL );

        delete ideRoot;
	}
    else
	{
		printf( "ERROR: Could not find any model definitions (ipl/*.ide)\n" );

#ifdef _WIN32
		getchar();
#endif //_WIN32
		return EXIT_FAILURE;
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

    bool success = false;

    if ( g_outputRoot )
    {
        g_outputRoot->CreateDir( "models/" );
        g_outputRoot->CreateDir( "textures/" );
        g_outputRoot->CreateDir( "coll/" );

	    // Branch to the handler
	    if ( stricmp( mode, "green" ) == 0 )
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
        printf( "error: failed to acquire output folder link" );
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
