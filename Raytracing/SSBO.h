#pragma once

#include<glm/glm.hpp>
#include<glad/glad.h>
#include<vector>

class SSBO {
public:
	static void Bind(const void* data, GLuint size, GLuint slot);
};