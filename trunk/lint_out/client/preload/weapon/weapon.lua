-- MTA:SA weapon system.
-- This is pretty much to-do.
local createElement = createElement;
local weaponData = weaponData;

local function toweapon(obj)
    -- TODO
    return obj;
end

local function toplayer()
    -- TODO
    return false;
end

function createWeapon(theType, x, y, z)
    -- TODO
    return false;
end
registerDebugProxy("createWeapon", "string", "number", "number", "number");

function fireWeapon(theWeapon)
    -- TODO
    return false;
end
registerDebugProxy("fireWeapon", toweapon);

local function toweaponinfo(val)
    local valtype = rawtype(val);
    
    if (valtype == "number") then
        for m,n in pairs(weaponData) do
            if (n.id == val) then
                return n;
            end
        end
    elseif (valtype == "string") then
        return weaponData[val];
    end
    
    return false;
end

function getWeaponProperty(weaponIdOrName, weaponSkill, property)
    local weaponInfo = toweaponinfo(weaponIdOrName);
    
    if not (weaponInfo) then return false; end;
    
    local property = weaponInfo.property;
    
    if not (property) then return nil; end;
    
    local skillInfo = property[weaponSkill];
    
    if not (skillInfo) then return false; end;
    
    return skillInfo[property];
end
registerDebugProxy("getWeaponProperty", toweaponinfo, "string", "string");

function getWeaponState(theWeapon)
    -- TODO
    return false;
end
registerDebugProxy("getWeaponState", toweapon);

function getWeaponTarget(theWeapon)
    -- TODO
    return false;
end
registerDebugProxy("getWeaponTarget", toweapon);

function getWeaponOwner(theWeapon)
    -- TODO
    return false;
end
registerDebugProxy("getWeaponOwner", toweapon);

function getWeaponFlags(theWeapon, theFlag)
    -- TODO
    return false;
end
registerDebugProxy("getWeaponFlags", toweapon, "string");

function getWeaponFiringRate(theWeapon)
    -- TODO
    return false;
end

function getWeaponAmmo(theWeapon)
    -- TODO
    return false;
end
registerDebugProxy("getWeaponAmmo", toweapon);

function getWeaponClipAmmo(theWeapon)
    -- TODO
    return false;
end
registerDebugProxy("getWeaponClipAmmo", toweapon);

function resetWeaponFiringRate(theWeapon)
    -- TODO
    return false;
end
registerDebugProxy("resetWeaponFiringRate", toweapon);

function setWeaponProperty(theWeapon, strProperty, theValue)
    -- TODO
    return false;
end
registerDebugProxy("setWeaponProperty", toweapon, "string", { "float", "int" });

function setWeaponState(theWeapon, theState)
    -- TODO
    return false;
end
registerDebugProxy("setWeaponState", toweapon, "string");

function setWeaponTarget(...)
    -- TODO: need to expand debuglib to support argument based closure callbacks.
    return false;
end

function setWeaponOwner(theWeapon, theOwner)
    -- TODO
    return false;
end
registerDebugProxy("setWeaponOwner", toweapon, toplayer);

function setWeaponFlags(theWeapon, theFlag, theValue)
    -- TODO
    return false;
end
registerDebugProxy("setWeaponFlags", toweapon, "string", "any");

function setWeaponFiringRate(theWeapon, firingRate)
    -- TODO
    return false;
end
registerDebugProxy("setWeaponFiringRate", toweapon, "number");

function setWeaponAmmo(theWeapon, Ammo)
    -- TODO
    return false;
end
registerDebugProxy("setWeaponAmmo", toweapon, "number");

function setWeaponClipAmmo(theWeapon, clipAmmo)
    -- TODO
    return false;
end
registerDebugProxy("setWeaponClipAmmo", toweapon, "number");