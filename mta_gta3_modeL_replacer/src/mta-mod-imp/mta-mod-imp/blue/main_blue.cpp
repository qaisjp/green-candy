#include "../StdInc.h"

enum eStreamMethod
{
	STREAM_STREAM,
	STREAM_DISTANCE
};

unsigned int jetpackHeight;
bool staticCompile;
bool cached;
unsigned int streamerMemory;
bool autoCollect;
unsigned int runtimeCorrection;
eStreamMethod streamMethod;

static inline void luaBegin( CFile *file )
{
	file->Printf(
		"-- File generated by MTASA model importer\n"
	);

	if ( debug )
	{
		file->Printf(
			"local modelsLoaded={};\n"
		);
	}

	if ( cached )
	{
		file->Printf(
			"local streamerMemory=0;\n" \
			"local cached={};\n" \
			"local resourceQueue={};\n"
		);
	}

	file->Printf(
		"local textureCache = {};\n" \
		"local streamedObjects = {};\n" \
		"local pModels = {};\n\n" \
		"setCloudsEnabled(false);\n"
	);

	if ( !lodSupport )
	{
		file->Printf(
			"setFarClipDistance(350);\n" \
			"setFogDistance(10);\n"
		);
	}

	file->Printf(
		"debug.sethook(nil);\n\n"
	);

	// getResourceSize if not cached...?

	file->Printf(
		"local function getResourceSize(path)\n" \
		"	if not (fileExists(path)) then return 0; end;\n\n" \
		"	local file = fileOpen(path);\n" \
		"	local size = fileGetSize(file)\n\n" \
		"	fileClose(file);\n" \
		"	return size;\n" \
		"end\n\n" \
		"local function requestTexture(model, path)\n" \
		"	local txd = textureCache[path];\n\n" \
		"	if not (txd) then\n"
		"		local size = getResourceSize(path);\n" \
		"		if (size == 0) then return false; end\n\n" \
	);

	if ( cached )
	{
		file->Printf(
			"		if ( streamerMemory + size > %u ) then\n" \
			"			outputDebugString(\"out of streaming memory! (\"..path..\")\");\n" \
			"			return false;\n" \
			"		end\n\n", streamerMemory
		);
	}

	file->Printf(
		"		txd = engineLoadTXD(path);\n\n" \
		"		if not (txd) then\n" \
		"			return false;\n" \
		"		end\n\n" \
		"		textureCache[path] = txd;\n"
	);

	if ( cached )
	{
		file->Printf(
			"		streamerMemory = streamerMemory + size;\n"
		);
	}

	file->Printf(
		"	end\n" \
		"	return txd;\n" \
		"end\n\n"
	);

	if ( cached )
	{
		file->Printf(
			"local function loadResources(model, size)\n" \
			"	if not (model.impTxd) then\n" \
			"		engineImportTXD(model.txd, model.id);\n\n" \
			"		model.impTxd = true;\n" \
			"	end\n\n" \
			"	model.model = engineLoadDFF(model.model_file, model.id);\n\n" \
			"	if not (model.model) then return false; end;\n\n"
		);

		if ( lodSupport )
		{
			file->Printf(
				"	if not (model.col_file) then\n" \
				"		if not (model.super) then return false; end\n\n" \
				"		model.col = pModels[model.super].col;\n" \
				"	else\n" \
				"		model.col = engineLoadCOL(model.col_file, 0);\n\n" \
				"		if not (model.col) then return false; end\n" \
				"	end\n\n"
			);
		}
		else
		{
			file->Printf(
				"	model.col = engineLoadCOL(model.col_file, 0);\n\n" \
				"	if not (model.col) then return false; end\n\n"
			);
		}

		file->Printf(
			"	cached[model.id] = {\n" \
			"		model = model,\n" \
			"		size = size\n" \
			"	};\n" \
			"	resourceQueue[model.id] = nil;\n" \
			"	streamerMemory = streamerMemory + size;\n" \
			"	return true;\n" \
			"end\n\n" \
			"local function freeResources(model)\n" \
			"	local cache = cached[model.id];\n\n" \
			"	if not (cache) then return true; end\n\n" \
			"	if (model.col_file) then\n" \
			"		destroyElement(model.col);\n" \
			"	end\n" \
			"	destroyElement(model.model);\n" \
			"	model.model = nil;\n" \
			"	model.col = nil;\n" \
			"	cached[model.id] = nil;\n" \
			"	streamerMemory = streamerMemory - cache.size;\n" \
			"	return true;\n" \
			"end\n\n" \
			"local function cacheResources(model)\n" \
			"	if (resourceQueue[model.id]) then return false; end\n\n" \
			"	if (cached[model.id]) then return true; end\n\n"
		);

		if ( lodSupport )
		{
			file->Printf(
				"	local size = getResourceSize(model.model_file);\n" \
				"	if (model.col_file) then\n" \
				"		size = size + getResourceSize(model.col_file);\n" \
				"	end\n"
			);
		}
		else
		{
			file->Printf(
				"	local size = getResourceSize(model.model_file) + getResourceSize(model.col_file);\n\n"
			);
		}

		file->Printf(
			"	if ( streamerMemory + size > %u ) then\n" \
			"		outputDebugString(\"streamer memory limit reached... queueing request!\");\n\n" \
			"		resourceQueue[model.id] = {\n" \
			"			size = size,\n" \
			"			model = model\n" \
			"		};\n" \
			"		return false;\n" \
			"	end\n\n" \
			"	return loadResources(model, size);\n" \
			"end\n\n", streamerMemory
		);
	}

	file->Printf(
		"local function loadModel(model)\n" \
		"	if (model.loaded) then return true; end\n\n"
	);

	if ( lodSupport )
	{
		file->Printf(
			"	if (model.super) then\n" \
			"		if not (loadModel(pModels[model.super])) then\n" \
			"			return false;\n" \
			"		end\n" \
			"	end\n\n"
		);
	}

	if ( cached )
	{
		file->Printf(
			"	if not (cacheResources(model)) then return false; end\n\n"
		);
	}

	file->Printf(
		"	engineReplaceModel(model.model, model.id);\n" \
		"	engineReplaceCOL(model.col, model.id);\n\n" \
		"	model.loaded = true;\n"
	);

	if ( debug )
	{
		file->Printf(
			"	table.insert(modelsLoaded, model);\n"
		);
	}

	file->Printf(
		"	return true;\n" \
		"end\n\n" \
		"local function freeModel(model)\n" \
		"	if not (model.loaded) then return true; end\n\n" \
		"	engineRestoreModel(model.id);\n" \
		"	engineRestoreCOL(model.id);\n\n"
	);

	if ( cached && autoCollect )
	{
		file->Printf(
			"	freeResources(model);\n\n"
		);
	}

	if ( debug )
	{
		file->Printf(
			"	for m,n in ipairs(modelsLoaded) do\n" \
			"		if (model == n) then\n" \
			"			table.remove(modelsLoaded, m);\n" \
			"			break;\n" \
			"		end\n" \
			"	end\n"
		);
	}

	file->Printf(
		"	model.loaded = false;\n"
	);

	if ( lodSupport )
	{
		file->Printf(
			"	if (model.super) then\n" \
			"		return freeModel(pModels[model.super]);\n" \
			"	end\n"
		);
	}

	file->Printf(
		"end\n\n"
	);
}

static inline void luaMethodBegin( CFile *file )
{
	file->Printf(
		"local function modelStreamOut ()\n" \
		"	local pModel = pModels[getElementModel(source)];\n\n" \
		"	if not (pModel) then return end;\n\n"
	);

	if ( lodSupport )
	{
		file->Printf(
			"	if (pModel.lodID) then return true; end\n\n"
		);
	}

	if ( streamMethod == STREAM_STREAM )
	{
		file->Printf(
			"	if (pModel.isRequesting == 2) then\n" \
			"		pModel.isRequesting = 1;\n" \
			"		return true;\n" \
			"	elseif (pModel.isRequesting == 3) then\n" \
			"		pModel.isRequesting = 4;\n" \
			"		return true;\n" \
			"	elseif (pModel.isRequesting) then return true; end\n\n"
		);
	}

	file->Printf(
		"	pModel.numStream = pModel.numStream - 1;\n\n" \
		"	if (pModel.numStream == 0) then\n"
	);

	if ( streamMethod == STREAM_STREAM )
	{
		file->Printf(
			"		pModel.isRequesting = 3;\n\n"
		);
	}

	file->Printf(
		"		freeModel(pModel);\n"
	);

	if ( streamMethod == STREAM_STREAM )
	{
		file->Printf(
			"\n" \
			"		pModel.isRequesting = false;\n"
		);
	}

	file->Printf(
		"	end\n" \
		"end\n\n" \
		"local function modelStreamIn ()\n" \
		"	local pModel = pModels[getElementModel(source)];\n\n" \
		"	if not (pModel) then return end;\n\n"
	);

	if ( lodSupport )
	{
		file->Printf(
			"	if (pModel.lodID) then return true; end\n\n"
		);
	}

	if ( streamMethod == STREAM_STREAM )
	{
		file->Printf(
			"	if (pModel.isRequesting == 1) then\n" \
			"		pModel.isRequesting = false;\n" \
			"		return true;\n" \
			"	elseif (pModel.isRequesting) then return true; end\n\n"
		);

		if ( cached && runtimeCorrection )
		{
			file->Printf(
				"	pModel.requestFrames = %u;\n", runtimeCorrection
			);
		}
	}

	file->Printf(
		"	pModel.numStream = pModel.numStream + 1;\n\n" \
		"	if not (pModel.numStream == 1) then return true; end\n\n"
	);

	if ( streamMethod == STREAM_STREAM )
	{
		file->Printf(
			"	pModel.isRequesting = 2;\n\n"
		);
	}

	file->Printf(
		"	if not (loadModel(pModel)) then\n" \
		"		setElementInterior(source, 123);\n" \
		"		setElementCollisionsEnabled(source, false);\n" \
		"	end\n"
	);

	if ( streamMethod == STREAM_STREAM )
	{
		file->Printf(
			"\n" \
			"	if (pModel.isRequesting == 2) then\n" \
			"		outputDebugString(\"invalid model request '\"..pModel.name..\"'\");\n" \
			"		freeModel(pModel);\n" \
			"		pModel.isRequesting = false;\n\n" \
			"		pModel.numStream = 0;\n" \
			"	end\n"
		);
	}

	file->Printf(
		"end\n\n"
	);
}

static inline void luaModelBeginLoader( CFile *file )
{
	file->Printf(
		"function loadModels ()\n" \
		"	local pModel, pTXD, pColl, pTable;\n\n"
	);
}

static inline void luaModelLoadBegin( CFile *file )
{
	if (!staticCompile)
	{
		file->Printf(
			"	pTable={\n"
		);
	}
}

static inline void luaModelLoadEntry( CFile *file, const char *name, const char *txdName, unsigned short id, const char *lod, unsigned short lodID )
{
	if (staticCompile)
	{
		file->Printf(
			"	pModels[%u]={};\n" \
			"	pTable=pModels[%u];\n" \
			"	pTable.txd=requestTexture(pTable, \"textures/%s.txd\");\n" \
			"	engineImportTXD(pTable.txd, %u);\n", id, id, txdName, id
		);

		if ( cached )
		{
			file->Printf(
				"	pTable.model_file=\"models/%s.dff\";\n" \
				"	pTable.col_file=\"coll/%s.col\";\n", name, name
			);
		}
		else
		{
			file->Printf(
				"	pTable.model=engineLoadDFF(\"models/%s.dff\", %u);\n" \
				"	pTable.col=engineLoadCOL(\"coll/%s.col\");\n", name, id, name
			);
		}

		file->Printf(
			"	pTable.numStream=0;\n" \
			"	pTable.lod=%s;\n" \
			"	pTable.id=%u;\n" \
			"	engineSetModelLODDistance(%u, %s);\n", lod, id, id, lod
		);

		if ( lodSupport && lodID != 0 )
		{
			file->Printf(
				"\n" \
				"	for m,n in ipairs(getElementsByType(\"object\", resourceRoot)) do\n" \
				"		local x, y, z = getElementPosition(n);\n" \
				"		local rx, ry, rz = getElementRotation(n);\n" \
				"		setLowLODElement(n, createObject(%u, x, y, z, rx, ry, rz, true));\n" \
				"	end\n\n", lodID
			);
		}
	}
	else
	{
		if ( tableCount++ != 0 )
			file->Printf( ",\n" );

		file->Printf(
			"		{ model=%u, model_file=\"%s\", txd_file=\"%s\", coll_file=\"%s\", lod=%s", id, name, txdName, name, lod
		);

		if ( lodSupport && lodID != 0 )
			file->Printf( ", lodID=%u }", lodID );
		else
			file->Printf( " }" );
	}
}

static inline void luaModelLODBegin( CFile *file )
{
	if ( !staticCompile )
	{
		file->Printf(
			"	pTable={\n"
		);
	}
}

static inline void luaModelLODEntry( CFile *file, unsigned short id, const char *name, const char *colName, const char *txdName, const char *lod, unsigned short super )
{
	if ( !staticCompile )
	{
		if ( tableCount++ != 0 )
			file->Printf( ",\n" );

		file->Printf(
			"		{ %u, \"%s\", \"%s\", %s", id, name, txdName, lod
		);

		if ( super != 0 )
		{
			file->Printf( ", %u", super );
		}

		if ( colName )
		{
			file->Printf(
				", \"%s\" }", colName
			);
		}
		else
		{
			file->Printf(
				" }"
			);
		}
	}
	else
	{
		file->Printf(
			"	pModels[%u] = {\n" \
			"		id = %u,\n" \
			"		name = %s,\n" \
			"		txd = requestTexture(false, \"textures/%s.txd\"),\n", id, id, name, txdName
		);

		file->Printf(
			"		numStream = 0,\n" \
			"		lod = %s\n", lod
		);

		if ( super != 0 )
		{
			file->Printf(
				",\n        super = %u", super
			);
		}

		file->Printf(
			"	};\n" \
			"	pModelEntry = pModels[%u];\n", id
		);

		if ( cached )
		{
			if ( colName )
			{
				file->Printf(
					"	pModelEntry.col_file = \"coll/%s.col\";\n", colName
				);
			}

			file->Printf(
				"	pModelEntry.model_file = \"models/%s.dff\";\n", name
			);
		}
		else
		{
			file->Printf(
				"	engineImportTXD(pModelEntry.txd, %u);\n" \
				"	pModelEntry.model = engineLoadDFF(\"models/%s.dff\", %u);\n" \
				"	engineReplaceModel(pModelEntry.model, %u);\n", id, name, id, id
			);

			if ( colName )
			{
				file->Printf(
					"	pModelEntry.col = engineLoadCOL(\"coll/%s.col\");\n" \
					"	engineReplaceCOL(pModelEntry.col, %u);\n", colName, id
				);
			}
		}

		file->Printf(
			"	engineSetModelLODDistance(%u, %s / 5);\n", id, lod
		);
	}
}

static inline void luaModelLODEnd( CFile *file )
{
	if ( !staticCompile )
	{
		file->Printf(
			"\n" \
			"	};\n\n" \
			"	for m,n in ipairs(pTable) do\n" \
			"		pModels[n[1]] = {\n" \
			"			id = n[1],\n" \
			"			name = n[2],\n" \
			"			txd = requestTexture(false, \"textures/\"..n[3]..\".txd\"),\n" \
			"			numStream = 0,\n" \
			"			lod = n[4] / 5,\n" \
			"			super = n[5]\n" \
			"		};\n" \
			"		pModelEntry = pModels[n[1]];\n"
		);

		if ( cached )
		{
			file->Printf(
				"		pModelEntry.model_file = \"models/\"..n[2]..\".dff\";\n"
			);

			if ( !forceLODInherit )
			{
				file->Printf(
					"		if ( #n == 6 ) then\n" \
					"			pModelEntry.col_file = \"coll/\"..n[6]..\".col\";\n" \
					"		end\n"
				);
			}
		}
		else
		{
			file->Printf(
				"		engineImportTXD(pModelEntry.txd, n[1]);\n" \
				"		pModelEntry.model = engineLoadDFF(\"models/\"..n[2]..\".dff\", n[1]);\n" \
				"		engineReplaceModel(pModelEntry.model, n[1]);\n" \
				"		if ( #n == 6 ) then\n" \
				"			pModelEntry.col = engineLoadCOL(\"coll/\"..n[6]..\".col\");\n" \
				"			engineReplaceCOL(pModelEntry.col, n[1]);\n" \
				"		end\n"
			);
		}

		file->Printf(
			"		engineSetModelLODDistance(n[1], n[4] / 5);\n" \
			"	end\n"
		);
	}

	file->Printf( "\n" );

	tableCount = 0;
}

static inline void luaModelLoadEnd( CFile *file )
{
	if (!staticCompile)
	{
		file->Printf(
			"\n" \
			"	};\n\n" \
			"	local n,m;\n\n" \
			"	for n,m in ipairs(pTable) do\n" \
			"		pModels[m.model] = {\n" \
			"			id = m.model,\n" \
			"			name = m.model_file,\n" \
			"			txd = requestTexture(false, \"textures/\"..m.txd_file..\".txd\"),\n" \
			"			numStream = 0,\n" \
			"			lod = m.lod,\n" \
			"			lodID = m.lodID\n" \
			"		};\n" \
			"		pModelEntry = pModels[m.model];\n" \
		);

		if ( cached )
		{
			file->Printf(
				"		pModelEntry.model_file=\"models/\"..m.model_file..\".dff\";\n" \
				"		pModelEntry.col_file=\"coll/\"..m.coll_file..\".col\";\n"
			);
		}
		else
		{
			file->Printf(
				"		engineImportTXD(pModelEntry.txd, m.model);\n" \
				"		pModelEntry.model=engineLoadDFF(\"models/\"..m.model_file..\".dff\", m.model);\n" \
				"		pModelEntry.col=engineLoadCOL(\"coll/\"..m.coll_file..\".col\");\n"
			);
		}

		file->Printf(
			"		engineSetModelLODDistance(m.model, m.lod);\n\n"
		);

		if ( lodSupport )
		{
			file->Printf(
				"		if (m.lodID) then\n" \
				"			for j,k in ipairs(getElementsByType(\"object\", resourceRoot)) do\n" \
				"				if (getElementModel(k) == m.model) then\n" \
				"					local x, y, z = getElementPosition(k);\n" \
				"					local rx, ry, rz = getElementRotation(k);\n" \
				"					setLowLODElement(k, createObject(m.lodID, x, y, z, rx, ry, rz, true));\n" \
				"				end\n" \
				"			end\n" \
				"		end\n"
			);
		}

		file->Printf(
			"	end\n"
		);
	}

	if ( !cached )
	{
		file->Printf(
			"\n" \
			"	for m,n in pairs(pModels) do\n" \
			"		if (n.super) and not (n.col) then\n" \
			"			n.col = pModels[n.super].col;\n" \
			"		end\n" \
			"	end\n"
		);
	}

	file->Printf(
		"end\n" \
		"loadModels();\n\n"
	);
}

static inline void luaMethodEnd( CFile *file )
{
	switch( streamMethod )
	{
	case STREAM_DISTANCE:
		file->Printf(
			"addEventHandler(\"onClientPreRender\", root, function()\n" \
			"		local m,n;\n" \
			"		local objects = getElementsByType(\"object\", resourceRoot);\n" \
			"		local x, y, z = getCameraMatrix();\n\n" \
			"		for m,n in ipairs(objects) do\n" \
			"			local model = pModels[getElementModel(n)];\n\n" \
			"			if (model) then\n" \
			"				local streamObject = streamedObjects[n];\n" \
			"				local distance = getDistanceBetweenPoints3D(x, y, z, getElementPosition(n));\n\n" \
			"				if not (streamObject) then\n" \
			"					if (distance < model.lod) then\n" \
			"						source = n;\n\n" \
			"						modelStreamIn();\n\n" \
			"						streamedObjects[n] = true;\n" \
			"					end\n" \
			"				elseif (distance > model.lod) then\n" \
			"					source = n;\n\n" \
			"					modelStreamOut();\n\n" \
			"					streamedObjects[n] = nil;\n" \
			"				end\n" \
			"			end\n" \
			"		end\n" \
			"	end\n" \
			");\n\n"
		);
		return;
	case STREAM_STREAM:
		if ( cached && runtimeCorrection )
		{
			file->Printf(
				"addEventHandler(\"onClientPreRender\", root, function()\n" \
				"		for m,n in pairs(pModels) do\n" \
				"			if (n.isRequesting) then\n" \
				"				n.requestFrames = n.requestFrames - 1;\n\n" \
				"				if (n.requestFrames == 0) then\n" \
				"					n.isRequesting = false;\n"
			);

			if ( debug )
			{
				file->Printf(
					"					outputDebugString(\"model '\" .. n.name ..\"' timed out\");\n"
				);
			}

			file->Printf(
				"					freeModel(n);\n" \
				"				end\n" \
				"			end\n" \
				"		end\n" \
				"	end\n" \
				");\n\n"
			);
		}

		file->Printf(
			"for m,n in ipairs(getElementsByType(\"object\", resourceRoot)) do\n" \
			"	if (isElementStreamedIn(n)) then\n" \
			"		source = n;\n" \
			"		modelStreamIn();\n" \
			"	end\n" \
			"end\n\n" \
			"addEventHandler(\"onClientElementStreamIn\", resourceRoot, function()\n" \
			"		modelStreamIn();\n" \
			"	end\n" \
			");\n" \
			"addEventHandler(\"onClientElementStreamOut\", resourceRoot, function()\n" \
			"		modelStreamOut();\n" \
			"	end\n" \
			");\n\n"
		);
		return;
	}
}

static inline void luaEnd( CFile *file )
{
	if ( cached )
	{
		file->Printf(
			"local function getBiggestCacheObject()\n" \
			"	local obj;\n" \
			"	local size = 0;\n\n" \
			"	for m,n in pairs(cached) do\n" \
			"		if (size < n.size) and not (n.model.loaded) then\n" \
			"			size = n.size;\n" \
			"			obj = n.model;\n" \
			"		end\n" \
			"	end\n" \
			"	return obj, size;\n" \
			"end\n\n" \
			"addEventHandler(\"onClientRender\", root, function()\n" \
			"		local _,request = ({ pairs(resourceQueue) })[1]( resourceQueue );\n" \
			"		if not (request) then return true; end;\n\n" \
			"		while ( streamerMemory + request.size > %u ) do\n" \
			"			local cacheObj = getBiggestCacheObject();\n" \
			"			if not (cacheObj) then\n" \
			"				outputDebugString(\"waiting for cache objects...\");\n" \
			"				return true;\n" \
			"			end\n\n" \
			"			freeResources(cacheObj);\n" \
			"		end\n" \
			"		resourceQueue[request.model.id] = nil;\n\n" \
			"		local model = request.model;\n\n", streamerMemory
		);

		if ( streamMethod == STREAM_STREAM )
		{
			file->Printf(
				"		if not ( model.numStream == 0 ) then\n" \
				"			model.isRequesting = 2;\n" \
				"		end\n\n"
			);
		}

		file->Printf(
			"		if not (loadModel(model)) then return false; end\n\n"
		);

		if ( streamMethod == STREAM_STREAM )
		{
			file->Printf(
				"		if ( model.isRequesting == 2 ) then\n" \
				"			outputDebugString(\"invalid model request '\" .. model.name ..\"'\");\n" \
				"			freeModel(model);\n" \
				"			model.isRequesting = false;\n\n" \
				"			model.numStream = 0;\n" \
				"		else\n" \
				"			for m,n in ipairs(getElementsByType(\"object\", resourceRoot)) do\n" \
				"				if (getElementModel(n) == model.id) then\n" \
				"					setElementInterior(n, 0);\n" \
				"					setElementCollisionsEnabled(n, true);\n" \
				"				end\n" \
				"			end\n" \
				"		end\n\n"
			);
		}
		else
		{
			file->Printf(
				"		for m,n in ipairs(getElementsByType(\"object\", resourceRoot)) do\n" \
				"			if (getElementModel(n) == model.id) then\n" \
				"				setElementInterior(n, 0);\n" \
				"				setElementCollisionsEnabled(n, true);\n" \
				"			end\n" \
				"		end\n"
			);
		}

		file->Printf(
			"	end\n" \
			");\n\n" \
			"addCommandHandler(\"mcollect\", function()\n" \
			"		outputDebugString(\"collecting map garbage...\");\n\n" \
			"		for m,n in pairs(cached) do\n" \
			"			if (n.model.loaded == false) then\n" \
			"				freeResources(n.model);\n" \
			"			end\n" \
			"		end\n" \
			"		collectgarbage();\n" \
			"	end\n" \
			");\n\n" \
			"addCommandHandler(\"mapmem\", function()\n" \
			"		outputChatBox(\"streamer memory: \"..streamerMemory..\" out of %u\");\n" \
			"	end\n" \
			");\n\n", streamerMemory, streamerMemory
		);
	}
	else
	{
		file->Printf(
			"addEventHandler(\"onClientResourceStop\", resourceRoot, function()\n" \
			"		for m,n in pairs(pModels) do\n" \
			"			if (isElement(n.col)) then destroyElement(n.col); end\n" \
			"			if (isElement(n.model)) then destroyElement(n.model); end\n" \
			"			engineRestoreCOL(m);\n" \
			"			engineRestoreModel(m);\n" \
			"		end\n" \
			"	end\n" \
			");\n\n"
		);
	}

	if ( debug )
	{
		file->Printf(
			"addEventHandler(\"onClientRender\", root, function()\n" \
			"		local screenX, screenY = guiGetScreenSize();\n"
		);

		if ( cached )
		{
			file->Printf(
				"		local perc = streamerMemory / %u;\n\n" \
				"		dxDrawText(\"streamerMemory: \" .. streamerMemory .. \" / %u (\" .. math.floor(perc * 100) .. \")\", screenX - 300, screenY - 100);\n" \
				"		dxDrawRectangle(screenX - 300, screenY - 80, 225, 40);\n" \
				"		dxDrawRectangle(screenX - 298, screenY - 78, 221, 36, tocolor(0, 0, 0, 255));\n" \
				"		dxDrawRectangle(screenX - 297, screenY - 77, 219 * perc, 34);\n", streamerMemory, streamerMemory
			);
		}

		file->Printf(
			"		dxDrawText(\"Models loaded: \" .. #modelsLoaded, screenX - 300, screenY - 120);\n" \
			"		for m,n in ipairs(modelsLoaded) do\n" \
			"			dxDrawText(n.name .. \" (\" .. n.numStream .. \", \" .. tostring(n.isRequesting) .. \")\", 50, 160 + m * 15);\n" \
			"		end\n" \
			"	end\n" \
			");\n\n" \
			"addEventHandler(\"onClientClick\", root, function(button, state, c, d, e, f, g, element)\n" \
			"		if not (button == \"left\") or not (state == \"down\") then return end\n\n" \
			"		if not (element) then return end\n\n" \
			"		local model = pModels[getElementModel(element)];\n\n" \
			"		if not (model) then return end\n\n" \
			"		outputChatBox(\"name: \" .. model.name .. \" (\" .. model.id .. \")\");\n" \
			"	end\n" \
			");\n\n" \
		);
	}

	file->Printf(
		"collectgarbage();\n"
	);

	if ( jetpackHeight != 0 )
	{
		file->Printf(
			"setJetpackMaxHeight(%u);\n", usZoffset + jetpackHeight
		);
	}
}

static const char *pServerHeader=
	"-- Automatically generated server script by MTASA map IPL map converter\n" \
	"setMapName(\"United\");\n";
#if (MAP_METHOD==MAP_LUA)
static const char *pServerMapHeader=
	"function loadMap ()\n" \
	"	local pObj;\n\n";
static const char *pServerMapEntry=
	"	pObj=createObject(%u,%f,%f,%f);\n" \
	"	setElementID(pObj,\"%s\");\n";
static const char *pServerMapEntry2=
	"	setObjectRotation(pObj,%u,%u,%u);\n";
static const char *pServerMapEnd=
	"end;\n" \
	"loadMap();\n";
#endif
static const char *pServerEnd=
	"-- The end\n";
#if (MAP_METHOD==MAP_XML)
static const char *pMapHeader=
	"<map>\n";
static const char *pMapEntry=
	"	<object id=\"%s\" dimension=\"0\" interior=\"0\" model=\"%u\" posX=\"%f\" posY=\"%f\" posZ=\"%f\" ";
static const char *pMapEntry2=
	"rotX=\"%u\" rotY=\"%u\" rotZ=\"%u\" />\n";
static const char *pMapEnd=
	"</map>\n";
#endif
static const char *pMetaHeader=
	"<meta>\n" \
	"	<info author=\"The_GTA\" description=\"Automatically generated map file by MTASA map converter\" version=\"1.2\" type=\"map\" />\n";
#if (MAP_METHOD==MAP_XML)
static const char *pMetaHeaderMap=
	"	<map src=\"gta3.map\" />\n";
#endif
static const char *pMetaHeader2=
	"	<script src=\"%s\" type=\"client\" />\n" \
	"	<script src=\"main_server.lua\" type=\"server\" />\n";
static const char *pMetaEnd=
	"</meta>\n";


static const char *txdNames[65536];
static const char *txdName;
static const char *colName;
static unsigned short usTxdNames = 0;
static char lodBuffer[128];
static CFile *pMetaFile = NULL;

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

	if ( !lod || !forceLODInherit )
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

bool bundleForBLUE( CINI *config )
{
	CFile *pLuaFile;
	CFile *pMapFile;
	CFile *pLuaServer;
	CINI::Entry *mainEntry;
	instanceList_t::iterator iter;

	printf( "Mode: MTA:BLUE\n" );

	if (config && (mainEntry = config->GetEntry("MainBLUE")))
	{
		// Apply configuration
		jetpackHeight = (unsigned int)mainEntry->GetInt("jetpackHeight");
		forceLODInherit = mainEntry->GetBool("forceLODInherit");
		staticCompile = mainEntry->GetBool("static");
		cached = mainEntry->GetBool("cached");
		streamerMemory = (unsigned int)mainEntry->GetInt("streamerMemory") * 1024 * 1024;
		autoCollect = mainEntry->GetBool("autoCollect");
		runtimeCorrection = (unsigned int)mainEntry->GetInt("runtimeCorrection");

		const char *method = mainEntry->Get("method");

		if ( strcmp(method, "stream") == 0 )
		{
			streamMethod = STREAM_STREAM;

			printf( "WARNING: This method might not work with asynchronous loading\n" );
		}
		else if ( strcmp(method, "distance") == 0 )
			streamMethod = STREAM_DISTANCE;
		else
		{
			printf( "Invalid streaming method; defaulting to stream\n" );

			streamMethod = STREAM_STREAM;

			goto nonotify;
		}

		printf( "Using StreamingMethod: %s\n", method );
	}
	else
	{
		// Default disabled
		jetpackHeight = 1000;
		forceLODInherit = true;
		staticCompile = false;
		cached = false;
		autoCollect = false;
		runtimeCorrection = 0;
	}

nonotify:
	// Create the .lua file
	pLuaFile=g_outputRoot->Open("script.lua","w");
	pMetaFile=g_outputRoot->Open("meta.xml","w");
#if (MAP_METHOD==MAP_XML)
	pMapFile=g_outputRoot->Open("gta3.map","w");
#endif
	pLuaServer=g_outputRoot->Open("main_server.lua","w");

	// Init files first
	luaBegin( pLuaFile );
	pMetaFile->Printf("%s", pMetaHeader);
	pLuaServer->Printf("%s", pServerHeader);
#if (MAP_METHOD==MAP_XML)
	pMetaFile->Printf("%s", pMetaHeaderMap);
	pMapFile->Write(pMapHeader,1,strlen(pMapHeader));
#elif (MAP_METHOD==MAP_LUA)
	fprintf(pLuaServer,pServerMapHeader);
#endif

	luaMethodBegin( pLuaFile );

	luaModelBeginLoader( pLuaFile );

	if ( doCompile )
		pMetaFile->Printf(pMetaHeader2, "script.luac");
	else
		pMetaFile->Printf(pMetaHeader2, "script.lua");

	if ( lodSupport )
	{
		objectList_t::iterator objIter;

		// LODs are loaded and stay this way
		luaModelLODBegin( pLuaFile );

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

			lodMap_t::const_iterator lodMapIter = lodMap.find( (*objIter)->m_modelID );
			CObject *obj = lodMapIter != lodMap.end() ? GetObjectByModel( lodMapIter->second->m_name ) : NULL;

			luaModelLODEntry( pLuaFile,
				(*objIter)->m_realModelID,
				(*objIter)->m_modelName,
				colName,
				(*objIter)->m_textureName,
				lodBuffer,
				obj ? obj->m_realModelID : 0
			);
		}

		luaModelLODEnd( pLuaFile );
	}

	luaModelLoadBegin( pLuaFile );

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
#if (MAP_METHOD==MAP_XML)
			// We add all map entries
			pMapFile->Printf(pMapEntry, name, modelIDs[(*iter)->m_modelID], (*iter)->m_position[0] + usXoffset, (*iter)->m_position[1] + usYoffset, (*iter)->m_position[2] + usZoffset);
			pMapFile->Printf(pMapEntry2, (int)(*iter)->m_rotation[0], (int)(*iter)->m_rotation[1], (int)(*iter)->m_rotation[2]);
#elif (MAP_METHOD==MAP_LUA)
			// Yup, we script our elements
			pLuaServer->Printf(pServerMapEntry, name, modelIDs[(*iter)->m_modelID], (*iter)->m_position[0] + usXoffset, (*iter)->m_position[1] + usYoffset, (*iter)->m_position[2] + usZoffset);
			pLuaServer->Printf(pServerMapEntry2, (int)(*iter)->m_rotation[0], (int)(*iter)->m_rotation[1], (int)(*iter)->m_rotation[2]);
#endif
			continue;
		}

		if ( !AllocateResources( name, false ) )
			continue;

#if (MAP_METHOD==MAP_XML)
		// We add all map entries
		pMapFile->Printf(pMapEntry, name, modelIDs[(*iter)->m_modelID], (*iter)->m_position[0] + usXoffset, (*iter)->m_position[1] + usYoffset, (*iter)->m_position[2] + usZoffset);
		pMapFile->Printf(pMapEntry2, (int)(*iter)->m_rotation[0], (int)(*iter)->m_rotation[1], (int)(*iter)->m_rotation[2]);
#elif (MAP_METHOD==MAP_LUA)
		// Yup, we script our elements
		pLuaServer->Printf(pServerMapEntry, name, modelIDs[(*iter)->m_modelID], (*iter)->m_position[0] + usXoffset, (*iter)->m_position[1] + usYoffset, (*iter)->m_position[2] + usZoffset);
		pLuaServer->Printf(pServerMapEntry2, (int)(*iter)->m_rotation[0], (int)(*iter)->m_rotation[1], (int)(*iter)->m_rotation[2]);
#endif

		unsigned short lod = 0;

		if ( CObject *lodObj = backLodMap[(*iter)->m_modelID] )
			lod = lodObj->m_realModelID;

		// Now LUA
		luaModelLoadEntry( pLuaFile, name, txdName, modelIDs[(*iter)->m_modelID], lodBuffer, lod );
	}

	luaModelLoadEnd( pLuaFile );

#if (MAP_METHOD==MAP_XML)
	pMapFile->Printf("%s", pMapEnd);
#elif (MAP_METHOD==MAP_LUA)
	pLuaServer->Printf(pServerMapEnd);
#endif

	luaMethodEnd( pLuaFile );

	pLuaServer->Printf("%s", pServerEnd);
	pMetaFile->Printf("%s", pMetaEnd);
	luaEnd( pLuaFile );
	// Close em
	delete pLuaFile;
	delete pMetaFile;
#if (MAP_METHOD==MAP_XML)
	delete pMapFile;
#endif

	if ( doCompile )
	{
#ifdef _WIN32
		system("luac5.1.exe -s -o output/script.luac output/script.lua");
#endif //OS DEPENDANT CODE

		g_outputRoot->Delete( "script.lua" );
	}

	return true;
}
