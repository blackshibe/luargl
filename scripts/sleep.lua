-- this was removed in favor of RGL's draw loop
local function fnow()
	return math.floor(now() / 1000)
end

local function get_time()
	print("the time is", fnow())
end

local function loop()
	get_time()
	set_timeout(loop, 1000)
end

get_time()
set_timeout(loop, 1000)
