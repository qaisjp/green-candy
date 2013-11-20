-- Scan all resources and run them
local file = file;
local exeRoot = file.root;
local xml = xml;

local function loadEnvironment(envRoot)
    local resRoot = file.createTranslator(envRoot.absPath("resources/"));

    local function makeProperName(fileRoot, path)
        local subName = fileRoot.relPath(path);
        return string.sub(subName, 1, #subName - 1);
    end

    local utilRoot = file.createTranslator(envRoot.absPath("preload/"));
    local utilEnv = false;  -- environment of all deathmatch utilities
    local utilDebugArgs = false;    -- debug argStream class

    local function loadDependencyRoot(loadRoot, loadEnv)
        local function loadUtilFile(path, loadRoot)
            local file = loadRoot.open(path, "rb");
            
            if not (file) then return false; end;
            
            local success, err = loadstring(file.read(file.size()), "@" .. exeRoot.relPathRoot(path));
            file.destroy();
            
            if not (success) then
                print("util syntax error (" .. loadRoot.relPath(path) .. "): " .. err);
                return false;
            end
            
            -- Export settings which apply per-root for script files.
            local translatorEnv = {
                scriptRoot = loadRoot
            };

            -- Anything that is written as global in the preload files
            -- should be exported to the resources (env variable)
            local env = {};
            local meta = {
                __index = function(t, k)
                    if (k == "_G") then
                        return env;
                    end
                
                    local val = loadEnv[k];
                    
                    if not (val == nil) then
                        return val;
                    end
                    
                    val = translatorEnv[k];
                    
                    if not (val == nil) then
                        return val;
                    end
                    
                    return _G[k];
                end,
                __newindex = loadEnv
            };
            setmetatable(env, meta);
            
            -- Set the script environment to the util file.
            setfenv(success, env);
            
            local proto, err = pcall(success);
            
            if not (proto) then
                if (err) then
                    print("util runtime error (" .. loadRoot.relPath(path) .. "): " .. err);
                else
                    print("unknown util runtime error (" .. loadRoot.relPath(path) .. ")");
                end
                
                return false;
            end
            
            return true;
        end
        
        local function wildLoadDir(root, path)
            return root.scanDirEx(path, "*.lua", nil,
                function(path)
                    loadUtilFile(path, root);
                end,
            false);
        end
        
        -- First wildload all preload root scripts.
        wildLoadDir(loadRoot, "/");
        
        -- Now go through directories and managed-load their contents
        local resources = {};
        local numRes = 0;
        local resByName = {};

        loadRoot.scanDirEx("/", "*",
            function(dirPath)
                local manageRoot = file.createTranslator(dirPath);
            
                if not (manageRoot) then return; end;
                
                local metaFile = manageRoot.open("meta.xml", "r");
                
                if not (metaFile) then
                    wildLoadDir(dirPath);
                    manageRoot.destroy();
                    return;
                end
                
                local metaNode = xml.parse(metaFile.read(metaFile.size()));
                metaFile.destroy();
                
                if not (metaNode) then
                    manageRoot.destroy();
                    return;
                end
                
                -- Create a resource entry
                local dependencies = {};
                local resource = {
                    name = makeProperName(loadRoot, dirPath),
                    fileRoot = manageRoot,
                    dependencies = dependencies,
                    metaNode = metaNode,
                    processed = false
                };
                
                local children = metaNode.getChildren();
                
                for m,n in ipairs(children) do
                    local name = n.name;
                    
                    if (name == "depend") then
                        local resName = n.attr.resource;
                        
                        if not (resName == nil) then
                            table.insert(dependencies, resName);
                        end
                    end
                end
                
                numRes = numRes + 1;
                resources[numRes] = resource;
                resByName[resource.name] = resource;
            end, nil, false
        );
        
        -- Forward declaration.
        local loadResource;
        
        function loadResource(res)
            if (res.processed) then return; end;
            
            res.processed = true;
           
            -- First load all dependencies
            do
                local dependencies = res.dependencies;
                
                for m,n in ipairs(dependencies) do
                    local _res = resByName[n];
                    
                    if (_res) then
                        loadResource(_res);
                    else
                        print("warning: missing dependency '" .. n .. "' for '" .. res.name .."'");
                    end
                end
            end
            
            local metaNode = res.metaNode;
            local manageRoot = res.fileRoot;
            local children = metaNode.getChildren();
            
            for m,n in ipairs(children) do
                local name = n.name;
                
                if (name == "wildload") then
                    local path = n.attr.dir;
                    
                    if not (path == nil) then
                        wildLoadDir(manageRoot, path);
                    else
                        wildLoadDir(manageRoot, "/");
                    end
                elseif (name == "script") then
                    local src = n.attr.src;
                    
                    if not (src == nil) then
                        loadUtilFile(manageRoot.absPath(src), manageRoot);
                    end
                end
            end
            
            -- Clean up resources.
            metaNode.destroy();
        end
        
        -- Load all resources
        for m,n in ipairs(resources) do
            loadResource(n);
        end
    end

    local function argumentErrorCallback(debuglib, methName, errMessage, debugInfo)
        print("(" .. methName ..", " .. debugInfo.short_src .. ":" .. debugInfo.currentline .. "): " .. errMessage);
        return false;
    end

    local function loadDependencyRoot_debug(fileRoot, env, debuglib)
        -- Set up the functions to register debug callbacks.
        function env.registerDebugProxy(name, ...)
            return debuglib.addDebugProxy(name, ...);
        end
        
        function env.registerArgTypeHandler(argName, handler)
            return debuglib.setArgumentTypeCallback(argName, handler);
        end
        
        debuglib.setErrorCallback(argumentErrorCallback);
        
        return loadDependencyRoot(fileRoot, env);
    end

    if (utilRoot) then
        -- Load all files from the utility root if we succeeded to bind it.
        print("Importing /preload/ environment...");
        
        utilEnv = {};
        utilDebugArgs = createDebugArgParser();
        
        loadDependencyRoot_debug(utilRoot, utilEnv, utilDebugArgs);
    end

    local function importUtilEnvironment(res)
        if not (utilEnv) then return false; end;

        local resEnv = res.getEnv();
        local type = type;
        
        -- Import the utility environment which we have previously loaded.
        for m,n in pairs(utilEnv) do
            local valType = rawtype(n);
        
            if (valType == "function") then
                -- Attempt to register the debug proxy instead of the raw function.
                local proxy = utilDebugArgs.getProxy(m, n);
                
                if (proxy) then
                    resEnv[m] = proxy;
                else
                    resEnv[m] = function(...)
                        return n(...);
                    end
                end
            else
                resEnv[m] = n;
            end
        end
        
        -- Also try to load the per-resource files.
        local perResRoot = file.createTranslator(envRoot.absPath("res_load/"));
        
        if (perResRoot) then
            -- Create a debug arg parsing library to enable function debugging
            -- like with argStreams in MTA.
            -- Debug libraries have to stay alive as long as the runtime is active.
            -- They will be garbage collected when not required anymore.
            local debuglib = createDebugArgParser();
        
            loadDependencyRoot_debug(perResRoot, resEnv, debuglib);
            
            -- Parse the environment for debug information.
            debuglib.parseEnvironment(resEnv);
            
            -- Clean up.
            perResRoot.destroy();
        end
        
        return true;
    end

    do
        local resources = {};
        local resByName = {};
        
        local function addResource(name, info)
            table.insert(resources, info);
            resByName[string.lower(name)] = info;
        end
        
        local function getResource(name)
            return resByName[string.lower(name)];
        end

        local function scanResourceRepository(repRoot)
            repRoot.scanDirEx("/", "*",
                function(dirPath)
                    local name = makeProperName(repRoot, dirPath);
                    local firstByte = string.byte(name, 1);
                    
                    if (firstByte == 91) then
                        local newRoot = file.createTranslator(dirPath);
                        
                        scanResourceRepository(newRoot);
                        
                        newRoot.destroy();
                    else
                        local fileRoot = file.createTranslator(dirPath);
                        
                        if not (fileRoot) then
                            print(name .. ": failed to bind file root");
                            return;
                        end
                        
                        -- Load the meta.xml
                        local metaFile = fileRoot.open("meta.xml", "r");
                    
                        if not (metaFile) then
                            print(name .. ": could not open meta.xml");
                            
                            fileRoot.destroy();
                            return;
                        end
                        
                        local metaNode = xml.parse(metaFile.read(metaFile.size()));
                        metaFile.destroy();
                        
                        if not (metaNode) then
                            print(name .. ": failed to parse meta.xml");
                        
                            fileRoot.destroy();
                            return;
                        end
                        
                        -- Pre-parse the meta.xml for dependencies, etc
                        local dependencies = {};
                        local children = metaNode.getChildren();
                        
                        for m,n in ipairs(children) do
                            local nodeName = n.name;
                            
                            if (nodeName == "include") then
                                local includeRes = n.attr.resource;
                                
                                if not (includeRes == nil) then
                                    table.insert(dependencies, includeRes);
                                end
                            end
                        end
                    
                        local info = {
                            name = name,
                            metaNode = metaNode,
                            fileRoot = fileRoot,
                            resource = false,
                            isProcessed = false,
                            isLoaded = false,
                            dependencies = dependencies
                        };
                        
                        addResource(name, info);
                    end
                end, nil, false
            );
        end
        scanResourceRepository(resRoot);
        
        local function loadResource(resInfo)
            if (resInfo.isProcessed) or (resInfo.isLoaded) then
                return true;
            end
            
            -- Mark us as processed so we do not cause infinite loops while initializing.
            resInfo.isProcessed = true;
            
            local name = resInfo.name;
        
            -- Start all dependencies first.
            for m,n in ipairs(resInfo.dependencies) do
                local resource = getResource(n);
                
                if (resource) then
                    loadResource(resource);
                else
                    print(name .. ": could not find resource dependency '" .. n .. "'");
                end
            end
        
            local fileRoot = resInfo.fileRoot;
            local resource = sysMakeResource(fileRoot.absPathRoot(), name);
            
            if not (resource) then return false; end;
            
            local xmlNode = resInfo.metaNode;
            resInfo.metaNode = false;
            
            local children = xmlNode.getChildren();
            local resEnv = resource.getEnv();
            
            -- Table of all allowed exports.
            local allowedExports = {};
            
            -- Set up the resource defaults.
            resource.extend(
                function()
                    registerForcedSuper("init");
                    
                    _ENV.fileRoot = fileRoot;
                    
                    function init()
                        return;
                    end
                    
                    function callExport(callResource, funcName, ...)
                        if not (allowedExports[funcName]) then return false; end;
                        
                        local funcExport = resEnv[funcName];
                        
                        if (funcExport) then
                            local _G = resource.getEnv();
                        
                            -- Set some globals.
                            local sourceResource = _G.sourceResource;
                            local sourceResourceRoot = _G.sourceResourceRoot;
                            
                            _G.sourceResource = callResource;
                            _G.sourceResourceRoot = callResource.resRoot;
                        
                            local result = { funcExport(...) };
                            
                            -- Restore globals.
                            _G.sourceResourceRoot = sourceResourceRoot;
                            _G.sourceResource = sourceResource;
                            
                            return unpack(result);
                        end
                        
                        return false;
                    end
                    
                    function getExports()
                        local exports = {};
                        
                        for m,n in pairs(allowedExports) do
                            table.insert(exports, m);
                        end
                        
                        return exports;
                    end
                end
            );
            
            -- Set up the special resource environment.
            importUtilEnvironment(resource);

            for m,n in ipairs(children) do
                local name = n.name;
                
                if (name == "script") then
                    local src = n.attr.src;
                    local srcType = n.attr.type;
                    
                    if not (src == nil) and (srcType == "client") then
                        local success, err = resource.loadScript(src);
                    
                        if not (success) then
                            if (err) then
                                print("failed to run script '" .. src .."': " .. err);
                            else
                                print("failed to run script '" .. src .."'");
                            end
                        end
                    end
                elseif (name == "export") then
                    local funcName = n.attr["function"];
                    
                    if ((n.attr.type == nil) or (n.attr.type == "client"))
                        and not (funcName == nil) then
                        
                        allowedExports[funcName] = true;
                    end
                end
            end
            
            -- 'start' the resource
            resource.init();
            
            -- Clean up xml data.
            xmlNode.destroy();
            
            -- We successfully loaded.
            resInfo.isLoaded = true;
            return true;
        end
        
        -- Start all resources
        for m,n in ipairs(resources) do
            loadResource(n);
        end
    end

    -- Clean up after us.
    
    return true;
end

-- Attempt to load all environments.
do
    local clientRoot = file.createTranslator("/client/");

    if (clientRoot) then
        print("loading clientside environment...");

        if not (loadEnvironment(clientRoot)) then
            error("failed to load clientside environment");
        end
    end
end

--[[
-- Does not work yet as...
1) MTA:Lua Interpreter needs to support multiple Resource Managers
2) MTA:Lua should be updated to support multi-threading

do
    local serverRoot = file.createTranslator("/server/");
    
    if (serverRoot) then
        print("loading serverside environment...");
        
        if not (loadEnvironment(serverRoot)) then
            error("failed to load serverside environment");
        end
    end
end
]]

