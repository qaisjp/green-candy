#include "../StdInc.h"
#include "../resources.h"

static const char *pServerHeader=
	"-- Automatically generated server script by MTASA map IPL map converter\n" \
	"setMapName(\"United\");\n";
static const char *pServerEnd=
	"-- The end\n";

static const char *pMapHeader=
	"<map>\n";
static const char *pMapEntry=
	"	<object id=\"%s\" dimension=\"0\" interior=\"0\" model=\"%u\" posX=\"%f\" posY=\"%f\" posZ=\"%f\" ";
static const char *pMapEntry2=
	"rotX=\"%u\" rotY=\"%u\" rotZ=\"%u\" />\n";
static const char *pMapEnd=
	"</map>\n";

static const char *pMetaHeader=
	"<meta>\n" \
	"	<info author=\"The_GTA\" description=\"Automatically generated map file by MTASA map converter\" version=\"1.2\" type=\"map\" />\n";
static const char *pMetaHeaderMap=
	"	<map src=\"gta3.map\" />\n";
static const char *pMetaHeader2=
	"	<script src=\"%s\" type=\"client\" />\n" \
	"	<script src=\"main_server.lua\" type=\"server\" />\n" \
	"	<file src=\"model_res.lua\" type=\"client\" />\n";
static const char *pMetaEnd=
	"</meta>\n";

static CFile *pMetaFile = NULL;

struct greenResourceDispatch
{
    inline bool requireCollision( const char *name, bool lod )
    {
        return !lod;
    }

    inline void OnModelEntry( const char *name )
    {
	    pMetaFile->Printf(
		    "	<file src=\"models\\%s.dff\" type=\"client\" />\n", name
	    );
    }

    inline void OnTxdEntry( const char *name )
    {
		pMetaFile->Printf(
			"	<file src=\"textures\\%s.txd\" type=\"client\" />\n", name
		);
    }

    inline void OnCOLEntry( const char *name )
    {
		pMetaFile->Printf(
			"	<file src=\"coll\\%s.col\" type=\"client\" />\n", name
		);
    }
};
typedef ResourceManager <greenResourceDispatch> greenResourceManager;

bool bundleForGREEN( CINI *config )
{
	CFile *pLuaFile;
	CFile *pMapFile;
	CFile *pLuaServer;
	CINI::Entry *mainEntry;
	instanceList_t::iterator iter;

    // Create the resource manager that manages everything.
    greenResourceDispatch dispatch;

    greenResourceManager resManager( 0, dispatch );

	printf( "Mode: MTA:GREEN\n" );

	if ( config && ( mainEntry = config->GetEntry( "MainGREEN" ) ) )
	{

	}
	else
	{
		// Defaults
	}

	// Create the .lua file
	pLuaFile = resManager.outputRoot->Open("model_res.lua","w");
	pMetaFile = resManager.outputRoot->Open("meta.xml","w");
	pMapFile = resManager.outputRoot->Open("gta3.map","w");
	pLuaServer = resManager.outputRoot->Open("main_server.lua","w");

	// Init files first
	pMetaFile->Printf("%s", pMetaHeader);
	pLuaServer->Printf("%s", pServerHeader);
	pMetaFile->Printf("%s", pMetaHeaderMap);
	pMapFile->Write(pMapHeader,1,strlen(pMapHeader));

	if ( doCompile )
		pMetaFile->Printf(pMetaHeader2, "script.luac");
	else
		pMetaFile->Printf(pMetaHeader2, "script.lua");

	if ( lodSupport )
	{
		pMetaFile->Printf(
			"	<file src=\"lod_res.lua\" type=\"client\" />\n"
		);

		objectList_t::iterator objIter;
		CFile *lodRes = resManager.outputRoot->Open("lod_res.lua", "w");

		// LODs are loaded and stay this way
		lodRes->Printf( "return {\n" );

		tableCount = 0;

		for ( objIter = lod.begin(); objIter != lod.end(); objIter++ )
		{
			unsigned int m;
			const char *name = (*objIter)->m_modelName;

			for (m=0; m<usNames; m++)
			{
				if (strcmp(names[m], name) == 0)
					break;
			}
			if (m != usNames)
				continue;

			if ( !resManager.AllocateResources( (*objIter)->m_modelName, true ) )
				continue;

			_snprintf( resManager.lodBuffer, 127, "%.0f", (*objIter)->m_drawDistance );

			if ( tableCount++ != 0 )
				lodRes->Printf( ",\n" );

			lodMap_t::const_iterator lodMapIter = lodMap.find( (*objIter)->m_modelID );
			CObject *obj = lodMapIter != lodMap.end() ? GetObjectByModel( lodMapIter->second->m_name ) : NULL;

			lodRes->Printf(
				"    { %u, \"%s\", \"%s\", \"%s\", %u",
				(*objIter)->m_realModelID,
				(*objIter)->m_modelName,
				(*objIter)->m_textureName,
				resManager.lodBuffer,
				obj ? obj->m_realModelID : 0
			);

			if ( resManager.colName )
			{
				lodRes->Printf( ", %s", resManager.colName );
			}

			lodRes->Printf( " }" );
		}

		lodRes->Printf( "\n};" );

		delete lodRes;
	}

	// Write the model information table
	pLuaFile->Printf( "return {\n" );

	tableCount = 0;

	for (iter = instances.begin(); iter != instances.end(); iter++)
	{
		unsigned short m;
		const char *name = (*iter)->m_name;

		if ( lodSupport && lodMap[(*iter)->m_modelID] )
			continue;

		for (m=0; m < usNames; m++)
		{
			if (strcmp(name, names[m]) == 0)
				break;
		}
		if (m != usNames)
		{
			// We add all map entries
			pMapFile->Printf(pMapEntry, name, modelIDs[(*iter)->m_modelID], (*iter)->m_position[0] + usXoffset, (*iter)->m_position[1] + usYoffset, (*iter)->m_position[2] + usZoffset);
			pMapFile->Printf(pMapEntry2, (int)(*iter)->m_rotation[0], (int)(*iter)->m_rotation[1], (int)(*iter)->m_rotation[2]);
			continue;
		}

		if ( !resManager.AllocateResources( name, false ) )
			continue;

		// We add all map entries
		pMapFile->Printf(pMapEntry, name, modelIDs[(*iter)->m_modelID], (*iter)->m_position[0] + usXoffset, (*iter)->m_position[1] + usYoffset, (*iter)->m_position[2] + usZoffset);
		pMapFile->Printf(pMapEntry2, (int)(*iter)->m_rotation[0], (int)(*iter)->m_rotation[1], (int)(*iter)->m_rotation[2]);

		unsigned short lod = 0;

		if ( CObject *lodObj = backLodMap[(*iter)->m_modelID] )
			lod = lodObj->m_realModelID;

		// Now LUA
		if ( tableCount++ != 0 )
			pLuaFile->Printf( ",\n" );

		pLuaFile->Printf(
			"    { model=%u, model_file=\"%s\", txd_file=\"%s\", coll_file=\"%s\", lod=%s", modelIDs[(*iter)->m_modelID], name, resManager.txdName, name, resManager.lodBuffer
		);

		if ( lodSupport && lod != 0 )
			pLuaFile->Printf( ", lodID=%u }", lod );
		else
			pLuaFile->Printf( " }" );
	}

	pLuaFile->Printf( "\n};" );

	pMapFile->Printf(pMapEnd);

	pLuaServer->Printf(pServerEnd);
	pMetaFile->Printf(pMetaEnd);
	// Close em
	delete pLuaFile;
	delete pMetaFile;
	delete pMapFile;
    delete pLuaServer;

	// Copy over the appropriate model loader
	fileRoot->Copy( "stockpile/green/lod_static.lua", "output/script.lua" );

	if ( doCompile )
	{
#ifdef _WIN32
		system("luac5.1.exe -s -o output/script.luac output/script.lua");
#endif //OS DEPENDANT CODE

		resManager.outputRoot->Delete("script.lua");
	}

	return true;
}
