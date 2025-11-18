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

void Renderer::Start(int width, int height) {
	// disable window resizing during rendering
	currentFrame = 0;
	renderering = true;
	renderStartTime = std::chrono::steady_clock::now();
	frameStartTime = std::chrono::steady_clock::now();

	if (!infoText)
		infoText = new Text();

	// setup ffmpeg
	const char* outputPath = "Renders/output.mp4";
	if (!formatContext) {
		avformat_alloc_output_context2(&formatContext, nullptr, nullptr, outputPath);
		if (!formatContext)
			throw std::runtime_error("Could not allocate output format context");
	}
	const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codecContext) {
		if (!codec)
			throw std::runtime_error("H.264 encoder not found");

		codecContext = avcodec_alloc_context3(codec);
		if (!codecContext)
			throw std::runtime_error("Could not allocate codec context");

		codecContext->codec_id = codec->id;
		codecContext->bit_rate = 4000000;
		codecContext->width = width;
		codecContext->height = height;
		codecContext->time_base = AVRational{ 1, fps };
		codecContext->framerate = AVRational{ fps, 1 };
		codecContext->gop_size = 12;
		codecContext->max_b_frames = 2;
		codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	}
	if (!videoStream) {
		videoStream = avformat_new_stream(formatContext, nullptr);
		avcodec_parameters_from_context(videoStream->codecpar, codecContext);
		videoStream->time_base = codecContext->time_base;
	}

	avio_open(&formatContext->pb, outputPath, AVIO_FLAG_WRITE);
	avcodec_open2(codecContext, codec, nullptr);
	avformat_write_header(formatContext, nullptr);

	swsCtx = sws_getContext(
		width, height, AV_PIX_FMT_RGBA,
		width, height, AV_PIX_FMT_YUV420P,
		SWS_BILINEAR, nullptr, nullptr, nullptr
	);

	if (!rgbFrame) {
		rgbFrame = av_frame_alloc();
		rgbFrame->format = AV_PIX_FMT_RGBA;
		rgbFrame->width = width;
		rgbFrame->height = height;
		av_frame_get_buffer(rgbFrame, 0);
	}

	if (!yuvFrame) {
		yuvFrame = av_frame_alloc();
		yuvFrame->format = AV_PIX_FMT_YUV420P;
		yuvFrame->width = width;
		yuvFrame->height = height;
		yuvFrame->pict_type = AV_PICTURE_TYPE_I;
		av_frame_get_buffer(yuvFrame, 32);
	}

	pkt = av_packet_alloc();
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

	// PNGs
	/*
	// add padding to frame num
	std::ostringstream oss;
	oss << std::setw(4) << std::setfill('0') << currentFrame;
	std::string filePathName = "Renders/Scene_" + std::to_string(Scene::getCurrSceneIndex()) + "-" + oss.str() + ".png";
	// write to .png file
	stbi_write_png(filePathName.c_str(), width, height, 4, bytes.data(), width * 4);
	*/

	// mp4
	// copy OpenGL pixels to frame->data[0]
	//memcpy(rgbFrame->data[0], bytes.data(), width * height * 4);
	for (int y = 0; y < height; y++) {
		memcpy(
			rgbFrame->data[0] + y * rgbFrame->linesize[0],
			bytes.data() + y * width * 4,
			width * 4
		);
	}

	int64_t pts = (currentFrame) * videoStream->time_base.den / (videoStream->time_base.num * fps);
	std::cout << "pts: " << pts << " = " << currentFrame << " * " << videoStream->time_base.den << " / (" << videoStream->time_base.num << " * " << fps << ")\n";
	rgbFrame->pts = pts;
	yuvFrame->pts = pts;

	// convert RGBA -> yuv420p
	sws_scale(
		swsCtx,
		rgbFrame->data, rgbFrame->linesize,
		0, height,
		yuvFrame->data, yuvFrame->linesize
	);
	// send it to the encoder
	if (avcodec_send_frame(codecContext, yuvFrame) < 0)
		std::runtime_error("Error sending frame to encoder");

	while (avcodec_receive_packet(codecContext, pkt) == 0) {
		pkt->stream_index = videoStream->index;
		std::cout << "writting frame: " << pkt->pts << "\n";
		av_interleaved_write_frame(formatContext, pkt);
		av_packet_unref(pkt);
	}

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
		std::cout << "Rendering complete! Took " << timeFromStart << " seconds\n";
		Finished();
	}
}

void Renderer::Finished() {
	renderering = false;

	avcodec_send_frame(codecContext, nullptr); // flush
	while (avcodec_receive_packet(codecContext, pkt) == 0) {
		pkt->stream_index = videoStream->index;
		std::cout << "flushing | writting frame: " << pkt->pts << "\n";
		av_interleaved_write_frame(formatContext, pkt);
		av_packet_unref(pkt);
	}

	av_write_trailer(formatContext);
	avio_close(formatContext->pb);
}

void Renderer::UpdateCamera(Camera* camera) {
	float PI = 3.14159265359;
	float x = currentFrame / 30.f - (PI / 3.f);
	glm::vec3 lookAt = glm::vec3(0.f, 3.5f, 0.f);
	camera->Position = glm::vec3(sin(x) * 15.f, 3.5f, cos(x) * 15.f);
	camera->Orientation = glm::normalize(lookAt - camera->Position);
}