function getDistanceBetweenPoints2D(posX, posY, targetX, targetY)
    local distX, distY = targetX - posX, targetY - posY;
    
    return math.sqrt( distX * distX + distY * distY );
end
registerDebugProxy("getDistanceBetweenPoints2D",
    "number", "number",
    "number", "number"
);

function getDistanceBetweenPoints3D(posX, posY, posZ, targetX, targetY, targetZ)
    local distX, distY, distZ = targetX - posX, targetY - posY, targetZ - posZ;
    
    return math.sqrt(distX * distX + distY * distY + distZ * distZ);
end
registerDebugProxy("getDistanceBetweenPoints3D",
    "number", "number", "number",
    "number", "number", "number"
);

local function easingTransform(strEasingType, fEasingPeriod, fEasingAmplitude, fEasingOvershoot, ...)
    local returnVals = {};
    
    for m,fProgress in ipairs({...}) do
        if (strEasingType == "Linear") then
            table.insert(returnVals, 
                fProgress
            );
        elseif (strEasingType == "InQuad") then
            table.insert(returnVals,
                fProgress * fProgress
            );
        elseif (strEasingType == "OutQuad") then
            table.insert(returnVals,
                1 - fProgress * fProgress
            );
        elseif (strEasingType == "InOutQuad") then
            table.insert(returnVals,
                -2 * fProgress * ( fProgress - 1.5 ) -- -2x^3 + 3x^2
            );
        elseif (strEasingType == "OutInQuad") then
            table.insert(returnVals,
                4 * fProgress * ( ( x - 0.75 ) ^ 2 + ( 0.75 - 0.75 ^ 2 ) )
            );
        elseif (strEasingType == "SineCurve") then
            table.insert(returnVals,
                math.sin(fProgress)
            );
        elseif (strEasingType == "CosineCurve") then
            table.insert(returnVals,
                math.cos(fProgress)
            );
        else
            table.insert(returnVals, false);
        end
    end

    -- TODO: support all functions.
    
    return unpack(returnVals);
end

function getEasingValue(fProgress, strEasingType, fEasingPeriod, fEasingAmplitude, fEasingOvershoot)
    if (fEasingPeriod == nil) then fEasingPeriod = 0.3; end
    if (fEasingAmplitude == nil) then fEasingAmplitude = 1.0; end
    if (fEasingOvershoot == nil) then fEasingOvershoot = 1.701; end
    
    return easingTransform(strEasingType, fEasingPeriod, fEasingAmplitude, fEasingOvershoot, fProgress);
end
registerDebugProxy("getEasingValue", "number", "string",
    { "opt", "number" },
    { "opt", "number" },
    { "opt", "number" }
);

function interpolateBetween(x1, y1, z1, x2, y2, z2, fProgress, strEasingType, fEasingPeriod, fEasingAmplitude, fEasingOvershoot)
    if (fEasingPeriod == nil) then fEasingPeriod = 0.3; end
    if (fEasingAmplitude == nil) then fEasingAmplitude = 1.0; end
    if (fEasingOvershoot == nil) then fEasingOvershoot = 1.701; end
    
    local distX, distY, distZ = x2 - x1, y2 - y1, z2 - z1;
    local mod = easingTransform(strEasingType, fEasingPeriod, fEasingAmplitude, fEasingOvershoot, fProgress);
    
    return x1 + distX * mod, y1 + distY * mod, z1 + distZ * mod;
end
registerDebugProxy("interpolateBetween",
    "number", "number", "number",
    "number", "number", "number",
    "number", "string",
    { "opt", "number" },
    { "opt", "number" },
    { "opt", "number" }
);

-- getColorFromString: internal funcition
-- getRealTime: internal function
-- getTickCount: internal function
-- getTimerDetails: internal function

function getNetworkStats()
    return {
        bytesReceived = 0,
        bytesSent = 0,
        packetsReceived = 0,
        packetsSent = 0,
        packetlossTotal = 100,
        packetlossLastSecond = 100,
        messagesInSendBuffer = 0,
        messagesInResendBuffer = 0,
        isLimitedByCongestionControl = false,
        isLimitedByOutgoingBandwidthLimit = false,
        encryptionStatus = 0
    };
end

function getPerformanceStats()
    return false;
end

function gettok(text, tokenNumber, seperatingCharacter)
    return false;
end
registerDebugProxy("gettok", "number", { "string", "number" });

-- killTimer: internal function
-- resetTimer: internal function
-- setTimer: internal function
-- split: internal function (?)

function getVersion()
    return {
        number = -1,
        mta = "-1",
        name = "MTA:Lua Interpreter",
        netcode = -1,
        os = "Lua Virtual Machine",
        type = "Release",
        tag = "1.0",
        sortable = "1.0.0-9.1.00000.0"
    };
end

-- isTimer: internal function
-- md5: internal function

function sha256(str)
    -- TODO
    return false;
end
registerDebugProxy("sha256", "string");

function tocolor(r, g, b, a)
    if (a == nil) then a = 255; end;
    
    -- Clamp the colors properly.
    r = math.min(255, math.max(0, math.floor(r)));
    g = math.min(255, math.max(0, math.floor(g)));
    b = math.min(255, math.max(0, math.floor(b)));
    a = math.min(255, math.max(0, math.floor(a)));
    
    return bit32.bor(r, g * 0x100, b * 0x10000, a * 0x1000000);
end
registerDebugProxy("tocolor", "number", "number", "number", { "opt", "number" });

-- utfChar: internal function
-- utfCode: internal function
-- utfLen: internal function
-- utfSeek: internal function
-- utfSub: internal function
-- toJSON: internal function
-- fromJSON: internal function

function setDevelopmentMode(enable)
    -- MTA:Lua Interpreter will not support development modes.
    return false;
end
registerDebugProxy("setDevelopmentMode", "boolean");

function getDevelopmentMode()
    return false;
end

function downloadFile(fileName)
    return false;
end
registerDebugProxy("downloadFile", "string");

-- TODO: encryption functions.
function teaEncode(string)
    return false;
end
registerDebugProxy("teaEncode", "string");

function teaDecode(string)
    return false;
end
registerDebugProxy("teaDecode", "string");

function base64encode(string)
    return false;
end
registerDebugProxy("base64encode", "string");

function base64decode(string)
    return false;
end
registerDebugProxy("base64decode", "string");

-- TODO: preg functions.
function pregFind(subject, pattern)
    return false;
end
registerDebugProxy("pregFind", "string", "string");

function pregReplace(subject, pattern, replacement)
    return false;
end
registerDebugProxy("pregReplace", "string", "string", "string");

function pregMatch(base, pattern)
    return false;
end
registerDebugProxy("pregMatch", "string", "string");

local bit32 = bit32;

function bitAnd(...)
    return bit32.band(...);
end

function bitNot(...)
    return bit32.bnot(...);
end

function bitOr(...)
    return bit32.bor(...);
end

function bitXor(...)
    return bit32.bxor(...);
end

function bitTest(...)
    return bit32.btest(...);
end

function bitLRotate(...)
    -- TODO
    return false;
end

function bitRRotate(...)
    -- TODO
    return false;
end

function bitLShift(...)
    return bit32.lshift(...);
end

function bitRShift(...)
    return bit32.rshift(...);
end

function bitArShift(...)
    return bit32.arshift(...);
end

function bitExtract(...)
    -- TODO
    return false;
end

function bitReplace(...)
    -- TODO
    return false;
end