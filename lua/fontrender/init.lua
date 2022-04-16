local flags = require("rgl.flags")

rgl.window_properties = {
	title = "fontrender",
	width = 800,
	height = 400,
}

local font
local cooler_font
local text = "The quick brown fox"

function rgl_app_init()
	print("init")

	font = rgl.load_font_from_file("rgl/font/arial.ttf")
	cooler_font = rgl.load_font_from_file("rgl/font/telactile.ttf")
end

function rgl_app_update(delta_time) end

function rgl_app_draw()
	rgl.debug.disable_text_sampling = false

	rgl.render_text(font, text, 0, 0, 0.5, { 255, 255, 255 })
	rgl.render_text(font, text, 0, BASE_FONT_HEIGHT * 1.5, 0.5, { 255, 255, 255 })
	rgl.render_text(cooler_font, text, 0, BASE_FONT_HEIGHT * 2, 0.2, { 255, 255, 255 })
	rgl.render_text(cooler_font, text, 0, BASE_FONT_HEIGHT * 2.5, 0.2, { 255, 0, 0 })

	rgl.debug.disable_text_sampling = true
	rgl.render_text(font, text, 0, BASE_FONT_HEIGHT * 0.5, 0.5, { 255, 255, 255 })
end

function rgl_app_quit()
	print("quit")
end
