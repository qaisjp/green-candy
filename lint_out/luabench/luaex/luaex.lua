local string = string;
local math = math;
local table = table;
local strsub = string.sub;
local strbyte = string.byte;
local strchar = string.char;
local strlen = string.len;
local strfind = string.find;
local fread = fileRead;
local ipairs = ipairs;
local pairs = pairs;
local collectgarbage = collectgarbage;

function isflag(a, b)
    return (a % (2 ^ (b+1)) - a % (2 ^ b)) == 2 ^ b;
end

function table.delete(tab, obj)
    local m,n;
    
    for m,n in ipairs(tab) do
        if (n == obj) then
            table.remove(tab, m);
            return m;
        end
    end
    
    error("failed to delete table item", 2);
    return false;
end

function traceback(msg)
    local level = debug.getinfo(1);
    local n = 1;
    
    while (level) do
        outputDebugString(level.short_src .. ": " .. level.currentline .. ", " .. level.linedefined);
        
        n = n + 1;
        level = debug.getinfo(n);
    end
    
    if (msg) then
        error(msg, 2);
    else
        error("traceback!", 2);
    end
end

function fileGetHash(path)
    if not (fileExists(path)) then return ""; end;
    
    local file = fileOpen(path);
    
    if not (file) then return ""; end;
    
    local content = fileRead(file, fileGetSize(file));
    fileClose(file);
    
    return md5(content);
end

bigendian = {
    int_to_bytes = function(n)
        n = (n < 0) and (4294967296 + n) or n;
        return (math.modf(n/0x1000000))%0xFF, (math.modf(n/0x10000))%0xFF, (math.modf(n/0x100))%0xFF, n%0xFF;
    end,
    
    fileReadInt = function(file)
        local a, b, c, d = strbyte(fread(file, 4), 1, 4);
        
        if not (d) then return false; end;
        
        return a * 0x1000000 + b * 0x10000 + c * 0x100 + d;
    end,
    
    fileReadShort = function(file)
        local high, low = strbyte(fread(file, 2), 1, 2);
        
        if not (low) then return false; end;
        
        return high * 256 + low;
    end,
    
    fileReadByte = function(file)
        local byte = fread(file, 1);
        
        if not (byte) then return false; end;
        
        return strbyte(byte);
    end,
    
    fileWriteInt = function(file, n)
        return fileWrite(file, strchar((math.modf(n/0x1000000))%0xFF, (math.modf(n/0x10000))%0xFF, (math.modf(n/0x100))%0xFF, n%0xFF));
    end,
    
    fileWriteShort = function(file, n)
        return fileWrite(file, strchar((math.modf(n/0x100))%0xFF, n%0xFF));
    end,
    
    fileWriteByte = function(file, n)
        return fileWrite(file, strchar(n%0xFF));
    end
}

littleendian = {
    int_to_bytes = function(n)
        n = (n < 0) and (4294967296 + n) or n;
        return n%0x100, (math.modf(n/0x100))%0x100, (math.modf(n/0x10000))%0x100, (math.modf(n/0x1000000))%0x100;
    end,
    
    fileReadInt = function(file)
        local a, b, c, d = strbyte(fread(file, 4), 1, 4);
        
        if not (d) then return false; end;
        
        return d * 0x1000000 + c * 0x10000 + b * 0x100 + a;
    end,
    
    fileReadShort = function(file)
        local high, low = strbyte(fread(file, 2), 1, 2);
        
        if not (low) then return false; end;
        
        return low * 256 + high;
    end,
    
    fileReadByte = function(file)
        local byte = fread(file, 1);
        
        if not (byte) then return false; end;
        
        return strbyte(byte);
    end,
    
    fileWriteInt = function(file, n)
        return fileWrite(file, strchar(n%0x100, (math.modf(n/0x100))%0x100, (math.modf(n/0x10000))%0x100, (math.modf(n/0x1000000))%0x100));
    end,
    
    fileWriteShort = function(file, n)
        return fileWrite(file, strchar(n%0x100, (math.modf(n/0x100))%0x100));
    end,
    
    fileWriteByte = function(file, n)
        return fileWrite(file, strchar(n%0x100));
    end
}

function fileSeek(file, seek)
    return fileSetPos(file, fileGetPos(file) + seek);
end

function getFileExtension(src)
    local lastOffset;
    local begin, last = strfind(src, "[.]");
    
    if not (begin) then return false; end;

    while (begin) do
        lastOffset = begin+1;
        
        begin, last = strfind(src, "[.]", lastOffset);
    end
    
    return strsub(src, lastOffset, strlen(src));
end

function getFilename(src)
    local lastOffset;
    local begin, last = strfind(src, "[.]");
    
    if not (begin) then return false; end;

    while (begin) do
        lastOffset = begin+1;
        
        begin, last = strfind(src, "[.]", lastOffset);
    end
    
    return strsub(src, 1, begin - 1);
end

function fileParsePath(path)
    local arches = {};
    local n = 0;
    local lastBreak = 1;
    local name;
    local len = strlen(path);
    local j,k;
    local output = "";

    while (true) do
        n = strfind(path, "[\\/]", lastBreak);

        if not (n) then break; end;

        local archName = strsub(path, lastBreak, n - 1);

        if (strlen(archName) == 0) then return false; end;

        if (strfind(archName, "[<>:\"/|?*]")) then return false; end;

        if (archName == "..") then
            if (#arches == 0) then return false; end;

            table.remove(arches);
        elseif not (archName == ".") then
            table.insert(arches, archName);
        end

        lastBreak = n + 1;
    end

    for j,k in ipairs(arches) do
        output = output .. k .. "/";
    end

    if (lastBreak <= len) then
        local filename = strsub(path, lastBreak, len);

        if (strfind(filename, "[<>:\"/|?*]")) then return false; end;

        return output .. filename, true;
    end

    return output, false;
end

-- New string.match routine
function globmatch(str, matchWith)
    local m,n;
    local lastBreak;
    local len = strlen(str);

    lastBreak = strfind(matchWith, "*", 1);

    if not (lastBreak) then
        return str == matchWith;
    end

    lastBreak = lastBreak + 1;

    if not (strsub(str, 1, lastBreak - 2) == strsub(matchWith, 1, lastBreak - 2)) then return false; end;

    m = lastBreak - 1;

    while (true) do
        n = strfind(matchWith, "*", lastBreak);

        if not (n) then break; end;

        local find = strsub(matchWith, lastBreak, n - 1);

        m = strfind(str, find, m, true);

        if not (m) then return false; end;

        m = m + strlen(find);

        lastBreak = n + 1;
    end

    local find = strsub(matchWith, lastBreak, strlen(matchWith));

    if (strlen(find) == 0) then return true; end;

    m = strfind(str, find, m, true);

    if (m) then
        m = m + 1;

        while (true) do
            n = strfind(str, find, m, true);

            if not (n) then break; end;

            m = n + 1;
        end

        m = m - 1 + strlen(find);
    else
        return false;
    end

    return m == len + 1;
end

function isName(char)
    if ((char > 96) and (char < 123)) or ((char > 64) and (char < 91))
        or (char == 95)     -- _
        or (char == 45)     -- -
        or (char == 46)     -- .
        or (char == 58) then    -- :
        
        return true;
    end
    
    return false;
end

function getNextSimpleItem(script, offset)
    local m,n;
    local len=strlen(script);
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
function getNextItem(script, offset)
    local m,n;
    local len=strlen(script);
    local char;
    local parseOffset;
    
    m = string.find(script, "[^%s%c]", offset);

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

function parseStringInternal(input)
    local n=1;
    local len=strlen(input);
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

function convertStringInternal(input)
    local n=1;
    local len=strlen(input);
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

function xmlParse(xml)
    return xmlParseNode(xml, 1);
end

function xmlParseNode(xml, xmlOffset)
    local token, offset = xmlScanNode(xml, xmlOffset);
    local setOffset, setToken;
    local key, value;
    
    if not (token) then return false; end;
    
    -- Parse the node, it is the node we want
    local setting = strsub(token, 2, strlen(token) - 1);
    
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
            outputDebugString("error "..setToken .. " " .. tostring(strbyte(setToken)));
            return false;
        end
        
        setOffset, value = getNextItem(setting, setOffset);
        
        -- Has to be string, yea
        if not (strbyte(value) == 34) then return false; end;
        
        -- But this string has to be parsed, then we can set the attribute
        node.attr[key] = parseStringInternal(strsub(value, 2, strlen(value)-1));     
    
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
            
            table.insert(node.children, child);
        end
        
        token, offset, key = xmlScanNode(xml, offset);
    end
    
    -- We need a closing node
    return false;
end

function xmlCreateNodeEx(name)
    local node = {};
    
    if not (name) or (strlen(name) == 0) then return false; end;
    
    if (strfind(name, "[^%a%d_-]")) then return false; end;
    
    node.name = name;
    node.children = {};
    node.attr = {};
    
    function node.writeTree(tabcount)
        local buff = string.rep(" ", tabcount * 4) .. "<" .. node.name;
        local m,n;
        
        if not (tabcount) then
            tabcount = 0;
        end
        
        for m,n in pairs(node.attr) do
            buff = buff .. " " .. m .. "=\"" .. convertStringInternal(n) .. "\"";
        end
        
        if (#node.children == 0) then
            buff = buff .. " />\n";
            return buff;
        end
        
        buff = buff .. ">\n";
        
        for j,k in ipairs(node.children) do
            buff = buff .. k.writeTree(tabcount + 1);
        end
        
        buff = buff .. string.rep(" ", tabcount * 4) ..  "</" .. node.name .. ">\n";
        return buff;
    end
    
    return node;
end

function xmlCreateChildEx(parent, name)
    local node = xmlCreateNodeEx(name);
    node.parent = parent;
    
    table.insert(parent.children, node);
    return node;
end

function xmlDestroyNodeEx(node)
    if (node.parent) then
        table.delete(node.parent.children, node);
    end
    
    return true;
end

-- Pull xml node into table
function xmlGetNode(pXml, parent)
    local pBuff;
    local m,n;
    local attribs;
    
    if not (pXml) then return false; end;
    
    if (parent) then
        pBuff = xmlCreateChildEx(parent, xmlNodeGetName(pXml));
    else
        pBuff = xmlCreateNodeEx(xmlNodeGetName(pXml));
    end
    
    attribs=xmlNodeGetAttributes(pXml);
    
    for m,n in pairs(attribs) do
        pBuff.attr[m]=n;
    end
    
    local children=xmlNodeGetChildren(pXml);
    
    for m,n in ipairs(children) do
        xmlGetNode(n, pBuff);
    end
    
    return pBuff;
end

-- Save the node from table
function xmlSetNode(pXml, content)
    local m,n;
    local children = xmlNodeGetChildren(pXml);
    local attributes = xmlNodeGetAttributes(pXml);
    
    -- Destroy previous children
    for m,n in ipairs(children) do
        xmlDestroyNode(n);
    end
    
    -- Unset previous attributes
    for m,n in pairs(attributes) do
        xmlNodeSetAttribute(pXml, m, nil);
    end
    
    xmlNodeSetName(pXml, content.name);
    
    for m,n in pairs(content.attr) do
        xmlNodeSetAttribute(pXml, m, n);
    end
    
    for m,n in ipairs(content.children) do
        xmlSetNode(xmlCreateChild(pXml, n.name), n);
    end
    
    return true;
end

function xmlFindSubNodeEx(node, name)
    local m,n;
    
    for m,n in ipairs(node.children) do
        if (n.name == name) then
            return n;
        end
    end
    
    return false;
end

function findCreateNode(node, name)
    local found = xmlFindSubNodeEx(node, name);
    
    if not (found) then
        found = xmlCreateNodeEx(name);
        table.insert(node.children, found);
    end

    return found;
end

--[[
    Double conversion code taken from ChunkSpy 0.9.8!
]]

local string = string;
local math = math;
local table = table;
local strsub = string.sub;
local strbyte = string.byte;
local strchar = string.char;
local strlen = string.len;
local strfind = string.find;
local ipairs = ipairs;
local pairs = pairs;
local collectgarbage = collectgarbage;
local isName = isName;
local type = type;
local rawtype = rawtype;

if not (rawtype) then
    rawtype = type;
end

-- For now, we do not support standard Lua.
-- It should be on our todo list to support it.
local fileRoot = _G.protoRoot;

local function getFileContent(filePath)
    local fileHandle = fileRoot.open( filePath, "rb" );
    
    if not (fileHandle) then return false; end;
    
    local fileContent = fileHandle.read( fileHandle.size() );
    
    fileHandle.destroy();
    
    return fileContent;
end

local function createCompiler()
    -- Create the compiler object that we will return.
    local compiler = {};
    
    -- Load the system environment into the compiler.
    local systemEnv = {};
    
    do

    end

    -- Compiler definitions
    local falseDef = strchar(0);
    local trueDef = strchar(1);
    local luaSignature = strchar(0x1B) .. "Lua";
    local luaVersion = strchar(0x51);
    local typeNil = strchar(0);
    local typeBool = strchar(1);
    local typeLightUserdata = strchar(2);
    local typeNumber = strchar(3);
    local typeString = strchar(4);
    local typeTable = strchar(5);
    local typeFunction = strchar(6);
    local typeUserdata = strchar(7);
    local typeThread = strchar(8);

    -- From ChunkSpy
    local function grab_byte(v)
        return math.floor(v / 256), strchar(math.floor(v) % 256)
    end

    local function doubleToByte(x)
        -- Well, we gotta encode infinity and stuff, too!
        if (x == 1/0) then
            return "\0\0\0\0\0\0\240\127";
        elseif (c == -1/0) then
            return "\0\0\0\0\0\0\240\255";
        end
        
        local sign = 0
        
        if x < 0 then sign = 1; x = -x end
        
        local mantissa, exponent = math.frexp(x)
        
        if x == 0 then -- zero
            mantissa, exponent = 0, 0
        else
            mantissa = (mantissa * 2 - 1) * math.ldexp(0.5, 53)
            exponent = exponent + 1022
        end
        
        local v, byte = "" -- convert to bytes
        x = mantissa;
        
        for i = 1,6 do
            x, byte = grab_byte(x); v = v..byte -- 47:0
        end
        
        x, byte = grab_byte(exponent * 16 + x); v = v..byte -- 55:48
        x, byte = grab_byte(sign * 128 + x); v = v..byte -- 63:56
        return v
    end
    -- ChunkSpy end

    -- Thanks to gwell from stackoverflow.com for int_to_bytes
    -- I changed this to return a string
    local function intToByte(n)
        return strchar(n%256) .. strchar((math.modf(n/256))%256) .. strchar((math.modf(n/65536))%256) .. strchar((math.modf(n/16777216))%256);
    end

    local function dumpInt(int)
        return intToByte(int);
    end

    local function dumpNumber(double)
        return doubleToByte(double);
    end

    local function dumpString(str)
        return intToByte(strlen(str) + 1) .. str .. strchar(0);
    end

    local function dumpBool(bool)
        if (bool) then
            return trueDef;
        end
        
        return falseDef;
    end

    local function dumpFunction(proto)
        local code = "";
        local m,n;

        -- General information
        code = code
            .. dumpString(proto.name)
            .. dumpInt(proto.lineDefined)
            .. dumpInt(proto.lastLineDefined)
            .. strchar(#proto.upValues)
            .. strchar(proto.numParams)
            .. dumpBool(proto.isVararg)
            .. strchar(proto.stackSize);
        
        -- Now the code
        code = code .. dumpInt(#proto.instructions);

        for m,n in ipairs(proto.instructions) do
            code = code .. dumpInt(n);
        end
        
        -- Constants
        code = code .. dumpInt(#proto.constants);
        
        for m,n in ipairs(proto.constants) do
            local t = type(n);
            
            if (t == "string") then
                code = code .. typeString .. dumpString(n);
            elseif (t == "number") then
                code = code .. typeNumber .. dumpNumber(n);
            elseif (t == "boolean") then
                code = code .. typeBool .. dumpBool(n);
            else
                error("Unknown constant " .. t);
            end
        end
        
        -- Functions
        code = code .. dumpInt(#proto.functions);
        
        for m,n in ipairs(proto.functions) do
            code = code .. dumpFunction(n);
        end
        
        -- Zero out debug info
        return code .. dumpInt(0) .. dumpInt(0) .. dumpInt(0);
    end

    function scanLines(script)
        local lineData = {};
        local info = {
            offset = 1
        };
        
        table.insert(lineData, info);
        
        while (true) do
            local offset, begin = strfind(script, "\n", info.offset);
            
            if not (offset) then
                info.offsetEnd = strlen(script);
            
                return lineData;
            end
            
            info.offsetEnd = offset;
            
            info = {
                offset = offset + 1
            };
            table.insert(lineData, info);
        end
    end

    function createProto()
        local proto = {
            name = "@LuaEX",
            lineDefined = 0,
            lastLineDefined = 0,
            upValues = {},
            numParams = 0,
            isVararg = false,
            stack = {},
            stackSize = 2,
            
            instructions = {},
            constants = {},
            functions = {}
        };
        
        function proto.clearRuntime()
            proto.instructions = {};
            return true;
        end
        
        function proto.addConstant(val)
            if not (({ string = true, number = true, boolean = true })[rawtype(val)]) then
                traceback("wrong type");
                return false;
            end
            
            -- Try to find it in the list already
            for m,n in ipairs(proto.constants) do
                if (n == val) then
                    return m - 1;
                end
            end
            
            table.insert(proto.constants, val);
            return #proto.constants - 1;
        end
        
        function proto.getConstant(idx)
            return proto.constants[idx + 1];
        end
        
        function proto.setStackSize(num)
            proto.stackSize = num;
        end
        
        function proto.setArgumentCount(num)
            proto.numParams = num;
        end
        
        function proto.move(dst, src)
            table.insert(proto.instructions, dst * 64 + src * 8388608);
        end
        
        function proto.loadConst(reg, const)
            table.insert(proto.instructions, 1 + reg * 64 + const * 16384);
        end
        
        function proto.loadBool(reg, bool, skip)
            if (bool == true) then
                bool = 1;
            elseif (bool == false) then
                bool = 0;
            end
            
            if (skip == true) then
                skip = 1;
            elseif (skip == false) then
                skip = 0;
            end
            
            table.insert(proto.instructions, 2 + reg * 64 + bool * 8388608 + skip * 16384);
        end
        
        function proto.loadNil(from, to)
            table.insert(proto.instructions, 3 + from * 64 + to * 8388608);
        end
        
        function proto.getUpValue(reg, upval)
            table.insert(proto.instructions, 4 + reg * 64 + upval * 8388608);
        end
        
        function proto.getGlobal(reg, constspec)
            table.insert(proto.instructions, 5 + reg * 64 + constspec * 16384);
        end
        
        function proto.getTable(dst, srcTable, index, isConst)
            local inst = 6 + dst * 64 + srcTable * 8388608 + index * 16384;
            
            if (isConst) then
                inst = inst + 4194304;
            end
        
            table.insert(proto.instructions, inst);
        end
        
        function proto.setGlobal(reg, index)
            table.insert(proto.instructions, 7 + reg * 64 + index * 16384);
        end
        
        function proto.setUpValue(upval, reg)
            table.insert(proto.instructions, 8 + upval * 8388608 + reg * 64);
        end
        
        function proto.setTable(reg, idx, val, idxConst, valConst)
            local inst = 0;
            
            if (idxConst) then
                inst = inst + 2147483648;
            end
            
            if (valConst) then
                inst = inst + 4194304;
            end
            
            table.insert(proto.instructions, inst + 9 + reg * 64 + idx * 8388608 + val * 16384);
        end
        
        function proto.newTable(reg, sizearr, nrec)
            table.insert(proto.instructions, 10 + reg * 64 + sizearr * 8388608 + nrec * 16384);
        end
        
        -- TODO: self
        
        local function cnstr_arith(idx)
            return function(reg, op1, op2, cnst1, cnst2)
                local inst = 0;
            
                if (cnst1) then
                    inst = inst + 2147483648;
                end
                
                if (cnst2) then
                    inst = inst + 4194304;
                end
            
                table.insert(proto.instructions, inst + idx + reg * 64 + op1 * 8388608 + op2 * 16384);
            end
        end
        
        -- Arithmetic opcodes
        proto.add = cnstr_arith(12);
        proto.sub = cnstr_arith(13);
        proto.mul = cnstr_arith(14);
        proto.div = cnstr_arith(15);
        proto.mod = cnstr_arith(16);
        proto.pow = cnstr_arith(17);
        
        function proto.negate(dst, src)
            table.insert(proto.instructions, 18 + src * 8388608 + dst * 64);
        end
        
        function proto.instNot(dst, src)
            table.insert(proto.instructions, 19 + src * 8388608 + dst * 64);
        end
        
        function proto.len(dst, src)
            table.insert(proto.instructions, 20 + src * 8388608 + dst * 64);
        end
        
        function proto.concat(dst, from, to)
            table.insert(proto.instructions, 21 + dst * 64 + from * 8388608 + to + 16384);
        end
        
        local function get_sbx(x)
            return (x + 0x1FFFF) * 16384;
        end
        
        function proto.jump(instCount)
            table.insert(proto.instructions, 22 + get_sbx(instCount));
        end
        
        local function constr_cond(idx)
            return function(op1, op2, cndVal, cnst1, cnst2)
                local inst = idx;
                
                if (cnst1) then
                    inst = inst + 2147483648;
                end
                
                if (cnst2) then
                    inst = inst + 4194304;
                end
                
                if (type(cndVal) == "boolean") and (cndVal) then
                    inst = inst + 64;
                elseif (type(cndVal) == "number") then
                    inst = inst + cndVal * 64;
                end
                
                table.insert(proto.instructions, inst + op1 * 8388608 + op2 * 16384);
                
                outputDebugString(tostring(inst + op1 * 8388608 + op2 * 16384));
                local a, b, c, d = littleendian.int_to_bytes(inst + op1 * 8388608 + op2 * 16384);
                outputDebugString(a .. ", " .. b .. ", " .. c .. ", " .. d);
            end
        end
        
        -- Conditional operations
        proto.equal = constr_cond(23);
        proto.lessthan = constr_cond(24);
        proto.lessequal = constr_cond(25);
        
        function proto.instReturn(from, count)
            table.insert(proto.instructions, 30 + from * 64 + count * 8388608);
        end
        
        function proto.construct()
            -- Construct header
            local code = luaSignature
                .. luaVersion
                .. string.char(0)
                .. string.char(1)
                .. string.char(4)
                .. string.char(4)
                .. string.char(4)
                .. string.char(8)
                .. string.char(0)
                .. dumpFunction(proto); -- add function
            
            return loadstring(code, proto.name);
        end
        
        return proto;
    end

    function createAdvProto()
        local proto = createProto();
        local registers = {};
        local maxReg = 0;
        local regRefData = {};
        
        local function setReg(n)
            if (maxReg < n + 1) then
                maxReg = n + 1;
            end
            
            registers[n] = true;
            regRefData[n] = {
                refCount = 0,
                reqDestruction = false
            };
        end
        
        function proto.attemptAllocReg(idx)
            if (registers[idx]) then return false; end;
            
            setReg(idx);
            return true;
        end
        
        function proto.allocReg()
            local n = 0;
            
            while not (registers[n] == nil) do
                n = n + 1;
            end
            
            setReg(n);
            return n;
        end
        
        local function getRegData(idx)
            local reg = registers[idx];
            
            if not (type(reg) == "table") then return false; end;
            
            return reg;
        end
        
        function proto.unsetRegData(idx)
            local data = getRegData(idx);
            
            if not (data) then return; end;
            
            local destructor = data.destructor;
            
            if (destructor) then
                destructor(data.value);
            end
        end
        
        function proto.setRegData(idx, val, destructor)
            if not (registers[idx]) then return; end;
            
            proto.unsetRegData(idx);
            
            registers[idx] = {
                value = val,
                destructor = destructor
            };
        end
        
        function proto.clearRegData(idx)
            if not (registers[idx]) then return; end;
            
            registers[idx] = true;
        end
        
        function proto.referenceReg(idx)
            local refData = regRefData[idx];
            
            if not (refData) then return; end;
            
            refData.refCount = refData.refCount + 1;
        end
        
        local function destroyReg(idx)
            proto.unsetRegData(idx);
            registers[idx] = nil;
            regRefData[idx] = nil;
        end
        
        function proto.dereferenceReg(idx)
            local refData = regRefData[idx];
            
            if not (refData) then return; end;
            
            local refCount = refData.refCount;
            
            if (refCount == 1) and (refData.reqDestruction == true) then
                destroyReg(idx);
                return;
            end
            
            refData.refCount = refCount - 1;
        end
        
        local function allocRange(startIdx, endIdx)
            local regRange = {};
            
            local function setAllRegisters(val)
                for idx = startIdx,endIdx do
                    if (val) then
                        setReg(idx);
                    else
                        proto.freeReg(idx);
                    end
                end
            end
            
            -- Reserve all registers
            setAllRegisters(true);
            
            -- Update the register maximum
            if (maxReg < endIdx + 1) then
                maxReg = endIdx + 1;
            end
            
            function regRange.get()
                return startIdx, endIdx;
            end
            
            function regRange.destroy()
                setAllRegisters(nil);
            end
            
            return regRange;
        end
        
        function proto.allocRegRange(count)
            local n = 0;
            
            -- The register stack has an infinite size.
            -- If not, we should change this condition.
            while (true) do
                if (registers[n] == nil) then
                    local curReg = n;
                    local ncount = count - 1;
                    
                    while not (ncount == 0) do
                        curReg = curReg + 1;
                        
                        if (registers[curReg]) then
                            break;
                        end
                        
                        ncount = ncount - 1;
                    end
                    
                    if (ncount == 0) then
                        return allocRange(n, curReg);
                    end
                end
                
                n = n + 1;
            end
            
            return false;
        end
        
        function proto.allocRegTailRange(count)
            return allocRange(maxReg, maxReg + count - 1);
        end
        
        function proto.freeReg(idx)
            local refData = regRefData[idx];
            
            if not (refData) then return; end;
            if not (refData.refCount == 0) then
                refData.reqDestruction = true;
                return;
            end
            
            destroyReg(idx);
        end
        
        function proto.assignStackSize()
            proto.setStackSize(maxReg + 4);
            return true;
        end
        
        return proto;
    end

    local function createObjectStack(destructor)
        local count = 0;
        local stack = {};
        
        function stack.push(obj)
            count = count + 1;
            stack[count] = obj;
        end
        
        function stack.insert(obj, pos)
            outputDebugString("wut");
            
            table.insert(stack, pos, obj);
            count = count + 1;
        end
        
        function stack.insertMultiple(objs, pos)
            outputDebugString("wut");
            
            pos = math.min(1, math.max(count + 1, pos));
            
            local objCount = #objs;
            local offset = count - pos;
            local conflictCount = objCount - offset;
            
            if (conflictCount > 0) then
                -- Move conflicting values to the top
                local conflictPos = pos + objCount;
                local n = conflictCount;

                repeat
                    stack[conflictPos + conflictCount] = stack[conflictPos];
                    
                    n = n + 1;
                    conflictPos = conflictPos + 1;
                until (n == conflictCount);
            end
                
            -- Move without preserving slots
            local i = pos;
            
            while (offset > 0) do
                stack[i + objCount] = stack[i];
                offset = offset - 1;
                i = i + 1;
            end
            
            -- Now write the objects
            i = pos;
            local n = 1;
            
            while (objCount > 0) do
                stack[i] = objs[n];
                objCount = objCount - 1;
                n = n + 1;
            end
        end
        
        function stack.delete(item)
            outputDebugString("deleted");
            
            table.delete(stack, item);
            count = count - 1;
        end
        
        function stack.first()
            return stack(1);
        end
        
        function stack.top()
            return stack[count];
        end
        
        function stack.size()
            return count;
        end
        
        function stack.pop()
            count = count - 1;
        end
        
        function stack.deflate(itemHandler)
            local item = stack.top();
            
            while (item) do
                if (itemHandler) then
                    itemHandler(item);
                end
                
                stack.pop();
                item = stack.top();
            end
        end
        
        function stack.clear()
            stack.deflate(destructor);
        end
        
        function stack.destroy()
            stack.clear();
        end
        
        return stack;
    end

    local function createVirtualMachine(proto, parent)
        local vm = {};
        
        local function createOpcode(name, ...)
            local op = {
                name = name,
                args = { ... }
            };
            local location = 1;
            
            function op.purge()
                return proto[name]( unpack( op.args ) );
            end
            
            function op.locate(loc)
                location = loc;
                return true;
            end
            
            function op.tell()
                return location;
            end
            
            function op.printArgs()
                if (#op.args == 0) then
                    return "";
                end
                
                local out = "\t";
                
                for m,n in ipairs(op.args) do
                    out = out .. tostring(n);
                    
                    if not (m == #op.args) then
                        out = out .. ",";
                    end
                end
                
                return out;
            end
            
            return op;
        end
        
        local op_dispatch = {
            jump = function(dest)
                local op = createOpcode("jump");
                
                function op.purge()
                    return proto.jump( dest.tell() - op.tell() );
                end
                
                function op.setTarget(target)
                    dest = target;
                    return true;
                end
                
                function op.getTarget()
                    return dest;
                end
                
                return op;
            end
        };
        setmetatable(op_dispatch,
            {
                __index = function(t, k)
                    return function(...)
                        return createOpcode(k, ...);
                    end
                end
            }
        );
        
        function vm.createOpcodeChain()
            local chain = {};
            local opcodeStack = createObjectStack(
                function(op)
                    op.destroy();
                end
            );
            
            function chain.addOpcode(name, ...)
                local op = op_dispatch[name]( ... );
                opcodeStack.push(op);
                return op;
            end
        
            local op = {};
            local opmeta = {
                __index = function(t, k)
                    return function(...)
                        return chain.addOpcode(k, ...);
                    end
                end
            };
            setmetatable(op, opmeta);
            chain.op = op;
            
            -- For easy modification
            chain.setOpcode = opcodeStack.push;
            chain.insertOpcode = opcodeStack.insert;
            chain.insertMultiple = opcodeStack.insertMultiple;
            
            function chain.push(onto)
                opcodeStack.deflate(onto.setOpcode);
                return true;
            end
            
            function chain.insert(pos, into)
                into.insertMultiple(opcodeStack, pos);
                opcodeStack.deflate();
                return true;
            end
            
            function chain.getStack()
                return opcodeStack;
            end
            
            return chain;
        end
        
        local rootChain = vm.createOpcodeChain();
        local rootExecStack = rootChain.getStack();
        
        -- Create the executive context to parse opcodes on
        local execContext = createObjectStack();
        
        -- First executive context is the root chain
        -- It is the only chain which gets dispatches to opcodes on the proto.
        execContext.push(rootChain);
        vm.execContextStack = execContext;
        
        -- Parse opcodes on the current chain
        local op = {};
        local opmeta = {
            __index = function(t, k)
                return function(...)
                    return execContext.top().addOpcode(k, ...);
                end
            end
        };
        setmetatable(op, opmeta);
        vm.op = op;
        
        local locals = {};
        
        function vm.createLocal(name)
            local var = {
            };
            local reg = false;
            
            function var.access()
                if not (reg) then
                    reg = proto.allocReg();
                end
                
                return reg;
            end
            
            function var.write(src, const)
                if not (reg) then
                    reg = proto.allocReg();
                end
                
                if (const) then
                    vm.op.loadConst(reg, src);
                else
                    vm.op.move(reg, src);
                end
            end
            
            function var.setnil()
                if not (reg) then
                    proto.allocReg();
                end

                vm.op.loadNil(reg, reg);
            end
            
            function var.isUsed()
                return reg;
            end
            
            function var.destroy()
                if (reg) then
                    proto.freeReg(reg);
                end
                
                locals[name] = nil;
            end
            
            locals[name] = var;
            return var;
        end
        
        function vm.obtainVariable(name)
            return locals[name];
        end
        
        function vm.compile()
            -- Clear any previous instructions
            proto.clearRuntime();
            
            -- Set the stack size
            proto.assignStackSize();
            
            local loc = 1;
            
            -- Write upvalue accesses
            
            -- First write the opcode locations
            for m,n in ipairs(rootExecStack) do
                n.locate(m);
            end
            
            -- Now purge them
            for m,n in ipairs(rootExecStack) do
                n.purge();
            end
            
            -- Return opcode for security
            proto.instReturn(0, 1);
            
            -- DEBUG stuff
            vm.printOpcodes();

            -- The runtime should construct the protoype itself
            return true;
        end
        
        function vm.printOpcodes()
            local out = "-- OPCODES --\n";
            
            for m,n in ipairs(rootExecStack) do
                out = out .. n.name .. n.printArgs() .. "\n";
            end
            
            print(out);
            return true;
        end
        
        function vm.destroy()
            for m,n in pairs(locals) do
                n.destroy();
            end
            
            execContext.destroy();
        end
        
        return vm;
    end

    local function strlimit(str, nchar)
        local len = strlen(str);

        if (len > nchar) then
            return strsub(str, len - nchar, len) .. "...";
        end
        
        return str;
    end

    local function createEnvironment()
        local env = {
            source = "LuaEX",
            tokens = {},
            line = 1,
            err = nil
        };
        local upvalues = {};
        
        function env.error(output)
            traceback(output);
            env.err = strlimit(env.source, 30) .. ":" .. env.line .. ": " .. output;
            error(output, 2);
        end
        
        function env.setSource(source)
            env.source = '[string: "' .. source .. '"]';
            return true;
        end
        
        function env.getLastError()
            return env.err;
        end

        return env;
    end

    local function createLifetimeData()
        local data = createClass();
        local localConstants = {};
        
        function data.unsetLocalConstantLink(name)
            local cnst = localConstants[name];
            
            if not (cnst) then return; end;
            
            cnst.destroy();
            cnst.dereference();
            
            localConstants[name] = nil;
        end
        
        -- Optimize away locals which hold true constant values
        function data.setLocalConstantLink(name, obj)
            data.unsetLocalConstantLink(name);
            
            obj.reference();
            
            localConstants[name] = obj;
        end
        
        function data.getLocalConstantLink(name)
            local link = localConstants[name];
            
            if (link) then return link; end;    
        
            local parent = data.getParent();
            
            return parent and parent.getLocalConstantLink(name) or false;
        end
        
        -- Make sure we update upvalues
        function data.isVariableUpdateForced(name)
            return false;
        end
        
        function data.clone()
            local data = createLifetimeData();
            
            for m,n in pairs(localConstants) do
                data.setLocalConstantLink(m, n);
            end
            
            return data;
        end
        
        function data.destroy()
            -- Destroy all referenced objects
            for m,n in pairs(localConstants) do
                data.unsetLocalConstantLink(m);
            end
        end
        
        return data;
    end

    local function createParser(chunk, environment)
        local offset = 1;
        local token, begin, errStart;
        
        local testOffset = false;
        
        local parser = {};
        
        function parser.read()
            if (offset == false) then return false; end;
            
            offset, token, begin, errStart = getNextItem(chunk, offset);
            
            if (offset == false) then
                if not (token) then
                    return false;
                end
                
                -- Output a syntax error and quit
                parser.error("syntax error");
                return false;
            end
            
            if (strsub(token, 1, 2) == "--") or (strsub(token, 1, 4) == "--[[") then
                return parser.read();
            end
            
            return token;
        end
        
        function parser.goto(find)
            local token = parser.read();
            
            while (token) and not (token == find) do
                token = parser.read();
            end
            
            return token;
        end
        
        function parser.testRead()
            local off = offset;
            local token = parser.read();
            
            testOffset = offset;
            
            parser.setOffset(off);
            return token;
        end
        
        function parser.findToken()
            local offset, token, begin = getNextItem(chunk, offset);
            return begin;
        end
        
        function parser.applyRead()
            if not (testOffset) then return false; end;
            
            offset = testOffset;
            
            testOffset = false;
            return true;
        end
        
        function parser.error(msg)
            environment.error(msg);
        end
        
        function parser.setOffset(off)
            offset = off;
            return true;
        end
        
        function parser.getOffset()
            return offset;
        end
        
        function parser.getTokenStartOffset()
            return begin;
        end
        
        function parser.sub(start, endoff)
            return strsub(chunk, start, endoff);
        end
        
        function parser.clone()
            local obj = createParser(chunk, environment);
            obj.setOffset(offset);
            obj.setLifetimeData(parser.lifetimeData);
            return obj;
        end
        
        function parser.branchLifetime()
            parser.setLifetimeData(parser.lifetimeData.clone());
            return true;
        end
        
        function parser.isolateLifetime(main)
            parser.lifetimeData.setParent(main.lifetimeData);
            return true;
        end
        
        function parser.transcend(obj)
            obj.setOffset(offset);
            return true;
        end
        
        function parser.setLifetimeData(data)
            local _data = parser.lifetimeData;
            
            if (_data) then
                _data.dereference();
                _data.destroy();
            end
            
            parser.lifetimeData = data;
            data.reference();
        end
            
        function parser.destroy()
            parser.lifetimeData.destroy();
        end
        
        parser.setLifetimeData(createLifetimeData());
        return parser;
    end

    local function makeScopedParser(orig, beg, term, status)
        local _parser = orig.clone();
        
        local function constructParser(parser)
            local _read = parser.read;
            local finished = false;
            
            function parser.read()
                if (finished) then return false; end;
                
                local item = _read();
                
                if (item == beg) then
                    status = status + 1;
                elseif (item == term) then
                    if (status == 0) then
                        finished = true;
                        return false;
                    end
                    
                    status = status - 1;
                end
                
                return item;
            end
            
            local clone = parser.clone;
            
            function parser.clone()
                return constructParser(clone());
            end
            
            local setOffset = parser.setOffset;
            
            function parser.setOffset(off)
                setOffset(off);
                
                finished = false;
            end
            
            return parser;
        end
            
        return constructParser(_parser);
    end

    local function createScope(vm)
        local scope = createClass();
        local variables = {};
        
        function scope.createLocal(name)
            local var = vm.createLocal(name);
            
            table.insert(variables, var);
            return var;
        end
        
        function scope.destroy()
            for m,n in ipairs(variables) do
                n.destroy();
            end
        end
        
        return scope;
    end

    -- Object reading algorithms
    local function scopedAcquire(parser, inItem, outItem, status)
        local token = parser.read();
        
        while (token) do
            local byte = strbyte(token);
            
            if (byte == inItem) then
                status = status + 1;
            elseif (byte == outItem) then
                -- Did scope successfully terminate?
                if (status == 0) then
                    return true;
                end
                
                status = status - 1;
            end
            
            token = parser.read();
        end
        
        -- Scope never ended
        error("trollface");
        parser.error("scope never ended (" .. inItem .. ", " .. outItem ..")");
    end

    local function createRegisterStack(proto)
        local obj = createObjectStack(proto.freeReg);
        
        function obj.alloc()
            local reg = proto.allocReg();
            obj.push(reg);
            return reg;
        end
        
        return obj;
    end

    -- Standard object destructor
    local function safeObjRelease(obj)
        if (rawtype(obj) == "table") then
            local destroy = obj.destroy;
            
            if (destroy) then
                destroy();
            end
        end
    end

    local function compile(chunk, environment)
        local proto = createAdvProto();
        local vm = createVirtualMachine(proto);
        local execContext = vm.execContextStack;
        
        -- Create arith descriptor
        local arithDesc = {};
        
        local function objCall(obj, name, ...)
            local method = obj[name];
            
            if (method) and (rawtype(method) == "function") then
                return method(...);
            else
                environment.error("unknown object");
            end
        end
        
        local function dispatchObj(obj)
            local objtype = rawtype(obj);
            
            if (objtype == "number") then
                return obj, false;
            elseif (objtype == "table") then
                return objCall(obj, "access");
            elseif (objtype == "string") then
                return proto.addConstant(obj), true;
            elseif (objtype == "boolean") then
                return (obj == true) and 1 or 0, false;
            elseif (objtype == "nil") then
                return nil, true;
            end
            
            environment.error("invalid dispatch object type " .. objtype);
        end
        
        local function isObject(obj)
            return rawtype(obj) == "table";
        end
        
        local function resolveObj(obj)
            if (isObject(obj)) and (obj.needsResolution()) then
                return resolveObj(obj.resolve());
            end
            
            return obj;
        end
        
        -- Default object destructor
        local function safeRelease(obj)
            local type = rawtype(obj);
            
            if (type == "table") then
                objCall(obj, "destroy");
            elseif (type == "number") then
                proto.freeReg(obj);
            end
        end
        
        local function referenceObject(obj)
            local type = rawtype(obj);
            
            if (type == "table") then
                objCall(obj, "reference");
            elseif (type == "number") then
                proto.referenceReg(obj);
            end
        end
        
        local function dereferenceObject(obj)
            local type = rawtype(obj);
            
            if (type == "table") then
                objCall(obj, "dereference");
            elseif (type == "number") then
                proto.dereferenceReg(obj);
            end
        end
        
        local function moveArbitrary(dst, src)
            local dstType = rawtype(dst);
            local srcType = rawtype(src);
            
            if (dstType == "table") then
                if (dst.isStaticRegister()) and (srcType == "table") and not (src.isConstant()) then
                    local assign = src.assign;
                    
                    if (assign) then
                        assign(dst.read());
                        return;
                    end
                else
                    local write = dst.write;
                    
                    if (write) then
                        write(src);
                        return;
                    end
                end
            elseif (srcType == "table") and not (src.isConstant()) and (dstType == "number") then
                local assign = src.assign;
                
                if (assign) then
                    assign(dst);
                    return;
                end
            end
            
            local source, sourceConst = dispatchObj(src);
            local dest, destConst = dispatchObj(dst);
            
            if (destConst) then
                environment.error("cannot write to constant value");
            end
            
            if (sourceConst) then
                if (source == nil) then
                    vm.op.loadNil(dest, dest);
                else
                    vm.op.loadConst(dest, source);
                end
            else
                vm.op.move(dest, source);
            end
        end
        
        local function createObject()
            local obj = createClass();
            
            function obj.isConditional()
                return false;
            end
            
            function obj.isStaticRegister()
                return false;
            end
            
            function obj.isConstant()
                return false;
            end
            
            function obj.needsResolution()
                return false;
            end
            
            function obj.resolve()
                return obj;
            end
            
            function obj.destroy()
                return;
            end
            
            return obj;
        end
        
        local function makeConstantObject(proto, obj, cnst)
            local resultReg = false;
            
            function obj.isConstant()
                return true;
            end
            
            function obj.getConstant()
                return cnst;
            end
            
            function obj.write(reg)
                environment.error("cannot write value to constant");
            end
            
            if (cnst == nil) then
                function obj.read()
                    if not (resultReg) then
                        resultReg = proto.allocReg();
                    end
                    
                    vm.op.loadNil(resultReg, resultReg);
                    return resultReg;
                end
                
                function obj.assign(reg)
                    vm.op.loadNil(reg, reg);
                end
                
                function obj.access()
                    return nil, true;
                end
            else
                function obj.read()
                    if not (resultReg) then
                        resultReg = proto.allocReg();
                    end
                    
                    vm.op.loadConst(resultReg, proto.addConstant(cnst));
                    return resultReg;
                end
                
                function obj.assign(reg)
                    vm.op.loadConst(reg, proto.addConstant(cnst));
                end
                
                function obj.access()
                    return proto.addConstant(cnst), true;
                end
            end
            
            function obj.destroy()
                if (resultReg) then
                    proto.freeReg(resultReg);
                end
            end
        end
        
        local function createConstant(cnst)
            local obj = createObject();
            makeConstantObject(proto, obj, cnst);
            return obj;
        end
        
        local function createRegisterObject()
            local obj = createObject();
            local reg = proto.allocReg();
            proto.setRegData(reg, obj,
                function(val)
                    reg = false;
                    
                    outputDebugString("destroyed register");
                    val.destroy();
                end
            );
            
            function obj.isStaticRegister()
                return true;
            end
            
            function obj.read()
                return reg;
            end
            
            function obj.assign(valReg)
                moveArbitrary(valReg, reg);
            end
            
            function obj.access()
                return reg, false;
            end
            
            function obj.write(val)
                moveArbitrary(reg, val);
            end
            
            function obj.destroy()
                if (reg) then
                    proto.clearRegData(reg);
                    proto.freeReg(reg);
                end
            end
            
            return obj;
        end
        
        local function dispatchObjSecure(obj, regStack)
            local res, const = dispatchObj(obj);
            
            if (const) and (res == nil) then
                local resReg = regStack.alloc();
                vm.op.loadNil(resReg, resReg);
                return resReg, false;
            end
            
            return res, const;
        end
        
        local function dispatchObjReg(obj, regStack)
            local objRes, objConst = dispatchObjSecure(obj);
            
            if (objConst) then
                local transReg = regStack.alloc();
                
                vm.op.loadConst(transReg, objRes);
                return transReg, false;
            end
            
            return objRes, objConst;
        end
        
        local function dispatchOperands(constmod, regStack, op1, op2)
            -- Optimize away constant objects (objects which are always the same)
            if (isObject(op1)) and (op1.isConstant()) and
               (isObject(op2)) and (op2.isConstant()) then
                
                local success, newConst = pcall(function() return constmod(op1.getConstant(), op2.getConstant()); end);
                
                if (success) then
                    return createConstant(newConst);
                end
            end
            
            -- Optimize away results of the objects which may have turned out constant (under current scenario)
            -- Side effect is that due to the object access constants have been allocated inside the program
            local first, firstConst = dispatchObjSecure(op1, regStack);
            local second, secondConst = dispatchObjSecure(op2, regStack);
            
            if (firstConst) and (secondConst) then
                local success, newConst = pcall(function() return constmod(proto.getConstant(first), proto.getConstant(second)); end);
                
                if (success) then
                    return createConstant(newConst);
                end
            end
                
            -- We have to calculate it dynamically
            return false, first, second, firstConst, secondConst;
        end
        
        local function performArith(dynObjConstructor, constmod, op1, op2)
            local regStack = createRegisterStack(proto);
            local constObj, first, second, firstConst, secondConst = dispatchOperands(constmod, regStack, op1, op2);    
            
            if (constObj) then
                -- The calculation was performed constant.
                return constObj;
            end
            
            -- Create a dynamic arithmetics object so that we can write result into correct location
            local obj = createObject();
            
            -- Reference the operands as we need them till destruction
            referenceObject(op1);
            referenceObject(op2);
            
            dynObjConstructor(obj, first, second, firstConst, secondConst);
            
            function obj.destroy()
                -- Make sure our operands (whereever they may come from) are released
                safeRelease(op1);
                safeRelease(op2);
                
                -- We do not need the operands anymore.
                dereferenceObject(op2);
                dereferenceObject(op1);
            end
            
            regStack.destroy();
            return obj;
        end
        
        local function performStandardArith(name, constmod, op1, op2)
            return performArith(
                function(obj, first, second, firstConst, secondConst)
                    local resultReg = false;
                    
                    local function writeArithIntoReg(reg)
                        vm.op[name](reg, first, second, firstConst, secondConst);
                    end
                    
                    function obj.read()
                        if not (resultReg) then
                            resultReg = proto.allocReg();
                        end
                        
                        writeArithIntoReg(resultReg);
                        return resultReg;
                    end
                    
                    function obj.assign(reg)
                        writeArithIntoReg(reg);
                    end
                    
                    function obj.access()
                        if not (resultReg) then
                            resultReg = proto.allocReg();
                        end
                        
                        writeArithIntoReg(resultReg);
                        return resultReg, false;
                    end
                    
                    function obj.write(val)
                        environment.error("cannot write value to arithmetics object");
                    end
                    
                    function obj.destroy()
                        if (resultReg) then
                            proto.freeReg(resultReg);
                        end
                    end
                end,
                constmod, op1, op2
            );
        end
        
        local function performStandardConditional(dynmod, constmod, cndVal, op1, op2)
            return performArith(
                function(obj, first, second, firstConst, secondConst)
                    local resultReg = false;
                    
                    function obj.isConditional()
                        return true;
                    end
                    
                    function obj.getConditionalValue()
                        return cndVal;
                    end
                    
                    function obj.performConditional()
                        dynmod(first, second, getConditionalValue(), firstConst, secondConst);
                    end
                    
                    local function writeConditionalIntoReg(reg)
                        dynmod(first, second, obj.getConditionalValue(), firstConst, secondConst);
                        
                        local op = vm.op.jump();
                        op.setTarget(op);
                        vm.op.loadBool(reg, false, true);
                        vm.op.loadBool(reg, true, false);
                    end
                    
                    function obj.read()
                        if not (resultReg) then
                            resultReg = proto.allocReg();
                        end
                        
                        writeConditionalIntoReg(resultReg);
                        return resultReg;
                    end
                    
                    function obj.assign(reg)
                        writeConditionalIntoReg(reg);
                    end
                    
                    function obj.access()
                        if not (resultReg) then
                            resultReg = proto.allocReg();
                        end
                        
                        writeConditionalIntoReg(resultReg);
                        return resultReg, false;
                    end
                    
                    function obj.write(val)
                        environment.error("cannot write value to conditional object");
                    end
                    
                    function obj.destroy()
                        if (resultReg) then
                            proto.freeReg(resultReg);
                        end
                    end
                end,
                constmod, op1, op2
            );
        end
        
        local arithTable =
        {
            -- Arithmetics
            ["+"] = {
                handler = function(...) return performStandardArith("add", function(op1, op2) return op1 + op2; end, ...); end,
                priority = 1
            },
            ["-"] = {
                handler = function(...) return performStandardArith("sub", function(op1, op2) return op1 - op2; end, ...); end,
                priority = 1
            },
            ["*"] = {
                handler = function(...) return performStandardArith("mul", function(op1, op2) return op1 * op2; end, ...); end,
                priority = 2
            },
            ["/"] = {
                handler = function(...) return performStandardArith("div", function(op1, op2) return op1 / op2; end, ...); end,
                priority = 2
            },
            ["%"] = {
                handler = function(...) return performStandardArith("mod", function(op1, op2) return op1 % op2; end, ...); end,
                priority = 3
            },
            ["^"] = {
                handler = function(...) return performStandardArith("pow", function(op1, op2) return op1 ^ op2; end, ...); end,
                priority = 4
            }
        };
        
        local condTable =
        {
            -- Conditionals
            ["=="] = {
                handler = function(...)
                    return performStandardConditional(
                        function(first, second, cndVal, firstConst, secondConst)
                            vm.op.equal(first, second, cndVal, firstConst, secondConst);
                        end,
                        function(op1, op2) return op1 == op2; end,
                        true, ...
                    );
                end,
                priority = 0
            },
            ["<="] = {
                handler = function(...)
                    return performStandardConditional(
                        function(first, second, cndVal, firstConst, secondConst)
                            vm.op.lessequal(first, second, cndVal, firstConst, secondConst);
                        end,
                        function(op1, op2) return op1 <= op2; end,
                        true, ...
                    );
                end,
                priority = 0
            },
            ["<"] = {
                handler = function(...)
                    return performStandardConditional(
                        function(first, second, cndVal, firstConst, secondConst)
                            vm.op.lessthan(first, second, cndVal, firstConst, secondConst);
                        end,
                        function(op1, op2) return op1 < op2; end,
                        true, ...
                    );
                end,
                priority = 0
            },
            [">="] = {
                handler = function(...)
                    return performStandardConditional(
                        function(first, second, cndVal, firstConst, secondConst)
                            vm.op.less(first, second, cndVal, firstConst, secondConst);
                        end,
                        function(op1, op2) return op1 >= op2; end,
                        false, ...
                    );
                end,
                priority = 0
            },
            [">"] = {
                handler = function(...)
                    return performStandardConditional(
                        function(first, second, cndVal, firstConst, secondConst)
                            vm.op.lessthan(first, second, cndVal, firstConst, secondConst);
                        end,
                        function(op1, op2) return op1 > op2; end,
                        false, ...
                    );
                end,
                priority = 0
            }
        };
        
        function arithDesc.getArith(parser)
            local token = parser.testRead();
            
            if not (token) then return false; end;
            
            local arith = arithTable[token];
            
            if (arith) then
                parser.applyRead();
                return arith;
            end
            
            local offset = parser.findToken();
            local cond = condTable[parser.sub(offset, offset + 1)];
            
            if (cond) then
                parser.setOffset(offset + 2);
                return cond;
            end
            
            cond = condTable[token];
            
            if (cond) then
                parser.applyRead();
                return cond;
            end
            
            return false;
        end
        
        local function preProcessObject(parser)
            local data = {};
            local infoCount = 0;
            
            function data.getTop()
                return data[infoCount];
            end
            
            function data.push(info)
                infoCount = infoCount + 1;
                data[infoCount] = info;
            end
            
            function data.pop()
                infoCount = infoCount - 1;
            end
            
            local token = parser.testRead();
            
            local function makeRegMod(opName, cnstHandler)
                return {
                    process = function(obj)
                        local op = vm.op[opName];
                        
                        local resultReg = false;
                        
                        if (obj.isConditional()) then
                            if not (opName == "instNot") then
                                environment.error("attempt to do illogical stuff with a conditional operation");
                            end
                            
                            function obj.getConditionalValue()
                                return cnstHandler(super());
                            end
                        end
                        
                        function obj.read()
                            local reg = super();
                            
                            op(reg, reg);
                            return reg;
                        end
                        
                        function obj.assign(reg)
                            super(reg);
                            op(reg, reg);
                        end
                        
                        function obj.access()
                            local accIdx, cnst = super();
                            
                            if (cnst) then
                                local success, newConstant = pcall(function() return cnstHandler(proto.getConstant(accIdx)); end);
                                
                                if (success) then
                                    return proto.addConstant(newConstant), true;
                                end
                                
                                if not (resultReg) then
                                    resultReg = proto.allocReg();
                                end
                                
                                vm.op.loadConst(resultReg, accIdx);
                                
                                accIdx = resultReg;
                            end
                            
                            op(accIdx, accIdx);
                            return accIdx, false;
                        end
                        
                        function obj.resolve()
                            local res = super();
                            
                            if (isObject(res)) and (res.isConstant()) then
                                local success, newConstant = pcall(function() return cnstHandler(res.getConstant()); end);
                                
                                if (success) then
                                    safeRelease(res);
                                    return createConstant(newConstant);
                                end
                            end
                            
                            return res;
                        end
                                
                        function obj.destroy()
                            if (resultReg) then
                                proto.freeReg(resultReg);
                            end
                        end
                    end,
                    constmod = cnstHandler
                };
            end
            
            while (token) do
                local item;
                
                if (token == "-") then
                    item = makeRegMod("negate", function(cnst) return -cnst; end);
                elseif (token == "not") then
                    item = makeRegMod("instNot", function(cnst) return not cnst; end);
                elseif (token == "#") then
                    item = makeRegMod("len", function(cnst) return #cnst; end);
                else
                    break;
                end
                
                function item.isStaticRegister()
                    return false;
                end
                
                function item.write(reg)
                    parser.error("cannot write to modded entity");
                end
                
                item.token = token;
                
                data.push(item);
                
                parser.applyRead();
                token = parser.testRead();
            end
            
            return data;
        end
        
        -- Optimize away constant modifiers (if possible)
        local function constDeform(preData, cnst)
            local item = preData.getTop();
            
            while (item) do
                local constmod = item.constmod;
                
                if not (constmod) then break; end;
                
                local success, newConstant = pcall(function() return constmod(cnst); end);
                
                if not (success) then break; end;
                
                cnst = newConstant;
                
                preData.pop();
                item = preData.getTop();
            end
            
            return cnst;
        end
        
        local function postProcessObject(obj, preData)
            local item = preData.getTop();
            
            while (item) do
                item.process(obj);
                
                preData.pop();
                item = preData.getTop();
            end
        end
        
        local parseArith = nil;
        
        local function arithParseIsolated(parser)
            local nparser = parser.clone();
            local arithObj = parseArith(nparser, arithDesc);
            nparser.destroy();
            return arithObj;
        end
        
        local function makeArithObject(obj, parser)
            local objStack = createObjectStack(safeRelease);
            
            local function deployArith()
                local arithObj = arithParseIsolated(parser);
                objStack.push(arithObj);
                return arithObj;
            end
            
            function obj.resolve()
                obj.destroy();
                return arithParseIsolated(parser);
            end
            
            function obj.needsResolution()
                return true;
            end
            
            function obj.assign(reg)
                moveAribitrary(reg, deployArith());
            end
            
            function obj.read()
                local resultReg = proto.allocReg();
                objStack.push(resultReg);
                
                moveArbitrary(resultReg, deployArith());
                return resultReg;
            end
            
            function obj.write(reg)
                environment.error("cannot write value to spec definitions");
            end
            
            function obj.access()
                return dispatchObj(deployArith());
            end
            
            function obj.destroy()
                parser.destroy();
                objStack.destroy();
            end
        end
        
        local function makeVariableObject(lifetimeData, proto, obj, name)
            local var = vm.obtainVariable(name);
            
            local regStack = createRegisterStack(proto);
            
            if (var) then
                function obj.isStaticRegister()
                    return not lifetimeData.getLocalConstantLink(name);
                end
                
                function obj.read()
                    local constObj = lifetimeData.getLocalConstantLink(name);
                    local used = var.isUsed();
                    local reg = var.access();
                    
                    if (constObj) and not (used) then
                        vm.op.loadConst(reg, proto.addConstant(constObj.getConstant()));
                    end
                    
                    return reg;
                end
                
                function obj.isConstant()
                    return lifetimeData.getLocalConstantLink(name);
                end
                
                function obj.getConstant()
                    local constObj = lifetimeData.getLocalConstantLink(name);
                    
                    if (constObj) then
                        return constObj.getConstant();
                    end
                    
                    return false;
                end
                
                function obj.needsResolution()
                    return lifetimeData.getLocalConstantLink(name);
                end
                
                function obj.resolve()
                    local constObj = lifetimeData.getLocalConstantLink(name);
                    
                    if (constObj) then
                        obj.destroy();
                        return constObj;    -- TODO: make sure this does not cause trouble.
                    end
                    
                    return obj;
                end
                
                function obj.write(reg)
                    if (isObject(reg)) and (reg.isConstant()) then
                        local cnst = reg.getConstant();
                        lifetimeData.setLocalConstantLink(name, createConstant(cnst));
                        
                        if (lifetimeData.isVariableUpdateForced(name)) then
                            var.write(proto.addConstant(cnst), true);
                        end
                    else
                        local objRes, objConst = dispatchObj(reg);
                        local cnst;
                        
                        if (objConst) and (objRes == nil) then
                            cnst = nil;
                        else
                            cnst = proto.getConstant(objRes);
                        end
                        
                        if (objConst) and not (var.isUsed()) then
                            lifetimeData.setLocalConstantLink(name, createConstant(cnst));
                        else
                            if (objConst) and (objRes == nil) then
                                var.setnil();
                            else
                                var.write(objRes, objConst);
                            end
                        end
                        
                        if (objConst) then
                            lifetimeData.setLocalConstantLink(name, createConstant(cnst));
                        else
                            lifetimeData.unsetLocalConstantLink(name);
                        end
                    end
                end
                
                function obj.access()
                    local constObj = lifetimeData.getLocalConstantLink(name);
                    
                    if (constObj) then
                        return proto.addConstant(constObj.getConstant()), true;
                    else
                        return var.access(), false;
                    end
                end
                
                function obj.assign(reg)
                    local constObj = lifetimeData.getLocalConstantLink(name);
                    
                    if (constObj) then
                        vm.op.loadConst(reg, proto.addConstant(constObj.getConstant()));
                    else
                        moveArbitrary(reg, var.access());
                    end
                end
            else
                local function readProc()
                    local resultReg = proto.allocReg();
                    regStack.push(resultReg);
                    
                    vm.op.getGlobal(resultReg, proto.addConstant(name));
                    return resultReg;
                end
                
                function obj.read()
                    return readProc();
                end
                            
                function obj.write(reg)
                    local resultReg = dispatchObjReg(reg, regStack);
                    vm.op.setGlobal(resultReg, proto.addConstant(name));
                end
                
                function obj.access()
                    return readProc(), false;
                end
                
                function obj.assign(reg)
                    vm.op.getGlobal(reg, proto.addConstant(name));
                end
            end
            
            function obj.destroy()
                regStack.destroy();
            end
        end
        
        local function parsePostAccessors(obj, parser)
            local token = parser.testRead();
            
            while (token) do
                local byte = strbyte(token);
                
                local regStack = createRegisterStack(proto);
                
                if (byte == 91) then
                    parser.applyRead();
                    
                    local arithObj = createObject();
                    makeArithObject(arithObj, makeScopedParser(parser, "[", "]", 0));
                    
                    local readObj = obj.read;
                    
                    local function readTableProc()
                        local resultReg = regStack.alloc();
                        
                        vm.op.getTable(resultReg, readObj(), dispatchObjSecure(arithObj, regStack));
                        return resultReg;
                    end
                    
                    function obj.read()
                        return readTableProc();
                    end
                    
                    function obj.write(reg)
                        local arithRes, arithConst = dispatchObjSecure(arithObj, regStack);
                        local objRes, objConst = dispatchObjSecure(reg, regStack);
                        
                        vm.op.setTable(readObj(), arithRes, objRes, arithConst, objConst);
                    end
                    
                    function obj.access()
                        return readTableProc(), false;
                    end
                    
                    function obj.resolve()
                        return obj;
                    end
                    
                    function obj.needsResolving()
                        return false;
                    end
                    
                    function obj.assign(reg)
                        vm.op.getTable(reg, readObj(), dispatchObjSecure(arithObj, regStack));
                    end
                    
                    scopedAcquire(parser, 91, 93, 0);
                else
                    break;
                end
                
                function obj.destroy()
                    regStack.destroy();
                end
                
                token = parser.testRead();
            end
        end
        
        local function allocObject(parser)
            local obj = createObject();
            local preData = preProcessObject(parser);
            
            local token = parser.read();
            
            if not (token) then return false; end;
            
            local byte = strbyte(token);
            
            if (byte == 34) or (byte == 39) then    -- " '
                makeConstantObject(proto, obj, constDeform(preData, strsub(token, 2, #token - 1)));
            elseif (byte == 91) then        -- [
                local byte2 = strbyte(token, 2);
            
                if (byte2 == 91) then           -- [[
                    makeConstantObject(proto, obj, constDeform(preData, strsub(token, 3, #token - 2)));
                else
                    parser.error("syntax error");
                end
            elseif (byte == 40) then
                makeArithObject(obj, makeScopedParser(parser, "(", ")", 0));
                scopedAcquire(parser, 40, 41, 0);
            elseif (token == "true") then
                makeConstantObject(proto, obj, constDeform(preData, true));
            elseif (token == "false") then
                makeConstantObject(proto, obj, constDeform(preData, false));
            elseif (token == "nil") then
                makeConstantObject(proto, obj, constDeform(preData, nil));
            elseif (isName(byte)) then
                makeVariableObject(parser.lifetimeData, proto, obj, token);
            else
                local num = tonumber(token);
                
                if not (num) then
                    environment.error("invalid number " .. token);
                end
                
                makeConstantObject(proto, obj, constDeform(preData, num));
            end
            
            -- Parse post accessors
            parsePostAccessors(obj, parser);
            
            postProcessObject(obj, preData);
            
            return obj;
        end
        
        local function allocVariable(lifetimeData, name)
            local obj = createObject();
            makeVariableObject(lifetimeData, proto, obj, name);
            return obj;
        end

        function parseArith(parser, desc, operandTraverse)
            local getArith = desc.getArith;
            
            -- First operand
            local op1;
            
            if (operandTraverse) then
                op1 = operandTraverse;
            else
                op1 = allocObject(parser);
            end
            
            local arith = getArith(parser);
            
            while (arith) do
                local pre2Offset = parser.getOffset();
                local op2 = allocObject(parser);
                local nextArith = arithTable[parser.testRead()];
                
                local doBreak = false;
                
                if (nextArith) then
                    if (nextArith.priority > arith.priority) then
                        op2 = parseArith(parser, desc, op2);
                    elseif (nextArith.priority < arith.priority) then
                        if (operandTraverse) then
                            doBreak = true;
                        end
                    end
                end
                
                -- Resolve objects
                op1 = resolveObj(op1);
                op2 = resolveObj(op2);
                
                local newObj = arith.handler(op1, op2);
                
                safeRelease(op2);
                safeRelease(op1);
                
                op1 = newObj;
                
                if (doBreak) then
                    break;
                end
                
                arith = getArith(parser);
            end
            
            return resolveObj(op1);
        end
        
        local function readMultiArguments(parser, handler)
            local objects = {};
            local cur = 1;
            local object = handler(parser, 1);
            
            while (object) do
                objects[cur] = object;
                cur = cur + 1;
                
                local token = parser.testRead();
                
                if not (parser.testRead() == ",") then
                    break;
                end
                
                parser.applyRead();
                object = handler(parser, cur);
            end
            
            return objects;
        end
        
        local function getArithArguments(parser)
            return readMultiArguments(parser,
                function(parser)
                    return parseArith(parser, arithDesc);
                end
            );
        end
        
        local function compileScope(parser, scope, transition)
            local token = parser.testRead();
            
            while (token) do
                if (token == "function") then
                    parser.error("not supported yet");
                elseif (token == "if") then
                    parser.applyRead();
                    
                    local result = parseArith(parser, arithDesc);
                    
                    if not (parser.read() == "then") then
                        environment.error("syntax error (expected 'then')");
                    end
                    
                    -- Dispatch the result depending on what we got
                    if (isObject(result)) and (result.isConditional()) then
                        
                    else
                        
                    end
                elseif (token == "return") then
                    parser.applyRead();
                    
                    if (parser.testRead() == ";") then
                        parser.applyRead();
                        vm.op.instReturn(0, 1);
                    else
                        -- Move objects onto a new range
                        local function createNewObjectRange(objects)
                            local regRange = proto.allocRegRange(#objects);
                            local startIdx = regRange.get();
                            
                            for m,n in ipairs(objects) do
                                moveArbitrary(startIdx + (m - 1), n);
                            end
                            
                            return regRange;
                        end
                        
                        -- Optimization to use already established register range
                        local function establishRegisterRange(objects)
                            local count = #objects;
                            
                            if (count == 0) then return false; end;
                            
                            local startObj = objects[1];
                            
                            if (isObject(startObj)) and (startObj.isStaticRegister()) then
                                local startIdx = startObj.read();
                                local n = 2;
                                local item = objects[2];
                                
                                local regStack = createRegisterStack(proto);
                                
                                while (item) do
                                    local newIdx = startIdx + (n - 1);
                                    
                                    if not ((isObject(item)) and (item.isStaticRegister()) and (item.read() == newIdx))
                                        and not ((type(item) == "number") and (item == newIdx)) then
                                        
                                        if not (proto.attemptAllocReg(newIdx)) then
                                            -- Release all used registers
                                            regStack.destroy();
                                            return false;
                                        end
                                        
                                        regStack.push(newIdx);
                                        
                                        moveArbitrary(newIdx, item);
                                    end
                                    
                                    n = n + 1;
                                    item = objects[n];
                                end
                            
                                return startIdx, regStack;
                            end
                            
                            return false;
                        end
                        
                        local objects = getArithArguments(parser);
                        local resCount = #objects;
                        
                        if (resCount == 0) then
                            vm.op.instReturn(0, 1);
                        else
                            local startIdx, regStack = establishRegisterRange(objects);
                            
                            if not (startIdx) then
                                local regRange = createNewObjectRange(objects);
                                
                                vm.op.instReturn(regRange.get(), resCount + 1);
                                
                                regRange.destroy();
                            else
                                vm.op.instReturn(startIdx, resCount + 1);
                                
                                regStack.destroy();
                            end
                        end
                    end
                    
                    -- Return from the scope
                    return true;
                elseif not (token == ";") then
                    local function arithMultiplexSet(constructor, obtainer, slacker, otherHandler)
                        local objects = readMultiArguments(parser, constructor);
                        local n = 1;
                        
                        if (parser.read() == "=") then
                            local result = getArithArguments(parser);
                            
                            local max = #result;
                            
                            while (n <= max) do
                                local obj = objects[n];
                                local res = result[n];
                                
                                if not (obj) then
                                    item.read();   -- we have to execute anyway
                                else
                                    obtainer(obj, res);
                                end
                                
                                safeRelease(res);
                                n = n + 1;
                            end
                        else
                            otherHandler();
                        end
                        
                        -- Do remainder business
                        max = #objects;
                        
                        while (n <= max) do
                            slacker(objects[n]);
                            n = n + 1;
                        end
                    end
        
                    if (token == "local") then
                        parser.applyRead();
                        
                        arithMultiplexSet(
                            function(parser)
                                return parser.read();
                            end,
                            function(name, res)
                                scope.createLocal(name);
                                
                                local obj = allocVariable(parser.lifetimeData, name);
                                moveArbitrary(obj, res);
                                obj.destroy();
                            end,
                            function(name)
                                scope.createLocal(name);
                                
                                local obj = allocVariable(parser.lifetimeData, name);
                                obj.write(nil);
                                obj.destroy();
                            end,
                            function()
                            end
                        );
                    else
                        arithMultiplexSet(
                            function(parser)
                                return allocObject(parser);
                            end,
                            function(obj, res)
                                moveArbitrary(obj, res);
                                
                                obj.destroy();
                            end,
                            function(obj)
                                obj.write(nil);
                                obj.destroy();
                            end,
                            function()
                                parser.error("syntax error");
                            end
                        );
                    end
                end
                
                parser.applyRead();
                token = parser.testRead();
            end
            
            -- We did not return from the scope
            return false;
        end
        
        -- Compile the main scope
        local scope = createScope(vm);
        local parser = createParser(chunk, environment);
        compileScope(parser, scope, false);
        
        -- Compile the virtual machine result
        vm.compile();
        
        scope.destroy();
        parser.destroy();
        return proto;
    end
    
    return compiler;
end

-- Create the compiler that is used by the runtime by default.
local globalCompiler = createCompiler();

function loadstringex(script, options)
    local environment = globalCompiler.createEnvironment();
    environment.setSource(script);
    
    local success, proto = pcall(function() return globalCompiler.compile(script, environment) end);
    
    if not (success) then
        if not (environment.err) then
            return false, proto;
        end
        
        return false, environment.err;
    end
    
    local routine, errors = proto.construct();
    
    if not (routine) then
        return false, errors;
    end
    
    return routine;
end

-- Export compiler creation to the global runtime.
_G.createCompiler = createCompiler;
