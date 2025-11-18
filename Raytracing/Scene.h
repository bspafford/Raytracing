#pragma once

#include <string>

#include "computeShader.h"

class Scene {
public:
	static void LoadScene(ComputeShader* computeShader, int sceneIndex);

	static int getCurrSceneIndex() { return currSceneIndex; };

	static void UnloadScene();

private:
	static void LoadGPUData(ComputeShader* computeShader);

	static inline int currSceneIndex;
};