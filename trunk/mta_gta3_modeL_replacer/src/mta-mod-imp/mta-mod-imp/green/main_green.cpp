#include "../main.h"

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
static char *pMapEnd=
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
static FILE *pMetaFile;

static inline bool AllocateResources( const char *name, bool lod )
{
	char buffer[1024];
	char copyBuffer[1024];
	unsigned int k;

	// Copy the model file
	_snprintf(buffer, 1023, "..\\resources\\%s.dff", name);

	if (!FileExists(buffer))
	{
		printf("error: model missing (%s)\n", buffer);
		return false;
	}

	_snprintf(copyBuffer, 1023, "..\\output\\models\\%s.dff", name);

	if (CopyFile(buffer, copyBuffer, true))
		printf("copying model '%s'\n", name);

	if ( !lod )
	{
		// Now the collision
		_snprintf(buffer, 1023, "..\\resources\\%s.col", name);

		if ( !FileExists(buffer) )
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

			_snprintf(copyBuffer, 1023, "..\\output\\coll\\%s.col", colName);

			if (CopyFile(buffer, copyBuffer, true))
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
		_snprintf(buffer, 1023, "..\\resources\\%s.txd", txdName);

		if (!FileExists(buffer))
			printf("texture missing: %s (ignoring)\n", buffer);
		else
		{
			_snprintf(copyBuffer, 1023, "..\\output\\textures\\%s.txd", txdName);

			// Copy the resource over
			if (CopyFile(buffer, copyBuffer, true))
				printf("copying texture '%s'\n", txdName);

			fprintf( pMetaFile,
				"	<file src=\"textures\\%s.txd\" type=\"client\" />\n", txdName
			);
		}

		txdNames[usTxdNames++] = txdName;
	}

	names[usNames++] = name;

	fprintf( pMetaFile,
		"	<file src=\"models\\%s.dff\" type=\"client\" />\n", name
	);

	if ( colName )
	{
		fprintf( pMetaFile, 
			"	<file src=\"coll\\%s.col\" type=\"client\" />\n", colName
		);
	}

	return true;
}


bool bundleForGREEN( CINI *config )
{
	FILE *pLuaFile;
	FILE *pMapFile;
	FILE *pLuaServer;
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
	pLuaFile=fopen("model_res.lua","w");
	pMetaFile=fopen("meta.xml","w");
	pMapFile=fopen("gta3.map","w");
	pLuaServer=fopen("main_server.lua","w");

	// Init files first
	fprintf(pMetaFile, pMetaHeader);
	fprintf(pLuaServer,pServerHeader);
	fprintf(pMetaFile,pMetaHeaderMap);
	fwrite(pMapHeader,1,strlen(pMapHeader),pMapFile);

	if ( doCompile )
		fprintf(pMetaFile, pMetaHeader2, "script.luac");
	else
		fprintf(pMetaFile, pMetaHeader2, "script.lua");

	if ( lodSupport )
	{
		fprintf( pMetaFile,
			"	<file src=\"lod_res.lua\" type=\"client\" />\n"
		);

		objectList_t::iterator objIter;
		FILE *lodRes = fopen("lod_res.lua", "w");

		// LODs are loaded and stay this way
		fprintf( lodRes, "return {\n" );

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
				fprintf( lodRes, ",\n" );

			lodMap_t::const_iterator lodMapIter = lodMap.find( (*objIter)->m_modelID );
			CObject *obj = lodMapIter != lodMap.end() ? GetObjectByModel( lodMapIter->second->m_name ) : NULL;

			fprintf( lodRes,
				"    { %u, \"%s\", \"%s\", \"%s\", %u",
				(*objIter)->m_realModelID,
				(*objIter)->m_modelName,
				(*objIter)->m_textureName,
				lodBuffer,
				obj ? obj->m_realModelID : 0
			);

			if ( colName )
			{
				fprintf( lodRes, ", %s", colName );
			}

			fprintf( lodRes, " }" );
		}

		fprintf( lodRes, "\n};" );
		
		fclose( lodRes );
	}

	// Write the model information table
	fprintf( pLuaFile, "return {\n" );

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
			fprintf(pMapFile, pMapEntry, name, modelIDs[(*iter)->m_modelID], (*iter)->m_position[0] + usXoffset, (*iter)->m_position[1] + usYoffset, (*iter)->m_position[2] + usZoffset);
			fprintf(pMapFile, pMapEntry2, (int)(*iter)->m_rotation[0], (int)(*iter)->m_rotation[1], (int)(*iter)->m_rotation[2]);
			continue;
		}

		if ( !AllocateResources( name, false ) )
			continue;

		// We add all map entries
		fprintf(pMapFile, pMapEntry, name, modelIDs[(*iter)->m_modelID], (*iter)->m_position[0] + usXoffset, (*iter)->m_position[1] + usYoffset, (*iter)->m_position[2] + usZoffset);
		fprintf(pMapFile, pMapEntry2, (int)(*iter)->m_rotation[0], (int)(*iter)->m_rotation[1], (int)(*iter)->m_rotation[2]);

		unsigned short lod = 0;

		if ( CObject *lodObj = backLodMap[(*iter)->m_modelID] )
			lod = lodObj->m_realModelID;

		// Now LUA
		if ( tableCount++ != 0 )
			fprintf( pLuaFile, ",\n" );

		fprintf( pLuaFile, 
			"    { model=%u, model_file=\"%s\", txd_file=\"%s\", coll_file=\"%s\", lod=%s", modelIDs[(*iter)->m_modelID], name, txdName, name, lodBuffer
		);

		if ( lodSupport && lod != 0 )
			fprintf( pLuaFile, ", lodID=%u }", lod );
		else
			fprintf( pLuaFile, " }" );
	}

	fprintf( pLuaFile, "\n};" );

	fprintf(pMapFile, pMapEnd);

	fprintf(pLuaServer, pServerEnd);
	fprintf(pMetaFile, pMetaEnd);
	// Close em
	fclose(pLuaFile);
	fclose(pMetaFile);
	fclose(pMapFile);

	SetCurrentDirectory("../");

	// Copy over the appropriate model loader
	CopyFile( "stockpile/green/lod_static.lua", "output/script.lua", false );

	if ( doCompile )
	{
		system("luac5.1.exe -s -o output/script.luac output/script.lua");

		DeleteFile("output/script.lua");
	}

	return true;
}