--[[
    MTA:Lua debug utilities.
    
    This library was written to emulate the argStream abilities
    of MTA. It allows you to register debug functions that are
    gateways to the original functions. They ensure that arguments
    are passed properly to the original function.
    
    To use this library you have to create a debug environment and
    wrap along your Lua script I/O.
]]
local math = math;
local min = math.min;
local rawtype = rawtype;
local type = type;
local error = error;
local print = print;
local class = class;

-- Create compatibility layer between MTA:Lua and regular Lua.
-- raw type retriever.
local type_proxy = rawtype;

if not ( type_proxy ) then
    type_proxy = type;
end

-- environment table creator.
local createEnvTable = nil;

if ( class ) then
    function createEnvTable()
        local theEnv = false;
    
        local myEnv = class.construct(
            function()
                theEnv = _ENV;
            end
        );
        
        return theEnv;
    end
else
    function createEnvTable()
        return {};
    end
end

function createDebugArgParser()
    local debugParser = createEnvTable();

    local error_callback = false;
    local proxies = {};
    local argTypeHandlers = {};
    local _stackErrorDepth = 0;

    local function outError(name, msg, errorDepth)
        errorDepth = errorDepth + 2 + _stackErrorDepth;
        
        if (error_callback) then
            return error_callback(c, name, msg, debug.getinfo(errorDepth));
        end
        
        error(msg .. " [in " .. name .. "]", errorDepth);
    end
    
    local function compareArgumentTypes(arg, argType, expectedType)
        local handler = argTypeHandlers[argType];
        
        if (handler) then
            local ret = handler(arg, argType);
            
            -- Force a resolve to true.
            if not (ret == true) then
                return false, "failed resolve";
            end
        end
        
        if ( rawtype ) then
            local expTypeType = rawtype(expectedType);
        
            -- Try individual callbacks
            if (expTypeType == "function") then
                if not (expectedType(arg, argType)) then
                    return false, "failed intricate resolve";
                end
            elseif (expTypeType == "string") then
                -- Do the regular check.
                if not (argType == expectedType) then
                    return false, "check failed";
                end
            end
        end
        
        -- Return true if all checks passed.
        return true;
    end
    
    local function outArgError(funcName, argNum, arg, argType, expectedType, errorDepth, appendix)
        local msg = "invalid argument #" .. argNum .. " type '" .. argType .. "' (";
        
        if (appendix) then
            msg = msg .. appendix;
        end
        
        if ( rawtype ) then
            local realtype = tostring(type(arg));
            
            if (rawtype(realtype) == "string") and not (realtype == argType) then
                msg = msg .. ", realtype '" .. realtype .. "'";
            end
        end
        
        if (type(expectedType) == "string") then
            msg = msg .. ", expected '" .. expectedType .. "'";
        end
        
        msg = msg .. ")";
        
        return outError(funcName, msg, errorDepth + 1);
    end
    
    local function checkArgumentTypes(funcName, argNum, arg, argType, expectedType, errorDepth)
        local success, err = compareArgumentTypes(arg, argType, expectedType, errorDepth + 1);
        
        if not (success) then
            return outArgError(funcName, argNum, arg, argType, expectedType, errorDepth + 1, err);
        end
        
        return true;
    end

    function debugParser.addDebugProxy(name, ...)
        if (proxies[name]) then
            print("WARNING: overwritting debug argStream proxy declaration '" .. name .. "'");
        end
    
        local argInfo = { ... };
        local infoCount = #argInfo;
        
        local proxy = {};
        proxy.super = false;
        
        proxy.handler = function(...)
            local args = { ... };
            local n = 1;
            local count = min(#args, infoCount);
            
            while (n <= count) do
                local arg = args[n];
                local this_type = type_proxy(arg);
                local exp_type = argInfo[n];
                local d_type = type_proxy(exp_type);
                
                if (d_type == "table") then
                    local i = 1;
                    local d_count = #exp_type;
                    local kind = exp_type[1];
                    local isOptional = false;
                    
                    if (kind == "opt") then
                        i = i + 1;
                        isOptional = true;
                    end
                    
                    if (i <= d_count) then
                        local found = false;
                        
                        while (i <= d_count) do
                            local expectedType = exp_type[i];
                        
                            if (expectedType == "any") or (compareArgumentTypes(arg, this_type, expectedType)) then
                                found = true;
                                break;
                            end
                            
                            i = i + 1;
                        end
                        
                        if (found == false) and not ((isOptional) and (this_type == "nil")) then
                            return outArgError(name, n, arg, this_type, exp_type, 1);
                        end
                    end
                elseif not (exp_type == "any") and not (exp_type == "vararg") and not (checkArgumentTypes(name, n, arg, this_type, exp_type, 1)) then
                    return false;
                end
            
                n = n + 1;
            end
            
            -- Continue the argument count.
            -- We have too many arguments.
            while (n <= infoCount) do
                local info = argInfo[n];
                local d_type = type_proxy(info);
                
                if (d_type == "table") then
                    if not (info[1] == "opt") then
                        outError(name, "invalid argument count (got " .. count ..", expected " .. infoCount ..")");
                        return false;
                    end
                elseif not ((d_type == "string") and (info == "vararg")) then
                    outError(name, "invalid argument count (got " .. count ..", expected " .. infoCount ..")");
                    return false;
                end
                
                n = n + 1;
            end
            
            -- TODO: we might need to unpack the args table if we allow
            -- editing it later.
            return proxy.super(...);
        end
        
        proxies[name] = proxy;
    end
    
    -- Arg type handlers are called when a specific type is found in the arguments.
    -- They establish that the given type is proper. They return a boolean.
    -- If said boolean is true, the argument is good; if it resolves to false, the
    -- function call will fail.
    function debugParser.setArgumentTypeCallback(typeName, callback)
        argTypeHandlers[typeName] = callback;
        return true;
    end
    
    -- Attempt to get a callback for said function.
    function debugParser.getProxy(name, super)
        local proxy = proxies[name];
        
        if not (proxy) then return false; end;
        
        proxy.super = super;
        return proxy.handler;
    end
    
    -- Lazy automatic callback.
    function debugParser.parseEnvironment(env)
        for m,n in pairs(env) do
            if (type_proxy(n) == "function") then
                local proxy = getProxy(m, n);
                
                if (proxy) then
                    env[m] = proxy;
                end
            end
        end
        
        return true;
    end
    
    function debugParser.clear()
        -- Reset all information, so that we can reuse this class.
        proxies = {};
    end
    
    function debugParser.setErrorCallback(callback)
        error_callback = callback;
        return true;
    end
    
    function debugParser.destroy()
        proxies = nil;
    end
    
    return debugParser;
end