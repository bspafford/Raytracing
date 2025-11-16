#include "Renderer.h"
#include "camera.h"
#include "Scene.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <ostream>
#include <iomanip>
#include <algorithm>

void Renderer::Start() {
	renderering = true;
	// needs to know
	// fps
	// total frames
	
	// start camera track / animations that i want to play
	
	// start the rendering

}

void Renderer::NextFrame(GLuint texId, GLuint width, GLuint height, Camera* camera) {
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

	// increase frame num
	currentFrame++;
	if (currentFrame >= totalFrames) {
		renderering = false;
		std::cout << "Rendering complete!\n";
	}
}

void Renderer::UpdateCamera(Camera* camera) {
	float PI = 3.14159265359;
	float x = currentFrame / 30.f - (PI / 3.f);
	glm::vec3 lookAt = glm::vec3(0.f, 3.5f, 0.f);
	camera->Position = glm::vec3(sin(x) * 15.f, 3.5f, cos(x) * 15.f);
	camera->Orientation = glm::normalize(lookAt - camera->Position);
}