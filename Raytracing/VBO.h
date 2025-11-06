#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include<glm/glm.hpp>
#include<glad/glad.h>
#include<vector>

// Structure to standardize the vertices used in the meshes
struct Vertex {
	glm::vec3 position;
	float pad1 = 0;
	glm::vec3 normal;
	float pad2 = 0;
	glm::vec3 color;
	float pad3 = 0;
	glm::vec2 texUV;
	glm::vec2 pad4 = glm::vec2(0);

	Vertex(glm::vec3 _position, glm::vec3 _normal, glm::vec3 _color, glm::vec2 _texUV) : position(_position), normal(_normal), color(_color), texUV(_texUV) {};
};

class VBO {
public:
	// Reference ID of the Vertex Buffer Object
	GLuint ID;
	// Constructor that generates a Vertex Buffer Object and links it to vertices
	VBO(std::vector<Vertex>& vertices);

	// Binds the VBO
	void Bind();
	// Unbinds the VBO
	void Unbind();
	// Deletes the VBO
	void Delete();
};

#endif