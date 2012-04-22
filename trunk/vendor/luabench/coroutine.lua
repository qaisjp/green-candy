local yield = coroutine.yield;

function createAsyncFile()
	return class.construct(function()
			function read()
				yield();
				return "";
			end
		end
	);
end