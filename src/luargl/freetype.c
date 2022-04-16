#include "freetype.h"

FT_Library ft;
rglShader* freetype_text_shader = { 0 };
rglShader* freetype_text_shader_nosampling = { 0 };

unsigned int VAO, VBO;
bool text_initialized = false;

const char* load_file(char* filename) {
	char* buffer = NULL;
	long length;
	FILE* file = fopen(filename, "r");

	if (file) {
		fseek(file, 0, SEEK_END);
		length = ftell(file) + 1;
		fseek(file, 0, SEEK_SET);
		buffer = malloc(length);
		if (buffer) fread(buffer, 1, length - 1, file);
		fclose(file);

		buffer[length - 1] = '\0';

	}

	return buffer;
}

void freetype_render(struct Character* characters, rglShader* shader, const char* text, float x, float y, float scale, rglColor color) {

	// activate corresponding render L	
	glUseProgram(shader->id);
	glUniform3f(glGetUniformLocation(shader->id, "textColor"), color.r / 255, color.g / 255, color.b / 255);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// iterate through all characters
	for (int i = 0; i < strlen(text); i++) {
		struct Character ch = characters[(int)text[i]];
		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		float xpos = x + (ch.Bearing.x * scale);
		float ypos = (_rgl_app_desc->height - y) - (FREETYPE_BASE_FONT_HEIGHT * scale) - ((ch.Size.y - ch.Bearing.y) * scale); // (ch.Bearing.y * scale); // - ((24 - ch.Bearing.y) * scale);

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

		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}

	glUseProgram(0);
}

void freetype_init(lua_State* L) {
	printf("[freetype.h] compiling text shader\n");

	lua_pushnumber(L, FREETYPE_BASE_FONT_HEIGHT);
	lua_setglobal(L, "BASE_FONT_HEIGHT");

	const char* text_vertex_shader = load_file("rgl/shader/text.vs");
	const char* text_fragment_shader = load_file("rgl/shader/text.fs");
	const char* text_fragment_nosampling_shader = load_file("rgl/shader/text_nosampling.fs");

	if (text_vertex_shader == NULL || text_fragment_shader == NULL || text_fragment_nosampling_shader == NULL) {
		printf("%s[freetype.h] couldn't find one of the text shaders%s\n", ANSI_RED, ANSI_NC);
		return;
	}

	rglShaderCreate(&freetype_text_shader, text_vertex_shader, text_fragment_shader);
	rglShaderCreate(&freetype_text_shader_nosampling, text_vertex_shader, text_fragment_nosampling_shader);

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

	printf("[freetype.h] finished\n");
}

bool freetype_load_font(struct Character* characters, char* font_directory) {

	printf("[freetype.h] compiling text shaders\n");

	if (FT_Init_FreeType(&ft)) {
		printf("[freetype.h] couldn't initialize\n");
		return false;
	}

	FT_Face face;
	if (FT_New_Face(ft, font_directory, 0, &face)) {
		printf("[freetype.h] couldn't load font\n");
		return false;
	}
	else {
		// set size to load glyphs as
		FT_Set_Pixel_Sizes(face, FREETYPE_BASE_FONT_HEIGHT, FREETYPE_BASE_FONT_HEIGHT);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// load first 256 characters of ASCII set
		for (unsigned char c = 0; c < 255; c++) {

			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;

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
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// destroy FreeType once we're finished
	FT_Done_Face(face);
	return true;
}

void freetype_update_init() {
	if (!text_initialized) {
		// rgl_width/height and rgl_vp_width/height both seem to use arbitrary values 
		// but sticking to the original window size is important here
		rglMat4 viewport_mat;
		rglMat4Ortho(viewport_mat, 0, _rgl_app_desc->width, 0, _rgl_app_desc->height, -1, 1);

		glUseProgram(&freetype_text_shader->id);
		glUniformMatrix4fv(glGetUniformLocation(&freetype_text_shader->id, "projection"), 1, false, viewport_mat);

		glUseProgram(&freetype_text_shader_nosampling->id);
		glUniformMatrix4fv(glGetUniformLocation(&freetype_text_shader_nosampling->id, "projection"), 1, false, viewport_mat);

		text_initialized = true;
	}
}

void freetype_quit() {
	FT_Done_FreeType(ft);
}

int luargl_load_font_from_file(lua_State* L) {

	const char* path = lua_tostring(L, 1);
	struct Character* character = lua_newuserdata(L, sizeof(struct Character[256]));
	if (freetype_load_font(character, path)) printf("[freetype.c] loaded font\n");
	else luax_push_error("couldn't load font");

	return 1;
}

int luargl_render_text(lua_State* L) {

	struct Character* font = lua_touserdata(L, 1);
	const char* text = lua_tostring(L, 2);
	float x = lua_tonumber(L, 3);
	float y = lua_tonumber(L, 4);
	float scale = lua_tonumber(L, 5);

	lua_rawgeti(L, 6, 1);
	lua_rawgeti(L, 6, 2);
	lua_rawgeti(L, 6, 3);

	luargl_get_flag("disable_text_sampling");
	if (lua_toboolean(L, -1) == 1)
		freetype_render(font, &freetype_text_shader_nosampling, text, x, y, scale, RGL_RGB(lua_tonumber(L, 7), lua_tonumber(L, 8), lua_tonumber(L, 9)));
	else
		freetype_render(font, &freetype_text_shader, text, x, y, scale, RGL_RGB(lua_tonumber(L, 7), lua_tonumber(L, 8), lua_tonumber(L, 9)));

	return 0;
}
