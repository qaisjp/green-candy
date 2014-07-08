local sourceSVN = file.createTranslator( "C:/Users/The_GTA/Desktop/mta_green/" );
local targetSVN = file.createTranslator( "C:/Users/The_GTA/Desktop/blue_revo_mergetest/" );

local svntools = {};

local impFiles =
{
    "MTA10/game_sa/CColModelSA.cpp",
    "MTA10/game_sa/CColModelSA.loader.*",
    "MTA10/game_sa/CObjectSA.render.*",
    "MTA10/game_sa/CEntitySA.render.*",
    "MTA10/game_sa/CEntitySA.renderutils.*",
    "MTA10/game_sa/CEntitySA.rendersetup.*",
    "MTA10/game_sa/CEntitySA.rmode.*",
    "MTA10/game_sa/CCacheSA.*",
    "MTA10/game_sa/CClumpModelInfoSA.*",
    "MTA10/game_sa/CExecutiveManagerSA.*",
    "MTA10/game_sa/CFileUtilsSA.*",
    "MTA10/game_sa/CIMGManagerSA.*",
    "MTA10/game_sa/CIPLSectorSA.*",
    "MTA10/game_sa/CModelInfoSA.base.*",
    "MTA10/game_sa/CModelInfoSA.ide.*",
    "MTA10/game_sa/CModelManagerSA.*",
    "MTA10/game_sa/CPlaceableSA.*",
    "MTA10/game_sa/CPtrNodeSA.*",
    "MTA10/game_sa/CQuadTreeSA.*",
    "MTA10/game_sa/CRecordingsSA.*",
    "MTA10/game_sa/CRenderWareExtensionSA.*",
    "MTA10/game_sa/CRenderWareSA.lighting.*",
    "MTA10/game_sa/CRenderWareSA.mem.*",
    "MTA10/game_sa/CRenderWareSA.pipeline.*",
    "MTA10/game_sa/CRenderWareSA.render.*",
    "MTA10/game_sa/CRenderWareSA.rmode.hxx",
    "MTA10/game_sa/CRenderWareSA.rtbucket.*",
    "MTA10/game_sa/CRenderWareSA.rtcallback.*",
    "MTA10/game_sa/CRenderWareSA.rwapi.*",
    "MTA10/game_sa/CRenderWareSA.state.*",
    "MTA10/game_sa/CRenderWareSA.stateman.hxx",
    "MTA10/game_sa/CStreamerSA.*",
    "MTA10/game_sa/CStreamingSA.*",
    "MTA10/game_sa/CTextureManagerSA.*",
    "MTA10/game_sa/CTransformationSA.*",
    "MTA10/game_sa/CVehicleModelInfoSA.*",
    "MTA10/game_sa/CVehicleSA.render.*",
    "MTA10/game_sa/CVehicleSA.customplate.*",
    "MTA10/sdk/game/CStreaming.h",
    "MTA10/game_sa/RenderWare.h",
    "MTA10/game_sa/RenderWare/*"
};

local sourceSharedNames =
{
    "Shared/core/*"
};

local targetSharedNames =
{
    "Shared/sdk/coresdk/*"
};

local function getSVNUpdateList(svnRoot)
    return impFiles;
end

local function executeParsedGetFiles(translator, wildName)
    -- Get the filename field of the wildName parameter and execute it as wildcard to its directory.
    local fileName, directory = file.pathToFilename( wildName, true );
    
    -- Return the list of files.
    local fileList = {};
    
    local function fileIterator(absPath)
        table.insert( fileList, translator.relPathRoot( absPath ) );
    end
    
    translator.scanDirEx( directory, fileName, nil, fileIterator, false );
    
    return fileList;
end

local function equalityComparator(left, right)
    return ( left == right );
end

local function filenameComparator(left, right)
    local leftName = file.pathToFilename( left, true );
    local rightName = file.pathToFilename( right, true );
    
    return ( leftName == rightName );
end

local function getSVNCommit(sourceSVN, targetSVN, sourceShared, targetShared)
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
    
    local function createFileInformation(sourceHandle, targetHandle, sourceName, targetName)
        local info = {};
    
        -- Has the target file existed before?
        local hasTargetExisted = not ( targetHandle == false );
        
        -- Determine whether the file has changed.
        local hasChanged = false;
        
        if ( sourceHandle ) and ( targetHandle ) then
            hasChanged = not ( file.streamCompare( sourceHandle, targetHandle ) );
        end
        
        -- If there is no target name, we set it to the source name.
        if not ( targetName ) then
            targetName = sourceName;
        end
        
        function info.details()
            local header = sourceName;
            
            if not ( sourceName == targetName ) then
                header = header .. " (diffpath)";
            end
            
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
    
    local function tfind(t,v, comparator)
        for m,n in ipairs( t ) do
            if ( comparator( n, v ) ) then
                return n;
            end
        end
        
        return false;
    end
    
    local function tmerge(left, right)
        local rightMax = #right;
        local leftIter = #left + rightMax;
        local rightIter = rightMax;
        
        while ( rightIter >= 0 ) do
            local rightItem = right[ rightIter ];
            
            left[ leftIter ] = rightItem;
            
            rightIter = rightIter - 1;
            leftIter = leftIter - 1;
        end
        
        return left;
    end
    
    local targetMatchings = {};
    
    local function matchSourceToTargetFiles(sourceFiles, targetFiles, comparator)
        -- Split entries into which exist in the target and which do not.
        for m,n in ipairs( sourceFiles ) do
            local doesExist = tfind( targetFiles, n, comparator );
            
            -- Make sure thr file is not being targetted already.
            if ( doesExist ) and ( targetMatchings[doesExist] ) then
                outputLog( "warning: SVN target file ambiguity (" .. n .. ")" );
            else
                local sourceHandle = sourceSVN.open( n, "rb" );
                local targetHandle = false;
                
                if ( doesExist ) then
                    targetHandle = targetSVN.open( doesExist, "rb+" );
                    
                    if not ( targetHandle ) then
                        outputLog( "failed to open " .. doesExist .. " in target SVN" );
                    end
                end
                
                createFileInformation(
                    sourceHandle,
                    targetHandle,
                    n,
                    doesExist
                );
                
                targetMatchings[doesExist] = true;
            end
        end
    end
    
    for m,n in ipairs( fileList ) do
        -- Open file handles to source and target SVNs.
        local sourceFiles = executeParsedGetFiles( sourceSVN, n );
        local targetFiles = executeParsedGetFiles( targetSVN, n );
        
        -- Does the source file exist? If not abort operation.
        if ( #sourceFiles == 0 ) then
            outputLog( "could not find " .. n .. " in source SVN" );
        end
        
        -- Match the entries.
        matchSourceToTargetFiles( sourceFiles, targetFiles, equalityComparator );
    end
    
    if ( true ) then
        -- Next parse shared names. Those are files that live in a global namespace and override same-named entries.
        -- Create source and target lists.
        local sourceResolvedShared = {};
        local targetResolvedShared = {};
        
        for m,n in ipairs( sourceShared ) do
            local diskNames = executeParsedGetFiles( sourceSVN, n );
            
            tmerge( sourceResolvedShared, diskNames );
        end
        
        for m,n in ipairs( targetShared ) do
            local diskNames = executeParsedGetFiles( targetSVN, n );
            
            tmerge( targetResolvedShared, diskNames );
        end
        
        -- Match the entries.
        matchSourceToTargetFiles(
            sourceResolvedShared,
            targetResolvedShared,
            filenameComparator
        );
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
    print( "MTA:Eir -> Eir fork" );

    local commit = getSVNCommit(
        sourceSVN, targetSVN,
        sourceSharedNames, targetSharedNames
    );
    
    commit.perform();
    
    commit.terminate();
end

function svntools.check()
    print( "MTA:Eir -> Eir fork" );

    local commit = getSVNCommit(
        sourceSVN, targetSVN,
        sourceSharedNames, targetSharedNames
    );
    
    commit.details();
    
    commit.terminate();
end

function svntools.revupdate()
    print( "Eir fork -> MTA:Eir" );

    local commit = getSVNCommit(
        targetSVN, sourceSVN,
        targetSharedNames, sourceSharedNames
    );
    
    commit.perform();
    
    commit.terminate();
end

function svntools.revcheck()
    print( "Eir fork -> MTA:Eir" );

    local commit = getSVNCommit(
        targetSVN, sourceSVN,
        targetSharedNames, sourceSharedNames
    );
    
    commit.details();
    
    commit.terminate();
end

_G.svn = svntools;