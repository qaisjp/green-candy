#include "../StdInc.h"
#include "../resources.h"

enum eStreamMethod
{
	STREAM_STREAM
};

static bool applyWorldConfig;
static unsigned int jetpackHeight;
static bool staticCompile;
static bool cached;
static unsigned int streamerMemory;
static bool autoCollect;
static eStreamMethod streamMethod;
static bool optimizeEngine;

static inline void luaBegin( CFile *file )
{
	file->Printf(
		"%s\n", GetGenericScriptHeader()
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
		"local pModels = {};\n\n"
	);

    if ( applyWorldConfig )
    {
        file->Printf(
            "setCloudsEnabled(false);\n"
        );
    }

	if ( applyWorldConfig && !lodSupport )
	{
		file->Printf(
			"setFarClipDistance(350);\n" \
			"setFogDistance(10);\n"
		);
	}

    if ( optimizeEngine )
    {
        file->Printf(
            "engineStreamingSetProperty( \"infiniteStreaming\", true );\n" \
            "engineStreamingSetProperty( \"nodeStealing\", false );\n" \
            "engineSetAsynchronousLoadingEnabled( true );\n"
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
			"    if not (model.impTxd) then\n" \
			"        engineImportTXD(model.txd, model.id);\n\n" \
			"        model.impTxd = true;\n" \
			"    end\n\n" \
            "    if not (model.model) then\n" \
			"        model.model = engineLoadDFF(model.model_file, model.id, true);\n\n" \
			"        if not (model.model) then return false; end;\n\n" \
            "    end\n\n"
		);

		if ( lodSupport )
		{
			file->Printf(
				"    if not (model.col_file) then\n" \
				"        if not (model.super) then return false; end\n\n" \
                "        local superModel = pModels[model.super];\n\n" \
                "        if (superModel) then\n" \
				"            model.col = superModel.col;\n" \
                "        end\n" \
				"    else\n" \
				"        model.col = engineLoadCOL(model.col_file, 0);\n\n" \
				"        if not (model.col) then return false; end\n" \
				"    end\n\n"
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
			"    if (model.super) then\n" \
            "        local superModel = pModels[model.super];\n\n" \
            "        if (superModel) then\n" \
			"            if not (loadModel(superModel)) then\n" \
			"                return false;\n" \
            "            end\n" \
			"        end\n" \
			"    end\n\n"
		);
	}

	if ( cached )
	{
		file->Printf(
			"	if not (cacheResources(model)) then return false; end\n\n"
		);
	}

	file->Printf(
        "    if not (model.model) or not (model.col) then return false; end\n\n" \
        "    local successModelReplace = engineReplaceModel(model.model, model.id);\n" \
        "    local successCOLReplace = false;\n\n" \
        "    if (successModelReplace) then\n" \
        "        successCOLReplace = engineReplaceCOL(model.col, model.id);\n" \
        "    end\n\n" \
        "    if not (successModelReplace) or not (successCOLReplace) then\n" \
        "        if (successModelReplace) then\n" \
        "            engineRestoreModel(model.id);\n" \
        "        end\n\n" \
        "        if (successCOLReplace) then\n" \
        "            engineRestoreCOL(model.id);\n" \
        "        end\n" \
        "        return false;\n" \
        "    end\n\n" \
        "    model.loaded = true;\n"
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
		"local function modelStreamOut (id)\n" \
		"	local pModel = pModels[id];\n\n" \
		"	if not (pModel) then return end;\n\n"
	);

	if ( lodSupport )
	{
		file->Printf(
			"	if (pModel.lodID) then return true; end\n\n"
		);
	}

	file->Printf(
		"	freeModel(pModel);\n" \
		"end\n\n" \
		"local function modelStreamIn (id)\n" \
		"	local pModel = pModels[id];\n\n" \
		"	if not (pModel) then return end;\n\n"
	);

	if ( lodSupport )
	{
		file->Printf(
			"	if (pModel.lodID) then return true; end\n\n"
		);
	}

	file->Printf(
		"    if not (loadModel(pModel)) then\n" \
        "        for m,n in ipairs(getElementsByType(\"object\", resourceRoot)) do\n" \
        "            if (getElementModel(n) == pModel.id) then\n" \
        "                setElementInterior(n, 255);\n" \
        "                setElementCollisionsEnabled(n, false);\n" \
        "            end\n" \
        "        end\n" \
        "    else\n" \
        "        for m,n in ipairs(getElementsByType(\"object\", resourceRoot)) do\n" \
        "            if (getElementModel(n) == pModel.id) then\n" \
        "                setElementInterior(n, 0);\n" \
        "                setElementCollisionsEnabled(n, true);\n" \
        "            end\n" \
        "        end\n" \
        "   end\n" \
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
				"	pTable.model=engineLoadDFF(\"models/%s.dff\", %u, true);\n" \
				"	pTable.col=engineLoadCOL(\"coll/%s.col\");\n", name, id, name
			);
		}

		file->Printf(
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
				"	end\n", lodID
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
				"	pModelEntry.model = engineLoadDFF(\"models/%s.dff\", %u, true);\n" \
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
				"		pModelEntry.model = engineLoadDFF(\"models/\"..n[2]..\".dff\", n[1], true);\n" \
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
				"		pModelEntry.model=engineLoadDFF(\"models/\"..m.model_file..\".dff\", m.model, true);\n" \
				"		pModelEntry.col=engineLoadCOL(\"coll/\"..m.coll_file..\".col\");\n"
			);
		}

		file->Printf(
			"		engineSetModelLODDistance(m.model, m.lod);\n"
		);

		if ( lodSupport )
		{
			file->Printf(
                "\n" \
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
	case STREAM_STREAM:
		file->Printf(
			"for m,n in pairs(pModels) do\n" \
			"	if (engineIsModelBeingUsed(m)) then\n" \
			"		modelStreamIn(m);\n" \
			"	end\n" \
			"end\n\n" \
			"addEventHandler(\"onClientModelRequest\", root, function(id)\n" \
			"		modelStreamIn(id);\n" \
			"	end\n" \
			");\n" \
			"addEventHandler(\"onClientModelFree\", root, function(id)\n" \
			"		modelStreamOut(id);\n" \
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

		file->Printf(
			"		if not (loadModel(model)) then return false; end\n\n"
		);

		file->Printf(
			"		for m,n in ipairs(getElementsByType(\"object\", resourceRoot)) do\n" \
			"			if (getElementModel(n) == model.id) then\n" \
			"				setElementInterior(n, 0);\n" \
			"				setElementCollisionsEnabled(n, true);\n" \
			"			end\n" \
			"		end\n"
		);

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
			"        for m,n in pairs(pModels) do\n" \
            "            freeModel(n);\n" \
			"            if (isElement(n.col)) then destroyElement(n.col); end\n" \
			"            if (isElement(n.model)) then destroyElement(n.model); end\n" \
			"        end\n" \
			"    end, false\n" \
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
			"			dxDrawText(n.name, 50, 160 + m * 15);\n" \
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
		"collectgarbage(\"collect\");\n"
	);

	if ( applyWorldConfig && jetpackHeight != 0 )
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
	"	<info author=\"%s\" description=\"Automatically generated map file by MTASA map converter\" version=\"1.2\" type=\"map\" />\n";
#if (MAP_METHOD==MAP_XML)
static const char *pMetaHeaderMap=
	"	<map src=\"gta3.map\" />\n";
#endif
static const char *pMetaHeader2=
	"	<script src=\"%s\" type=\"client\" />\n" \
	"	<script src=\"main_server.lua\" type=\"server\" />\n";
static const char *pMetaEnd=
	"</meta>\n";


static CFile *pMetaFile = NULL;

struct eirforkResourceDispatch
{
    inline bool requireCollision( const char *name, bool lod )
    {
        return !lod || !forceLODInherit;
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
typedef ResourceManager <eirforkResourceDispatch> eirforkResourceManager;

bool bundleForEIRFORK( CINI *config )
{
	CFile *pLuaFile;
	CFile *pMapFile;
	CFile *pLuaServer;
	CINI::Entry *mainEntry;
	instanceList_t::iterator iter;

    // Instance of resource allocation.
    eirforkResourceDispatch dispatch;

    eirforkResourceManager resManager( 0, dispatch );

	printf( "Mode: EIRFORK\n" );

	if (config && (mainEntry = config->GetEntry("MainEIRFORK")))
	{
		// Apply configuration
        applyWorldConfig = mainEntry->GetBool("applyWorldConfig");
		jetpackHeight = (unsigned int)mainEntry->GetInt("jetpackHeight");
		forceLODInherit = mainEntry->GetBool("forceLODInherit");
		staticCompile = mainEntry->GetBool("static");
		cached = mainEntry->GetBool("cached");
		streamerMemory = (unsigned int)mainEntry->GetInt("streamerMemory") * 1024 * 1024;
		autoCollect = mainEntry->GetBool("autoCollect");
        optimizeEngine = mainEntry->GetBool("optimizeEngine");

		const char *method = mainEntry->Get("method");

		if ( strcmp(method, "stream") == 0 )
		{
			streamMethod = STREAM_STREAM;
		}
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
        applyWorldConfig = true;
		jetpackHeight = 1000;
		forceLODInherit = true;
		staticCompile = false;
		cached = false;
		autoCollect = false;
        optimizeEngine = true;
	}

nonotify:
	// Create the .lua file
	pLuaFile = resManager.outputRoot->Open("script.lua","w");
	pMetaFile = resManager.outputRoot->Open("meta.xml","w");
#if (MAP_METHOD==MAP_XML)
	pMapFile = resManager.outputRoot->Open("gta3.map","w");
#endif
	pLuaServer = resManager.outputRoot->Open("main_server.lua","w");

	// Init files first
	luaBegin( pLuaFile );
	pMetaFile->Printf(pMetaHeader, GetCompilatorName());
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
			const char *name = (*objIter)->m_modelName;

			if ( g_usedModelNames.Exists( name ) )
				continue;

			if ( !resManager.AllocateResources( name, true ) )
				continue;

			_snprintf( resManager.lodBuffer, 127, "%.0f", (*objIter)->m_drawDistance );

			lodMap_t::const_iterator lodMapIter = lodMap.find( (*objIter)->m_modelID );
			CObject *obj = lodMapIter != lodMap.end() ? GetObjectByModel( lodMapIter->second->m_name ) : NULL;

			luaModelLODEntry( pLuaFile,
				(*objIter)->m_realModelID,
				name,
				resManager.colName,
				(*objIter)->m_textureName,
				resManager.lodBuffer,
				obj ? obj->m_realModelID : 0
			);
		}

		luaModelLODEnd( pLuaFile );
	}

	luaModelLoadBegin( pLuaFile );

	for (iter = instances.begin(); iter != instances.end(); iter++)
	{
		const char *name = (*iter)->m_name;

		if ( lodSupport && lodMap[(*iter)->m_modelID] )
			continue;

		if ( g_usedModelNames.Exists( name ) )
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

		if ( !resManager.AllocateResources( name, false ) )
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
		luaModelLoadEntry( pLuaFile, name, resManager.txdName, modelIDs[(*iter)->m_modelID], resManager.lodBuffer, lod );
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
    delete pLuaServer;

	if ( doCompile )
	{
#ifdef _WIN32
		system("luac5.1.exe -s -o output/script.luac output/script.lua");
#endif //OS DEPENDANT CODE

		resManager.outputRoot->Delete( "script.lua" );
	}

	return true;
}
