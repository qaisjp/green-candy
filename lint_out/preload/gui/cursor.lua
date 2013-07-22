function getCursorPosition()
    local window = getDrawingWindow();
    local mouseX, mouseY = win32.getCursorPos();
    local clientLeft, clientTop, clientRight, clientBottom = window.getClientRect();
    
    local x = math.min(math.max(mouseX - clientLeft, 0), clientRight - clientLeft);
    local y = math.min(math.max(mouseY - clientTop, 0), clientBottom - clientTop);
    
    -- Return 0, 0, 0 for world position as there is no world.
    return x, y, 0, 0, 0;
end

function setCursorPosition(cursorX, cursorY)
    local window = getDrawingWindow();
    local clientLeft, clientTop, clientRight, clientBottom = window.getClientRect();
    
    local x = math.min(math.max(cursorX - clientLeft, 0), clientRight - clientLeft);
    local y = math.min(math.max(cursorY - clientTop, 0), clientBottom - clientTop);
    
    return win32.setCursorPos(x, y);
end

function isCursorShowing()
    -- Cursor cannot be hidden in MTA:Lua Interpreter.
    return true;
end

function showCursor(show, toggleControls)
    if (toggleControls == nil) then toggleControls = true; end
    
    -- 
end