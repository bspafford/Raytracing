#pragma once

#include <glad/glad.h>
#include <chrono>

class Camera;
class Shader;
class Text;

class Renderer {
public:
	static void Start();
	static void NextFrame(GLuint texId, GLuint width, GLuint height, Camera* camera, Shader* textShader);

	static bool isRenderering() { return renderering; };

private:
	static void UpdateCamera(Camera* camera);

	static inline bool renderering = false;
	static inline float fps = 60.f;
	static inline int totalFrames = 60;
	static inline int currentFrame;

	// rendering stats
	static inline std::chrono::steady_clock::time_point renderStartTime;
	static inline std::chrono::steady_clock::time_point frameStartTime;

	static inline Text* infoText;
};