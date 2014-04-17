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


static void mapEntryCallback( CFile *file, const char *instName, unsigned int modelIndex, double posX, double posY, double posZ, double eulerX, double eulerY, double eulerZ )
{
    file->Printf( pMapEntry, instName, modelIndex, (float)posX, (float)posY, (float)posZ );
    file->Printf( pMapEntry2, (int)eulerX, (int)eulerY, (int)eulerZ );
}


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
	CFile *pLuaFile = NULL;
	CFile *pMapFile = NULL;
	CFile *pLuaServer = NULL;
	CINI::Entry *mainEntry;
	instanceList_t::iterator iter;

    // Create the resource manager that manages everything.
    greenResourceDispatch dispatch;

    greenResourceManager resManager( 0, dispatch );

	printf( "Mode: MTA:Eir\n" );

	if ( config && ( mainEntry = config->GetEntry( "MainGREEN" ) ) )
	{

	}
	else
	{
		// Defaults
	}

	// Create the .lua file
	pLuaFile = resManager.OpenOutputFile("model_res.lua","w");
	pMetaFile = resManager.OpenOutputFile("meta.xml","w");
	pMapFile = resManager.OpenOutputFile("gta3.map","w");
	pLuaServer = resManager.OpenOutputFile("main_server.lua","w");

    // Create the instance processor.
    // We only support mapping to XML file.
    InstanceProcessorDesc instDesc;
    instDesc.mapHeader = pMapHeader;
    instDesc.mapEntry = mapEntryCallback;
    instDesc.mapEnd = pMapEnd;

    InstanceProcessor instProc( pMapFile, instDesc );

	// Init files first
	pMetaFile->Printf("%s", pMetaHeader);
	pLuaServer->Printf("%s", pServerHeader);
	pMetaFile->Printf("%s", pMetaHeaderMap);

    {
        InstanceMapper <InstanceProcessor> mapper( instProc );

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
			    const char *name = (*objIter)->m_modelName;

			    if ( g_usedModelNames.Exists( name ) )
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
		    const char *name = (*iter)->m_name;

		    if ( lodSupport && lodMap[(*iter)->m_modelID] )
			    continue;

		    if ( g_usedModelNames.Exists( name ) )
		    {
			    // We add all map entries
			    mapper.MapInstance( name, modelIDs[(*iter)->m_modelID], (*iter)->m_position[0], (*iter)->m_position[1], (*iter)->m_position[2], (*iter)->m_rotation[0], (*iter)->m_rotation[1], (*iter)->m_rotation[2] );
		    }
            else if ( resManager.AllocateResources( name, false ) )
            {
		        // We add all map entries
		        mapper.MapInstance( name, modelIDs[(*iter)->m_modelID], (*iter)->m_position[0], (*iter)->m_position[1], (*iter)->m_position[2], (*iter)->m_rotation[0], (*iter)->m_rotation[1], (*iter)->m_rotation[2] );

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
	    }

	    pLuaFile->Printf( "\n};" );
    }

	pLuaServer->Printf(pServerEnd);
	pMetaFile->Printf(pMetaEnd);
	// Close em
    if ( pLuaFile )
    {
	    delete pLuaFile;
    }
    if ( pMetaFile )
    {
	    delete pMetaFile;
    }
    if ( pMapFile )
    {
	    delete pMapFile;
    }
    if ( pLuaServer )
    {
        delete pLuaServer;
    }

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
