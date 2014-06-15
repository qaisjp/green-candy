local file = file;
local root = file.root;

-- Load the LuaEX core.
local bootLoaderHandle = root.open( "luabench/luaex/luaex.lua", "rb" );

if (bootLoaderHandle) then
    -- We found the file, compile and run it.
    local bootContent = bootLoaderHandle.read( bootLoaderHandle.size() );
    
    bootLoaderHandle.destroy();
    
    -- Compile using standard Lua compiler.
    local bootProto, errorMsg = loadstring( bootContent, "luaex.lua" );
    
    if not ( bootProto ) then
        print( "failed compilation of LuaEX core:" );
        
        if (errorMsg) then
            print( tostring( errorMsg ) );
        end
    else
        -- Set up the proto root.
        local protoRoot = file.createTranslator( "luabench/luaex/" );
        
        _G.protoRoot = protoRoot;
    
        -- Run the boot prototype natively.
        bootProto();
        
        print( "successfully booted LuaEX core" );
    end
else
    print( "could not detect LuaEX bootloader (ignoring)" );
end