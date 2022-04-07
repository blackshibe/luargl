local test = require("test")
local Vector3 = require("scripts/ext/Vector3")

local vector = Vector3.new()
vector.x = 7.1231
vector.y = 5
vector.z = 3.14

test.test_vector3(vector)
