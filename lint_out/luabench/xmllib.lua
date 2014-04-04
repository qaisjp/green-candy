-- MTA:Lua XML library
local string = string;
local strfind = string.find;
local strsub = string.sub;
local strbyte = string.byte;
local strchar = string.char;

-- List of all XML nodes.
local xml_nodes = {};

local function getNextSimpleItem(script, offset)
    local m,n;
    local len = #script;
    local char;
    local parseOffset;
    
    parseOffset = strfind(script, "[%a%d_.:]", offset);
    
    if not (parseOffset) then
        return false, nil, len, offset;
    end
    
    m = strfind(script, "[^%a%d_.:]", parseOffset+1);
    
    if (m == nil) then
        m = len+1;
    end
    
    return m, strsub(script, parseOffset, m-1), parseOffset;
end

-- returns new offset and item
local function getNextItem(script, offset)
    local m,n;
    local len = #script;
    local char;
    local parseOffset;
    
    m = strfind(script, "[^%s%c]", offset);

    if (m == nil) then
        return false, nil, len, offset;
    end
    
    char = strbyte(script, m);
        
    if (char == 45) and (strbyte(script, m+1) == 45) then   -- --
        parseOffset = m;
    
        if (strbyte(script, m+2) == 91) and (strbyte(script, m+3) == 91) then  -- --[[
            m, n = strfind(script, "]]", m+4);
            
            if (m == nil) then
                return false, "--[[", len, parseOffset;
            end
            
            return m+2, strsub(script, parseOffset, n), parseOffset;
        end
        
        m = strfind(script, "\n", m+2);
        
        if (m == nil) then
            m = len + 1;
        end
        
        return m+1, strsub(script, parseOffset, m-1), parseOffset;
    elseif (char == 91) and (strbyte(script, m+1) == 91) then  -- [[
        parseOffset = m;
        m = strfind(script, "]]", m+2);
        
        if (m == nil) then
            return false, "[[", len, parseOffset;
        end
        
        return m+2, strsub(script, parseOffset, m+1), parseOffset;
    elseif (char == 34) then    -- "
        parseOffset = m;
        
        repeat
            m = strfind(script, '[\n\\"]', m+1);
            
            if (m == nil) then
                return false, '"', len, parseOffset;
            end
            
            char = strbyte(script, m);
            
            if (char == 92) then
                m = m + 1;
            elseif (char == 10) then
                return false, '"', m-1, parseOffset;
            end
        until (char == 34);
        
        return m+1, strsub(script, parseOffset, m), parseOffset;
    elseif (char == 39) then     -- '
        parseOffset = m;
        
        repeat
            m = strfind(script, "[\n\\']", m+1);
            
            if (m == nil) then
                return false, '"', len, parseOffset;
            end
            
            char = strbyte(script, m);
            
            if (char == 92) then
                m = m + 1;
            elseif (char == 10) then
                return false, '"', m-1, parseOffset;
            end
        until (char == 39);
        
        return m+1, strsub(script, parseOffset, m), parseOffset;
    end
    
    parseOffset = m;
    
    m = strfind(script, "[^%a%d%._:]", m);
    
    if (m == nil) then
        m = len + 1;
    elseif (m == parseOffset) then
        return m+1, strchar(char), m;
    end
    
    return m, strsub(script, parseOffset, m-1), parseOffset;
end

local function xmlScanNode(xml, offset)
    local offset, token, nodeEnd = getNextItem(xml, offset);
    local nodeStart;
    local inNode=false;
    
    while (offset) do
        token = strbyte(token);
    
        if (inNode) then
            if (token == 33) or (token == 63) then
                local begin, offsetEnd = strfind(xml, ">", offset);
                
                if not (begin) then
                    return false;
                end
                
                offset = begin + 1;
                inNode = false;
            elseif (token == 62) then
                return strsub(xml, nodeStart, nodeEnd), offset, nodeStart;
            elseif (token == 60) then
                return false;
            end
        else
            if (token == 60) then
                if (inNode) then
                    return false;
                end
                
                inNode = true;
                nodeStart = nodeEnd;
            elseif (token == 62) then
                return false;
            end
        end
        
        offset, token, nodeEnd = getNextItem(xml, offset);
    end
    
    return false;
end

local function parseStringInternal(input)
    local n=1;
    local len = #input;
    local output = "";
    
    while (n <= len) do
        local char = strsub(input, n, n);
        
        if (char == "\\") then
            n = n + 1;
            
            char = strsub(input, n, n);
            
            if (char == "n") then
                output = output .. "\n";
            elseif (char == "a") then
                output = output .. "\a";
            end
        else
            output = output .. char;
        end
        
        n = n + 1;
    end
    
    return output;
end

local function convertStringInternal(input)
    local n = 1;
    local len = #input;
    local output = "";
    
    while (n <= len) do
        local char = strsub(input, n, n);
        
        if (char == "\n") then
            output = output .. "\\n";
        elseif (char == "\a") then
            output = output .. "\\a";
        else
            output = output .. char;
        end
        
        n = n + 1;
    end
    
    return output;
end

-- Forward declaration.
local xmlCreateNodeEx;

local function isValidNodeName(name)
    return not (strfind(name, "[^%a%d_-]"));
end

function xmlCreateNodeEx(name)
    if not (name) or (#name == 0) then return false; end;
    
    if not (isValidNodeName(name)) then return false; end;
    
    return class.construct(
        function(c)
            local children = {};
        
            -- Exports attributes directly.
            _ENV.name = name;
            _ENV.children = children;
            _ENV.attr = {};
            _ENV.data = {}; -- used by Lua runtime.
            
            __newindex = _ENV;
            
            function createChild(name)
                local node = xmlCreateNodeEx(name);
                
                if not (node) then return false; end;
                
                node.setParent(c);
                return node;
            end
            
            function isValidChild(child)
                -- It has to be a xmlnode.
                -- Registry check is the safest we can do.
                return not (xml_nodes[child] == nil);
            end
            
            function setChild(child)
                local childAPI = super(child);
                
                table.insert(children, child);
                
                childAPI.extend(
                    function()
                        function notifyDestroy()
                            for m,n in ipairs(children) do
                                if (n == child) then
                                    table.remove(children, m);
                                    break;
                                end
                            end
                        end
                    end
                );
                
                return childAPI;
            end
            
            function setParent(p)
                local ret = super(p);
                
                if (ret) then
                    parent = p;
                end
                
                return ret;
            end
            
            function writeTree(tabcount)
                local buff = string.rep(" ", tabcount * 4) .. "<" .. name;
                local m,n;
                
                if not (tabcount) then
                    tabcount = 0;
                end
                
                for m,n in pairs(attr) do
                    buff = buff .. " " .. m .. "=\"" .. convertStringInternal(n) .. "\"";
                end
                
                if (#children == 0) then
                    buff = buff .. " />\n";
                    return buff;
                end
                
                buff = buff .. ">\n";
                
                for j,k in ipairs(children) do
                    buff = buff .. k.writeTree(tabcount + 1);
                end
                
                buff = buff .. string.rep(" ", tabcount * 4) ..  "</" .. name .. ">\n";
                return buff;
            end
            
            function findSubNode(name)
                local m,n;
                
                for m,n in ipairs(children) do
                    if (n.name == name) then
                        return n;
                    end
                end
                
                return false;
            end

            function findCreateNode(name)
                local found = findSubNode(name);
                
                if not (found) then
                    found = xmlCreateNodeEx(name);
                    found.setParent(c);
                end
                
                return found;
            end
            
            function clone()
                local root = xmlCreateNodeEx(name);
                
                -- Copy all attributes
                for m,n in pairs(attr) do
                    root.attr[m] = n;
                end
                
                -- Copy all children
                local children = getChildren();
                
                for m,n in ipairs(children) do
                    n.clone().setParent(root);
                end
                
                -- Return the result.
                return root;
            end
            
            function setName(newName)
                if not (isValidNodeName(newName)) then return false; end;
                
                _ENV.name = newName;
                return true;
            end
            
            function getName()
                return _ENV.name;
            end
            
            function destroy()
                xml_nodes[c] = nil;
            end
            
            __type = "xmlnode";
            
            xml_nodes[c] = true;
        end
    );
end

local function xmlParseNode(xml, xmlOffset)
    local token, offset = xmlScanNode(xml, xmlOffset);
    local setOffset, setToken;
    local key, value;
    
    if not (token) then return false; end;
    
    -- Parse the node, it is the node we want
    local setting = strsub(token, 2, #token - 1);
    
    setOffset, setToken = getNextItem(setting, 1);
    
    local node = xmlCreateNodeEx(setToken);
    
    if not (node) then return false; end;
    
    setOffset, key = getNextItem(setting, setOffset);
    
    while (setOffset) do
        if (key == "/") then
            -- The node is parsed correctly
            return node, offset;
        elseif not (isName(strbyte(key))) then
            return false;   -- Some wrong char was input
        end
        
        setOffset, setToken = getNextItem(setting, setOffset);
        
        -- There can be only '='
        if not (setToken == "=") then
            print("XML library error: invalid token " .. setToken);
            return false;
        end
        
        setOffset, value = getNextItem(setting, setOffset);
        
        -- Has to be string, yea
        local byteString = strbyte(value);
        
        if not (byteString == 34) and not (byteString == 39) then return false; end;
        
        -- But this string has to be parsed, then we can set the attribute
        node.attr[key] = parseStringInternal(strsub(value, 2, #value -1));     
    
        setOffset, key = getNextItem(setting, setOffset);
    end
    
    token, offset, key = xmlScanNode(xml, offset);
    
    while (token) do
        -- See whether this token closes us
        setting = strsub(token, 2, offset - 2);
        
        setOffset, setToken = getNextItem(setting, 1);
        
        if (setToken == "/") then
            setOffset, setToken = getNextItem(setting, setOffset);
            
            if not (setToken == node.name) then return false; end;
            
            -- Finished children correctly
            return node, offset;
        elseif not (setToken == "!") then
            local child;
            child, offset = xmlParseNode(xml, key);
            
            if not (child) then return false; end;
            
            child.setParent(node);
        end
        
        token, offset, key = xmlScanNode(xml, offset);
    end
    
    -- We need a closing node
    return false;
end

-- Create the xml library
local xml = {};
_G.xml = xml;

function xml.parse(xml)
    return xmlParseNode(xml, 1);
end

function xml.isNode(val)
    return not (xml_nodes[val] == nil);
end

xml.createNode = xmlCreateNodeEx;