#pragma once

#include <glad/glad.h>

class Camera;

class Renderer {
public:
	static void Start();
	static void NextFrame(GLuint texId, GLuint width, GLuint height, Camera* camera);

	static bool isRenderering() { return renderering; };

private:
	static void UpdateCamera(Camera* camera);

	static inline bool renderering = false;
	static inline float fps = 60.f;
	static inline int totalFrames = 60;
	static inline int currentFrame;
};