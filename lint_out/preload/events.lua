-- MTA event system implementation
local events = {};

function addEvent(name, remoteCall)
    local event = events[name];
    
    if (event) then return false; end;
    
    events[name] = {
        name = name,
        remoteCall = remoteCall
    };
    return true;
end

function addEventHandler(name, element, handler)
    local eventInfo = events[name];

    if not (eventInfo) then return false; end;
    
    return true;
end

function triggerEvent(name, element, ...)
    local eventInfo = events[name];
    
    if not (eventInfo) then return false; end;
    
    return true;
end

function removeEventHandler(name, element, handler)
    local eventInfo = events[name];
    
    if not (eventInfo) then return false; end;
    
    return true;
end