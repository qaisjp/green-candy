-- A utility to simplify things a tad.
local sysRegisterEventRegistrationCallback = sysRegisterEventRegistrationCallback;
local sysRegisterEventUnregistrationCallback = sysRegisterEventUnregistrationCallback;

function sysEventListenCreation(refEvents, refCallback, unrefCallback)
    local refCount = 0;
    
    local function regCallback()
        refCount = refCount + 1;
        
        if (refCount == 1) then
            refCallback();
        end
    end

    local function unregCallback()
        if (refCount == 0) then return; end;
        
        refCount = refCount - 1;
        
        if (refCount == 0) then
            unrefCallback();
        end
    end

    for m,n in ipairs(refEvents) do
        sysRegisterEventRegistrationCallback(n, regCallback);
        sysRegisterEventUnregistrationCallback(n, unregCallback);
    end
    
    return true;
end