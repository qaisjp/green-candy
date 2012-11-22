/*
	GTA model importer creator for mtasa dm.
	This executable creates files which are needed due to limitation of mtasa dm.
	Created by The_GTA
*/
#include "main.h"

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


bool FileExists(const char *fileName)
{
    return (GetFileAttributes(fileName) != 0xFFFFFFFF);
}

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
		if (!lodSupport)
		{
			if (ipl->IsLOD(numInst))
				continue;
		}
		else if ( CInstance *lod = ipl->GetLod( *i_iter ) )
			lodMap[lod->m_modelID] = *i_iter;

		instances.push_back(*i_iter);
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

// Entry
int		main (int argc, char *argv[])
{
	WIN32_FIND_DATA findData;
	HANDLE find;
	CINI *config = LoadINI("config.ini");
	CINI::Entry *mainEntry;
	const char *mode;
	unsigned int n;

	if (config && (mainEntry = config->GetEntry("General")))
	{
		// Apply configuration
		doCompile = mainEntry->GetBool("compile");
		debug = mainEntry->GetBool("debug");
		mode = mainEntry->Get("mode");
		lodSupport = mainEntry->GetBool("lodSupport");
		preserveMainWorldIntegrity = mainEntry->GetBool("preserveMainWorldIntegrity");
		usXoffset = mainEntry->GetInt("xOffset");
		usYoffset = mainEntry->GetInt("yOffset");
		usZoffset = mainEntry->GetInt("zOffset");
	}
	else
	{
		// Defaults
		doCompile = true;
		debug = false;
		mode = "green";
		lodSupport = true;
		preserveMainWorldIntegrity = false;
	}

	if (preserveMainWorldIntegrity)
		printf( "Compiling with main world integrity...\n" );
	
	// Reset the IDs
	for (n=0; n < 65536; n++)
		avalID[n] = NULL;

	numIPL = 0;
	numIDE = 0;

	if ( preserveMainWorldIntegrity )
	{
		memset( modelLOD, 0, sizeof(modelLOD) );

		// Load all GTA:SA static scene objects (hack)
		SetCurrentDirectory("rplipl\\");

		if ((find = FindFirstFile("*.ipl", &findData)) == INVALID_HANDLE_VALUE)
		{
			printf("Error: Could not find any GTA:SA item placement information (rplipl/.ipl)\n");

			getchar();
			return EXIT_FAILURE;
		}

		LoadReplaceIPL( findData.cFileName );

		while (FindNextFile( find, &findData ))
			LoadReplaceIPL( findData.cFileName );

		FindClose( find );

		SetCurrentDirectory("..");
	}

	// We must get all replacable IDE model ids
	SetCurrentDirectory("rplide\\");

	if ((find = FindFirstFile("*.ide", &findData)) == INVALID_HANDLE_VALUE)
	{
		printf( "ERROR: Could not find GTA:SA model definitions" );

		getchar();
		return EXIT_FAILURE;
	}

	LoadReplaceIDE (findData.cFileName);

	while (FindNextFile(find, &findData))
		LoadReplaceIDE(findData.cFileName);

	FindClose(find);

	// We change into ipl directory
	SetCurrentDirectory("..\\ipl\\");

	// We scan through all ipl files and load em
	if ((find = FindFirstFile("*.ipl", &findData)) == INVALID_HANDLE_VALUE)
	{
		printf( "ERROR: Could not find any item placement files (ipl/.ipl)" );

		getchar();
		return EXIT_FAILURE;
	}

	LoadTargetIPL(findData.cFileName);

	while (FindNextFile(find, &findData))
		LoadTargetIPL(findData.cFileName);

	FindClose(find);

	// Now proceed through all .ide files
	if ((find = FindFirstFile("*.ide", &findData)) == INVALID_HANDLE_VALUE)
	{
		printf( "ERROR: Could not find any model definitions (ipl/.ide)\n" );

		getchar();
		return EXIT_FAILURE;
	}

	LoadTargetIDE(findData.cFileName);

	while (FindNextFile(find, &findData))
		LoadTargetIDE(findData.cFileName);

	FindClose(find);

	// Set up the directory scheme
	CreateDirectory("..\\output", NULL);

	SetCurrentDirectory("..\\output");

	CreateDirectory("models", NULL);
	CreateDirectory("textures", NULL);
	CreateDirectory("coll", NULL);

	// Branch to the handler
	if ( stricmp( mode, "green" ) == 0 )
		return bundleForGREEN( config ) ? EXIT_SUCCESS : EXIT_FAILURE;

	return bundleForBLUE( config ) ? EXIT_SUCCESS : EXIT_FAILURE;
}