local elements = {};
local elementMap = {};
local globalEventInfo = {};

-- Cache global functions which will be used a lot!
local registerDebugProxy = registerDebugProxy;

local function tdelete(t, v)
    for m,n in ipairs(t) do
        if (n == v) then
            table.remove(t, m);
            return true;
        end
    end
    
    return false;
end

local function titercopy(t)
    local tcopy = {};
    local n = 1;
    local max = #t;
    
    while (n <= max) do
        tcopy[n] = t[n];
    end
    
    return tcopy;
end

-- System functions are not meant to be exported to the official environment.
local eventInfo = {};

local function sysGetEventInfo(name)
    local info = eventInfo[name];
    
    if not (info) then
        info = {
            refCount = 0,
            regCallbacks = {},
            unregCallbacks = {}
        }
        eventInfo[name] = info;
    end
    
    return info;
end

function sysRegisterEventRegistrationCallback(name, handler)
    local info = sysGetEventInfo(name);
    
    table.insert(info.regCallbacks, handler);
    return true;
end

function sysRegisterEventUnregistrationCallback(name, handler)
    local info = sysGetEventInfo(name);
    
    table.insert(info.unregCallbacks, handler);
    return true;
end

local function sysAddEventReference(name)
    local info = sysGetEventInfo(name);
    
    for m,n in ipairs(info.regCallbacks) do
        n(info.refCount, name);
    end
    
    info.refCount = info.refCount + 1;
    return true;
end

local function sysRemoveEventReference(name)
    local info = eventInfo[name];
    
    if not (info) then
        error("event system error (no event found when unregistering)");
    end
    
    info.refCount = info.refCount - 1;
    
    for m,n in ipairs(info.unregCallbacks) do
        n(info.refCount, name);
    end
    
    if (info.refCount == 0) then
        eventInfo[name] = nil;
    end
    
    return true;
end

-- Forward declaration.
local isElement;

function createElement(elementType, elementID)
    return class.construct(
        function(c)
            local this = c;
            local attached = {};
            local attachedTo = false;
            local attachX, attachY, attachZ, attachRotX, attachRotY, attachRotZ;
            local elementData = {};
            local events = {};
            local childElements = {};
            local parentElement = false;
            local accessEnv = _ENV;
            
            do
                local publicFields = {
                    collEnabled = { false, "boolean" },
                    dimension = { 0, "number" },
                    interior = { 0, "number" },
                    x = { 0, "number" },
                    y = { 0, "number" },
                    z = { 0, "number" },
                    alpha = { 255, "number" },
                    doublesided = { false, "boolean" },
                    rotX = { 0, "number" },
                    rotY = { 0, "number" },
                    rotZ = { 0, "number" },
                    frozen = { false, "boolean" }
                };
                
                function __newindex(c, k, v)
                    local field = publicFields[k];
                    
                    if (field) then
                        local ftype = field[2];
                        
                        if (ftype == "boolean") then
                            if (v) then
                                field[1] = true;
                            else
                                field[1] = false;
                            end
                        elseif (ftype == "string") then
                            field[1] = tostring(v);
                        elseif (ftype == "number") then
                            field[1] = tonumber(v);
                        end
                        
                        return;
                    end
                    
                    accessEnv[k] = v;
                    --error("invalid field '" .. tostring(k) .. "'");
                end
                
                function __index(c, k)
                    local field = publicFields[k];
                    
                    if (field) then
                        return field[1];
                    end
                    
                    if (k == "data") then
                        return elementData;
                    end
                    
                    return _OUTENV[k];
                end
            end
            
            function attachElement(element, x, y, z, rotX, rotY, rotZ)
                for m,n in ipairs(attached) do
                    if (n == element) then
                        return false;
                    end
                end
                
                element._attach(c, x, y, z, rotX, rotY, rotZ);
            
                table.insert(attached, element);
                return true;
            end
            
            function _attach(elem, x, y, z, rotX, roY, rotZ)
                attachedTo = elem;
                attachX, attachY, attachZ, attachRotX, attachRotY, attachRotZ = x, y, z, rotX, rotY, rotZ;
            end
            
            function _detach()
                attachedTo = false;
            end
            
            function getAttachmentOffsets()
                return attachX, attachY, attachZ, attachRotX, attachRotY, attachRotZ;
            end
            
            function detachElement(element)
                return tdelete(elements, element);
            end
            
            function getAttached()
                return titercopy(attached);
            end
            
            function getAttachedTo()
                return attachedTo;
            end
            
            function getHealth()
                return 100;
            end
            
            function setID(id)
                selementID = id;
            end
            
            function getID()
                return elementID;
            end
        
            function getType()
                return elementType;
            end
            
            function addEventHandler(name, handler, propagate)
                local info = events[name];
                
                if not (info) then
                    info = {
                        handlers = {}
                    };
                    events[name] = info;
                else
                    -- If the handler already is added, return false.
                    for m,n in ipairs(info.handlers) do
                        if (n.handler == handler) then
                            return false;
                        end
                    end
                end
                
                local entry = {
                    handler = handler,
                    propagate = propagate,
                    resource = getRuntimeResource()
                };
                table.insert(info.handlers, entry);
                
                -- Notify the system.
                sysAddEventReference(name);
                return true;
            end
            
            function removeEventHandler(name, handler)
                local info = events[name];
                
                if not (info) then return false; end;
                
                for m,n in ipairs(info.handlers) do
                    if (n.handler == handler) then
                        table.remove(info.handlers, m);
                        
                        -- Notify the system.
                        sysRemoveEventReference(name);
                        return true;
                    end
                end
                
                return false;
            end
            
            local function executeEventHandlers(name, source, ...)
                local info = events[name];
                
                if not (info) then return false; end;
                
                for m,n in ipairs(info.handlers) do
                    n.handler(...);
                end
            end
            
            function executeEventHandlersParent(name, source, ...)
                executeEventHandlers(name, source, ...);
                
                if (parentElement) then
                    parentElement.executeEventHandlersParent(name, source, ...);
                end 
            end
            
            function executeEventHandlersChild(name, source, ...)
                executeEventHandlers(name, source, ...);
                
                for m,n in ipairs(childElements) do
                    n.executeEventHandlersChild(name, source, ...);
                end
            end
            
            function triggerEvent(name, ...)
                local info = events[name];
                
                if not (info) then return false; end;
                
                executeEventHandlersParent(name, c, ...);
                
                for m,n in ipairs(childElements) do
                    n.executeEventHandlersChild(name, c, ...);
                end
                
                return true;
            end
            
            function triggerEventSys(name, ...)
                local info = events[name];
                
                if not (info) then return false; end;
                
                executeEventHandlersParent(name, c, ...);
                return true;
            end
            
            function cleanUpForResource(res)
                for m,n in pairs(events) do
                    local i = 1;
                    local itemArray = n.handlers;
                    local maxItems = #itemArray;
                    local newItems = {};
                    local newNumItems = 0;
                    
                    while (i <= maxItems) do
                        local item = itemArray[i];
                        
                        if not (item.resource == res) then
                            newNumItems = newNumItems + 1;
                            newItems[newNumItems] = item;
                        end
                        
                        i = i + 1;
                    end
                    
                    n.handlers = newItems;
                end
            end
            
            function isValidChild(child)
                return isElement(child);
            end
            
            function setChild(child)
                local childAPI = super(child);
            
                if (isElement(child)) then
                    table.insert(childElements, child);
                    
                    childAPI.extend(
                        function()
                            function notifyDestroy()
                                tdelete(childElements, child);
                            end
                        end
                    );
                end
                
                return childAPI;
            end
            
            function setParent(parent)
                local res = super(parent);
            
                if (res) then
                    if (isElement(parent)) then
                        parentElement = parent;
                    else
                        parentElement = false;
                    end
                end
                
                return res;
            end
            
            function destroy()
                while not (#attached == 0) do
                    n.element._detach();
                end
            
                elementMap[c] = nil;
                tdelete(elements, c);
                
                -- Remove event references
                for m,n in pairs(events) do
                    for j,k in ipairs(n.handlers) do
                        sysRemoveEventReference(m);
                    end
                end
            end
            
            __type = "userdata";
            
            table.insert(elements, c);
            elementMap[c] = true;
        end
    );
end
registerDebugProxy("createElement", "string", { "opt", "string" });

-- Make sure we clean up for resources when they terminate.
registerResourceExtension(
    function(res)
        function destroy()
            for m,n in ipairs(elements) do
                n.cleanUpForResource(res);
            end
        end
    end
);

local function isElement(theValue)
    return not (elementMap[theValue] == nil);
end
_G.isElement = isElement;

function destroyElement(elementToDestroy)
    elementToDestroy.destroy();
    return true;
end
registerDebugProxy("destroyElement", isElement);

function getAttachedElements(theElement)
    return theElement.getAttached();
end
registerDebugProxy("getAttachedElements", isElement);

function getElementAttachedOffsets(theElement)
    return theElement.getAttachmentOffsets();
end
registerDebugProxy("getElementAttachedOffsets", isElement);

function getElementCollisionsEnabled(theElement)
    return theElement.collEnabled;
end
registerDebugProxy("getElementCollisionsEnabled", isElement);

function getElementAttachedTo(theElement)
    return theElement.getAttachedTo();
end
registerDebugProxy("getElementAttachedTo", isElement);

function getElementBoundingBox(theElement)
    return -1, -1, -1, 1, 1, 1;
end
registerDebugProxy("getElementBoundingBox", isElement);

function getElementByID(id, index)
    if (index == nil) then index = 0; end
    
    local curIndex = 0;
    
    for m,n in ipairs(elements) do
        if (n.getID() == id) then
            if (curIndex == index) then
                return n;
            end
            
            curIndex = curIndex + 1;
        end
    end
    
    return false;
end
registerDebugProxy("getElementByID", "string", { "opt", "number" });

function getElementChild(parent, index)
    index = index + 1;
    return parent.getChildren()[index];
end
registerDebugProxy("getElementChild", isElement, "number");

function getElementChildren(parent, theType)
    local children = parent.getChildren();
    
    if (theType == nil) then return children; end;
    
    local newList = {};
    
    for m,n in ipairs(children) do
        if (n.getType() == theType) then
            table.insert(newList, n);
        end
    end
    
    return newList;
end
registerDebugProxy("getElementChildren", isElement, { "opt", "string" });

function getElementChildrenCount(parent)
    return #parent.getChildren();
end
registerDebugProxy("getElementChildrenCount", isElement);

function getElementData(theElement, key, inherit)
    if (inherit == nil) then inherit = true; end
    
    return theElement.data[key];
end
registerDebugProxy("getElementData", isElement, "string", { "opt", "boolean" });

function getElementDimension(theElement)
    return theElement.dimension;
end
registerDebugProxy("getElementDimension", isElement);

function getElementDistanceFromCentryOfMassToBaseOfModel(theElement)
    return 1;
end
registerDebugProxy("getElementDistanceFromCentryOfMassToBaseOfModel", isElement);

function getElementHealth(theElement)
    return theElement.getHealth();
end
registerDebugProxy("getElementHealth", isElement);

function getElementID(theElement)
    return theElement.getID();
end
registerDebugProxy("getElementID", isElement);

function getElementInterior(theElement)
    return theElement.interior;
end
registerDebugProxy("getElementInterior", isElement);

function getElementParent(theElement)
    return theElement.getParent();
end
registerDebugProxy("getElementParent", isElement);

function getElementPosition(theElement)
    return theElement.x, theElement.y, theElement.z;
end
registerDebugProxy("getElementPosition", isElement);

function getElementRadius(theElement)
    return 1.5;
end
registerDebugProxy("getElementRadius", isElement);

-- Forward declaration.
local _insertChildrenElementsByType;

function _insertChildrenElementsByType(list, parent, theType)
    local children = parent.getChildren();
    
    for m,n in ipairs(children) do
        if (n.getType() == theType) then
            table.insert(list, n);
        end
    
        _insertChildrenElementsByType(list, n, theType);
    end
end

function getElementsByType(theType, startat, streamedIn)
    if (startat == nil) then startat = root; end
    if (streamedIn == nil) then streamedIn = false; end
    
    -- All elements are streamed in by default.
    
    local newList = {};
    
    if (startat == root) then
        for m,n in ipairs(elements) do
            if (n.getType() == theType) then
                table.insert(newList, n);
            end
        end
    else
        _insertChildrenElementsByType(newList, startat);
    end
    
    return newList;
end
registerDebugProxy("getElementsByType", "string", { "opt", isElement }, { "opt", "boolean" });

function getElementsWithinColShape(shape, elemType)
    -- We do not do colshapes on MTA:Lua Interpreter
    return {};
end
registerDebugProxy("getElementsWithinColShape", isElement, { "opt", "string" });

function getElementType(theElement)
    return theElement.getType();
end
registerDebugProxy("getElementType", isElement);

function getElementVelocity(theElement)
    return 0, 0, 0;
end
registerDebugProxy("getElementVelocity", isElement);

function getRootElement()
    return root;
end

function isElementAttached(theElement)
    return not (theElement.getAttachedTo() == false);
end
registerDebugProxy("isElementAttached", isElement);

function isElementLocal(theElement)
    -- MTA:Lua Interpreter only supports local elements.
    return true;
end
registerDebugProxy("isElementLocal", isElement);

function isElementOnScreen(theElement)
    -- We cannot render elements, so they cannot be on screen.
    return false;
end
registerDebugProxy("isElementOnScreen", isElement);

function isElementStreamable(theElement)
    -- No element is streamable
    return false;
end
registerDebugProxy("isElementStreamable", isElement);

function isElementStreamedIn(theElement)
    -- Every element is streamed in though.
    return true;
end
registerDebugProxy("isElementStreamedIn", isElement);

function isElementWithinColShape(theElement, theShape)
    -- We do not do colshape checks.
    return false;
end
registerDebugProxy("isElementWithinColShape", isElement, isElement);

function setElementAlpha(theElement, alpha)
    theElement.alpha = alpha;
    return true;
end
registerDebugProxy("setElementAlpha", isElement, "number");

function setElementCollisionsEnabled(theElement, enabled)
    theElement.collEnabled = enabled;
    return true;
end
registerDebugProxy("setElementCollisionsEnabled", isElement, "boolean");

function setElementData(theElement, key, value, synchronize)
    if (synchronize == nil) then synchronize = true; end
    
    -- We do not synchronize data.
    
    theElement.data[key] = value;
    return true;
end
registerDebugProxy("setElementData", "string", "any", { "opt", "boolean" });

function setElementDimension(theElement, dimension)
    theElement.dimension = dimension;
    return true;
end
registerDebugProxy("setElementDimension", isElement, "number");

function setElementDoubleSided(theElement, enable)
    theElement.doublesided = enable;
    return true;
end
registerDebugProxy("setElementDoubleSided", isElement, "boolean");

function setElementHealth(theElement, newHealth)
    -- Cannot damage elements.
    return false;
end
registerDebugProxy("setElementHealth", isElement, "number");

function setElementID(theElement, name)
    theElement.setID(name);
    return true;
end
registerDebugProxy("setElementID", isElement, "string");

function setElementInterior(theElement, interior, x, y, z)
    theElement.interior = interior;
    theElement.x, theElement.y, theElement.z = x, y, z;
    return true;
end
registerDebugProxy("setElementInterior",
    isElement, "number", "number", "number", "number"
);

function setElementParent(theElement, parent)
    return theElement.setParent(parent);
end
registerDebugProxy("setElementParent", isElement, isElement);

function setElementPosition(theElement, x, y, z, warp)
    if (warp == nil) then warp = true; end
    
    theElement.x, theElement.y, theElement.z = x, y, z;
    return true;
end
registerDebugProxy("setElementPosition",
    isElement, "number", "number", "number", { "opt", "boolean" }
);

function setElementStreamable(theElement, streamable)
    -- No elements can be streamable.
    return false;
end
registerDebugProxy("setElementStreamable", isElement, "boolean");

function setElementVelocity(theElement, x, y, z)
    -- Elements cannot have velocity.
    return false;
end
registerDebugProxy("setElementVelocity",
    isElement, "number", "number", "number"
);

function detachElements(theElement, theAttachToElement)
    local attachedTo = theElement.getAttachedTo();

    if not (theAttachToElement == nil) then
        if not (attachedTo == theAttachToElement) then
            return false;
        end
    end
    
    attachedTo.detachElement(theElement);
    return true;
end
registerDebugProxy("detachElements", isElement, { "opt", isElement });

function getElementColShape(theElement)
    -- The interpreter does not support col shapes.
    return false;
end
registerDebugProxy("getElementColShape", isElement);

function getElementMatrix(theElement)
    -- We do not support element matrices.
    return false;
end
registerDebugProxy("getElementMatrix", isElement);

function getElementModel(theElement)
    -- We do not support element models.
    return false;
end
registerDebugProxy("getElementModel", isElement);

function getElementRotation(theElement)
    return theElement.rotX, theElement.rotY, theElement.rotZ;
end
registerDebugProxy("getElementRotation", isElement);

function isElementInWater(theElement)
    -- There is no water!
    return false;
end
registerDebugProxy("isElementInWater", isElement);

function isElementSyncer(theElement)
    -- Yes, we sync everything ourselves.
    return true;
end
registerDebugProxy("isElementSyncer", isElement);

function setElementModel(theElement, model)
    -- No model support for now.
    return false;
end
registerDebugProxy("setElementModel", isElement, "number");

function setElementRotation(theElement, rotX, rotY, rotZ)
    theElement.rotX, theElement.rotY, theElement.rotZ = rotX, rotY, rotZ;
    return true;
end
registerDebugProxy("setElementRotation",
    isElement, "number", "number", "number"
);

function isElementCollidableWith(theElement, withElement)
    -- There are no collision checks!
    return false;
end
registerDebugProxy("isElementCollidableWith", isElement, isElement);

function setElementCollidableWith(theElement, withElement, enabled)
    -- No, we do not support this.
    return false;
end
registerDebugProxy("setElementCollidableWith", isElement, isElement, "boolean");

function isElementDoubleSided(theElement)
    return theElement.doublesided;
end
registerDebugProxy("isElementDoubleSided", isElement);

function setLowLODElement(theElement, lowLODElement)
    -- We ain't rendering anything, so screw LODs.
    return false;
end
registerDebugProxy("setLowLODElement", isElement, { "opt", isElement });

function getLowLODElement(theElement)
    return false;
end
registerDebugProxy("getLowLODElement", isElement);

function isElementLowLOD(theElement)
    return false;
end
registerDebugProxy("isElementLowLOD", isElement);

function attachElementToElement(theElement, theAttachToElement, xPosOffset, yPosOffset, zPosOffset, xRotOffset, yRotOffset, zRotOffset)
    if (xPosOffset == nil) then xPosOffset = 0; end
    if (yPosOffset == nil) then yPosOffset = 0; end
    if (zPosOffset == nil) then zPosOffset = 0; end
    if (xRotOffset == nil) then xRotOffset = 0; end
    if (yRotOffset == nil) then yRotOffset = 0; end
    if (zRotOffset == nil) then zRotOffset = 0; end

    return theAttachToElement.attachElement(theElement, xPosOffset, yPosOffset, zPosOffset, xRotOffset, yRotOffset, zRotOffset);
end
registerDebugProxy("attachElementToElement",
    isElement, isElement, { "opt", "number" }, { "opt", "number" }, { "opt", "number" }, { "opt", "number" }, { "opt", "number" }, { "opt", "number" }
);

function detachElementFromElement(theElement, theAttachToElement)
    local attachedTo = theElement.getAttachedTo();
    
    if not (theAttachToElement == nil) then
        if not (attachedTo == theAttachToElement) then
            return false;
        end
    end
    
    attachedTo.detachElement(theElement);
    return true;
end
registerDebugProxy("detachElementFromElement", isElement, { "opt", isElement });

function isElementFrozen(theElement)
    return theElement.frozen;
end
registerDebugProxy("isElementFrozen", isElement);

function setElementFrozen(theElement, freezeStatus)
    theElement.frozen = freezeStatus;
    return true;
end
registerDebugProxy("setElementFrozen", isElement);

function setElementMatrix(theElement, theMatrix)
    -- No matrices for you!
    return false;
end
registerDebugProxy("setElementMatrix", isElement, "table");

-- Create the root element.
root = createElement("root");