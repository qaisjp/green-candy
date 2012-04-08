function createResource()
	return class.construct(function()
			function __index(c, k)
				print("index " .. k);
			
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
			end
			
			function rootMethod()
				print("root2 called");
				return super();
			end
			
			function safeDestroy()
				destroy();
				
				print("called destructor");
				return true;
			end
			
			function destroy()
				print("destroying class");
			end
			
			__type = "resource";
		end
	);
end