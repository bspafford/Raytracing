#include "Renderer.h"
#include "camera.h"
#include "Scene.h"
#include "shaderClass.h"
#include "Text.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <ostream>
#include <iomanip>
#include <algorithm>

#include "debugger.h"

void Renderer::Start() {
	// disable window resizing during rendering
	renderering = true;
	renderStartTime = std::chrono::steady_clock::now();
	frameStartTime = std::chrono::steady_clock::now();

	if (!infoText)
		infoText = new Text();
}

void Renderer::NextFrame(GLuint texId, GLuint width, GLuint height, Camera* camera, Shader* textShader) {
	if (!renderering)
		return;

	// read back texture from GPU
	glBindTexture(GL_TEXTURE_2D, texId);
	std::vector<float> floats(width * height * 4);
	stbi_flip_vertically_on_write(true);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, floats.data());

	// convert GL_RGBA32F to 8 bit for png
	std::vector<unsigned char> bytes(width * height * 4);
	for (int i = 0; i < width * height * 4; i++) {
		float f = std::clamp(floats[i], 0.f, 1.f);
		bytes[i] = static_cast<unsigned char>(f * 255.f);
	}

	// add padding to frame num
	std::ostringstream oss;
	oss << std::setw(4) << std::setfill('0') << currentFrame;
	std::string filePathName = "Renders/Scene_" + std::to_string(Scene::getCurrSceneIndex()) + "-" + oss.str() + ".png";
	// write to .png file
	stbi_write_png(filePathName.c_str(), width, height, 4, bytes.data(), width * 4);

	UpdateCamera(camera);

	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	float frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - frameStartTime).count() / 1000.f;
	float timeFromStart = std::chrono::duration_cast<std::chrono::milliseconds>(now - renderStartTime).count() / 1000.f;
	float estimatedRemainingTime = timeFromStart / (currentFrame+1) * (totalFrames) - timeFromStart;

	std::string infoString = "Frame " + std::to_string(currentFrame) + " | Last: " + std::to_string(frameTime) + " | Time: " + std::to_string(timeFromStart) + " | Remaining : " + std::to_string(estimatedRemainingTime);
	infoText->draw(textShader, infoString);

	frameStartTime = now;

	// increase frame num
	currentFrame++;
	if (currentFrame >= totalFrames) {
		renderering = false;
		std::cout << "Rendering complete! Took " << timeFromStart << " seconds\n";
	}
}

void Renderer::UpdateCamera(Camera* camera) {
	float PI = 3.14159265359;
	float x = currentFrame / 30.f - (PI / 3.f);
	glm::vec3 lookAt = glm::vec3(0.f, 3.5f, 0.f);
	camera->Position = glm::vec3(sin(x) * 15.f, 3.5f, cos(x) * 15.f);
	camera->Orientation = glm::normalize(lookAt - camera->Position);
}