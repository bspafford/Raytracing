#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#include <nlohmann/json.hpp>
#include "Mesh.h"

using json = nlohmann::json;

struct MaterialData {
	GLuint64 baseColorTexture = -1;
	GLuint64 normalTexture = -1;
	GLuint64 metallicRoughnessTexture = -1;
	int hasBaseTexture = 0;
	int hasNoramlTexture = 0;
	int hasMetallicRoughnessTexture = 0;
	float metallicFactor = 1.f;
	float roughnessFactor = 1.f;

	MaterialData(GLuint64 _baseColorTexture, GLuint64 _normalTexture, GLuint64 _metallicRoughnessTexture, int _hasBaseTexture, int _hasNoramlTexture, int _hasMetallicRoughnessTexture, float _metallicFactor, float _roughnessFactor) :
		baseColorTexture(_baseColorTexture),
		normalTexture(_normalTexture),
		metallicRoughnessTexture(_metallicRoughnessTexture),
		hasBaseTexture(_hasBaseTexture),
		hasNoramlTexture(_hasNoramlTexture),
		hasMetallicRoughnessTexture(_hasMetallicRoughnessTexture),
		metallicFactor(_metallicFactor),
		roughnessFactor(_roughnessFactor) 
	{};
};

struct Triangle {
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 p3;
	GLuint i1;
	GLuint i2;
	GLuint i3;

	Triangle() {};
	Triangle(glm::vec3 _p1, glm::vec3 _p2, glm::vec3 _p3, GLuint _i1, GLuint _i2, GLuint _i3) : p1(_p1), p2(_p2), p3(_p3), i1(_i1), i2(_i2), i3(_i3) {};
};

struct GPUBoundingBox {
	GLuint hasChildren = true;
	GLuint hasTriangle = false; glm::ivec2 pad1;
	glm::ivec3 triangleIndices;	float pad2;
	glm::vec3 minLoc;			float pad3;
	glm::vec3 maxLoc;			float pad4;
	glm::vec3 centroidLoc;		float pad5;

	GPUBoundingBox() {
		hasChildren = false;
	}

	GPUBoundingBox(GPUBoundingBox* left, GPUBoundingBox* right) {
		glm::vec3 leftMin = left ? left->minLoc : glm::vec3(FLT_MAX);
		glm::vec3 leftMax = left ? left->maxLoc : glm::vec3(-FLT_MAX);
		glm::vec3 rightMin = right ? right->minLoc : glm::vec3(FLT_MAX);
		glm::vec3 rightMax = right ? right->maxLoc : glm::vec3(-FLT_MAX);
		minLoc = glm::min(leftMin, rightMin);
		maxLoc = glm::max(leftMax, rightMax);
		centroidLoc = (minLoc + maxLoc) * 0.5f;
	}

	GPUBoundingBox(Triangle* triangle) {
		// find the min max, give triangle indice
		minLoc = glm::vec3(FLT_MAX);
		maxLoc = glm::vec3(-FLT_MAX);
		triangleIndices = glm::vec3(triangle->i1, triangle->i2, triangle->i3);
		hasTriangle = true;

		std::vector<glm::vec3> pointsList = { triangle->p1, triangle->p2, triangle->p3 };
		for (glm::vec3& p : pointsList) {
			minLoc = glm::min(minLoc, p);
			maxLoc = glm::max(maxLoc, p);
		}
		centroidLoc = (minLoc + maxLoc) * 0.5f;
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

	void BVH();
	std::vector<GPUBoundingBox*> buildBVH(std::vector<GPUBoundingBox*> boundingBoxes);
	std::vector<std::string> testBVH(std::vector<std::string> strings);
	std::vector<GPUBoundingBox*> BVHList;

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
#endif