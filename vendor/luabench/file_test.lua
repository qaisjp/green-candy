root = file.getRoot();

function dircmd()
	local m,n;
	local files = root.scanDir("", "*", true);
	
	for m,n in ipairs(files) do
		local rel = root.relPath(n);
		
		print(rel .. " (size=" .. root.size(rel) .. ")");
	end
	
	print("\n" .. #files .. " entries found");
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

function dircmdex()
	local filenum = 0;
	local dirnum = 0;

	local function filecb(path)
		local rel = root.relPath(path);
	
		print(rel .. " (size=" .. root.size(rel) .. ")");
		
		filenum = filenum + 1;
	end
	
	local function dircb(path)
		print(path);
		
		dirnum = dirnum + 1;
	end
	
	root.scanDirEx("", "*", dircb, filecb, true);
	
	print("\nfiles: " .. filenum .. ", dirs: " .. dirnum);
end