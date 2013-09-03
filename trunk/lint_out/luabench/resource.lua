-- Resource extensions
local resources = {};
local res_constructors = {};
local num_constructors = 0;

local function tdelete(t, v)
    for m,n in ipairs(t) do
        if (n == v) then
            table.remove(t, m);
            return true;
        end
    end
    
    return false;
end

function registerResourceExtension(constructor)
    num_constructors = num_constructors + 1;
    res_constructors[num_constructors] = constructor;
    
    -- Extend all alive resources
    for m,n in ipairs(resources) do
        n.extend(constructor);
    end
    
    return true;
end

function sysMakeResource(...)
    local res = newResource(...);

    -- System extension
    res.extend(
        function()
            function destroy()
                tdelete(resources, res);
            end
        end
    );
    
    -- Extend it by all constructors
    local n = 1;
    
    while (n <= num_constructors) do
        res.extend(res_constructors[n]);
        n = n + 1;
    end
    
    return res;
end