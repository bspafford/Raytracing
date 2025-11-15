#include "light.h"

Light::Light(glm::vec3 loc, glm::vec3 dir, int lightType, float strength) {
	this->loc = loc;
	this->dir = dir;
	this->lightType = lightType;
	this->strength = strength;

	instances.push_back(*this);
}

std::vector<GPULight> Light::ToGPU(bool& hasSun) {
	hasSun = false;
	std::vector<GPULight> gpuLights;
	gpuLights.reserve(instances.size());
	for (Light& light : instances) {
		if (light.lightType == LightType::Sun)
			hasSun = true;

		GPULight gpuLight(
			light.loc,
			glm::normalize(light.dir),
			light.color,
			light.strength,
			light.lightType
		);
		gpuLights.push_back(gpuLight);
	}
	return gpuLights;
}

void Light::DeleteAll() {
	instances.clear();
}