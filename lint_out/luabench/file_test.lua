root = file.getRoot();
dynroot = file.createTranslator(root.absPath("luabench/"));

function archiveTest()
	return file.createArchiveTranslator(dynroot.open("lol.zip", "rb+"));
end

function zipcep()
	zz = file.createArchiveTranslator(dynroot.open("zipception.zip", "rb"));
	
	return file.createArchiveTranslator(zz.open("lol.zip", "rb"));
end

function dirzip()
    local zstream = root.open( "test.zip", "rb" );
    
    dzip = file.createArchiveTranslator( zstream );
    
    zstream.destroy();
    return dzip;
end

function dirzipcreat()
    local zipwritestream = root.open( "testw.zip", "wb" );
    
    dzip = file.createZIPArchive( zipwritestream );
    
    dzip.createDir( "testdir/" );
    
    zipwritestream.destroy();
    
    dzip.save();
    
    dzip.destroy();
end

function dircmd()
	local m,n;
	local files = root.scanDir("", "*", true);
	
	for m,n in ipairs(files) do
		local rel = root.relPath(n);
		
		print(rel .. " (size=" .. root.size(rel) .. ")");
	end
	
	print("\n" .. #files .. " entries found");
end

function dorot()
	local zfile = dynroot.open("lol.zip", "rb+");
	local z = file.createArchiveTranslator(zfile);
	
	z.open("proto.jpg", "rb+").destroy();

	z.save();
	z.destroy();
	zfile.destroy();
end

function ctest()
	local z = archiveTest();
	
	z.open("lol.jpg", "rb+");
end

local function countsize(files)
	local m,n;
	local size = 0;
	
	for m,n in ipairs(files) do
		size = size + root.size(n);
	end
	
	return size;
end

function countcode()
	print("size: " .. (countsize(root.getFiles("/", "*.c", true)) 
		+ countsize(root.getFiles("/", "*.cpp", true)) 
		+ countsize(root.getFiles("/", "*.h", true))) / 1024 / 1024 .. " mb");
end

function dircmdex(rt)
	local filenum = 0;
	local dirnum = 0;
	
	if not (rt) then
		rt = root;
	end

	local function filecb(path)
		local rel = rt.relPath(path);
	
		print(rel .. " (size=" .. rt.size(rel) .. ")");
		
		filenum = filenum + 1;
	end
	
	local function dircb(path)
		print(path);
		
		dirnum = dirnum + 1;
	end
	
	rt.scanDirEx("", "*", dircb, filecb, true);
	
	print("\nfiles: " .. filenum .. ", dirs: " .. dirnum);
end

function findD3D()
	local r = file.createTranslator( root.absPath() .. "../" );
	r.scanDir("/", "d3dx9.h", print, print, true);
end

function fileGetContent(path)
    local file = fileOpen(path);
    
    if not (file) then return false; end;
    
    local content = file.read(file.size());
    file.destroy();
    
    return content;
end

function fileSeekWriteTest()
    local file = dynroot.open("testfile.txt", "wb");
    
    -- Do basic and simple seek-based write and read tests.
    file.seek( 10, "set" );
    file.write( "Hello world!" );
    
    file.seek( -6, "cur" );
    file.write( "fiend" );
    
    file.flush();
    
    file.seek( -5, "cur" );
    
    local testString1 = file.read(5);
    
    print( "test 1: " .. testString1 .. " -> fiend" );
    
    file.seek( 0, "set" );
    
    local nullBytes = file.read( 10 );
    local reqNullBytes = string.rep( string.char( 0 ), 10 );
    
    print( "test2: " .. ( ( nullBytes == reqNullBytes ) and "success" or "failure" ) );
    
    -- Advanced test: write a series of blocks into a file and verify that they
    -- have been properly stored.
    file.seek( 0, "set" );
    
    -- Create a series of blocks that should be written to the file.
    local blockSize = 100;
    local blocksToAllocate = 10;
    
    local function allocateBlock()
        local memStrings = {};
        
        for m=1,blockSize do
            table.insert( memStrings,
                string.char(
                    math.floor( math.random() * 256 )
                )
            );
        end
        
        return table.concat( memStrings, "" );
    end
    
    local memBlocks = {};
    
    for m=1,blocksToAllocate do
        table.insert( memBlocks, allocateBlock() );
    end
    
    -- Write the blocks to the file.
    for m,n in ipairs(memBlocks) do
        file.write( n );
    end
    
    -- Verify that the blocks have been written correctly.
    file.seek( 0, "set" );
    
    local correctWriteThrough = true;
    
    for m,n in ipairs(memBlocks) do
        local fileContent = file.read( blockSize );
        local requiredBlock = n;
        
        if not ( fileContent == requiredBlock ) then
            correctWriteThrough = false;
            
            print( "block " .. m .. " is invalid" );
        end
    end
    
    if ( correctWriteThrough ) then
        print( "succeeded the write through test" );
    else
        print( "failed the write through test" );
    end
    
    file.destroy();
end