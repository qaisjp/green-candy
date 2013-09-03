#include "../StdInc.h"

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

static const char *txdNames[65536];
static const char *txdName;
static const char *colName;
static unsigned short usTxdNames = 0;
static char lodBuffer[128];
static CFile *pMetaFile;

inline bool _CopyOnlyIfRequired( const char *src, const char *dst )
{
    if ( fileRoot->Exists( dst ) )
        return false;

    return fileRoot->Copy( src, dst );
}

static inline bool AllocateResources( const char *name, bool lod )
{
	char buffer[1024];
	char copyBuffer[1024];
	unsigned int k;

	// Copy the model file
	_snprintf(buffer, 1023, "resources\\%s.dff", name);

	if (!fileRoot->Exists(buffer))
	{
		printf("error: model missing (%s)\n", buffer);
		return false;
	}

	_snprintf(copyBuffer, 1023, "output\\models\\%s.dff", name);

	if (_CopyOnlyIfRequired(buffer, copyBuffer))
		printf("copying model '%s'\n", name);

	if ( !lod )
	{
		// Now the collision
		_snprintf(buffer, 1023, "resources\\%s.col", name);

		if ( !fileRoot->Exists(buffer) )
		{
			if ( !lod )
			{
				printf("error: collision missing (%s)\n", buffer);
				return false;
			}

			colName = NULL;
		}
		else
		{
			colName = name;

			_snprintf(copyBuffer, 1023, "output\\coll\\%s.col", colName);

			if (_CopyOnlyIfRequired(buffer, copyBuffer))
				printf("copying collision '%s'\n", colName);
		}
	}
	else
		colName = NULL;

	CObject *txdObj = GetObjectByModel(name);

	if (!txdObj)
	{
		printf("could not find object def for '%s'\n", name);
		return false;
	}

	if (lodSupport)
		_snprintf(lodBuffer, 127, "%.0f", txdObj->m_drawDistance);
	else
		strcpy(lodBuffer, "500");

	txdName = txdObj->m_textureName;

	for (k=0; k < usTxdNames; k++)
		if (strcmp(txdNames[k], txdName) == 0)
			break;

	// Little hack
	if (k == usTxdNames)
	{
		// Copy over resources
		_snprintf(buffer, 1023, "resources\\%s.txd", txdName);

		if (!fileRoot->Exists(buffer))
			printf("texture missing: %s (ignoring)\n", buffer);
		else
		{
			_snprintf(copyBuffer, 1023, "output\\textures\\%s.txd", txdName);

			// Copy the resource over
			if (_CopyOnlyIfRequired(buffer, copyBuffer))
				printf("copying texture '%s'\n", txdName);

			pMetaFile->Printf(
				"	<file src=\"textures\\%s.txd\" type=\"client\" />\n", txdName
			);
		}

		txdNames[usTxdNames++] = txdName;
	}

	names[usNames++] = name;

	pMetaFile->Printf(
		"	<file src=\"models\\%s.dff\" type=\"client\" />\n", name
	);

	if ( colName )
	{
		pMetaFile->Printf(
			"	<file src=\"coll\\%s.col\" type=\"client\" />\n", colName
		);
	}

	return true;
}


bool bundleForGREEN( CINI *config )
{
	CFile *pLuaFile;
	CFile *pMapFile;
	CFile *pLuaServer;
	CINI::Entry *mainEntry;
	instanceList_t::iterator iter;

	printf( "Mode: MTA:GREEN\n" );

	if ( config && ( mainEntry = config->GetEntry( "MainGREEN" ) ) )
	{

	}
	else
	{
		// Defaults
	}

	// Create the .lua file
	pLuaFile=g_outputRoot->Open("model_res.lua","w");
	pMetaFile=g_outputRoot->Open("meta.xml","w");
	pMapFile=g_outputRoot->Open("gta3.map","w");
	pLuaServer=g_outputRoot->Open("main_server.lua","w");

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
		CFile *lodRes = g_outputRoot->Open("lod_res.lua", "w");

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

			if ( !AllocateResources( (*objIter)->m_modelName, true ) )
				continue;

			_snprintf( lodBuffer, 127, "%.0f", (*objIter)->m_drawDistance );

			if ( tableCount++ != 0 )
				lodRes->Printf( ",\n" );

			lodMap_t::const_iterator lodMapIter = lodMap.find( (*objIter)->m_modelID );
			CObject *obj = lodMapIter != lodMap.end() ? GetObjectByModel( lodMapIter->second->m_name ) : NULL;

			lodRes->Printf(
				"    { %u, \"%s\", \"%s\", \"%s\", %u",
				(*objIter)->m_realModelID,
				(*objIter)->m_modelName,
				(*objIter)->m_textureName,
				lodBuffer,
				obj ? obj->m_realModelID : 0
			);

			if ( colName )
			{
				lodRes->Printf( ", %s", colName );
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

		if ( !AllocateResources( name, false ) )
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
			"    { model=%u, model_file=\"%s\", txd_file=\"%s\", coll_file=\"%s\", lod=%s", modelIDs[(*iter)->m_modelID], name, txdName, name, lodBuffer
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

	// Copy over the appropriate model loader
	fileRoot->Copy( "stockpile/green/lod_static.lua", "output/script.lua" );

	if ( doCompile )
	{
#ifdef _WIN32
		system("luac5.1.exe -s -o output/script.luac output/script.lua");
#endif //OS DEPENDANT CODE

		g_outputRoot->Delete("script.lua");
	}

	return true;
}
