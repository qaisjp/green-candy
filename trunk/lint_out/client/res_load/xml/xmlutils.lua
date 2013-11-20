-- MTA XML library
-- We depend on the fileSystem functions.
local registerDebugProxy = registerDebugProxy;
local xml = xml;
local fileOpen = fileOpen;
local fileCreate = fileCreate;
local fileExists = fileExists;

-- All active XML objects
local xml_roots = {};

local function toxmlnode(val)
    if (xml.isNode(val)) then
        return val;
    end
    
    return false;
end

local function makeRootNode(node, path)
    local data = {
        filePath = path
    };
    
    node.data.mtaRuntime = data;
end

function xmlCopyFile(nodeToCopy, newFilePath)
    local newNode = nodeToCopy.clone();
    
    makeRootNode(newNode, newFilePath);
    return newNode;
end
registerDebugProxy("xmlCopyFile", toxmlnode, "string");

function xmlCreateFile(filePath, rootNodeName)
    local newNode = xml.createNode(rootNodeName);
    
    if not (newNode) then return false; end;
    
    makeRootNode(newNode, filePath);
    return newNode;
end
registerDebugProxy("xmlCreateFile", "string", "string");

function xmlDestroyNode(theXMLNode)
    theXMLNode.destroy();
end
registerDebugProxy("xmlDestroyNode", toxmlnode);

function xmlLoadFile(filePath)
    local file = fileOpen(filePath);
    
    if not (file) then return false; end;
    
    local content = file.read(file.size());
    file.destroy();
    
    local node = xml.parse(content);
    
    if not (node) then return false; end;
    
    makeRootNode(node, filePath);
    return node;
end
registerDebugProxy("xmlLoadFile", "string");

function xmlNodeGetAttribute(node, name)
    return node.attr[name];
end
registerDebugProxy("xmlNodeGetAttribute", toxmlnode, "string");

function xmlNodeGetValue(theXMLNode)
    -- Is that not deprecated?
    -- Once I figure out how to handle this, I will add it.
    return false;
end
registerDebugProxy("xmlNodeGetValue", toxmlnode);

function xmlNodeSetAttribute(node, name, value)
    node.attr[name] = tostring(value);
    return true;
end
registerDebugProxy("xmlNodeSetAttribute", toxmlnode, "string", { "string", "float" });

function xmlNodeSetValue(theXMLNode, value)
    -- Read note at xmlNodeGetValue.
    return false;
end
registerDebugProxy("xmlNodeSetValue", toxmlnode, "string");

function xmlSaveFile(rootNode)
    -- Only work for root nodes.
    local runtimeData = rootNode.data.mtaRuntime;
    
    if not (runtimeData) then return false; end;
    
    local filePath = runtimeData.filePath;
    local file;
    
    if (fileExists(filePath)) then
        file = fileOpen(filePath, false);
    else
        file = fileCreate(filePath);
    end
    
    if not (file) then return false; end;
    
    local content = rootNode.writeTree(0);
    
    local writeCount = file.write(content);
    
    return (writeCount == #content);
end
registerDebugProxy("xmlSaveFile", toxmlnode);

function xmlUnloadFile(node)
    -- Is that not the same as xmlDestroyNode?
    node.destroy();
    return true;
end
registerDebugProxy("xmlUnloadFile", toxmlnode);

function xmlCreateChild(parentNode, tagName)
    return parentNode.createChild(tagName);
end
registerDebugProxy("xmlCreateChild", toxmlnode, "string");

function xmlFindChild(parent, tagName, index)
    local children = parent.getChildren();
    local curIndex = 0;
    
    -- Normalize index.
    index = math.floor(index);
    
    -- Scan for the child case-sensitively.
    for m,n in ipairs(children) do
        if (n.name == tagName) then
            -- Make sure the index matches.
            if (curIndex == index) then
                return n;
            end
            
            curIndex = curIndex + 1;
        end
    end
    
    return false;
end
registerDebugProxy("xmlFindChild", toxmlnode, "string", "number");

function xmlNodeGetAttributes(node)
    -- Obtain a dictionary of all attributes.
    -- We should create a copy tho.
    local attrCopy = {};
    
    for m,n in pairs(node.attr) do
        attrCopy[m] = n;
    end
    
    return attrCopy;
end
registerDebugProxy("xmlNodeGetAttributes", toxmlnode);

function xmlNodeGetChildren(parent, index)
    local children = parent.getChildren();

    if (index == nil) then
        return children;
    end
    
    return children[index + 1];
end
registerDebugProxy("xmlNodeGetChildren", toxmlnode, { "opt", "number" });

function xmlNodeGetName(node)
    return node.getName();
end
registerDebugProxy("xmlNodeGetName", toxmlnode);

function xmlNodeGetParent(node)
    return node.getParent();
end
registerDebugProxy("xmlNodeGetParent", toxmlnode);

function xmlNodeSetName(node, name)
    return node.setName(name);
end
registerDebugProxy("xmlNodeSetName", toxmlnode, "string");

-- Extension functions
function xmlNodeSetParent(node, parent)
    return node.setParent(parent);
end
registerDebugProxy("xmlNodeSetParent", toxmlnode, toxmlnode);