#include "Box.h"
#include "shaderClass.h"
#include "VAO.h"
#include "EBO.h"

Box::Box(glm::vec3 min, glm::vec3 max) {
	instances.push_back(this);

	vertices = {
		Vertex(glm::vec3(min.x, min.y, min.z), glm::vec3(0), glm::vec3(1, 1, 0), glm::vec2(0), glm::vec4(0)),
		Vertex(glm::vec3(max.x, min.y, min.z), glm::vec3(0), glm::vec3(1, 1, 0), glm::vec2(0), glm::vec4(0)),
		Vertex(glm::vec3(max.x, max.y, min.z), glm::vec3(0), glm::vec3(1, 1, 0), glm::vec2(0), glm::vec4(0)),
		Vertex(glm::vec3(min.x, max.y, min.z), glm::vec3(0), glm::vec3(1, 1, 0), glm::vec2(0), glm::vec4(0)),
		Vertex(glm::vec3(min.x, min.y, max.z), glm::vec3(0), glm::vec3(1, 1, 0), glm::vec2(0), glm::vec4(0)),
		Vertex(glm::vec3(max.x, min.y, max.z), glm::vec3(0), glm::vec3(1, 1, 0), glm::vec2(0), glm::vec4(0)),
		Vertex(glm::vec3(max.x, max.y, max.z), glm::vec3(0), glm::vec3(1, 1, 0), glm::vec2(0), glm::vec4(0)),
		Vertex(glm::vec3(min.x, max.y, max.z), glm::vec3(0), glm::vec3(1, 1, 0), glm::vec2(0), glm::vec4(0))
	};

	indices = std::vector<GLuint>{
		// Front edges
		0, 1,
		1, 2,
		2, 3,
		3, 0,

		// Back edges
		4, 5,
		5, 6,
		6, 7,
		7, 4,

		// Vertical edges
		0, 4,
		1, 5,
		2, 6,
		3, 7
	};

	VAO.Bind();
	VBO VBO(vertices);
	EBO EBO(indices);

	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(4 * sizeof(float)));
	VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(8 * sizeof(float)));
	VAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(12 * sizeof(float)));
	VAO.LinkAttrib(VBO, 4, 4, GL_FLOAT, sizeof(Vertex), (void*)(16 * sizeof(float)));

	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}

void Box::draw(Shader* shader) {
	shader->Activate();
	shader->setVec3("color", color);
	VAO.Bind();
	glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
}

void Box::setColor(glm::vec3 color) {
	this->color = color;
}