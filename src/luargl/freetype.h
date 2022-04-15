#include "other/includes.h"

// move /usr/include/freetype2/* to /usr/lib/ to get it to work
#include <ft2build.h>
#include FT_FREETYPE_H
#include <rgl/rgl_shader.h>

extern rglShader* text_shader;
struct Character {
	unsigned int TextureID; // ID handle of the glyph texture
	rglV2   Size;      // Size of glyph
	rglV2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
};

void freetype_render(rglShader* shader, char* text, float x, float y, float scale, rglColor color);
void freetype_init();
void freetype_update_init();