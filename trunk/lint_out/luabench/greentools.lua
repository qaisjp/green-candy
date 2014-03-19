local relDir = file.createTranslator("C:/Program Files/MTA San Andreas 1.2/");

local function writeFileList(list, path, root)
	local zip_stream = root.open(root.absPath() .. path, "rb+");
	local zip;

	if not (zip_stream) then
		zip_stream = root.open(root.absPath() .. path, "wb+");
		
		if not (zip_stream) then
			error("-- could not find build .zip in '" .. root.absPath() .. "' to import files into");
		end
		
		zip = file.createZIPArchive(zip_stream);
	else
		zip = file.createArchiveTranslator(zip_stream);
	end
	
	local count = 0;
	
	if not (zip) then
		zip_stream.destroy();	-- free the file stream resource
	
		error("-- loading of .zip archive failed in '" .. root.absPath() .. "'");
	end
	
	-- Grab all files, write them into the .zip
	for m,n in ipairs(list) do
		local file = root.open(n, "rb");
		
		if (file) then
			local relPath = root.relPathRoot(n);
		
			local content = file.read(file.size());
			file.destroy();
			
			local compr_file = zip.open(relPath, "wb");
			compr_file.write(content);
			compr_file.destroy();
			
			print("wrote " .. relPath);
			
			count = count + 1;
		else
			print(n .. " does not exist");
		end
	end
	
	print("writing to .zip file... (do not close!)");
	
	zip.save();
	zip.destroy();
	zip_stream.destroy();
	
	print("finished writing " .. count .. " files");
end

function bundle()
	local impFiles =
	{
		"MTA/core.dll",
		"MTA/game_sa.dll",
		"MTA/cgui.dll",
		"MTA/libcurl.dll",
		"MTA/loader.dll",
		"MTA/multiplayer_sa.dll",
		"MTA/netc.dll",
		"MTA/xmll.dll",
		"Multi Theft Auto.exe",
		"mods/deathmatch/Client.dll",
		"mods/deathmatch/lua5.1.dll",
		
		-- Standard data libraries
		"MTA/bass.dll",
		"MTA/bass_aac.dll",
		"MTA/bass_ac3.dll",
		"MTA/bass_fx.dll",
		"MTA/bassflac.dll",
		"MTA/bassmidi.dll",
		"MTA/bassmix.dll",
		"MTA/basswma.dll",
		"MTA/D3DCompiler_42.dll",
		"MTA/D3DX9_42.dll",
		"MTA/pthreadVC2.dll",
		"MTA/tags.dll",
		"MTA/sa.dat"
	};
	
	local function includeFile(path)
		table.insert(impFiles, relDir.relPath(path));
	end
	
	-- Include special directories
	relDir.scanDirEx("mta/cgui/", "*", nil, includeFile, true);
	relDir.scanDirEx("skins/", "*", nil, includeFile, true);
    
    relDir.scanDirEx("mods/deathmatch/shaders/", "*", nil, includeFile, true);
	
	writeFileList(impFiles, "green_alpha.zip", relDir);
end

local servDir = file.createTranslator("C:\\Users\\The_GTA\\Desktop\\green\\server\\");

function bundleServer(includeScenalizer)
	local impFiles =
	{
		"core.dll",
		"libcurl.dll",
		"net.dll",
		"xmll.dll",
		"MTA Server.exe",
		"mods/deathmatch/acl.xml",
		"mods/deathmatch/dbconmy.dll",
		"mods/deathmatch/deathmatch.dll",
		"mods/deathmatch/editor.conf",
		"mods/deathmatch/editor_acl.xml",
		"mods/deathmatch/local.conf",
		"mods/deathmatch/lua5.1.dll",
		"mods/deathmatch/mtaserver.conf",
		"mods/deathmatch/pcre3.dll",
		"mods/deathmatch/pthreadVC2.dll",
		"mods/deathmatch/settings.xml",
		"mods/deathmatch/sqlite3.dll",
		"mods/deathmatch/vehiclecolors.conf"
	};
	
	local function includeFile(path)
		table.insert(impFiles, path);
	end
	
	-- Include special directories
	servDir.scanDirEx("mods/deathmatch/modules/", "*", nil, includeFile, true);
	
	-- Include resources
	for m,n in ipairs(({
		"commonutil",
		"downup",
		"dxelements",
		"dxtopia",
		"luaex",
		"math",
		"renderware",
		"resedit",
		"resedit_gdk",
        "streaming_opt",
		"[Admin]",
		"[editor]",
		"[gameplay]",
		"[gamemodes]",
		"[managers]",
		"[web]"
	})) do
		servDir.scanDirEx("mods/deathmatch/resources/" .. n .. "/", "*", nil, includeFile, true);
	end
	
	-- Special scenalizer resource
    if (includeScenalizer) then
        servDir.chdir("mods/deathmatch/resources/rwscenalizer/");
        
        servDir.scanDirEx("maps/", "*", nil, includeFile, true);
        servDir.scanDirEx("replace/", "*", nil, includeFile, true);
        servDir.scanDirEx("", "*", nil, includeFile, false);
	
        -- Change back to root
        servDir.chdir("/");
    end
	
	writeFileList(impFiles, "green_server.zip", servDir);
end