-- MTA event system implementation
local events = {};

function addEvent(name, remoteCall)
    if (remoteCall == nil) then remoteCall = false; end;

    local event = events[name];
    
    if (event) then return false; end;
    
    events[name] = {
        name = name,
        remoteCall = remoteCall
    };
    return true;
end
registerDebugProxy("addEvent", "string", { "opt", "boolean" });

function addEventHandler(name, element, handler, propagate)
    if (propagate == nil) then propagate = false; end;
    
    local eventInfo = events[name];

    if not (eventInfo) then return false; end;
    
    return element.addEventHandler(name, handler, propagate);
end
registerDebugProxy("addEventHandler",
    "string", isElement, "function", { "opt", "boolean" }
);

function triggerEvent(name, element, ...)
    local eventInfo = events[name];
    
    if not (eventInfo) then return false; end;
    
    element.triggerEvent(name, ...);
    return true;
end
registerDebugProxy("triggerEvent",
    "string", isElement, "vararg"
);

function removeEventHandler(name, element, handler)
    local eventInfo = events[name];
    
    if not (eventInfo) then return false; end;
    
    return element.removeEventHandler(name, handler);
end
registerDebugProxy("removeEventHandler",
    "string", isElement, "function"
);

-- Add all default MTA:SA events.
local addEvent = addEvent;
addEvent("onClientColShapeHit");
addEvent("onClientColShapeLeave");
addEvent("onClientElementDestroy");
addEvent("onClientElementColShapeHit");
addEvent("onClientElementColShapeLeave");
addEvent("onClientElementDataChange");
addEvent("onClientElementStreamIn");
addEvent("onClientGUIClick");
addEvent("onClientGUIDoubleClick");
addEvent("onClientGUIChanged");
addEvent("onClientGUIAccepted");
addEvent("onClientGUIScroll");
addEvent("onClientClick");
addEvent("onClientDoubleClick");
addEvent("onClientKey");
addEvent("onClientCharacter");
addEvent("onClientMouseEnter");
addEvent("onClientMouseLeave");
addEvent("onClientMouseMove");
addEvent("onClientMouseWheel");
addEvent("onClientCursorMove");
addEvent("onClientGUIMouseDown");
addEvent("onClientGUIMouseUp");
addEvent("onClientGUIMove");
addEvent("onClientGUISize");
addEvent("onClientGUITabSwitched");
addEvent("onClientGUIComboBoxAccepted");
addEvent("onClientGUIFocus");
addEvent("onClientGUIBlur");
addEvent("onClientMarkerHit");
addEvent("onClientMarkerLeave");
addEvent("onClientPedDamage");
addEvent("onClientPedWasted");
addEvent("onClientPedWeaponFire");
addEvent("onClientPedChoke");
addEvent("onClientPedHeliKilled");
addEvent("onClientPedHitByWaterCannon");
addEvent("onClientPickupHit");
addEvent("onClientPickupLeave");
addEvent("onClientPlayerJoin");
addEvent("onClientPlayerQuit");
addEvent("onClientPlayerWeaponFire");
addEvent("onClientPlayerWeaponSwitch");
addEvent("onClientPlayerTarget");
addEvent("onClientPlayerDamage");
addEvent("onClientPlayerSpawn");
addEvent("onClientPlayerWasted");
addEvent("onClientPlayerVehicleEnter");
addEvent("onClientPlayerVehicleExit");
addEvent("onClientPlayerChangeNick");
addEvent("onClientPlayerStuntStart");
addEvent("onClientPlayerStuntFinish");
addEvent("onClientPlayerChoke");
addEvent("onClientPlayerRadioSwitch");
addEvent("onClientPlayerStealthKill");
addEvent("onClientPlayerVoiceStart");
addEvent("onClientPlayerVoiceStop");
addEvent("onClientPlayerHeliKilled");
addEvent("onClientPlayerHitByWaterCannon");
addEvent("onClientPlayerPickupHit");
addEvent("onClientPlayerPickupLeave");
addEvent("onClientObjectDamage");
addEvent("onClientObjectBreak");
addEvent("onClientProjectileCreation");
addEvent("onClientResourceStart");
addEvent("onClientResourceStop");
addEvent("onClientSoundChangedMeta");
addEvent("onClientSoundFinishedDownload");
addEvent("onClientSoundStream");
addEvent("onClientSoundBeat");
addEvent("onClientSoundStarted");
addEvent("onClientSoundStopped");
addEvent("onClientVehicleRespawn");
addEvent("onClientVehicleEnter");
addEvent("onClientVehicleExit");
addEvent("onClientVehicleExplode");
addEvent("onClientVehicleStartEnter");
addEvent("onClientVehicleStartExit");
addEvent("onClientTrailerAttach");
addEvent("onClientTrailerDetach");
addEvent("onClientVehicleCollision");
addEvent("onClientVehicleNitroStateChange");
addEvent("onClientWeaponFire");
addEvent("onClientConsole");
addEvent("onClientDebugMessage");
addEvent("onClientHUDRender");
addEvent("onClientMinimize");
addEvent("onClientRestore");
addEvent("onClientRender");
addEvent("onClientExplosion");
addEvent("onClientChatMessage");
addEvent("onClientPreRender");
addEvent("onClientFileDownloadComplete");

-- MTA:Lua Interpreter events.
addEvent("onClientCursorEnter");
addEvent("onClientCursorLeave");