local luargl = require("luargl")
local key = require("scripts.rgl.key")
local scheduler = require("scripts.rgl.scheduler")

-- i literally don't know how to create objects lmao
luargl.window_properties = {
	title = "lua + rgl <<333",
	width = 1500,
	height = 1500,
}


local circles = {}
local CIRCLES_COUNT = 100
local image = luargl.load_image_from_file("scripts/rgl/test.png")
print(image)

function rgl_app_init()
	print("app init from called inside Lua")

	math.randomseed(now())
	for i = 1, CIRCLES_COUNT do
		table.insert(circles, {
			{ math.random(-1000, 1000), math.random(-1000, 1000) },
			{ math.random(1, 255) * i, math.random(1, 255) * i, math.random(1, 255) * i },
			math.random(1, 10),
		})
	end
end

local loop_speed = 1000
function rgl_app_update(delta_time)
	local position = luargl.data.get_camera_position()

	if luargl.is_key_pressed(key.RGL_KEY_W) then
		position[2] = position[2] - delta_time * 1000
	end
	if luargl.is_key_pressed(key.RGL_KEY_A) then
		position[1] = position[1] - delta_time * 1000
	end
	if luargl.is_key_pressed(key.RGL_KEY_S) then
		position[2] = position[2] + delta_time * 1000
	end
	if luargl.is_key_pressed(key.RGL_KEY_D) then
		position[1] = position[1] + delta_time * 1000
	end

	if luargl.is_key_just_pressed(key.RGL_KEY_E) then
		loop_speed = loop_speed + 20
	end

	if luargl.is_key_just_pressed(key.RGL_KEY_Q) then
		loop_speed = loop_speed - 20
	end
	luargl.data.set_camera_position(position)

	scheduler.update()
end

function rgl_app_draw()
	for _, v in pairs(circles) do
		luargl.draw_circle(v[2], v[1], v[3])
	end
end

function loop()
	print("memory usage:", collectgarbage("count"))
	circles = {}
	for _ = 1, CIRCLES_COUNT do
		table.insert(circles, {
			{ math.random(-1000, 1000), math.random(-1000, 1000) },
			{ math.random(1, 255), math.random(1, 255), math.random(1, 255) },
			math.random(1, 150),
		})
	end

	scheduler.task(loop, loop_speed)
end

scheduler.task(loop, 1000)

-- todo: decide how this should work
luargl.make_window()
