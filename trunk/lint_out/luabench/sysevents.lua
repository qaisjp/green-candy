-- MTA:Lua Interpreter system events
local event_info = {};

function sysAddEvent(name)
    local info = event_info[name];
    
    if (info) then return true; end;
    
    event_info[name] = {
        handlers = {}
    };
    return true;
end

function sysAddEventHandler(name, handler)
    local info = event_info[name];
    
    if not (info) then return false; end;
    
    table.insert(info.handlers, handler);
    return true;
end

function sysTriggerEvent(name, ...)
    local info = event_info[name];
    
    if not (info) then return false; end;
    
    local result = true;
    
    for m,n in ipairs(info.handlers) do
        if (n(...) == false) then
            result = false;
        end
    end
    
    return result;
end