local relDir = file.createTranslator("C:/Program Files/MTA San Andreas 1.2/");

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
	
	local zip_stream = relDir.open(relDir.absPath() .. "green_alpha.zip", "rb+");

	if not (zip_stream) then
		error("-- could not find build .zip in '" .. relDir.absPath() .. "' to import files into");
	end
	
	local zip = file.createArchiveTranslator(zip_stream);
	local count = 0;
	
	if not (zip) then
		zip_stream.destroy();	-- free the file stream resource
	
		error("-- loading of .zip archive failed in '" .. relDir.absPath() .. "'");
	end
	
	-- Grab all files, write them into the .zip
	for m,n in ipairs(impFiles) do
		local file = relDir.open(n, "rb");
		
		if (file) then
			local content = file.read(file.size());
			file.destroy();
			
			local compr_file = zip.open(n, "wb");
			compr_file.write(content);
			compr_file.destroy();
			
			print("wrote " .. n);
			
			count = count + 1;
		else
			print(n .. " does not exist");
		end
	end
	
	print("finished writing " .. count .. " files");
	
	zip.save();
	zip.destroy();
	zip_stream.destroy();
end