function createResource()
	return class.construct(function()
			function __index(c, k)
				-- Secure the metamethods
				if (string.sub(k, 1, 2) == "__") then return end;
				
				return _ENV[k];
			end
			
			function __newindex(c, k, v)
				print("setting " .. k .. " to " .. tostring(v));
				_OUTENV[k] = v;
			end
			
			function rootMethod()
				print("root1 called");
				return safeDestroy();
			end
			
			function rootMethod()
				print("root2 called");
				return super();
			end
			
			function safeDestroy()
				destroy();
				
				error("destroyed resource...");
			end
			
			function destroy()
				print("destroyed resource!");
			end
			
			__type = "resource";
		end
	);
end

function createClass()
	return class.construct(function()
			function __index(c, k)
				return _OUTENV[k];
			end
	
			function __newindex(c, k, v)
				_ENV[k] = v;
			end
		end
	);
end

function tr()
	local a = createClass();
	b = createClass();
	
	function a.destroy()
		print("a destroyed");
	end
	
	function b.destroy()
		print("b destroyed");
	end
	
	a.setParent(b);
	
	function b.method()
		print(":D");
		return true;
	end
	
	function a.method()
		destroy();
	
		b.destroy();
		
		print("stage 2");
		
		--destroy();
		
		print("stage 3");
		
		return b.method();
	end
	
	a.method();
end

function bug()
	local c = {};

	for m=1,1000 do
		table.insert(c, class.construct(function() end));
	end
	
	collectgarbage("collect");
end