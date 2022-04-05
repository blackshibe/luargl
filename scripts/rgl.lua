local luargl = require("luargl")
local key = require("scripts.rgl.key")
local scheduler = require("scripts.rgl.scheduler")

for i, v in pairs(luargl.data) do
	print(i, v)
end

-- i literally don't know how to create objects lmao
luargl.window_properties = {
	title = "lua + rgl <<333",
	width = 1920,
	height = 1080,
}

local circles = {}
local CIRCLES_COUNT = 100

function rgl_app_init()
	print("app init from called inside Lua")

	math.randomseed(now())
	for i = 1, CIRCLES_COUNT do
		table.insert(circles, {
			{ math.random(-1000, 1000), math.random(-1000, 1000) },
			{ math.random(1, 255), math.random(1, 255), math.random(1, 255) },
			math.random(1, 255),
		})
	end
end

function rgl_app_update(delta_time)
	if luargl.is_key_pressed(key.RGL_KEY_W) then
		local position = luargl.data.get_camera_position()
		position[1] = position[1] + delta_time * 1000
		luargl.data.set_camera_position(position)
	end

	scheduler.update()
end

function rgl_app_draw()
	for _, v in pairs(circles) do
		luargl.draw_circle(v[2], v[1], v[3])
	end
end

function loop()
	-- print("memory usage:", collectgarbage("count"))
	circles = {}
	for _ = 1, CIRCLES_COUNT do
		table.insert(circles, {
			{ math.random(-1000, 1000), math.random(-1000, 1000) },
			{ math.random(1, 255), math.random(1, 255), math.random(1, 255) },
			math.random(1, 255),
		})
	end

	scheduler.task(loop, 1000)
end

scheduler.task(loop, 1000)

-- todo: decide how this should work
luargl.make_window()
