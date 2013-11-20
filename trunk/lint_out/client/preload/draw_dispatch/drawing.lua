-- Launch the MTA window if "onClientRender" event is issued.
local drawRefCount = 0;
local root = root;

local renderHandler = false;

local function renderRegCallback()
    drawRefCount = drawRefCount + 1;

    if not (drawRefCount == 1) then return; end;
    
    local gl = dxGetDriver();
    
    function renderHandler()
        -- Clear the current screen
        gl.clear("color");
        gl.clear("depth");
    
        -- Notify the environment so it can draw on the screen.
        root.triggerEventSys("onClientPreRender");
        root.triggerEventSys("onClientHUDRender");
        root.triggerEventSys("onClientRender");
        
        -- Present all drawing operations issued by the event handlers.
        gl.present();
    end
    
    gl.addEventHandler("onFrame", renderHandler);
end

local function renderUnregCallback()
    drawRefCount = drawRefCount - 1;
    
    if not (drawRefCount == 0) then return; end;
    
    -- TODO: terminate drawing environment.
    -- Maybe reference the driver (so here it would be dereferenced)
    gl = dxGetDriver();
    
    gl.removeEventHandler("onFrame", renderHandler);
end

do
    local sysRegisterEventRegistrationCallback = sysRegisterEventRegistrationCallback;
    sysRegisterEventRegistrationCallback("onClientRender", renderRegCallback);
    sysRegisterEventRegistrationCallback("onClientPreRender", renderRegCallback);
    sysRegisterEventRegistrationCallback("onClientHUDRender", renderRegCallback);
    sysRegisterEventUnregistrationCallback("onClientRender", renderUnregCallback);
    sysRegisterEventUnregistrationCallback("onClientPreRender", renderUnregCallback);
    sysRegisterEventUnregistrationCallback("onClientHUDRender", renderUnregCallback);
end