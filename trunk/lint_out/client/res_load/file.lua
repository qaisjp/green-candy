-- MTA:SA fileSystem functions.
-- The framework does export these functions by default, but they lack
-- private directory support. Hence we can easily rewrite those functions.
local registerDebugProxy = registerDebugProxy;
local resource = resource;
local fileRoot = resource.fileRoot;

local function tofile(obj)
    local objtype = type(obj);
    
    if (objtype == "file") then
        return obj;
    end
    
    return false;
end

function fileClose(theFile)
    theFile.destroy();
    return true;
end
registerDebugProxy("fileClose", tofile);

function fileCreate(filePath)
    return fileRoot.open(filePath, "wb");
end
registerDebugProxy("fileCreate", "string");

function fileDelete(filePath)
    return fileRoot.delete(filePath);
end
registerDebugProxy("fileDelete", "string");

function fileExists(filePath)
    return fileRoot.exists(filePath);
end
registerDebugProxy("fileExists", "string");

function fileFlush(theFile)
    return theFile.flush();
end
registerDebugProxy("fileFlush", tofile);

function fileGetPos(theFile)
    return theFile.tell();
end
registerDebugProxy("fileGetPos", tofile);

function fileGetSize(theFile)
    return theFile.size();
end
registerDebugProxy("fileGetSize", tofile);

function fileIsEOF(theFile)
    return theFile.eof();
end
registerDebugProxy("fileIsEOF", tofile);

function fileOpen(filePath, readOnly)
    if (readOnly == nil) then readOnly = false; end;
    
    local mode;
    
    if (readOnly) then
        mode = "r";
    else
        mode = "r+";
    end
    
    return fileRoot.open(filePath, mode);
end
registerDebugProxy("fileOpen", "string", { "opt", "boolean" });

function fileRead(theFile, count)
    return theFile.read(count);
end
registerDebugProxy("fileRead", tofile, "number");

function fileRename(filePath, newFilePath)
    return fileRoot.rename(filePath, newFilePath);
end
registerDebugProxy("fileRename", "string", "string");

function fileSetPos(theFile, offset)
    return theFile.seek(offset, "set");
end
registerDebugProxy("fileSetPos", tofile, "number");

function fileWrite(theFile, ...)
    local writeStrings = { ... };
    local write_fn = theFile.write;
    local total_out = 0;
    
    for m,n in ipairs(writeStrings) do
        local input = tostring(n);
        
        if (input) then
            total_out = total_out + write_fn(input);
        end
    end
    
    return total_out;
end
registerDebugProxy("fileWrite", tofile, "vararg");

function fileCopy(filePath, copyToFilePath, overwrite)
    if (overwrite == nil) then overwrite = false; end;
    
    if not (overwrite) and (fileRoot.exists(copyToFilePath)) then
        return true;
    end
    
    return fileRoot.copy(filePath, copyToFilePath);
end
registerDebugProxy("fileCopy", "string", "string", { "opt", "boolean" });

function fileCreateTranslator(filePath)
    local resFilePath = fileRoot.absPath(filePath);
    
    if not (resFilePath) then
        return false;
    end
    
    return file.createTranslator( resFilePath );
end
registerDebugProxy("fileCreateTranslator", "string");