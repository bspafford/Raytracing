#include "computeShader.h"
#include "shaderClass.h"
#include <glm/gtc/type_ptr.hpp>

ComputeShader::ComputeShader(const char* filePath) {
	std::string computeCode = get_file_contents(filePath);

	const char* computeSource = computeCode.c_str();

	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &computeSource, NULL);
	glCompileShader(computeShader);
	compileErrors(computeShader, "COMPUTE");

	ID = glCreateProgram();
	glAttachShader(ID, computeShader);
	glLinkProgram(ID);
	compileErrors(ID, "PROGRAM");

	glDeleteShader(computeShader);
}

void ComputeShader::Activate() {
	glUseProgram(ID);
}

void ComputeShader::Delete() {
	glDeleteProgram(ID);
}

void ComputeShader::compileErrors(unsigned int shader, const char* type) {
	// Stores status of compilation
	GLint hasCompiled;
	// Character array to store error message in
	char infoLog[1024];
	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	} else {
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
}

void ComputeShader::setMat4(std::string key, glm::mat4 value) {
	glUniformMatrix4fv(glGetUniformLocation(ID, key.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void ComputeShader::setVec4(std::string key, glm::vec4 value) {
	glUniform4f(glGetUniformLocation(ID, key.c_str()), value.x, value.y, value.z, value.w);
}

void ComputeShader::setVec3(std::string key, glm::vec3 value) {
	glUniform3f(glGetUniformLocation(ID, key.c_str()), value.x, value.y, value.z);
}

void ComputeShader::setVec2(std::string key, glm::vec2 value) {
	glUniform2f(glGetUniformLocation(ID, key.c_str()), value.x, value.y);
}

void ComputeShader::setInt(std::string key, int value) {
	glUniform1i(glGetUniformLocation(ID, key.c_str()), value);
}

void ComputeShader::setFloat(std::string key, float value) {
	glUniform1f(glGetUniformLocation(ID, key.c_str()), value);
}