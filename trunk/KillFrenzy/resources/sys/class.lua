function table.delete(t, v)
	local m,n;
	
	for m,n in ipairs(t) do
		if (n == v) then
			table.remove(t, m);
			return true;
		end
	end
	
	return false;
end

local classNoAccess = {
    super = true
};

local outputDebugString = print;

local function createClass(members, forcedSuper)
    local class = members or {};
    local meta = {};
    local methods = {};
    local classmeta = {};
    local methodenv = {};
    local forceSuperHandlers = {};
    local m,n;
    local reqDestruction = false;
	local destroying = false;
    local inMethod = 0;
    local refCount = 0;
    local lastDestroy;
    local children = {};
	local cAPIs = {};
	local childAPI;
    local _G = debug.getfenv(debug.getinfo(2).func);
	local setmetatable = setmetatable;
    
	-- Compatibility values
	class._ENV = methodenv;
	
    if (forcedSuper) then
        for m,n in pairs(forcedSuper) do
            forceSuperHandlers[m] = function(method)
                local submethod = methods[m];
                local function subCall()
                    error("cannot call submethod in '" .. m .. "'", 2);
                end
            
                if (submethod) then
                    return function(...)
                        local previous = methods.super;
                        
                        methods.super = subCall;
                    
                        inMethod = inMethod + 1;
                        method(...);
                        inMethod = inMethod - 1;
                        
                        methods.super = previous;
                        return submethod(...);
                    end
                end
                
                return function(...)
                    local previous = methods.super;
                    
                    methods.super = subCall;
                
                    inMethod = inMethod + 1;
                    method(...);
                    inMethod = inMethod - 1;
                    
                    methods.super = previous;
                    
                    if (reqDestruction) and (inMethod == 0) then
                        class.destroy();
                    end
                end
            end
        end
    end
	
	local function preDestructor()
		local destructor = methods.destroy;
		
		function methods.destroy()
			return false;
		end
		
		destroying = true;
		
		local m,n;
		
		n = 1;
	
        while (n <= #children) do
			local child = children[n];
			
            if not (child.isDestroying()) or not (child.destroy()) then
				n = n + 1;
			end
        end
		
		if not (#children == 0) then
			outputDebugString("DEBUG " .. class.getType(), 2);
		
			for m,n in pairs(cAPIs) do
				function n.cleanUp()
					if not (#children == 0) then return; end;
					
					outputDebugString("DESTR " .. class.getType(), 1);
					
					destructor();
				end
			end
			
			return false;
		end
		
		if (childAPI) then
			childAPI.destroy();
		end
		
		rawset(class, "children", {});
		return true;
	end
    
    function forceSuperHandlers.destroy(method)
        local inherit;
        
        if not (lastDestroy) then
            function inherit()
                if not (inMethod == 0) then
                    reqDestruction = true;
                    return false;
                end
				
				reqDestruction = false;
				
				if not (preDestructor()) then
					return false;
				end
                
                method();
				return true;
            end
            
            lastDestroy = method;
        else
            local prev = lastDestroy;

            function inherit()
                if not (inMethod == 0) then
                    reqDestruction = true;
                    return false;
                end
				
				reqDestruction = false;
				
				if not (preDestructor()) then
					return false;
				end
                
                method();
                prev();
				return true;
            end
            
            function lastDestroy()
                method();
                return prev();
            end
        end
        
        return inherit;
    end
    
    function classmeta.__index(t, key)
		local item = rawget(methods, key);
	
        if not (item == nil) then
            return item;
        end
		
		item = rawget(class, key);
		
		if not (item == nil) then
			return item;
		end
		
        return _G[key];
    end
    
    function classmeta.__newindex(t, key, value)
        if (type(value) == "function") then
            class[key] = value;
            return true;
        elseif (methods[key]) then
            error("cannot overwrite method '" .. key .. "'", 2);
        end
        
        rawset(class, key, value);
        return true;
    end
    
    classmeta.__metatable = false;
    setmetatable(methodenv, classmeta);
    
    function methods.__newindex(tab, key, value)
        if (classNoAccess[key]) then
            error("invalid member overload '" .. key .. "'", 2);
        end
		
        if (type(value) == "function") then
            local method;
            local superHandler = forceSuperHandlers[key];
        
            if (superHandler) then
                method = superHandler(value);
            else
                local submethod = methods[key];
                
                if (submethod) then
                    function method(...)
                        local previous = methods.super;
                        local ret;
                    
                        methods.super = submethod;
                    
                        inMethod = inMethod + 1;
                        ret = { value(...) };
                        inMethod = inMethod - 1;
                        
                        methods.super = previous;
                        
                        if (reqDestruction) and (inMethod == 0) then
                            class.destroy();
                        end
                        
                        return unpack(ret);
                    end
                else
                    function method(...)
                        local previous = methods.super;
                        local ret;
                    
                        methods.super = false;
                    
                        inMethod = inMethod + 1;
                        ret = { value(...) };
                        inMethod = inMethod - 1;
                        
                        methods.super = previous;
                        
                        if (reqDestruction) and (inMethod == 0) then
                            class.destroy();
                        end
                        
                        return unpack(ret);
                    end
                end
            end
            
            setfenv(value, methodenv);
            
            rawset(methods, key, method);
            return true;
        elseif (methods[key]) then
            error("'" .. key .. "' cannot overwrite method with " .. type(value), 2);
        end
		
        rawset(class, key, value);
        return true;
    end
    
    function methods.__index(t, k)
		return rawget(methods, k);
	end
    
    -- Make sure this stays a class
    methods.__metatable = class;
    setmetatable(class, methods);
    
    -- Reference system to avoid illegal destructions
    function class.reference()
        local m,n;
        
        for m,n in ipairs(children) do
            n.reference();
        end
        
        inMethod = inMethod + 1;
        refCount = refCount + 1;
        return true;
    end
	
	function class.incrementMethodStack()
		inMethod = inMethod + 1;
	end
	
	function class.decrementMethodStack()
		inMethod = inMethod - 1;
	end
    
    function class.dereference()
        local n = 1;

        while (n <= #children) do
            if (children[n].dereference()) then
                n = n + 1;
            end
        end
        
        inMethod = inMethod - 1;
        refCount = refCount - 1;
        return not (reqDestruction) or not (inMethod == 1);
    end
    
    function class._aref(cnt)
        inMethod = inMethod + cnt;
        refCount = cnt;
        return true;
    end
	
	function class._dref()
		inMethod = inMethod - refCount;
		refCount = 0;
		return true;
	end
    
    function class.getType()
        return "class";
    end
	
	function class.setGlobal(g)
		_G = g;
		return true;
	end
	
	function class.getGlobal()
		return _G;
	end
    
    function class.setChild(child)
        local childAPI = createClass();
		
		function childAPI.cleanUp()
			return;
		end
        
        function childAPI.destroy()
			child._dref();
			
            table.delete(children, child);
			cAPIs[child] = nil;
			
			cleanUp();
        end
        
        child._aref(refCount);
        
        table.insert(children, child);
		cAPIs[child] = childAPI;
        return childAPI;
    end
	
	function class.setParent(c)
		if (childAPI) then
			childAPI.destroy();
		end
	
		childAPI = c.setChild(class);
		return childAPI;
	end
	
	function class.isDestroying()
		return destroying;
	end
    
    function class.getChildren()
        return children;
    end
    
    class.children = children;
    
    -- Every class is destroyable
    function class.destroy()
        methods.__metatable = nil;
		
        setmetatable(class, {});
        
        methods = nil;
        children = nil;
        lastDestroy = nil;
        forceSuperHandlers = nil;
        methodenv = nil;
        meta = nil;
        classmeta = nil;
        class = nil;
    end
    
    return class, methodenv;
end
_G.createClass = createClass;