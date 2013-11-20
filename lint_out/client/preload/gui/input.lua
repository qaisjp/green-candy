-- MTA:Lua Interpreter Resource-side input management.
-- Input functions should trigger window creation, hinting at a minimum-reg model.
local window = false;
local resource = resource;
local keyInfo = {};
local root = root;
local win32 = win32;

-- Optimizations
local registerDebugProxy = registerDebugProxy;

local function checkWindow()
    if (window == false) then
        window = getDrawingWindow();
    end
end

-- Create window if specific events are handled.
local _inputHandler;
local _keyHandler;
local _mouseclickHandler;

sysEventListenCreation({ "onClientKey", "onClientCharacter" },
    function()
        checkWindow();
        
        -- string key, bool state
        function _keyHandler(key, state)
            local info = keyInfo[key];
            
            if (info) then
                -- Run all binds
                for m,n in ipairs(info.binds) do
                    if (b.state == state) or (b.state == "both") then
                        b.routine(key, state);
                    end
                end
            end
            
            -- Execute the global event handlers.
            root.triggerEvent("onClientKey", key, state);
        end
        
        function _inputHandler(char)
            -- Let us notify the environment.
            root.triggerEvent("onClientCharacter", char);
        end
        
        local mouseredirect =
        {
            left = "mouse1",
            middle = "mouse3",
            right = "mouse2"
        };
        
        function _mouseclickHandler(button, state, x, y)
            local redirect = mouseredirect[button];
            
            if (redirect) then
                _keyHandler(redirect, state == "down");
            end
        end
        
        window.addEventHandler("onKey", _keyHandler);
        window.addEventHandler("onInput", _inputHandler);
        window.addEventHandler("onMouseClick", _mouseclickHandler);
        
        -- Listen to mouse clicks for special keys.
    end,
    function()
        window.removeEventHandler("onKey", _keyHandler);
        window.removeEventHandler("onInput", _inputHandler);
        window.removeEventHandler("onMouseClick", _mouseclickHandler);
    end
);

-- Official key table definition from MTA wiki.
keyTable = { "mouse1", "mouse2", "mouse3", "mouse4", "mouse5", "mouse_wheel_up", "mouse_wheel_down", "arrow_l", "arrow_u",
 "arrow_r", "arrow_d", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k",
 "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "num_0", "num_1", "num_2", "num_3", "num_4", "num_5",
 "num_6", "num_7", "num_8", "num_9", "num_mul", "num_add", "num_sep", "num_sub", "num_div", "num_dec", "F1", "F2", "F3", "F4", "F5",
 "F6", "F7", "F8", "F9", "F10", "F11", "F12", "backspace", "tab", "lalt", "ralt", "enter", "space", "pgup", "pgdn", "end", "home",
 "insert", "delete", "lshift", "rshift", "lctrl", "rctrl", "[", "]", "pause", "capslock", "scroll", ";", ",", "-", ".", "/", "#", "\\", "=" };

 -- Initialize the keyInfo structure.
 for m,n in ipairs(keyTable) do
    local info = {
        binds = {},
        commands = {}
    };
    keyInfo[n] = info;
 end

function bindKey(key, keyState, handlerFunction, ...)
    local info = keyInfo[key];
    
    if not (info) then return false; end;
    
    checkWindow();
    
    table.insert(info.binds, {
        routine = handlerFunction,
        args = { ... },
        state = keyState
    });
    return true;
end
registerDebugProxy("bindKey", "string", "string", "function", "vararg");

local function scanTable(t, indexer)
    local n = 1;
    local num = #t;
    
    while (n <= num) do
        if not (indexer(t[n]) == false) then
            n = n + 1;
        else
            table.remove(t, n);
            num = num - 1;
        end
    end
end

function unbindKey(key, keyState, handler)
    local info = keyInfo[key];
    
    if not (info) then return false; end;
    
    checkWindow();
    
    if (keyState == nil) then
        info.binds = {};
    elseif (handler == nil) then
        scanTable(info.binds,
            function(item)
                return not (item.state == keyState);
            end
        );
    else
        scanTable(info.binds,
            function(item)
                return not (item.state == keyState and item.routine == handler);
            end
        );
    end
    
    return true;
end
registerDebugProxy("unbindKey", "string", { "opt", "string" }, { "opt", "function" });

function getKeyState(keyName)
    checkWindow();
    
    -- Our window must have input focus to retrieve key states.
    -- Otherwise all keys are disabled.
    -- In the past we used the dialog key tracking; that has turned out as bad idea.
    -- win32.getKeyState() uses the system keyboard status.
    if not (window.isFocused()) then return false; end;

    return win32.getKeyState(keyName);
end
registerDebugProxy("getKeyState", "string");

function getCommandsBoundToKey(theKey, keyState)
    -- TODO
    return {};
end
registerDebugProxy("getCommandsBoundToKey", "string", "string");

function getFunctionsBoundToKey(theKey)
    -- TODO
    return {};
end
registerDebugProxy("getFunctionsBoundToKey", "string");

function getKeyBoundToCommand(command)
    -- TODO
    return false;
end
registerDebugProxy("getKeyBoundToCommand", "string");

function getKeyBoundToFunction(theFunction)
    -- TODO
    return false;
end
registerDebugProxy("getKeyBoundToFunction", "function");

function getControlState(controlName)
    -- There are no control states in the interpreter.
    -- Maybe we should add emulation to this?
    return false;
end
registerDebugProxy("getControlState", "string");

function isControlEnabled(control)
    -- See above.
    return false;
end
registerDebugProxy("isControlEnabled", "string");

function setControlState(control, state)
    -- See above.
    return false;
end
registerDebugProxy("setControlState", "string", "boolean");

function troggleControl(control, enabled)
    -- See above.
    return false;
end
registerDebugProxy("toggleControl", "string", "boolean");

function toggleAllControls(enabled, gtaControls, mtaControls)
    -- See above.
    return false;
end
registerDebugProxy("toggleAllControls", "boolean", { "opt", "boolean" }, { "opt", "boolean" });

function addCommandHandler(commandName, handlerFunction, caseSensitive)
    -- If commands are really required, I will think of something.
    return false;
end
registerDebugProxy("addCommandHandler", "string", "function", { "opt", "boolean" });

function removeCommandHandler(commandName, handler)
    return false;
end
registerDebugProxy("removeCommandHandler", "string", "function");

function executeCommandHandler(commandName, args)
    return false;
end
registerDebugProxy("executeCommandHandler", "string", "string");

function getBoundKeys(commandOrControl)
    return false;
end
registerDebugProxy("getBoundKeys", "string");

function getAnalogControlState(controlName)
    return false;
end
registerDebugProxy("getAnalogControlState", "string");