#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include<glm/glm.hpp>
#include<glad/glad.h>
#include<vector>

// Structure to standardize the vertices used in the meshes
struct Vertex {
	glm::vec3 position;	float pad1;
	glm::vec3 normal;	float pad2;
	glm::vec3 color;	float pad3;
	glm::vec2 texUV;	glm::vec2 pad4;
	glm::vec4 tangent;

	Vertex(glm::vec3 _position, glm::vec3 _normal, glm::vec3 _color, glm::vec2 _texUV, glm::vec4 _tangent) : position(_position), normal(_normal), color(_color), texUV(_texUV), tangent(_tangent) {};
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