#pragma once

#include <string>

#include "computeShader.h"

class Scene {
public:
	static void LoadScene(ComputeShader* computeShader, int sceneIndex);

private:
	static void UnloadScene();
	static void LoadGPUData(ComputeShader* computeShader);

	static inline int currSceneIndex;
};