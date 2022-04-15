-- todo: fix this
local luargl = require("luargl")

local key = require("rgl.key")
local scheduler = require("rgl.scheduler")

luargl.window_properties = {
	title = "Luargl Playground",
	width = 600,
	height = 600,
}

local circles = {}
local CIRCLES_COUNT = 5000
local last_fps = 0

local image
local sprite

function rgl_app_init()
	image = luargl.load_image_from_file("playground/assets/test.jpg")
	sprite = luargl.create_sprite(image)

	math.randomseed(now())
	for i = 1, CIRCLES_COUNT do
		table.insert(circles, {
			{ math.random(-1000, 1000), math.random(-1000, 1000) },
			{ math.random(1, 255) * i, math.random(1, 255) * i, math.random(1, 255) * i },
			math.random(1, 10),
		})
	end
end

local function bob(speed, size)
	return math.sin(now() * speed) * size
end

local loop_speed = 1000
function rgl_app_update(delta_time)
	local position = luargl.camera.position

	if luargl.is_key_pressed(key.RGL_KEY_W) then
		position[2] = position[2] - delta_time * 1000 / luargl.camera.zoom
	end
	if luargl.is_key_pressed(key.RGL_KEY_A) then
		position[1] = position[1] - delta_time * 1000 / luargl.camera.zoom
	end
	if luargl.is_key_pressed(key.RGL_KEY_S) then
		position[2] = position[2] + delta_time * 1000 / luargl.camera.zoom
	end
	if luargl.is_key_pressed(key.RGL_KEY_D) then
		position[1] = position[1] + delta_time * 1000 / luargl.camera.zoom
	end

	if luargl.is_key_just_pressed(key.RGL_KEY_E) then
		loop_speed = loop_speed + 20
	end

	if luargl.is_key_just_pressed(key.RGL_KEY_Q) then
		loop_speed = loop_speed - 20
	end

	if luargl.is_key_pressed(key.RGL_KEY_R) then
		luargl.camera.zoom = luargl.camera.zoom + (2 * delta_time)
	end

	if luargl.is_key_pressed(key.RGL_KEY_F) then
		luargl.camera.zoom = luargl.camera.zoom - (2 * delta_time)
	end

	luargl.camera.zoom = math.max(luargl.camera.zoom, 0.1)
	luargl.camera.position = position

	scheduler.update()
end

local fps = 0
function rgl_app_draw()
	for _, v in pairs(circles) do
		-- luargl.draw_circle(v[1], v[2], v[3])
	end

	local mouse_position = luargl.get_mouse_position_in_world_space()
	-- local pos = luargl.vector2(bob(0.003, 10), bob(0.001, 10))
	-- sprite.position = pos
	sprite.size = luargl.vector2(256 + bob(0.001, 100), 256 + bob(0.004, 100))

	luargl.draw_sprite(sprite)
	luargl.draw_circle({ 255, 0, 0 }, mouse_position, 50)

	fps = fps + 1

	local info = string.format("memory usage: %.2f mb, fps: %.2f", collectgarbage("count") * 0.001, last_fps)
	luargl.render_text(info, 10, 10, 0.3 + bob(0.001, 0.2), { 255, 255, 255 })
	luargl.render_text("middle", 10, 300, 0.5, { 0, 255, 0 })
	luargl.render_text("left bottom", 10, 600 - 34, 0.5, { 255, 0, 0 })
end

function rgl_app_quit()
	print("leaving")
	luargl.destroy_sprite(sprite)
	luargl.destroy_image(image)
end

function loop()
	last_fps = fps
	fps = 0

	circles = {}
	for _ = 1, CIRCLES_COUNT do
		table.insert(circles, {
			{ math.random(1, 255), math.random(1, 255), math.random(1, 255) },
			{ math.random(-10000, 10000), math.random(-10000, 10000) },
			math.random(1, 150),
		})
	end

	scheduler.task(loop, loop_speed)
end

scheduler.task(loop, 1000)

-- todo: decide how this should work
luargl.make_window()
