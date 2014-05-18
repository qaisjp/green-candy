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