#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define NUMBER_OF_TEXTURES 1
#define LEVEL_OF_DETAIL    0
#define TEXTURE_BORDER     0
#define FONTBANK_SIZE      16

#include "Utility.h"
#include <SDL_image.h>
#include "stb_image.h"

/*
* Loads a texture to be used for each sprite
*
* @param filepath, an array of chars that represents the text name
  of the filepath of the texture for the sprite
*/
GLuint load_texture(const char* filepath)
{
	// Load image file from filepath
	int width, height, number_of_components;
	unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

	// Throw error if no image found at filepath
	if (image == NULL)
	{
		LOG(" Unable to load image. Make sure the path is correct.");
		assert(false);
	}

	// Generate and bind texture ID to image
	GLuint textureID;
	glGenTextures(NUMBER_OF_TEXTURES, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

	// Setting up texture filter parameters
	// NEAREST better for pixel art
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Release from memory and return texture id
	stbi_image_free(image);

	return textureID;
}

/*
* Draws all the game's text
*
* @param program, a reference to the shader program that the game is using
* @param font_texture_id, the image that contains our font
* @param text, the text to be rendered
* @param screen_size, size of the game's display
* @param spacing, spacing between characters
* @param position, position of the text
*/
void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
	// Scale the size of the fontbank in the UV-plane
	// We will use this for spacing and positioning
	float width = 1.0f / FONTBANK_SIZE;
	float height = 1.0f / FONTBANK_SIZE;

	// Instead of having a single pair of arrays, we'll have a series of pairs�one for each character
	// Don't forget to include <vector>!
	std::vector<float> vertices;
	std::vector<float> texture_coordinates;

	// For every character...
	for (int i = 0; i < text.size(); i++) {
		// 1. Get their index in the spritesheet, as well as their offset (i.e. their position
		//    relative to the whole sentence)
		int spritesheet_index = (int)text[i];  // ascii value of character
		float offset = (screen_size + spacing) * i;

		// 2. Using the spritesheet index, we can calculate our U- and V-coordinates
		float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
		float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

		// 3. Inset the current pair in both vectors
		vertices.insert(vertices.end(), {
			offset + (-0.5f * screen_size), 0.5f * screen_size,
			offset + (-0.5f * screen_size), -0.5f * screen_size,
			offset + (0.5f * screen_size), 0.5f * screen_size,
			offset + (0.5f * screen_size), -0.5f * screen_size,
			offset + (0.5f * screen_size), 0.5f * screen_size,
			offset + (-0.5f * screen_size), -0.5f * screen_size,
			});

		texture_coordinates.insert(texture_coordinates.end(), {
			u_coordinate, v_coordinate,
			u_coordinate, v_coordinate + height,
			u_coordinate + width, v_coordinate,
			u_coordinate + width, v_coordinate + height,
			u_coordinate + width, v_coordinate,
			u_coordinate, v_coordinate + height,
			});
	}

	// 4. And render all of them using the pairs
	glm::mat4 model_matrix = glm::mat4(1.0f);
	model_matrix = glm::translate(model_matrix, position);

	program->set_model_matrix(model_matrix);
	glUseProgram(program->get_program_id());

	glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
	glEnableVertexAttribArray(program->get_position_attribute());
	glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
	glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

	glBindTexture(GL_TEXTURE_2D, font_texture_id);
	glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

	glDisableVertexAttribArray(program->get_position_attribute());
	glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}