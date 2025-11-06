#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include <chrono>

#include "vector.h"

class Camera;
class Model;
class Shader;
class ComputeShader;

class Main {
public:
	int createWindow();

private:
	void Start();
	void Update(float deltaTime);
	void Draw(Shader* shader);
	
	GLFWwindow* window;

	Camera* camera;
	Model* cube;

	Shader* shader;
	Shader* quadShader;
	ComputeShader* computeShader;

	GLuint quadVAO = 0;
	GLuint quadVBO;
	void setupQuad();
	void renderQuad();
};