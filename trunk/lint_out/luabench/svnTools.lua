local sourceSVN = file.createTranslator( "C:/Users/The_GTA/Desktop/mta_green/" );
local targetSVN = file.createTranslator( "C:/Users/The_GTA/Desktop/blue_revo_mergetest/" );

local svntools = {};

local impFiles =
{
    "MTA10/game_sa/CObjectSA.render.cpp",
    "MTA10/game_sa/CObjectSA.render.h",
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

function svntools.revupdate()
    local commit = getSVNCommit( targetSVN, sourceSVN );
    
    commit.perform();
    
    commit.terminate();
end

function svntools.revcheck()
    local commit = getSVNCommit( targetSVN, sourceSVN );
    
    commit.details();
    
    commit.terminate();
end

_G.svn = svntools;