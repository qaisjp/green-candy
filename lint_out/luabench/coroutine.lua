local yield = coroutine.yield;

function createAsyncFile()
	return class.construct(function()
			function read()
				yield();
				return "";
			end
			
			function destroy()
				print("destroyed async file");
			end
		end
	);
end

function corotentry()
	local file = createAsyncFile();
	
	return file.read();
end