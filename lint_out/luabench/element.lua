local elements = {};
local elementMap = {};

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
                return true;
            end
            
            function removeEventHandler(name, handler)
                local info = events[name];
                
                if not (info) then return false; end;
                
                for m,n in ipairs(info.handlers) do
                    if (n.handler == handler) then
                        table.remove(info.handlers, m);
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
                    
                    h.handlers = newItems;
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
            end
            
            __type = "userdata";
            
            table.insert(elements, c);
            elementMap[c] = true;
        end
    );
end

function isElement(theValue)
    return not (elementMap[theValue] == nil);
end
_G.isElement = isElement;