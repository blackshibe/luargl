local proxy = {}
local meta = {
	__index = function(...)
		print("index:", ...)
	end,
}

local metatable = setmetatable(proxy, meta)
local b = metatable.index
metatable.otherindex = 15
