#include <pthread.h>
#include <rgl/rgl.h>
#include <rgl/rgl_shader.h>
#include <sys/time.h>
#include <math.h>

// move /usr/include/freetype2/* to /usr/lib/ to get it to work
#include <ft2build.h>
#include FT_FREETYPE_H

#include <lua.h>
#include "lua5.3/lualib.h"
#include "lua5.3/lauxlib.h"

#include "../colors.h"

#include "sprite.h"
#include "vector2.h"
#include "camera.h"


lua_State* state = NULL;
int library_reference = -1;
bool started = false;
struct Character {
	unsigned int TextureID; // ID handle of the glyph texture
	rglV2   Size;      // Size of glyph
	rglV2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
};

struct Character characters[256];

typedef int(access_function)(lua_State* L, int idx);

int64_t now() {
	struct timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec;
}

void luax_push_error(const char* error) {
	lua_pushstring(state, error);
	lua_error(state);
}

void luax_call_global_if_exists(const char* global, int args) {
	lua_getglobal(state, global);
	if (lua_isfunction(state, -1)) {
		int run_result = lua_pcall(state, args, 0, 0);
		if (run_result != LUA_OK) {
			// todo: stack traceback
			printf("%s[Lua] Runtime error: \"%s\"\n%s", ANSI_BOLD_RED, lua_tostring(state, -1), ANSI_NC);
		}
	}
}

int luax_access_field(int tbl_stack_pos, char* strindex, access_function func) {
	int stack_pos = lua_getfield(state, tbl_stack_pos, strindex);
	if (func(state, stack_pos)) {
		return stack_pos;
	}

	return -1;
}

// rgl defs
FT_Library ft;
FT_Face face;
unsigned int VAO, VBO;

void RenderText(rglShader* shader, char* text, float x, float y, float scale, rglColor color) {

	// activate corresponding render state	
	glUseProgram(shader->id);
	glUniform3f(glGetUniformLocation(shader->id, "textColor"), color.b / 255, color.g / 255, color.b / 255);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// printf("len %i '%s'\n", strlen(text), text);

	// iterate through all characters
	for (int i = 0; i < strlen(text); i++) {
		{
			struct Character ch = characters[(int)text[i]];
			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;
			// update VBO for each character
			float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};
			// render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			// update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			printf("%c, %f, %f\n", text[i], xpos, ypos);


			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		}
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	GLenum code;

	const GLubyte* string;
	code = glGetError();
	// fprintf(stderr, "OpenGL error: %i\n", code);
}

const char* load_file(char* filename) {
	char* buffer = 0;
	long length;
	FILE* file = fopen(filename, "r");

	if (file) {
		fseek(file, 0, SEEK_END);
		length = ftell(file) + 1;
		fseek(file, 0, SEEK_SET);
		buffer = malloc(length);

		if (buffer) {
			fread(buffer, 1, length - 1, file);
		}
		fclose(file);
	}

	buffer[length - 1] = '\0';
	return buffer;
}

static rglShader* text_shader = { 0, 0 };
void rgl_app_init(void) {
	luax_call_global_if_exists("rgl_app_init", 0);

	printf("%s'%s'%s\n", ANSI_RED, load_file("scripts/rgl/text.vs"), ANSI_NC);
	rglShaderCreate(&text_shader, load_file("scripts/rgl/text.vs"), load_file("scripts/rgl/text.fs"));

	if (FT_Init_FreeType(&ft))
	{
		printf("freetype bromk\n");
		return -1;
	}

	if (FT_New_Face(ft, "scripts/rgl/arial.ttf", 0, &face))
	{
		printf("font bromk\n");
		return -1;
	}
	else {
		// set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, 48);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// load first 128 characters of ASCII set
		for (unsigned char c = 0; c < 128; c++)

		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				continue;
			}
			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// now store character for later use
			struct Character character = {
				texture,
				(rglV2) {
					face->glyph->bitmap.width, face->glyph->bitmap.rows
				},
				(rglV2) {
					face->glyph->bitmap_left, face->glyph->bitmap_top
				},
				(unsigned int)(face->glyph->advance.x)
			};

			characters[(int)c] = character;
			// Characters.insert(std::pair<char, Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	// destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);


	// configure VAO/VBO for texture quads
	// -----------------------------------
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	printf("compiled\n");
}

void rgl_app_update(f32 dt) {
	lua_getglobal(state, "rgl_app_update");
	if (lua_isfunction(state, -1)) {
		lua_pushnumber(state, dt);
		int run_result = lua_pcall(state, 1, 0, 0);
		if (run_result != LUA_OK) {
			// todo: stack traceback
			printf("%s[Lua] Runtime error: \"%s\"\n%s", ANSI_BOLD_RED, lua_tostring(state, -1), ANSI_NC);
		}
	}
}

void rgl_app_draw(void) {
	luax_call_global_if_exists("rgl_app_draw", 0);
	RenderText(&text_shader, "hello world", -1000 + 10, 900 + 10, 1, RGL_GREEN);

	glUseProgram(text_shader->id);
	glUniformMatrix4fv(glGetUniformLocation(text_shader->id, "projection"), 1, GL_FALSE, (float*)_rgl_camera->projection);

}

void rgl_app_quit(void) {
	luax_call_global_if_exists("rgl_app_quit", 0);
}


// lua lib defs

int luargl_make_window(lua_State* state) {
	if (started) {
		luax_push_error("cannot call make_window when one was already created!");
		return 0;
	}

	int width = 1280;
	int height = 720;
	const char* name = "Window";

	// read window properties or apply defaults
	// printf("%b", lua_isstring(state, library_reference));
	lua_rawgeti(state, LUA_REGISTRYINDEX, library_reference);
	lua_getfield(state, -1, "window_properties");

	if (lua_istable(state, -1)) {
		int width_pos = luax_access_field(-1, "width", lua_isinteger);
		if (width_pos != -1) {
			width = lua_tointeger(state, width_pos);
			lua_pop(state, 1);
		}

		int height_pos = luax_access_field(-1, "height", lua_isinteger);
		if (height_pos != -1) {
			height = lua_tointeger(state, height_pos);
			lua_pop(state, 1);
		}

		// luax access field did not work lol
		lua_getfield(state, -1, "title");
		if (lua_isstring(state, -1)) {
			name = luaL_tolstring(state, -1, NULL);
		}
	}

	printf("new window with size %i, %i\n", width, height);

	rglAppDesc desc = (rglAppDesc){
		.title = name,
		.width = width,
		.height = height,
		.background_color = RGL_RGB(40, 40, 40),
		.init_f = rgl_app_init,
		.update_f = rgl_app_update,
		.draw_f = rgl_app_draw,
		.quit_f = rgl_app_quit,
	};

	rglStart(&desc);

	return 0;
}

int luargl_draw_circle(lua_State* state) {
	// printf("draw circle\n");

	// color: float, position: float[], size: int
	int size = lua_tointeger(state, 3);

	lua_rawgeti(state, 1, 1);
	lua_rawgeti(state, 1, 2);
	lua_rawgeti(state, 1, 3);
	float r = lua_tonumber(state, -3);
	float g = lua_tonumber(state, -2);
	float b = lua_tonumber(state, -1);

	// printf("R: %f G: %f B: %f\n", r, g, b);

	lua_rawgeti(state, 2, 1);
	lua_rawgeti(state, 2, 2);
	float x = lua_tonumber(state, -2);
	float y = lua_tonumber(state, -1);

	// printf("X: %f Y: %f\n", x, y);
	// printf("Radius: %i\n", size);

	rglV2 position = { x, y };

	rglDrawCircle(RGL_RGB(r, g, b), position, size);
	lua_settop(state, 0);
	return 0;
}

int luargl_is_key_just_pressed(lua_State* state) {
	int key = (rglKey)lua_tointeger(state, -1);
	lua_pushboolean(state, rglIsKeyJustPressed(key));

	return 1;
}

int luargl_is_key_pressed(lua_State* state) {
	int key = (rglKey)lua_tointeger(state, -1);
	lua_pushboolean(state, rglIsKeyPressed(key));

	return 1;
}

int luargl_load_image_from_file(lua_State* state) {

	const char* path = lua_tostring(state, 1);
	rglTexture* texture = lua_newuserdata(state, sizeof(rglTexture));
	rglTextureLoadFromFile(texture, path, RGL_TEXTURE_FILTER_NONE);

	return 1;
}

int luargl_create_sprite(lua_State* state) {

	if (!_rgl_running) {
		luax_push_error("cannot load assets when rgl hasn't started");
		return 1;
	}

	rglTexture* texture = lua_touserdata(state, 1);

	// creates a table: { __sprite = userdata } with a Sprite metatable
	lua_newtable(state);
	rglSprite* sprite = lua_newuserdata(state, sizeof(rglSprite));
	lua_setfield(state, -2, "__sprite");
	luaL_getmetatable(state, "Sprite");
	lua_setmetatable(state, -2);

	rglSpriteCreate(sprite, texture);

	return 1;
}

int luargl_draw_sprite(lua_State* state) {
	lua_getfield(state, -1, "__sprite");
	rglSpriteRender(lua_touserdata(state, -1));

	return 0;
}

int luargl_destroy_sprite(lua_State* state) {
	lua_getfield(state, -1, "__sprite");
	rglSpriteDestroy(lua_touserdata(state, -1));

	return 0;
}

int luargl_destroy_image(lua_State* state) {
	rglTextureDestroy(lua_touserdata(state, 1));

	return 0;
}

int luargl_vector2(lua_State* state) {

	lua_Number x = lua_tonumber(state, 1);
	lua_Number y = lua_tonumber(state, 2);
	luargl_create_vector2(state, x, y);

	return 1;
}

int luargl_get_mouse_position(lua_State* state) {
	rglV2 pos;
	rglGetCursorPos(&pos);
	luargl_create_vector2(state, pos.x, pos.y);
	return 1;
}

int luargl_get_mouse_position_in_world_space(lua_State* state) {
	rglV2 pos;
	rglGetCursorPosInWorld(&pos);
	luargl_create_vector2(state, pos.x, pos.y);
	return 1;
}

int luargl_meta_index(lua_State* state) {
	return 0;
}

int luargl_meta_newindex(lua_State* state) {
	return 0;
}

static const luaL_Reg lib[] = {
	{"make_window", luargl_make_window},
	{"draw_circle", luargl_draw_circle},
	{"is_key_just_pressed", luargl_is_key_just_pressed},
	{"is_key_pressed", luargl_is_key_pressed},

	{"load_image_from_file", luargl_load_image_from_file},
	{"destroy_image", luargl_destroy_image},

	{"create_sprite", luargl_create_sprite},
	{"draw_sprite", luargl_draw_sprite},
	{"destroy_sprite", luargl_destroy_sprite},

	{"get_mouse_position", luargl_get_mouse_position },
	{"get_mouse_position_in_world_space", luargl_get_mouse_position_in_world_space },

	{"vector2", luargl_vector2 },

	{NULL, NULL},
};

static const luaL_Reg libmeta[] = {
	{"__index", luargl_meta_index},
	// {"__newindex", luargl_meta_newindex},
	{NULL, NULL},
};

LUALIB_API int luaopen_luargl(lua_State* L) {
	state = L;

	luaL_newmetatable(state, "Camera");
	luaL_setfuncs(state, luargl_camera_methods, 0);

	luaL_newmetatable(state, "Sprite");
	luaL_setfuncs(state, luargl_sprite_methods, 0);

	luaL_newmetatable(state, "Vector2");
	lua_pushcclosure(state, luargl_vector2_print, 0);
	lua_setfield(state, -2, "__tostring");

	lua_newtable(state);
	luaL_setfuncs(state, luargl_vector2_methods, 0);
	lua_setfield(state, -2, "__index");

	// create a table, put it inside the registry, register library functions inside it
	lua_newtable(L);
	library_reference = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_rawgeti(L, LUA_REGISTRYINDEX, library_reference);
	luaL_setfuncs(L, lib, 0);
	init_camera(L);

	// assigns a metatable to the luargl variable
	lua_newtable(L);
	luaL_setfuncs(L, libmeta, 0);
	lua_setmetatable(L, -2);

	return 1;
}
