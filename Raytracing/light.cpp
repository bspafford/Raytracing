#include "light.h"

Light::Light(glm::vec3 loc, glm::vec3 dir, int lightType) {
	this->loc = loc;
	this->dir = dir;
	this->lightType = lightType;

	instances.push_back(*this);
}

std::vector<GPULight> Light::ToGPU() {
	std::vector<GPULight> gpuLights;
	gpuLights.reserve(instances.size());
	for (Light& light : instances) {
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