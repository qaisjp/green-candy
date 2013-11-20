-- Optimizations
local string = string;
local lower = string.lower;
local registerDebugProxy = registerDebugProxy;

-- Set up the general resource business
local resource = resource;
local createElement = createElement;
local guiRoot = createElement("guiroot");
local colRoot = createElement("colmodelroot");
local modelRoot = createElement("dffroot");
local txdRoot = createElement("txdroot");
local resourceRoot = createElement("resource");
local mapRoot = createElement("map");

guiRoot.reference();
colRoot.reference();
modelRoot.reference();
txdRoot.reference();
resourceRoot.reference();
mapRoot.reference();

-- Cache important prototypes.
local triggerEvent = triggerEvent;

resource.extend(
    function()
        -- Set the elements to our resource.
        _ENV.resRoot = resourceRoot;
        _ENV.guiRoot = guiRoot;
        _ENV.colRoot = colRoot;
        _ENV.dffRoot = modelRoot;
        _ENV.txdRoot = txdRoot;
        _ENV.dynRoot = mapRoot;
        
        function destroy()
            -- Trigger resource destruction.
            triggerEvent("onClientResourceStop", resourceRoot);
        
            -- Destroy our entities.
            mapRoot.destroy(); mapRoot.dereference();
            resourceRoot.destroy(); resourceRoot.dereference();
            txdRoot.destroy(); txdRoot.destroy();
            modelRoot.destroy(); modelRoot.dereference();
            colRoot.destroy(); colRoot.dereference();
            guiRoot.destroy(); guiRoot.dereference();
        end
        
        function init()
            -- Trigger resource start.
            triggerEvent("onClientResourceStart", resourceRoot);
        end
    end
);

-- Set globals.
_G.guiRoot = guiRoot;
_G.resourceRoot = resourceRoot;

local function toresource(val)
    if (type(val) == "resource") then
        return val;
    end
    
    return false;
end

-- Register all resource functions.
function call(theResource, theFunction, ...)
    return theResource.callExport(resource, theFunction, ...);
end
registerDebugProxy("call", toresource, "string", "vararg");

function fetchRemote(URL, connectionAttempts, callbackFunction, postData, postIsBinary, ...)
    return false;
end
registerDebugProxy("fetchRemote", "string", "number", "function", "string", "boolean", "vararg");

function getResourceConfig(filePath)
    -- TODO: config parameter.
    return xmlLoadFile(filePath);
end
registerDebugProxy("getResourceConfig", "string");

function getResourceDynamicElementRoot(theResource)
    return theResource.dynRoot;
end
registerDebugProxy("getResourceDynamicElementRoot", toresource);

function getResourceExportedFunctions(res)
    return res.getExports();
end
registerDebugProxy("getResourceExportedFunctions", toresource);

function getResourceFromName(resourceName)
    local resources = getResources();
    
    for m,n in ipairs(resources) do
        if (lower(n.getName()) == lower(resourceName)) then
            return n;
        end
    end
    
    return false;
end
registerDebugProxy("getResourceFromName", "string");

function getResourceGUIElement(theResource)
    if (theResource == nil) then theResource = resource; end;
    
    return theResource.guiRoot;
end
registerDebugProxy("getResourceGUIElement", toresource);

function getResourceName(res)
    return res.getName();
end
registerDebugProxy("getResourceName", toresource);

function getResourceRootElement(theResource)
    if (theResource == nil) then theResource = resource; end;
    
    return theResource.dynRoot;
end
registerDebugProxy("getResourceRootElement", { "opt", toresource });

function getThisResource()
    return resource;
end