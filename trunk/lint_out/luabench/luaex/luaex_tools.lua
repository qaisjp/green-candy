local tryScriptPath = "luaex_test.lua";

function runtest(...)
    local loadstringex = loadstringex;
    
    if not (loadstringex) then
        error( "failed to bind LuaEX compiler" );
        return false;
    end

    local dynroot = dynroot;
    
    if not (dynroot) then
        error( "failed to get dynamic root handle" );
        return false;
    end

    local fileHandle = dynroot.open( tryScriptPath, "rb" );
    
    if not (fileHandle) then
        error( "failed to open test script file " .. tryScriptPath );
        return false;
    end
    
    local fileContent = fileHandle.read( fileHandle.size() );
    
    fileHandle.destroy();
    
    -- Run the script using luaex.
    local proto, errorMsg = loadstringex( fileContent );
    
    if not (proto) then
        local errorOut = "error";
        
        if (errorMsg) then
            errorOut = errorOut .. " (" .. errorMsg .. ")";
        else
            errorOut = errorOut .. " (unknown)";
        end
        
        return false, errorOut;
    end
    
    return true, proto(...);
end