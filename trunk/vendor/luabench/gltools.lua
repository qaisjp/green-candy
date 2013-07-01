local win32 = win32;
local gl = gl;

local function getFuncItems(path)
	local file = fileOpen(path);
	
	if not (file) then
		error("could not open function list text file");
	end
	
	-- Cache the content
	local content = file.read(file.size());
	file.destroy();
	
    content = string.gsub(content, "\r", "");
	
	local items = {};
	local n = 1;
	local maxLength = 0;
	
    string.gsub(content, "[^%s+\n]+",
        function(line)
			items[n] = line;
			
			local len = #line;
			
			if (maxLength < len) then
				maxLength = len;
			end
			
			n = n + 1;
		end
	);
	
	if (maxLength == 0) then
		error("no items with valid length");
	end
	
	return items, n, maxLength;
end

function makefunclist(path)
	local items, n, maxLength = getFuncItems(path);
	
	local outFile = fileCreate("gl/conv_funclist.txt");
	
	local lenRationale = math.ceil((maxLength+2) / 4) * 4;
	local lenOffset = 8;
	
	local i = 1;
	
	while (i < n) do
		local item = items[i];
		local len = #item;
		
		local firstOff = lenRationale - (len+2);
		
		local newLine = item .. "_t" .. string.rep(" ", firstOff + lenOffset) .. "_" .. item .. " = NULL;";
		
		outFile.write(newLine .. "\n");
	
		i = i + 1;
	end
	
	outFile.destroy();
end

function makefuncextern(path)
	local items, num, maxLength = getFuncItems(path);
	
	local outFile = fileCreate("gl/conv2_funclist.txt");
	
	local lenRationale = math.ceil((maxLength+9) / 4) * 4;
	local lenOffset = 8;
	
	local i = 1;
	
	while (i < num) do
		local item = items[i];
		local len = #item;
		
		local firstOff = lenRationale - (len+9);
		
		local newLine = "extern " .. item .. "_t" .. string.rep(" ", firstOff + lenOffset) .. "_" .. item .. ";";
		
		outFile.write(newLine .. "\n");
	
		i = i + 1;
	end
	
	outFile.destroy();
end

function makeapiloader(path)
	local items, num, maxLength = getFuncItems(path);
	
	local outFile = fileCreate("gl/conv3_funclist.txt");
	
	local lenRationale = math.ceil((maxLength+3) / 4) * 4;
	local lenOffset = 4;
	
	local i = 1;
	
	while (i < num) do
		local item = items[i];
		local newLine = "API_LOAD( " .. item .. " );\n";
		
		outFile.write(newLine);
	
		i = i + 1;
	end
	
	i = 1;
	
	outFile.write("\n");
	
	while (i < num) do
		outFile.write("API_CHECK( \"opengl32.dll\", _" .. items[i] .. " );\n");
		
		i = i + 1;
	end
	
	outFile.destroy();
end

-----------------------------------------------------------------------------------------
-- OPENGL EXTENSION FUNCTIONS--------------------------------------------
-----------------------------------------------------------------------------------------
function makefunclist_ext(path)
	local items, n, maxLength = getFuncItems(path);
	
	local outFile = fileCreate("gl/convext_funclist.txt");
	
	local lenRationale = math.ceil((maxLength+2) / 4) * 4;
	local lenOffset = 4;
	
	local i = 1;
	
	while (i < n) do
		local item = items[i];
		local len = #item;
		
		local firstOff = lenRationale - (len+2);
		
		local newLine = "    " .. item .. "_t" .. string.rep(" ", firstOff + lenOffset) .. item .. ";";
		
		outFile.write(newLine .. "\n");
	
		i = i + 1;
	end
	
	outFile.destroy();
end

function makeapiloader_ext(path)
	local items, num, maxLength = getFuncItems(path);
	
	local outFile = fileCreate("gl/conv3ext_funclist.txt");
	
	local lenRationale = math.ceil((maxLength+3) / 4) * 4;
	local lenOffset = 4;
	
	local i = 1;
	
	while (i < num) do
		local item = items[i];
		local newLine = "API_LOAD_EXT( " .. item .. " );\n";
		
		outFile.write(newLine);
	
		i = i + 1;
	end
	
	i = 1;
	
	outFile.write("\n");
	
	while (i < num) do
		outFile.write("API_CHECK_EXT( " .. items[i] .. " );\n");
		
		i = i + 1;
	end
	
	outFile.destroy();
end

function doglfunclist(path)
	makefunclist(path);
	makefuncextern(path);
	makeapiloader(path);
end

function doglextfunclist(path)
	makefunclist_ext(path);
	makeapiloader_ext(path);
end

function loadParamList()
	local file = fileOpen("gl/paramlist2.txt");
	
	if not (file) then
		error("could not open/find 'gl/paramlist2.txt'");
	end
	
	local content = file.read(file.size());
	file.destroy();
	
	local params = {};
	local n = 1;

    string.gsub(content, "[^\n]+",
        function(line)
			local name, type, num = string.match(line, "([^%s]+)%s+([^%s]+)%s+([^%s]+)");
			
			if (num == "unk") then return; end;
			
			params[n] = {
				name = name,
				type = type,
				argNum = tonumber(num)
			};
			n = n + 1;
		end
	);
	
	return params, n -1;
end

local function gout(file, msg)
	file.write(msg);
	print(msg);
end

function makeparamget()
	local params, num = loadParamList();
	
	local file = fileCreate("gl/conv_paramlist.txt");

	file.write("static const GLparaminfo param_list[] =\n{\n");
	
	local n = 1;

	while (n <= num) do
		local param = params[n];
		
		local typename = "GLPARAM_INT";
		
		if (param.type == "int") then
			typename = "GLPARAM_INT";
		elseif (param.type == "float") then
			typename = "GLPARAM_DOUBLE";
		elseif (param.type == "bool") then
			typename = "GLPARAM_BOOLEAN";
		end
		
		gout(file, "    { \"" .. param.name .. "\", " .. param.name .. ", 0x" .. string.format("%.x", hashString(param.name)) .. ", " .. typename .. ", " .. param.argNum .. " }");
		
		if not (n == num) then
			file.write(",\n");
		else
			file.write("\n");
		end
	
		n = n + 1;
	end
	
	file.write("};\n\n");
	
	n = 1;
	
	file.write( "lua_createtable( L, " .. num .. ", 0 );\n" );
	
	while (n <= num) do
		local param = params[n];
		
		file.write(
[[lua_pushlstring( L, "]] .. param.name .. [[", ]] .. #param.name .. [[ );
lua_rawseti( L, -2, ]] .. n .. [[ );
]]);
		
		n = n + 1;
	end
	
	file.destroy();
end

local function makelocalconvparamfile(path)
	local _path, isFile, filename = fileParsePath(path);
	
	if not (isFile) then return false; end;

	local froot = file.createTranslator(root.absPath("luaBench/" .. path));
	
	if not (froot) then return false; end;
	
	local file = root.open(froot.absPath("conv_" .. filename), "w");
	
	froot.destroy();
	return file;
end

function makeparameasy(path)
	local params, num = getFuncItems(path);
	
	if not (params) then return false; end;
	
	local file = makelocalconvparamfile(path);

	file.write("static const GLparaminfo items[] =\n{\n");
	
	local n = 1;

	while (n < num) do
		local param = params[n];
		
		file.write("    { \"" .. param .. "\", " .. param .. ", 0x" .. string.format("%.x", hashString(param)) .. " }");
		
		if not (n == num - 1) then
			file.write(",\n");
		else
			file.write("\n");
		end
	
		n = n + 1;
	end
	
	file.write("};\n\n");
	
	n = 1;
	
	file.write( "lua_newtable( L );\n" );
	
	while (n < num) do
		local param = params[n];
		
		file.write(
[[lua_pushlstring( L, "]] .. param .. [[", ]] .. #param .. [[ );
lua_rawseti( L, -2, n++ );
]]);
		
		n = n + 1;
	end
	
	file.destroy();
end

function makeintformat()
	local root = file.createTranslator(root.absPath("luaBench/"));
	
	root.scanDirEx("gl/internal_format/", "*.txt", nil,
		function(path)
			local relPath = root.relPath(path);
			local _path, isFile, filename = fileParsePath(relPath);
			
			if (string.find(filename, "conv_")) then return; end;
		
			makeparameasy(relPath);
		end, true
	);
end

function printGLParams(driver)
	for m,n in ipairs(gl.getParamNames()) do
		local vals = { driver.get(n) };
		local num = #vals;
		local i = 1;
		
		local out = n .. ": ";
		
		while (i <= num) do
			local val = vals[i];
			out = out .. tostring(val);
			
			if not (n == num) then
				out = out .. ", ";
			end
			
			i = i + 1;
		end
		
		print(out);
	end
end

local function gen(str)
	local len = #str;
	
	while not (len == 2) do
		str = "0" .. str;
		
		len = len + 1;
	end
	
	return str;
end

function doUnicodeTable()
	local file = fileCreate("unicode_table.txt");
	
	file.write("const static wchar_t unicode_table[] =\n{\n");
	
	local n = 0;
	local num = 0;
	local out_line = "    ";
	
	while (n < 256) do
		out_line = out_line .. "L'\\x" .. gen(string.format("%.x", n)) .. "', ";
		
		if (num == 30) then
			file.write(out_line .. "\n");
			
			num = 0;
			out_line = "    ";
		else
			num = num + 1;
		end
		
		n = n + 1;
	end
	
	if not (num == 0) then
		file.write(out_line .. "\n");
	end
	
	file.write("};\n");
	
	file.destroy();
end