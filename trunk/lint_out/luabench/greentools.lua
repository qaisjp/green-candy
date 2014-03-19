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

local sourceSVN = file.createTranslator( "C:/Users/The_GTA/Desktop/mta_green/" );
local targetSVN = file.createTranslator( "C:/Users/The_GTA/Desktop/blue_revo_mergetest/" );

local svntools = {};

local impFiles =
{
    "MTA10/game_sa/CEntitySA.render.cpp",
    "MTA10/game_sa/CEntitySA.render.h",
    "MTA10/game_sa/CEntitySA.render.hxx",
    "MTA10/game_sa/CEntitySA.renderutils.cpp",
    "MTA10/game_sa/CEntitySA.renderutils.h",
    "MTA10/game_sa/CEntitySA.rendersetup.cpp",
    "MTA10/game_sa/CEntitySA.rendersetup.h",
    "MTA10/game_sa/CCacheSA.cpp",
    "MTA10/game_sa/CCacheSA.h",
    "MTA10/game_sa/CClumpModelInfoSA.cpp",
    "MTA10/game_sa/CClumpModelInfoSA.h",
    "MTA10/game_sa/CExecutiveManagerSA.cpp",
    "MTA10/game_sa/CExecutiveManagerSA.h",
    "MTA10/game_sa/CExecutiveManagerSA.fiber.cpp",
    "MTA10/game_sa/CExecutiveManagerSA.fiber.h",
    "MTA10/game_sa/CFileUtilsSA.cpp",
    "MTA10/game_sa/CFileUtilsSA.h",
    "MTA10/game_sa/CIMGManagerSA.cpp",
    "MTA10/game_sa/CIMGManagerSA.h",
    "MTA10/game_sa/CIPLSectorSA.cpp",
    "MTA10/game_sa/CIPLSectorSA.h",
    "MTA10/game_sa/CModelInfoSA.base.cpp",
    "MTA10/game_sa/CModelInfoSA.base.h",
    "MTA10/game_sa/CModelInfoSA.ide.cpp",
    "MTA10/game_sa/CModelInfoSA.ide.h",
    "MTA10/game_sa/CModelManagerSA.cpp",
    "MTA10/game_sa/CModelManagerSA.h",
    "MTA10/game_sa/CPlaceableSA.cpp",
    "MTA10/game_sa/CPlaceableSA.h",
    "MTA10/game_sa/CPtrNodeSA.cpp",
    "MTA10/game_sa/CPtrNodeSA.h",
    "MTA10/game_sa/CQuadTreeSA.cpp",
    "MTA10/game_sa/CQuadTreeSA.h",
    "MTA10/game_sa/CRecordingsSA.cpp",
    "MTA10/game_sa/CRecordingsSA.h",
    "MTA10/game_sa/CRenderWareExtensionSA.cpp",
    "MTA10/game_sa/CRenderWareExtensionSA.h",
    "MTA10/game_sa/CRenderWareSA.lighting.cpp",
    "MTA10/game_sa/CRenderWareSA.lighting.h",
    "MTA10/game_sa/CRenderWareSA.mem.cpp",
    "MTA10/game_sa/CRenderWareSA.mem.h",
    "MTA10/game_sa/CRenderWareSA.pipeline.cpp",
    "MTA10/game_sa/CRenderWareSA.pipeline.h",
    "MTA10/game_sa/CRenderWareSA.render.cpp",
    "MTA10/game_sa/CRenderWareSA.render.h",
    "MTA10/game_sa/CRenderWareSA.rtcallback.cpp",
    "MTA10/game_sa/CRenderWareSA.rtcallback.h",
    "MTA10/game_sa/CRenderWareSA.rwapi.cpp",
    "MTA10/game_sa/CRenderWareSA.rwapi.h",
    "MTA10/game_sa/CStreamerSA.cpp",
    "MTA10/game_sa/CStreamerSA.h",
    "MTA10/game_sa/CStreamingSA.cache.cpp",
    "MTA10/game_sa/CStreamingSA.cache.h",
    "MTA10/game_sa/CStreamingSA.clump.cpp",
    "MTA10/game_sa/CStreamingSA.clump.h",
    "MTA10/game_sa/CStreamingSA.collision.cpp",
    "MTA10/game_sa/CStreamingSA.collision.h",
    "MTA10/game_sa/CStreamingSA.cpp",
    "MTA10/game_sa/CStreamingSA.h",
    "MTA10/game_sa/CStreamingSA.gc.cpp",
    "MTA10/game_sa/CStreamingSA.gc.h",
    "MTA10/game_sa/CStreamingSA.init.cpp",
    "MTA10/game_sa/CStreamingSA.init.h",
    "MTA10/game_sa/CStreamingSA.ipl.cpp",
    "MTA10/game_sa/CStreamingSA.ipl.h",
    "MTA10/game_sa/CStreamingSA.loader.cpp",
    "MTA10/game_sa/CStreamingSA.loader.h",
    "MTA10/game_sa/CStreamingSA.runtime.cpp",
    "MTA10/game_sa/CStreamingSA.runtime.h",
    "MTA10/game_sa/CStreamingSA.sectorize.h",
    "MTA10/game_sa/CStreamingSA.textures.cpp",
    "MTA10/game_sa/CStreamingSA.textures.h",
    "MTA10/game_sa/CStreamingSA.utils.cpp",
    "MTA10/game_sa/CStreamingSA.utils.h",
    "MTA10/game_sa/CStreamingSA.utils.hxx",
    "MTA10/game_sa/CTextureManagerSA.cpp",
    "MTA10/game_sa/CTextureManagerSA.h",
    "MTA10/game_sa/CTextureManagerSA.instance.cpp",
    "MTA10/game_sa/CTextureManagerSA.instance.h",
    "MTA10/game_sa/CTransformationSA.cpp",
    "MTA10/game_sa/CTransformationSA.h",
    "MTA10/game_sa/CVehicleModelInfoSA.config.cpp",
    "MTA10/game_sa/CVehicleModelInfoSA.config.h",
    "MTA10/game_sa/CVehicleModelInfoSA.cpp",
    "MTA10/game_sa/CVehicleModelInfoSA.h",
    "MTA10/game_sa/CVehicleModelInfoSA.render.cpp",
    "MTA10/game_sa/CVehicleModelInfoSA.render.h",
    "MTA10/game_sa/CVehicleSA.render.cpp",
    "MTA10/game_sa/CVehicleSA.render.h",
    "MTA10/game_sa/CVehicleSA.customplate.cpp",
    "MTA10/game_sa/CVehicleSA.customplate.h",
    "MTA10/game_sa/RenderWare.h"
};

local function getSVNUpdateList(svnRoot)
    local fileList = {};
    local fileIter = 1;
    
    -- Include all hardcoded filenames.
    for m,n in ipairs( impFiles ) do
        fileList[fileIter] = n;
        fileIter = fileIter + 1;
    end
    
    -- Include special folders.
    local function includeFolderFiles( folderPath )
        local function includeFile( absPath )
            fileList[fileIter] = svnRoot.relPathRoot( absPath );
            fileIter = fileIter + 1;
        end
        
        svnRoot.scanDirEx( folderPath, "*", nil, includeFile, true );
    end
    
    includeFolderFiles( "MTA10/game_sa/RenderWare/" );
    
    return fileList;
end

local function getSVNCommit(sourceSVN, targetSVN)
    -- Create a new logging instance.
    local logger = createLogger( getUniqueLogFilename() );
    
    local function outputLog(msg)
        if ( logger ) then
            logger.output( msg );
        end
        
        print( msg );
    end

    -- Scan through all files and determine the operations that have to be done on them.
    local fileList = getSVNUpdateList( sourceSVN );
    
    -- Create information entries for each file.
    local infoList = {};
    
    for m,n in ipairs( fileList ) do
        local info = {};
        
        -- Open file handles to source and target SVNs.
        local sourceHandle = sourceSVN.open( n, "rb" );
        local targetHandle = targetSVN.open( n, "rb+" );
        
        -- Does the source file exist? If not abort operation.
        if not ( sourceHandle ) then
            outputLog( "could not open " .. n .. " in source SVN" );
        end
        
        -- Has the target file existed before?
        local hasTargetExisted = not ( targetHandle == false );
        
        -- Determine whether the file has changed.
        local hasChanged =
            not ( sourceHandle.read( sourceHandle.size() ) == targetHandle.read( targetHandle.size() ) );
        
        function info.details()
            local header = n;
            
            if not ( hasTargetExisted ) then
                header = "new " .. header;
            else
                if ( hasChanged ) then
                    header = "* " .. header;
                end
            end
            
            header = "-- " .. header;
            
            outputLog( header );
        end
        
        function info.commit()
            if not ( hasChanged ) then return; end;
        
            -- If the target has not existed before, we create it
            if not ( targetHandle ) then
                targetHandle = targetSVN.open( n, "wb+" );
                
                outputLog( "created " .. n );
            end
            
            -- Reset seek pointers and write the contents.
            sourceHandle.seek( 0, "set" );
            targetHandle.seek( 0, "set" );
            
            targetHandle.write( sourceHandle.read( sourceHandle.size() ) );
            targetHandle.seekEnd();
        end
        
        function info.terminate()
            if ( sourceHandle ) then
                sourceHandle.destroy();
                sourceHandle = false;
            end
            
            if ( targetHandle ) then
                targetHandle.destroy();
                targetHandle = false;
            end
        end
        
        table.insert( infoList, info );
    end
    
    -- Clean memory.
    fileList = nil;
    
    local commit = {};
    
    function commit.details()
        outputLog( "* commit details:" );
    
        for m,n in ipairs( infoList ) do
            n.details();
        end
    end
    
    function commit.perform()
        for m,n in ipairs( infoList ) do
            n.details();
            n.commit();
        end
    end
    
    function commit.terminate()
        -- Terminate all SVN entries.
        for m,n in ipairs( infoList ) do
            n.terminate();
        end
    
        logger.destroy();
        
        logger = false;
    end
    
    return commit;
end

function svntools.update()
    local commit = getSVNCommit( sourceSVN, targetSVN );
    
    commit.perform();
    
    commit.terminate();
end

function svntools.check()
    local commit = getSVNCommit( sourceSVN, targetSVN );
    
    commit.details();
    
    commit.terminate();
end

_G.svn = svntools;