#include "Scene.h"
#include "light.h"
#include "model.h"
#include "SSBO.h"
#include "Box.h"

void Scene::UnloadScene() {
	Model::DeleteAll();
	Light::DeleteAll();
	Box::DeleteAll();
}

void Scene::LoadScene(ComputeShader* computeShader, int sceneIndex) {
	if (currSceneIndex == sceneIndex)
		return;

	UnloadScene();

	switch(sceneIndex) {
	case 1:
		new Light(glm::vec3(0, 5, 0), glm::vec3(0), LightType::Point, 50.f);
		new Model("models/sphere/sphere.gltf");
		new Model("models/cube/cube.gltf");
		new Model("models/sphere1/sphere.gltf");
		new Model("models/glassSphere/sphere.gltf");
		new Model("models/walls/walls.gltf");
		new Model("models/roughSphere/roughSphere.gltf");
		break;
	case 2:
		new Light(glm::vec3(0, 6.75254, 0), glm::vec3(0), LightType::Point, 50.f);
		//new Light(glm::vec3(0), glm::vec3(0, -1, -1), LightType::Sun, 50.f);
		new Model("models/scene2/scene2.gltf");
		break;
	}

	LoadGPUData(computeShader);
}

void Scene::LoadGPUData(ComputeShader* computeShader) {
	computeShader->Activate();
	std::vector<GPUBoundingBox> BVHList = Model::BVH();

	// assign Lights to SSBO
	bool hasSun;
	std::vector<GPULight> lights = Light::ToGPU(hasSun);
	SSBO::Bind(lights.data(), lights.size() * sizeof(GPULight), 5);
	computeShader->setInt("numLights", lights.size());
	computeShader->setInt("hasSun", hasSun);

	// build box list for visual
	for (int i = 0; i < BVHList.size(); i++) {
		GPUBoundingBox& boundingBox = BVHList[i];
		Box* box = new Box(boundingBox.minLoc, boundingBox.maxLoc);
		if (boundingBox.isLeaf) // second half of list, meaning its last node
			box->setColor(glm::vec3(0, 1, 1));
	}
}