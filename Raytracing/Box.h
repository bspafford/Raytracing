#pragma once

#include<glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include "VAO.h"

class Shader;

class Box {
public:
	static inline std::vector<Box*> instances;

	Box(glm::vec3 min, glm::vec3 max);

	void draw(Shader* shader);

	void setColor(glm::vec3 color);

private:
	std::vector<GLuint> indices;
	std::vector<Vertex> vertices;

	glm::vec3 color = glm::vec3(1.f, 1.f, 0.f);

	VAO VAO;
};