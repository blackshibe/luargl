local key = require("rgl.key")
local scheduler = require("rgl.scheduler")

rgl.window_properties = {
	title = "Luargl Playground",
	width = 600,
	height = 600,
}

local circles = {}
local CIRCLES_COUNT = 5000
local last_fps = 0

local font
local image
local sprite

function rgl_app_init()
	font = rgl.load_font_from_file("rgl/font/FiraCode-Regular.ttf")
	image = rgl.load_image_from_file("playground/assets/fox.jpg")
	sprite = rgl.create_sprite(image)

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
	local position = rgl.camera.position

	if rgl.is_key_pressed(key.RGL_KEY_W) then
		position[2] = position[2] - delta_time * 1000 / rgl.camera.zoom
	end
	if rgl.is_key_pressed(key.RGL_KEY_A) then
		position[1] = position[1] - delta_time * 1000 / rgl.camera.zoom
	end
	if rgl.is_key_pressed(key.RGL_KEY_S) then
		position[2] = position[2] + delta_time * 1000 / rgl.camera.zoom
	end
	if rgl.is_key_pressed(key.RGL_KEY_D) then
		position[1] = position[1] + delta_time * 1000 / rgl.camera.zoom
	end

	if rgl.is_key_just_pressed(key.RGL_KEY_E) then
		loop_speed = loop_speed + 20
	end

	if rgl.is_key_just_pressed(key.RGL_KEY_Q) then
		loop_speed = loop_speed - 20
	end

	if rgl.is_key_pressed(key.RGL_KEY_R) then
		rgl.camera.zoom = rgl.camera.zoom + (2 * delta_time)
	end

	if rgl.is_key_pressed(key.RGL_KEY_F) then
		rgl.camera.zoom = rgl.camera.zoom - (2 * delta_time)
	end

	rgl.camera.zoom = math.max(rgl.camera.zoom, 0.1)
	rgl.camera.position = position

	scheduler.update()
end

local fps = 0
function rgl_app_draw()
	for _, v in pairs(circles) do
		rgl.draw_circle(v[1], v[2], v[3])
	end

	local mouse_position = rgl.get_mouse_position_in_world_space()
	sprite.size = rgl.vector2(256 + bob(0.001, 100), 256 + bob(0.004, 100))

	rgl.draw_sprite(sprite)
	rgl.draw_circle({ 255, 0, 0 }, mouse_position, 50)

	local info = string.format("memory usage: %.2f mb, fps: %.2f", collectgarbage("count") * 0.001, last_fps)
	local speed = string.format("loop speed: %ss", loop_speed / 1000)

	rgl.render_text(font, info, 10, 10, 0.15 + bob(0.005, 0.05), { 255, 255, 255 })
	rgl.render_text(font, speed, 10, 10 + (96 * (0.15 + bob(0.005, 0.05))), 0.15 + bob(0.005, 0.05), { 255, 255, 255 })

	fps = fps + 1
end

function rgl_app_quit()
	rgl.destroy_sprite(sprite)
	rgl.destroy_image(image)
end

function fps_loop()
	last_fps = fps
	fps = 0
	scheduler.task(fps_loop, 1000)
end

function loop()
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
scheduler.task(fps_loop, 1000)
