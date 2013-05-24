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