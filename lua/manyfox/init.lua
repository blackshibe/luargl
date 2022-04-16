local flags = require("rgl.flags")
local scheduler = require("rgl.scheduler")

rgl.window_properties = {
	title = "manyfox",
	width = 900,
	height = 900,
}

local function bob(speed, size, offset)
	return math.sin(offset + now() * speed) * size
end

local function text_size(px)
	return px / BASE_FONT_HEIGHT
end

-- todo: Vector2.magnitude
local function magnitude(vec)
	return math.sqrt(vec[1] ^ 2 + vec[2] ^ 2)
end

local function rotate(cx, cy, x, y, angle)
	local radians = (math.pi / 180) * angle
	local cos = math.cos(radians)
	local sin = math.sin(radians)
	local nx = (cos * (x - cx)) + (sin * (y - cy)) + cx
	local ny = (cos * (y - cy)) - (sin * (x - cx)) + cy
	return { nx, ny }
end

local last_fps = 0
local fps = 0

local FOXES = 500
local foxes = {}

local font
local image

function rgl_app_init()
	font = rgl.load_font_from_file("rgl/font/FiraCode-Regular.ttf")
	image = rgl.load_image_from_file("manyfox/assets/fox.jpg")

	math.randomseed(now())

	-- i am aware you can generate random points with math.sqrt(random(-1, 1))
	for _ = 1, FOXES do
		local sprite = rgl.create_sprite(image)
		local rand_vector = {
			(math.random() * 2) - 1,
			(math.random() * 2) - 1,
		}

		local mag = magnitude({ rand_vector[1], rand_vector[2] })
		local radius = math.random(1, 700)
		sprite.position = rgl.vector2(rand_vector[1] / mag * radius, rand_vector[2] / mag * radius)
		table.insert(foxes, {
			sprite = sprite,
			speed_offset = math.random(5, 55),
			size_offset = math.random(5, 55),
			other_size_offset = math.random(1, 100),
		})
	end
end

function rgl_app_update(delta_time)
	scheduler.update()

	for _, v in pairs(foxes) do
		v.sprite.size = rgl.vector2(
			math.abs(bob(0.0001 * v.speed_offset, 10 + v.size_offset, v.other_size_offset)),
			math.abs(bob(0.0001 * v.speed_offset, 10 + v.size_offset, v.other_size_offset))
		)
		v.sprite.rotation = bob(0.00001 * v.speed_offset, 30, v.speed_offset)
	end
end

-- 10k images runs at roughly 30fps on my shitty laptop
-- but love2d gets like 2-3 fps in comparison
-- so i'll take it
function rgl_app_draw()
	local start = now()
	local white = { 255, 255, 255 }

	for _, v in pairs(foxes) do
		rgl.draw_sprite(v.sprite)
	end

	rgl.render_text(font, string.format("foxes: %s", FOXES), 10, 10, text_size(24), white)
	rgl.render_text(font, string.format("fps: %s", last_fps), 10, 20 + 24, text_size(24), white)
	rgl.render_text(font, string.format("frametime: %sms", now() - start), 10, 30 + 24 + 24, text_size(24), white)

	fps = fps + 1
end

function rgl_app_quit()
	print("quit")
end

function loop()
	last_fps = fps
	fps = 0
	scheduler.task(loop, 1000)
end

scheduler.task(loop, 1000)
