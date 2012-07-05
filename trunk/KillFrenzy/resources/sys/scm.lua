function scmCreatePlayer(x, y, z)
	return callSCM(0x0053, "ifff", 1, 0, x, y, z);
end

function scmPlayMusic(id)
	return callSCM(0x0394, "i", 0, id);
end

function scmCreateCar(model, x, y, z)
	return callSCM(0x00A5, "ifff", 1, model, x, y, z);
end

function scmFade(ms, type)
	return callSCM(0x016A, "ii", 0, ms, type);
end

function scmRequestModel(id)
	return callSCM(0x0247, "i", 0, id);
end

function scmCreateActor(type, model, x, y, z)
	return callSCM(0x009A, "iifff", 1, type, model, x, y, z);
end