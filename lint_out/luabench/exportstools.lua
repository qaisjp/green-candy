local xmodRoot = file.createTranslator(
    "C:/Users/The_GTA/Desktop/dropbox/Dropbox/[sharedresources]/xmod/"
);
local xmodOutRoot = file.createTranslator(
    "C:/Users/The_GTA/Desktop/mta eir arran props/xmod_custom/"
);

local function xmlLoadFile(path)
    local file = xmodRoot.open( path, "rb" );
    
    if not ( file ) then return false; end;
    
    local content = file.read( file.size() );
    
    file.destroy();
    
    return xml.parse( content );
end

local function xmlWriteFile(path, xml)
    local file = xmodOutRoot.open(path, "wb");
    
    if not ( file ) then return; end;
    
    file.write( xml.writeTree( 0 ) );
    
    file.destroy();
end

local function copyFileList(srcTranslator, dstTranslator, fileList)
    local alreadyCopied = {};
    
    for m,n in ipairs(fileList) do
        if not ( alreadyCopied[n] ) then
            print( "copying " .. n );
            
            file.copyFile(srcTranslator, n, dstTranslator, n);
        
            alreadyCopied[n] = true;
        end
    end
end

function exportxmod()
    local resourceFiles = {};

    do
        -- Convert the meta.xml file properly.
        local sourceMeta = xmlLoadFile( "meta.xml" );
        
        if not ( sourceMeta ) then return; end;
        
        -- Strip all file nodes from the config.
        while ( true ) do
            local fileNode = sourceMeta.findSubNode( "file" );
            
            if not ( fileNode ) then break; end;
            
            fileNode.destroy();
        end
        
        -- Write the meta.xml
        xmlWriteFile( "meta.xml", sourceMeta );
        
        -- Construct a file list from the meta file.
        for m,n in ipairs( sourceMeta.getChildren() ) do
            local srcField = n.attr.src;
            
            if not ( srcField == nil ) then
                table.insert( resourceFiles, srcField );
            end
        end
        
        -- Destroy it.
        sourceMeta.destroy();
    end
    
    table.insert( resourceFiles, "readme.txt" );
    
    -- Write the resource files.
    copyFileList( xmodRoot, xmodOutRoot, resourceFiles );
    
    -- Create some mod folders if not present.
    xmodOutRoot.createDir( "skins/" );
    xmodOutRoot.createDir( "vehicles/" );
    xmodOutRoot.createDir( "weapons/" );
end

local radarRoot = file.createTranslator(
    "C:/Users/The_GTA/Desktop/conv_stuff/"
);

function renameradarfiles()
    local filesToRename = {};

    local function fileIterator(filePath)
        local relPath = radarRoot.relPath(filePath);

        table.insert(filesToRename, relPath);
    end
    
    radarRoot.scanDirEx( "@", "*.jpg", nil, fileIterator, false);
    
    -- Rename the files.
    for m,n in ipairs(filesToRename) do
        local radar_num = string.match( n, "radar(%d+).jpg" );
        local radarNumber = tonumber( radar_num );
        
        if ( radarNumber ) then
            local column = math.floor(radarNumber / 12);
            local row = radarNumber % 12;
            
            radarRoot.rename( n, "sattelite/sattelite_" .. column .. "_" .. row.. ".jpeg" );
        end
    end
end