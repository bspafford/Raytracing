#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

struct LightType {
	static inline int Point = 0;
	static inline int Sun = 1;
	//static inline int Spot = 2;
	//static inline int Area = 3;
};

struct GPULight {
	glm::vec3 loc;
	float strength;
	glm::vec3 dir;
	GLuint lightType;
	glm::vec3 color;	float pad1;

	GPULight(glm::vec3 _loc, glm::vec3 _dir, glm::vec3 _color, float _strength, int _lightType)
		: loc(_loc), dir(_dir), color(_color), strength(_strength), lightType(_lightType) {}
};

class Light {
public:
	Light(glm::vec3 loc, glm::vec3 dir, int lightType, float strength = 1.f);
	static std::vector<GPULight> ToGPU(bool& hasSun);

	static void DeleteAll();

	static inline std::vector<Light> instances;

private:
	glm::vec3 loc;
	glm::vec3 dir = glm::vec3(0);
	glm::vec3 color = glm::vec3(1);
	float strength = 1.f;
	int lightType;
};