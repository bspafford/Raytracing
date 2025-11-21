#pragma once

#include <glad/glad.h>
#include <chrono>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include "Texture.h"

class Camera;
class Shader;
class Text;

class Renderer {
public:
	static void Start(int width, int height);
	static void NextFrame(Texture tex, GLuint width, GLuint height, Camera* camera, Shader* textShader);

	static bool isRenderering() { return renderering; };

private:
	static void UpdateCamera(Camera* camera);
	static void Finished();

	static inline bool renderering = false;
	static inline int fps = 60;
	static inline int totalFrames = 60;
	static inline int currentFrame;

	// rendering stats
	static inline std::chrono::steady_clock::time_point renderStartTime;
	static inline std::chrono::steady_clock::time_point frameStartTime;

	static inline Text* infoText;

	static inline AVCodecContext* codecContext;
	static inline AVFormatContext* formatContext;
	static inline AVStream* videoStream = nullptr;
	static inline SwsContext* swsCtx = nullptr;
	static inline AVFrame* yuvFrame;
	static inline AVFrame* rgbFrame;
	static inline AVPacket* pkt;
};