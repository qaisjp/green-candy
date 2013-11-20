-- Optimizations
local registerDebugProxy = registerDebugProxy;

-- Standard GUI functions.
function guiGetChatboxLayout()
    return false;
end

local function toguielement(val)
    -- TODO
    return val;
end

function guiBringToFront(guiElement)
    -- TODO
    return false;
end
registerDebugProxy("guiBringToFront", toguielement);

local function toguifont(val)
    -- TODO
    return val;
end

function guiCreateFont(path, size)
    -- There is only one font type.
    return dxCreateFont(path);
end
registerDebugProxy("guiCreateFont", "string", "number");

function guiGetAlpha(guiElement)
    -- TODO
    return false;
end
registerDebugProxy("guiGetAlpha", toguielement);

function guiGetEnabled(guiElement)
    -- TODO
    return false;
end
registerDebugProxy("guiGetEnabled", toguielement);

function guiGetFont(element)
    -- TODO
    return false;
end
registerDebugProxy("guiGetFont", toguielement);

function guiGetInputEnabled()
    -- Since there is no "game", input is always focused on "GUI"
    return true;
end

function guiGetInputMode()
    -- Return the default input mode
    return "allow_binds";
end

function guiGetPosition(guiElement)
    -- TODO
    return false;
end
registerDebugProxy("guiGetPosition", toguielement);

function guiGetProperties(guiElement, relative)
    -- TODO
    return false;
end
registerDebugProxy("guiGetProperties", toguielement, "boolean");

function guiGetProperty(guiElement, property)
    -- TODO
    return false;
end
registerDebugProxy("guiGetProperty", toguielement, "string");

function guiGetScreenSize()
    return dxGetResolution();
end

function guiGetSize(guiElement, relative)
    -- TODO
    return false;
end
registerDebugProxy("guiGetSize", toguielement, "boolean");

function guiGetText(guiElement)
    -- TODO
    return "";
end
registerDebugProxy("guiGetText", toguielement);

function guiGetVisible(guiElement)
    -- TODO
    return false;
end
registerDebugProxy("guiGetVisible", toguielement);

function guiMoveToBack(guiElement)
    -- TODO
    return false;
end
registerDebugProxy("guiMoveToBack", toguielement);

function guiSetAlpha(guiElement, alpha)
    -- TODO
    return false;
end
registerDebugProxy("guiSetAlpha", toguielement, "number");

function guiSetEnabled(guiElement, enabled)
    -- TODO
    return false;
end
registerDebugProxy("guiSetEnabled", toguielement, "boolean");

function guiSetFont(guiElement, font)
    -- TODO
    return false;
end
registerDebugProxy("guiSetFont", toguielement, toguifont);

function guiSetInputEnabled(enabled)
    -- We cannot change input mode in MTA:Lua Interpreter
    return false;
end
registerDebugProxy("guiSetInputEnabled", "boolean");

function guiSetInputMode(mode)
    -- We cannot change input mode in MTA:Lua Interpreter
    return false;
end
registerDebugProxy("guiSetInputMode", "string");

function guiSetPosition(guiElement, x, y, relative)
    -- TODO
    return false;
end
registerDebugProxy("guiSetPosition", toguielement, "number", "number", "boolean");

function guiSetProperty(guiElement, property, value)
    -- TODO
    return false;
end
registerDebugProxy("guiSetProperty", toguielement, "string", "string");

function guiSetSize(guiElement, width, height, relative)
    -- TODO
    return false;
end
registerDebugProxy("guiSetSize", toguielement, "number", "number", "boolean");

function guiSetText(guiElement, text)
    -- TODO
    return false;
end
registerDebugProxy("guiSetText", toguielement, "string");

function guiSetVisible(guiElement, state)
    -- TODO
    return false;
end
registerDebugProxy("guiSetVisible", toguielement, "boolean");

function isChatBoxInputActive()
    -- We do not have a chatbox.
    return false;
end

function isConsoleActive()
    -- We do have a console which is always active!
    return true;
end

function isDebugViewActive()
    -- Just like our console this is active.
    return true;
end

function isMainMenuActive()
    -- We do not have a main menu.
    return false;
end

function isTransferBoxActive()
    -- We do not have a transfer box.
    return false;
end

function isMTAWindowActive()
    -- There are no MTA windows to be active.
    return false;
end