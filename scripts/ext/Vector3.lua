local Vector3 = {}

function Vector3.new()
	local self = {}

	self.x = 0
	self.y = 0
	self.z = 0

	return setmetatable(self, { __index = Vector3 })
end

return Vector3
