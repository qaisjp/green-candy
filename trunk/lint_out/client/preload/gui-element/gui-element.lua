-- MTA CEGUI implementation.
local gui_elements = {};

function guiCreateElement()
    local element = createElement("gui-element");
    
    -- For now a stub.
    
    return element;
end

function toguielement(obj)
    if not (gui_elements[obj] == nil) then
        return obj;
    end
    
    return false;
end