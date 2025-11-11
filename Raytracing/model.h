#pragma once

#include <nlohmann/json.hpp>
#include "Mesh.h"

using json = nlohmann::json;

struct MaterialData {
	glm::vec4 baseColorFactor;

	GLuint64 baseColorTexture = -1;
	int hasBaseTexture = 0;
	float transmissionFactor = 0.f;

	GLuint64 normalTexture = -1;
	int hasNormalTexture = 0;
	float metallicFactor = 1.f;

	GLuint64 metallicRoughnessTexture = -1;
	int hasMetallicRoughnessTexture = 0;
	float roughnessFactor = 1.f;

	MaterialData(GLuint64 _baseColorTexture, GLuint64 _normalTexture, GLuint64 _metallicRoughnessTexture, int _hasBaseTexture, int _hasNormalTexture, int _hasMetallicRoughnessTexture, float _metallicFactor, float _roughnessFactor, float _transmissionFactor) :
		baseColorTexture(_baseColorTexture),
		normalTexture(_normalTexture),
		metallicRoughnessTexture(_metallicRoughnessTexture),
		hasBaseTexture(_hasBaseTexture),
		hasNormalTexture(_hasNormalTexture),
		hasMetallicRoughnessTexture(_hasMetallicRoughnessTexture),
		metallicFactor(_metallicFactor),
		roughnessFactor(_roughnessFactor),
		transmissionFactor(_transmissionFactor)
	{};
};

struct Triangle {
	alignas(16) glm::uvec3 indices;
	alignas(16) glm::vec3 p1;
	alignas(16) glm::vec3 p2;
	alignas(16) glm::vec3 p3;
	alignas(16) glm::vec3 centroidLoc;
	glm::uvec4 meshIndex;

	Triangle(glm::vec3 _p1, glm::vec3 _p2, glm::vec3 _p3, glm::uvec3 _indices, GLuint _meshIndex) : p1(_p1), p2(_p2), p3(_p3), indices(_indices), meshIndex(glm::uvec4(_meshIndex)) {
		centroidLoc = (p1 + p2 + p3) / 3.f;
	};

	static void computeBounds(std::vector<Triangle*>& list, int start, int end, glm::vec3& min, glm::vec3& max) {
		min = glm::vec3(FLT_MAX);
		max = glm::vec3(-FLT_MAX);
		for (int i = start; i < end; i++) {
			glm::vec3 points[] = {list[i]->p1, list[i]->p2, list[i]->p3};
			for (int p = 0; p < 3; p++) {
				min = glm::min(points[p], min);
				max = glm::max(points[p], max);
			}
		}
	}
};

struct BoundingBox {
	BoundingBox* left = nullptr;
	BoundingBox* right = nullptr;
	std::vector<Triangle*> triangles;

	glm::vec3 minLoc;
	glm::vec3 maxLoc;
	glm::vec3 centroidLoc;

	BoundingBox(BoundingBox* left, BoundingBox* right) {
		this->left = left;
		this->right = right;

		glm::vec3 leftMin = left ? left->minLoc : glm::vec3(FLT_MAX);
		glm::vec3 leftMax = left ? left->maxLoc : glm::vec3(-FLT_MAX);
		glm::vec3 rightMin = right ? right->minLoc : glm::vec3(FLT_MAX);
		glm::vec3 rightMax = right ? right->maxLoc : glm::vec3(-FLT_MAX);
		minLoc = glm::min(leftMin, rightMin);
		maxLoc = glm::max(leftMax, rightMax);
		centroidLoc = (minLoc + maxLoc) * 0.5f;
	}

	BoundingBox(std::vector<Triangle*> triangles) {
		this->triangles = triangles;
		// find the min max, give triangle indice
		minLoc = glm::vec3(FLT_MAX);
		maxLoc = glm::vec3(-FLT_MAX);

		for (Triangle* triangle : triangles) {
			std::vector<glm::vec3> pointsList = { triangle->p1, triangle->p2, triangle->p3 };
			for (glm::vec3& p : pointsList) {
				minLoc = glm::min(minLoc, p);
				maxLoc = glm::max(maxLoc, p);
			}
		}
		centroidLoc = (minLoc + maxLoc) * 0.5f;
	}

	float calcSurfaceArea() {
		glm::vec3 e = maxLoc - minLoc;
		return 2.f * (e.x * e.y + e.y * e.z + e.z * e.x);
	}

	static float calcSurfaceArea(glm::vec3 min, glm::vec3 max) {
		glm::vec3 e = max - min;
		return 2.f * (e.x * e.y + e.y * e.z + e.z * e.x);
	}

	static void computeBounds(std::vector<BoundingBox>& list, int start, int end, glm::vec3& min, glm::vec3& max) {
		min = glm::vec3(FLT_MAX);
		max = glm::vec3(-FLT_MAX);
		for (int i = start; i < end; i++) {
			min = glm::min(list[i].minLoc, min);
			max = glm::max(list[i].maxLoc, max);
		}
	}
};

struct GPUBoundingBox {
	glm::vec3 minLoc;		float pad1;
	glm::vec3 maxLoc;
	GLuint isLeaf;
	glm::ivec4 children = glm::ivec4(-1); // index of children


	GPUBoundingBox(BoundingBox* box) {
		minLoc = glm::min(box->left ? box->left->minLoc : glm::vec3(FLT_MAX), box->right ? box->right->minLoc : glm::vec3(FLT_MAX));
		maxLoc = glm::max(box->left ? box->left->maxLoc : glm::vec3(-FLT_MAX), box->right ? box->right->maxLoc : glm::vec3(-FLT_MAX));
		isLeaf = !box->left && !box->right;
	}
};

class Model {
public:
	// Loads in a model from a file and stores tha information in 'data', 'JSON', and 'file'
	Model(const char* file);

	void Draw(Shader* shader, Camera* camera);

	void setPos(glm::vec3 pos);
	void setScale(glm::vec3 pos);
	void setColor(glm::vec3 color);
	glm::vec3 getPos();

	std::vector<Mesh>& getMeshes();
	std::vector<glm::mat4> getMatricesMeshes();
	std::vector<MaterialData> getMaterialData();
	std::vector<Texture> getLoadedTex();

	static std::vector<GPUBoundingBox> BVH();
	static BoundingBox* buildBVH(std::vector<Triangle*> triangles);
	static GLuint convertToGPU(BoundingBox* box, std::vector<GPUBoundingBox>& outList, std::unordered_map<Triangle*, int>& triangleMap);

	static inline std::vector<Model*> instances;

private:
	glm::vec3 pos = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);
	glm::vec3 color = glm::vec3(255);

	// Variables for easy access
	const char* file;
	std::vector<unsigned char> data;
	json JSON;

	// All the meshes and transformations
	std::vector<Mesh> meshes;
	std::vector<glm::vec3> translationsMeshes;
	std::vector<glm::quat> rotationsMeshes;
	std::vector<glm::vec3> scalesMeshes;
	std::vector<glm::mat4> matricesMeshes;

	// Prevents textures from being loaded twice
	std::vector<std::string> loadedTexName;
	std::vector<Texture> loadedTex;

	std::vector<MaterialData> materialData;

	// Loads a single mesh by its index
	void loadMesh(unsigned int indMesh);

	// Traverses a node recursively, so it essentially traverses all connected nodes
	void traverseNode(unsigned int nextNode, glm::mat4 matrix = glm::mat4(1.0f));

	// Gets the binary data from a file
	std::vector<unsigned char> getData();
	// Interprets the binary data into floats, indices, and textures
	std::vector<float> getFloats(json accessor);
	std::vector<GLuint> getIndices(json accessor);
	std::vector<Texture> getTextures();

	// Assembles all the floats into vertices
	std::vector<Vertex> assembleVertices
	(
		std::vector<glm::vec3> positions,
		std::vector<glm::vec3> normals,
		std::vector<glm::vec2> texUVs,
		std::vector<glm::vec4> tangents
	);

	// Helps with the assembly from above by grouping floats
	std::vector<glm::vec2> groupFloatsVec2(std::vector<float> floatVec);
	std::vector<glm::vec3> groupFloatsVec3(std::vector<float> floatVec);
	std::vector<glm::vec4> groupFloatsVec4(std::vector<float> floatVec);
};