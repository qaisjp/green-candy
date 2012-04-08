function createResource()
	return class.construct(function()
			local disallow = {
				__index = true,
				__newindex = true,
				__type = true,
				_ENV = true
			};
	
			function __index(c, k)
				if (disallow[k]) then
					return false;
				end
				
				return _ENV[k];
			end
			
			function __newindex(c, k, v)
				if (disallow[k]) then
					return false;
				end
			
				print("setting " .. k .. " to " .. tostring(v));
				_ENV[k] = v;
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
			
			__newindex = "FUCKYOU";
		end
	);
end