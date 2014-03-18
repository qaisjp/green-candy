local root = file.getRoot();
root.createDir("luabench/logs/");
root = file.createTranslator("luabench/logs/");

function createLogger(path)
	local file = root.open(path, "a");
	
	if not (file) then return false; end;
	
	return class.construct(function()
			function output(line)
				line = line .. "\n";
				
				file.write(line);
				return true;
			end
			
			function destroy()
				file.destroy();
			end
		end
	);
end

function getUniqueLogFilename()
    return "log.txt";
end