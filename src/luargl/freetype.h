#include "other/includes.h"

// move /usr/include/freetype2/* to /usr/lib/ to get it to work
#include <ft2build.h>
#include FT_FREETYPE_H
#include <rgl/rgl_shader.h>

#define FREETYPE_BASE_FONT_HEIGHT 96

extern rglShader* freetype_text_shader;
extern rglShader* freetype_text_shader_nosampling;
struct Character {
	unsigned int TextureID; // ID handle of the glyph texture
	rglV2   Size;           // Size of glyph
	rglV2   Bearing;        // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Horizontal offset to advance to next glyph
};

void freetype_render(struct Character* characters, rglShader* shader, const char* text, float x, float y, float scale, rglColor color);
bool freetype_load_font(struct Character* characters, char* font_directory);
void freetype_init(lua_State* L);
void freetype_update_init();
void freetype_quit();

int luargl_load_font_from_file(lua_State* L);
int luargl_render_text(lua_State* L);