local rescallMT = {}
function rescallMT:__index(k)
        if type(k) ~= 'string' then k = tostring(k) end
        self[k] = function(resExportTable, ...)
                if type(self.res) == 'resource' and getResourceRootElement(self.res) then
                        return call(self.res, k, ...)
                else
                        return nil
                end
        end
        return self[k]
end
local exportsMT = {}
function exportsMT:__index(k)
        if type(k) == 'resource' and getResourceRootElement(k) then
                return setmetatable({ res = k }, rescallMT)
        elseif type(k) ~= 'string' then
                k = tostring(k)
        end
        local res = getResourceFromName(k)
        if res then
                return setmetatable({ res = res }, rescallMT)
        else
                outputDebugString('exports: Call to non-running client resource (' .. k .. ')', 1)
                return setmetatable({}, rescallMT)
        end
end
exports = setmetatable({}, exportsMT)