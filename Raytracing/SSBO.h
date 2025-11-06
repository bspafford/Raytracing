#pragma once

#include<glm/glm.hpp>
#include<glad/glad.h>
#include<vector>

class SSBO {
public:
	SSBO(const void* data, GLuint size);

private:
	static inline GLuint bindSlot = 0;

	GLuint ID;
};