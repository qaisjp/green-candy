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

addCommandHandler("getrot", function()
		local x, y, z = localPlayer.getRotation();
		
		print("X: " .. x .. ", Y: " .. y .. ", Z: " .. z);
	end
);

local testVehicles = {};

addCommandHandler("testlag", function()
		local m;
		local x, y, z = localPlayer.getPosition();
		
		y = y + 15;
		z = z + 5;
		
		for m=1,100 do
			local veh = createVehicle(100, x, y, z + m * 3);
		
			if (veh) then
				table.insert(testVehicles, veh);
			end
		end
	end
);

addCommandHandler("clear", function()
		local m,n;
		
		for m,n in ipairs(testVehicles) do
			n.destroy();
		end
		
		testVehicles = {};
	end
);

local aa = {};

addCommandHandler("tt", function()
		for m=1,100 do
			table.insert(aa, class.construct(function()
				
				end
			));
		end
	end
);

addCommandHandler("cjerk", function()
		local a = class.construct(function() end);
		local b = class.construct(function() end);
		
		a.setParent(b);
		
		if (b.setParent(a)) then
			print("WARNING: circle jerk!");
		end
	end
);