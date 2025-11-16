#pragma once

#include "shaderClass.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <glad/glad.h>

#include "stb_truetype.h"

class Text {
public:
	Text();

	void draw(Shader* shader, std::string text);

private:
	GLuint fontTex;
	GLuint VAO, VBO;

	const int atlasWidth = 1024;
	const int atlasHeight = 1024;
	stbtt_bakedchar cdata[96]; // ASCII 32..126
};