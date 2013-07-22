function string.explode ( str, token )
	local t = {}
	if type ( token ) == "string" and token == "" then
		for c=1,#str do
			table.insert ( t, string.sub(str,c,c) )
		end
	else
		t = split ( str, token )
	end
	return t
end
