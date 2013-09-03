-- Optimizations
local registerDebugProxy = registerDebugProxy;
local getDrawingWindow = getDrawingWindow;
local math = math;
local min = math.min;
local max = math.max;
local win32 = win32;

function getCursorPosition()
    local window = getDrawingWindow();
    local mouseX, mouseY = win32.getCursorPos();
    
    if not (mouseX) then return false; end;
    
    -- Convert to client coordinates.
    mouseX, mouseY = window.getClientOffset(mouseX, mouseY);
    
    local clientLeft, clientTop, clientRight, clientBottom = window.getClientRect();
    
    local x = mouseX - clientLeft;
    local y = mouseY - clientTop;
    
    if (x < 0) or (x >= clientRight) or (y < 0) or (y >= clientBottom) then
        -- Returning false means the cursor is out of the screen.
        return false;
    end
    
    -- Return 0, 0, 0 for world position as there is no world.
    return x / clientRight, y / clientBottom, 0, 0, 0;
end

function setCursorPosition(cursorX, cursorY)
    local window = getDrawingWindow();
    local clientLeft, clientTop, clientRight, clientBottom = window.getClientRect();
    
    local x = min(max(cursorX - clientLeft, 0), clientRight - clientLeft);
    local y = min(max(cursorY - clientTop, 0), clientBottom - clientTop);
    
    return win32.setCursorPos(x, y);
end
registerDebugProxy("setCursorPosition", "number", "number");

function isCursorShowing()
    -- Cursor cannot be hidden in MTA:Lua Interpreter.
    return true;
end

function showCursor(show, toggleControls)
    if (toggleControls == nil) then toggleControls = true; end
    
    -- 
end
registerDebugProxy("showCursor", "boolean", { "opt", "boolean" });