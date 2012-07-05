print("SysUtils loading...");

addCommandHandler("test", function(...)
		local args = { ... };
		print("hello world!" .. #args);
	end
);

addCommandHandler("tr", function()
		local a = createClass();
		local b = createClass();
		local c = createClass();
		
		a.setParent(b);
		
		function b.testme()
			a.destroy();
			return true;
		end
		
		function a.lol()
			b.destroy();
			
			b.testme();
		end
		
		a.lol();
	end
);

addCommandHandler("ss", function(id)
		playSoundFrontEnd(tonumber(id));
	end
);

addCommandHandler("run", function(...)
		local cmdline = table.concat({...}, " ");
		
		local p, e = loadstring("return " .. cmdline);
		
		if not (p) then
			p, e = loadstring(cmdline);
			
			if not (p) then
				error(e);
			end
		end
		
		local ret = { p() };
		
		if (#ret == 0) then
			print("Command successful!");
			return true;
		end
		
		local output = "Result: ";
		
		for m,n in ipairs(ret) do
			output = output .. tostring(n) .. " [" .. type(n) .. "], ";
		end
		
		print(output);
	end
);

addCommandHandler("psvc", function(id)
		if not (id) then
			print("Need model id!");
			return false;
		end
		
		local matrix = localPlayer.getMatrix();
		local veh = createVehicle(tonumber(id), matrix.offset(0, 5, 2));
		veh.setRotation(localPlayer.getRotation());
	end
);