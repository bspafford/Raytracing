#pragma once

#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>
#include <glm/glm.hpp>

class ComputeShader {
public:
	ComputeShader(const char* filePath);

	GLuint ID;

	// Activates the Shader Program
	void Activate();
	// Deletes the Shader Program
	void Delete();

	void setMat4(std::string key, glm::mat4 value);
	void setVec4(std::string key, glm::vec4 value);
	void setVec3(std::string key, glm::vec3 value);
	void setVec2(std::string key, glm::vec2 value);
	void setInt(std::string key, int value);
	void setFloat(std::string key, float value);
private:
	// Checks if the different Shaders have compiled properly
	void compileErrors(unsigned int shader, const char* type);
};