-- Standard GUI functions.
function guiGetChatboxLayout()
    return false;
end

function guiBringToFront(guiElement)
    -- TODO
    return false;
end

function guiCreateFont(path, size)
    -- There is only one font type.
    return dxCreateFont(path);
end

function guiGetAlpha(guiElement)
    -- TODO
    return false;
end

function guiGetEnabled(guiElement)
    -- TODO
    return false;
end

function guiGetFont(element)
    -- TODO
    return false;
end

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

function guiGetProperties(guiElement, relative)
    -- TODO
    return false;
end

function guiGetProperty(guiElement, property)
    -- TODO
    return false;
end

function guiGetScreenSize()
    return dxGetResolution();
end

function guiGetSize(guiElement, relative)
    -- TODO
    return false;
end

function guiGetText(guiElement)
    -- TODO
    return "";
end

function guiGetVisible(guiElement)
    -- TODO
    return false;
end

function guiMoveToBack(guiElement)
    -- TODO
    return false;
end

function guiSetAlpha(guiElement, alpha)
    -- TODO
    return false;
end

function guiSetEnabled(guiElement, enabled)
    -- TODO
    return false;
end

function guiSetFont(guiElement, font)
    -- TODO
    return false;
end

function guiSetInputEnabled(enabled)
    -- We cannot change input mode in MTA:Lua Interpreter
    return false;
end

function guiSetInputMode(mode)
    -- We cannot change input mode in MTA:Lua Interpreter
    return false;
end

function guiSetPosition(guiElement, x, y, relative)
    -- TODO
    return false;
end

function guiSetProperty(guiElement, property, value)
    -- TODO
    return false;
end

function guiSetSize(guiElement, width, height, relative)
    -- TODO
    return false;
end

function guiSetText(guiElement, text)
    -- TODO
    return false;
end

function guiSetVisible(guiElement, state)
    -- TODO
    return false;
end

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

function setCursorAlpha(alpha)
    -- We do not have a cursor.
    return false;
end

function getCursorAlpha()
    -- We do not have a cursor.
    return 0;
end