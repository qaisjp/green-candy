local win32 = win32;
local gl = gl;

function makeglwnd()
	local dlg = win32.createDialog(640, 480);
	local gl = gl.create(dlg);
	
	dlg.setVisible(true);
	dlg.setText("OpenGL Test Device");
	
	dlg.addEventHandler("onClose",
		function()
			print("destroyed OpenGL device");
		
			gl.destroy();
		end
	);
	return dlg, gl;
end

local function getFuncItems()
	local file = fileOpen("gl/funclist.txt");
	
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

function makefunclist()
	local items, n, maxLength = getFuncItems();
	
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

function makefuncextern()
	local items, num, maxLength = getFuncItems();
	
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

function makeapiloader()
	local items, num, maxLength = getFuncItems();
	
	local outFile = fileCreate("gl/conv3_funclist.txt");
	
	local lenRationale = math.ceil((maxLength+3) / 4) * 4;
	local lenOffset = 4;
	
	local i = 1;
	
	while (i < num) do
		local item = items[i];
		local len = #item;
		
		local firstOff = lenRationale - (len+3);
		
		local newLine = "_" .. item .. " =" .. string.rep(" ", firstOff + lenOffset) .. "(" .. item .. "_t)GetProcAddress( glMainLibrary, \"" .. item .. "\" );";
		
		outFile.write(newLine .. "\n");
	
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

function makeapiloader2()
	local items, num, maxLength = getFuncItems();
	
	local outFile = fileCreate("gl/conv4_funclist.txt");
	
	local i = 1;
	
	while (i < num) do
		local item = items[i];
		local newLine = "API_LOAD( " .. item .. " );\n";
		
		outFile.write(newLine);
	
		i = i + 1;
	end
	
	outFile.destroy();
end