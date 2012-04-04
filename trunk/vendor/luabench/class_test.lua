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
			
			__type = "resource";
		end
	);
end