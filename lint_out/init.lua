-- Scan all resources and run them
local resRoot = file.createTranslator("resources/");
local xml = xml;

local function makeProperName(fileRoot, path)
    local subName = fileRoot.relPath(path);
    return string.sub(subName, 1, #subName - 1);
end

local utilRoot = file.createTranslator("preload/");

local function importUtilEnvironment(res)
    if not (utilRoot) then return; end;
    
    -- Set up globals for the script.
    local prev_loadResource = _G.loadResource;
    _G.loadResource = res;
    
    local function loadUtilFile(path)
        local file = utilRoot.open(path, "rb");
        
        if not (file) then return false; end;
        
        local success, err = loadstring(file.read(file.size()));
        file.destroy();
        
        if not (success) then
            print("util syntax error (" .. utilRoot.relPath(path) .. "): " .. err);
            return false;
        end
        
        local resEnv = res.getEnv();
        local env = {};
        local meta = {
            __index = function(t, k)
                if (k == "_G") then
                    return env;
                end
            
                local val = _G[k];
                
                if not (val == nil) then
                    return val;
                end
                
                return resEnv[k];
            end,
            __newindex = resEnv
        };
        setmetatable(env, meta);
        setfenv(success, env);
        
        local proto, err = pcall(success);
        
        if not (proto) then
            if (err) then
                print("util runtime error (" .. utilRoot.relPath(path) .. "): " .. err);
            else
                print("unknown util runtime error (" .. utilRoot.relPath(path) .. ")");
            end
            
            return false;
        end
        
        return true;
    end
    
    local function wildLoadDir(root, path)
        return root.scanDirEx(path, "*.lua", nil, loadUtilFile, false);
    end
    
    -- First wildload all preload root scripts.
    wildLoadDir(utilRoot, "/");
    
    -- Now go through directories and managed-load their contents
    local resources = {};
    local numRes = 0;
    local resByName = {};

    utilRoot.scanDirEx("/", "*",
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
                name = makeProperName(utilRoot, dirPath),
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
                    loadUtilFile(manageRoot.absPath(src));
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
    
    -- Restore globals.
    _G.loadResource = prev_loadResource;
end

resRoot.scanDirEx("/", "*",
    function(dirPath)
        local resource = newResource(dirPath, makeProperName(resRoot, dirPath));
        
        if not (resource) then return; end;
        
        local fileRoot = file.createTranslator(dirPath);
        
        if not (fileRoot) then
            resource.destroy();
            return;
        end
        
        local metaFile = fileRoot.open("meta.xml", "r");
        
        if not (metaFile) then
            resource.destroy();
            fileRoot.destroy();
            return;
        end
        
        local xmlNode = xml.parse(metaFile.read(metaFile.size()));
        metaFile.destroy();
        
        if not (xmlNode) then
            resource.destroy();
            fileRoot.destroy();
            return;
        end
        
        local children = xmlNode.getChildren();
        local resEnv = resource.getEnv();
        
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
            end
        end
        
        -- Clean up xml data.
        xmlNode.destroy();
    end, nil, false
);