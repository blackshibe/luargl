#include "freetype.h"

FT_Library ft;
FT_Face face;
rglShader* text_shader = { 0, 0 };

struct Character characters[256];
unsigned int VAO, VBO;
bool text_initialized = false;

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

void freetype_render(rglShader* shader, char* text, float x, float y, float scale, rglColor color) {

	// activate corresponding render L	
	glUseProgram(shader->id);
	glUniform3f(glGetUniformLocation(shader->id, "textColor"), color.r / 255, color.g / 255, color.b / 255);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// iterate through all characters
	for (int i = 0; i < strlen(text); i++) {
		{
			struct Character ch = characters[(int)text[i]];
			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;

			float xpos = x + ch.Bearing.x * scale;
			float ypos = 600 - y - (ch.Bearing.y * scale); // - y - ;

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
	}

	glUseProgram(0);
}

void freetype_init() {
	printf("[freetype.h] compiling text shaders\n");

	rglShaderCreate(&text_shader, load_file("playground/assets/text.vs"), load_file("playground/assets/text.fs"));

	if (FT_Init_FreeType(&ft)) {
		printf("[freetype.h] couldn't initialize\n");
		return;
	}

	if (FT_New_Face(ft, "font/telactile.ttf", 0, &face)) {
		printf("[freetype.h] couldn't load font\n");
		return;
	}
	else {
		// set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, 48);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// load first 256 characters of ASCII set
		for (unsigned char c = 0; c < 255; c++) {
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

	printf("[freetype.h] finished\n");
}

void freetype_update_init() {
	if (!text_initialized) {
		glUseProgram(&text_shader->id);

		rglMat4 viewport_mat;
		// rgl_width/height and rgl_vp_width/height both seem to use arbitrary values 
		// but sticking to the original window size is important here
		rglMat4Ortho(viewport_mat, 0, _rgl_app_desc->width, 0, _rgl_app_desc->height, -1, 1);
		glUniformMatrix4fv(glGetUniformLocation(&text_shader->id, "projection"), 1, false, viewport_mat);

		text_initialized = true;
	}
}