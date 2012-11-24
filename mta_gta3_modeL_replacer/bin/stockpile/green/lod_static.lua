local textureCache = {};
local pModels = {};

setCloudsEnabled(false);
debug.sethook(nil);

setFogDistance(800);
setFarClipDistance(1500);

local function getResourceSize(path)
	if not (fileExists(path)) then return 0; end;

	local file = fileOpen(path);
	local size = fileGetSize(file)

	fileClose(file);
	return size;
end

local function requestTexture(path)
	local txd = textureCache[path];

	if not (txd) then
		local size = getResourceSize(path);
		if (size == 0) then return false; end

		txd = engineLoadTXD(path);

		if not (txd) then
			return false;
		end

		textureCache[path] = txd;
	end
	return txd;
end

local function engineClumpObtainAtomic(dff)
	local atom = dff.getAtomics()[1];
	
	if not (atom) then return false; end;
	
	-- TODO: Setup the atomic rendering
	
	atom.setClump(nil);
	atom.setParent(nil);
	return atom;
end

local function loadModel(txd_name, dff_name)
	local txd = requestTexture("textures/"..txd_name..".txd");
	if (txd) then
		txd.setGlobalEmitter();	-- Load from this txd
	end
	
	local dff = engineLoadDFF("models/"..dff_name..".dff", 0);
	
	if (dff) then
		local atom = engineClumpObtainAtomic(dff);
		
		dff.destroy();
		
		if not (atom) then
			outputDebugString("invalid model '" .. dff_name .. "'", 2);
		else
			return atom, txd;
		end
	else
		outputDebugString("corrupted dff file - " .. dff_name, 1);
	end
	
	return false;
end

function loadModels()
	local m,n;
	local file = fileOpen("lod_res.lua");

	for m,n in ipairs(loadstring(file.read(file.size()), "@lodRes")()) do
		local atom, txd = loadModel(n[3], n[2]);
		
		if (atom) then
			pModels[n[1]] = {
				id = n[1],
				name = n[2],
				txd = txd,
				model = atom,
				lod = n[4] / 5,
				super = n[5]
			};
		end
	end
	
	file.destroy();
	file = fileOpen("model_res.lua");

	for n,m in ipairs(loadstring(file.read(file.size()), "@modelRes")()) do
		local atom, txd = loadModel(m.txd_file, m.model_file);
	
		if (atom) then
			local col = engineLoadCOL("coll/"..m.coll_file..".col");
		
			pModels[m.model] = {
				id = m.model,
				name = m.model_file,
				txd = txd,
				model = atom,
				col = col,
				lod = m.lod,
				lodID = m.lodID
			};
			engineSetModelLODDistance(m.model, m.lod);
			
			col.replace(m.model);
			atom.replaceModel(m.model);
		end
	end
	
	file.destroy();
end
loadModels();

local objects = getElementsByType("object", resourceRoot);
local lodLinks = {};

for m,n in ipairs(objects) do
	local info = pModels[n.getModel()];
	
	if (info) then
		local lodInfo = pModels[info.lodID];

		if (lodInfo) then
			local atom = lodInfo.model.clone();
			local frame = engineCreateFrame();
			
			-- Position the scene object
			local mat = frame.getLTM();
			mat.setEulerAngles(n.getRotation());
			mat.setPosition(n.getPosition());
			frame.setLTM(mat);
			mat.destroy();
			
			-- Assign the frame
			atom.setParent(frame);
		
			table.insert(lodLinks, {
				model = atom,
				obj = n,
				lod = info.lod
			});
		end
	else
		setElementAlpha(n, 0);
		setElementCollisionsEnabled(n, false);
	end
end

addEventHandler("onClientGameRender", root, function()
		for m,n in ipairs(lodLinks) do
			local x,y,z = n.obj.getPosition();
			local dist = getDistanceBetweenPoints3D(x, y, z, camera.getPosition());
		
			if not (isElementOnScreen(n.obj)) or (dist > n.lod / 5) then
				if (dist < getFarClipDistance() + 100) then
					n.model.render();
				end
			end
		end
	end
);

addEventHandler("onClientResourceStop", resourceRoot, function()
		for m,n in pairs(pModels) do
			if (n.col) then n.col.destroy(); end
			if (n.model) then n.model.destroy(); end
			engineRestoreCOL(m);
			engineRestoreModel(m);
		end
		
		for m,n in pairs(textureCache) do
			n.destroy();
		end
	end
);

collectgarbage("collect");
setJetpackMaxHeight(1300);