#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include <chrono>

class Camera;
class Model;
class Shader;
class ComputeShader;
class Box;

class Main {
public:
	int createWindow();

private:
	void Start();
	void Update(float deltaTime);
	void Draw(Shader* shader);

	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
	
	GLFWwindow* window;
	static inline glm::vec2 screenSize = glm::vec2(800, 600);

	static inline unsigned int quadTexture;

	Camera* camera;
	Model* cube;
	Model* sphere;
	Model* sphereFlat;

	Shader* shader;
	Shader* quadShader;
	ComputeShader* computeShader;

	GLuint quadVAO = 0;
	GLuint quadVBO;
	void setupQuad();
	void renderQuad();
};