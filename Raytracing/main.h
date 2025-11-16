#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera;
class Light;
class Model;
class Shader;
class ComputeShader;
class Box;

class Main {
public:
	int createWindow();

	static glm::vec2 getScreenSize() { return screenSize; }

private:
	void Start();
	void Update(float deltaTime);
	void DrawBoxes(Shader* shader);
	void DrawModels(Shader* shader);
	void setShaderUniforms(Shader* shader);

	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	
	GLFWwindow* window;
	static inline glm::vec2 screenSize = glm::vec2(800, 600);

	static inline unsigned int quadTexture;

	Camera* camera;
	Light* light;
	Model* cube;
	Model* sphere;
	Model* sphereFlat;

	Shader* shader;
	Shader* quadShader;
	Shader* textShader;
	static inline ComputeShader* computeShader;

	GLuint quadVAO = 0;
	GLuint quadVBO;
	void setupQuad();
	void renderQuad();

	static inline bool rayTraceEnabled = true;
};