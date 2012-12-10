-- Math for school :D
function flock(n)
	local res = 1;
	
	while not (n == 0) do
		res = res * n;
		n = n - 1;
	end
	
	return res;
end

function pflock(n,k)
	return flock(n) / (flock(n - k) * flock(k));
end

function pp(n,k,p)
	return pflock(n,k) * (p ^ k) * ((1 - p) ^ (n - k));
end

function r2p(n,k,p)
	k = math.floor(k);
	
	if (k < 0) then return false; end;
	if (p <= 0) or (p >= 1) then return false; end;

	local e = 0;
	
	repeat
		e = e + pp(n,k,p);
		k = k - 1;
	until (k < 0)
	
	return e;
end

function stddif(n,p)
	
end