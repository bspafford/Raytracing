#include"Model.h"
#include "SSBO.h"

#include <future>

#include "debugger.h"

Model::Model(const char* file) {
	instances.push_back(this);

	// Make a JSON object
	std::string text = get_file_contents(file);
	JSON = json::parse(text);

	// Get the binary data
	Model::file = file;
	data = getData();

	// Traverse all nodes
	traverseNode(0);
}

void Model::Draw(Shader* shader, Camera* camera) {
	// Go over all meshes and draw each one
	glUniform3f(glGetUniformLocation(shader->ID, "color"), color.x / 255.f, color.y / 255.f, color.z / 255.f);

	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader, camera, matricesMeshes[i], -pos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), scale);
}

void Model::setPos(glm::vec3 pos) {
	this->pos = pos;
}

glm::vec3 Model::getPos() {
	return pos;
}

void Model::setScale(glm::vec3 scale) {
	this->scale = scale;
}

void Model::setColor(glm::vec3 color) {
	this->color = color;
}

void Model::loadMesh(unsigned int indMesh) {
	// Get all accessor indices
	unsigned int posAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["POSITION"];
	unsigned int normalAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["NORMAL"];
	unsigned int texAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TEXCOORD_0"];
	unsigned int tanAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TANGENT"];
	unsigned int indAccInd = JSON["meshes"][indMesh]["primitives"][0]["indices"];

	// Use accessor indices to get all vertices components
	std::vector<float> posVec = getFloats(JSON["accessors"][posAccInd]);
	std::vector<glm::vec3> positions = groupFloatsVec3(posVec);
	std::vector<float> normalVec = getFloats(JSON["accessors"][normalAccInd]);
	std::vector<glm::vec3> normals = groupFloatsVec3(normalVec);
	std::vector<float> texVec = getFloats(JSON["accessors"][texAccInd]);
	std::vector<glm::vec2> texUVs = groupFloatsVec2(texVec);
	std::vector<float> tangentVec = getFloats(JSON["accessors"][tanAccInd]);
	std::vector<glm::vec4> tangents = groupFloatsVec4(tangentVec);

	// Combine all the vertex components and also get the indices and textures
	std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs, tangents);
	std::vector<GLuint> indices = getIndices(JSON["accessors"][indAccInd]);
	std::vector<Texture> textures = getTextures();

	// Combine the vertices, indices, and textures into a mesh
	meshes.push_back(Mesh(vertices, indices, textures));
}

void Model::traverseNode(unsigned int nextNode, glm::mat4 matrix) {
	// Current node
	json node = JSON["nodes"][nextNode];

	// Get translation if it exists
	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
	if (node.find("translation") != node.end()) {
		float transValues[3];
		for (unsigned int i = 0; i < node["translation"].size(); i++)
			transValues[i] = (node["translation"][i]);
		translation = glm::make_vec3(transValues);
	}
	// Get quaternion if it exists
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	if (node.find("rotation") != node.end()) {
		float rotValues[4] =
		{
			node["rotation"][3],
			node["rotation"][0],
			node["rotation"][1],
			node["rotation"][2]
		};
		rotation = glm::make_quat(rotValues);
	}
	// Get scale if it exists
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	if (node.find("scale") != node.end()) {
		float scaleValues[3];
		for (unsigned int i = 0; i < node["scale"].size(); i++)
			scaleValues[i] = (node["scale"][i]);
		scale = glm::make_vec3(scaleValues);
	}
	// Get matrix if it exists
	glm::mat4 matNode = glm::mat4(1.0f);
	if (node.find("matrix") != node.end()) {
		float matValues[16];
		for (unsigned int i = 0; i < node["matrix"].size(); i++)
			matValues[i] = (node["matrix"][i]);
		matNode = glm::make_mat4(matValues);
	}

	// Initialize matrices
	glm::mat4 trans = glm::mat4(1.0f);
	glm::mat4 rot = glm::mat4(1.0f);
	glm::mat4 sca = glm::mat4(1.0f);

	// Use translation, rotation, and scale to change the initialized matrices
	trans = glm::translate(trans, translation);
	rot = glm::mat4_cast(rotation);
	sca = glm::scale(sca, scale);

	// Multiply all matrices together
	glm::mat4 matNextNode = matrix * matNode * trans * rot * sca;

	// Check if the node contains a mesh and if it does load it
	if (node.find("mesh") != node.end()) {
		translationsMeshes.push_back(translation);
		rotationsMeshes.push_back(rotation);
		scalesMeshes.push_back(scale);
		matricesMeshes.push_back(matNextNode);

		loadMesh(node["mesh"]);
	}

	// if node has children
	if (node.find("children") != node.end()) {
		for (unsigned int i = 0; i < node["children"].size(); i++)
			traverseNode(node["children"][i], matNextNode);
	}

	//if node has multiple meshes in model
	if (nextNode + 1 < JSON["nodes"].size())
		traverseNode(nextNode + 1);
}

std::vector<unsigned char> Model::getData() {
	// Create a place to store the raw text, and get the uri of the .bin file
	std::string bytesText;
	std::string uri = JSON["buffers"][0]["uri"];

	// Store raw text data into bytesText
	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);
	bytesText = get_file_contents((fileDirectory + uri).c_str());

	// Transform the raw text data into bytes and put them in a vector
	std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
	return data;
}

std::vector<float> Model::getFloats(json accessor) {
	std::vector<float> floatVec;

	// Get properties from the accessor
	unsigned int buffViewInd = accessor.value("bufferView", 1);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	std::string type = accessor["type"];

	// Get properties from the bufferView
	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	// Interpret the type and store it into numPerVert
	unsigned int numPerVert;
	if (type == "SCALAR") numPerVert = 1;
	else if (type == "VEC2") numPerVert = 2;
	else if (type == "VEC3") numPerVert = 3;
	else if (type == "VEC4") numPerVert = 4;
	else throw std::invalid_argument("Type is invalid (not SCALAR, VEC2, VEC3, or VEC4)");

	// Go over all the bytes in the data at the correct place using the properties from above
	unsigned int beginningOfData = byteOffset + accByteOffset;
	unsigned int lengthOfData = count * 4 * numPerVert;
	for (unsigned int i = beginningOfData; i < beginningOfData + lengthOfData; i += 4) {
		unsigned char bytes[] = { data[i], data[i + 1], data[i + 2], data[i + 3] };
		float value;
		std::memcpy(&value, bytes, sizeof(float));
		floatVec.push_back(value);
	}

	return floatVec;
}

std::vector<GLuint> Model::getIndices(json accessor) {
	std::vector<GLuint> indices;

	// Get properties from the accessor
	unsigned int buffViewInd = accessor.value("bufferView", 0);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	unsigned int componentType = accessor["componentType"];

	// Get properties from the bufferView
	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	// Get indices with regards to their type: unsigned int, unsigned short, or short
	unsigned int beginningOfData = byteOffset + accByteOffset;
	if (componentType == 5125) {
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 4; i += 4) {
			unsigned char bytes[] = { data[i], data[i + 1], data[i + 2], data[i + 3] };
			unsigned int value;
			std::memcpy(&value, bytes, sizeof(unsigned int));
			indices.push_back((GLuint)value);
		}
	} else if (componentType == 5123) {
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i += 2) {
			unsigned char bytes[] = { data[i], data[i + 1] };
			unsigned short value;
			std::memcpy(&value, bytes, sizeof(unsigned short));
			indices.push_back((GLuint)value);
		}
	} else if (componentType == 5122) {
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i += 2) {
			unsigned char bytes[] = { data[i], data[i + 1] };
			short value;
			std::memcpy(&value, bytes, sizeof(short));
			indices.push_back((GLuint)value);
		}
	}

	return indices;
}

std::vector<Texture> Model::getTextures() {
	materialData.clear();
	std::vector<Texture> textures;

	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);

	json materials = JSON["materials"];
	json images = JSON["images"];

	for (json& material : materials) {
		json pbr = material["pbrMetallicRoughness"];

		glm::vec4 baseColorFactor(1.f);
		if (pbr.contains("baseColorFactor")) {
			for (int i = 0; i < 4; i++)
				baseColorFactor[i] = pbr["baseColorFactor"][i];
		}
		int baseColorIndex = -1;
		if (pbr.contains("baseColorTexture"))
			baseColorIndex = pbr["baseColorTexture"].value("index", -1);
		int normalIndex = -1;
		if (material.contains("normalTexture"))
			normalIndex = material["normalTexture"].value("index", -1);
		int metallicRoughnessIndex = -1;
		if (pbr.contains("metallicRoughnessTexture"))
			metallicRoughnessIndex = pbr["metallicRoughnessTexture"].value("index", -1);
		float metallicFactor = pbr.value("metallicFactor", 1.f);
		float roughnessFactor = pbr.value("roughnessFactor", 1.f);
		float transmissionFactor = 0;
		float ior = -1;
		if (material.contains("extensions")) {
			json extensions = material["extensions"];
			if (extensions.contains("KHR_materials_transmission"))
				transmissionFactor = material["extensions"]["KHR_materials_transmission"].value("transmissionFactor", 0);
			if (extensions.contains("KHR_materials_ior"))
				ior = material["extensions"]["KHR_materials_ior"].value("ior", 1.f);
		}

		int indexList[] = { normalIndex, baseColorIndex, metallicRoughnessIndex };
		std::string typeList[] = { "normal", "diffuse", "specular" };

		GLuint64 normalID = 0, baseID = 0, metallicID = 0;
		int hasNormal = 0, hasBase = 0, hasMetallic = 0;

		for (int i = 0; i < sizeof(indexList) / sizeof(int); i++) {
			if (indexList[i] == -1)
				continue;

			std::string texPath = images[indexList[i]]["uri"];
			Texture texture = Texture((fileDirectory + texPath).c_str(), typeList[i].c_str(), loadedTex.size());
			textures.push_back(texture);
			loadedTex.push_back(texture);
			loadedTexName.push_back(texPath);

			if (typeList[i] == "normal") {
				normalID = texture.ID;
				hasNormal = 1;
			} else if (typeList[i] == "diffuse") {
				baseID = texture.ID;
				hasBase = 1;
			} else if (typeList[i] == "specular") {
				metallicID = texture.ID;
				hasMetallic = 1;
			}
		}

		materialData.push_back(MaterialData(
			baseID,
			normalID,
			metallicID,
			hasBase,
			hasNormal,
			hasMetallic,
			baseColorFactor,
			metallicFactor,
			roughnessFactor,
			transmissionFactor,
			ior
		));
	}

	return textures;
}

std::vector<Vertex> Model::assembleVertices
(
	std::vector<glm::vec3> positions,
	std::vector<glm::vec3> normals,
	std::vector<glm::vec2> texUVs,
	std::vector<glm::vec4> tangents
) {
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++) {
		vertices.push_back
		(
			Vertex
			(
				positions[i],
				normals[i],
				glm::vec3(1.0f, 1.0f, 1.0f),
				texUVs[i],
				tangents[i]
			)
		);
	}
	return vertices;
}

std::vector<glm::vec2> Model::groupFloatsVec2(std::vector<float> floatVec) {
	const unsigned int floatsPerVector = 2;

	std::vector<glm::vec2> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector) {
		vectors.push_back(glm::vec2(0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++) {
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}
std::vector<glm::vec3> Model::groupFloatsVec3(std::vector<float> floatVec) {
	const unsigned int floatsPerVector = 3;

	std::vector<glm::vec3> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector) {
		vectors.push_back(glm::vec3(0, 0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++) {
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}
std::vector<glm::vec4> Model::groupFloatsVec4(std::vector<float> floatVec) {
	const unsigned int floatsPerVector = 4;

	std::vector<glm::vec4> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector) {
		vectors.push_back(glm::vec4(0, 0, 0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++) {
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}

std::vector<Mesh>& Model::getMeshes() {
	return meshes;
}

std::vector<glm::mat4> Model::getMatricesMeshes() {
	std::vector<glm::mat4> matMeshes = matricesMeshes;

	// Transform the matrices to their correct form
	glm::mat4 trans = glm::translate(glm::mat4(1.0f), pos);
	glm::mat4 rot = glm::mat4_cast(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
	glm::mat4 sca = glm::scale(glm::mat4(1.0f), scale);

	glm::mat4 modelTransform = trans * rot * sca;

	for (glm::mat4& mat : matMeshes) {
		mat *= modelTransform;
	}
	return matMeshes;
}

std::vector<MaterialData> Model::getMaterialData() {
	return materialData;
}

std::vector<Texture> Model::getLoadedTex() {
	return loadedTex;
}

// bestCost, bestIndex
std::pair<float, int> Model::computeForAxis(std::vector<Triangle*>::iterator begin, std::vector<Triangle*>::iterator end, std::vector<Triangle*>& tris, int axis) {
	tris = std::vector<Triangle*>(begin, end);

	// sort on axis
	std::sort(tris.begin(), tris.end(), [axis](const Triangle* a, const Triangle* b) {
		return a->centroidLoc[axis] < b->centroidLoc[axis];
		});

	size_t size = tris.size();
	std::vector<glm::vec3> prefixMin(size), prefixMax(size);
	std::vector<glm::vec3> suffixMin(size), suffixMax(size);

	// prefix
	prefixMin[0] = tris[0]->minBounds;
	prefixMax[0] = tris[0]->maxBounds;
	for (int i = 1; i < size; ++i) {
		prefixMin[i] = glm::min(prefixMin[i - 1], tris[i]->minBounds);
		prefixMax[i] = glm::max(prefixMax[i - 1], tris[i]->maxBounds);
	}

	// suffix
	suffixMin[size - 1] = tris[size-1]->minBounds;
	suffixMax[size - 1] = tris[size-1]->maxBounds;
	for (int i = size - 2; i >= 0; i--) {
		suffixMin[i] = glm::min(suffixMin[i + 1], tris[i]->minBounds);
		suffixMax[i] = glm::max(suffixMax[i + 1], tris[i]->maxBounds);
	}

	float bestCost = FLT_MAX;
	int bestIndex = size / 2;
	for (int i = 0; i < size; i++) {
		float parentSurfaceArea = BoundingBox::calcSurfaceArea(glm::min(prefixMin[i], suffixMin[i]), glm::max(prefixMax[i], suffixMax[i]));

		// Left: triangles [0, i)
		// Right: triangles [i, N);
		float cost = 1.f + ((BoundingBox::calcSurfaceArea(prefixMin[i], prefixMax[i]) / parentSurfaceArea) * i +
			(BoundingBox::calcSurfaceArea(suffixMin[i], suffixMax[i]) / parentSurfaceArea) * (size - i));

		if (cost < bestCost) {
			bestCost = cost;
			bestIndex = i;
		}
	}

	return { bestCost, bestIndex };
}

// SAH
// look at each axis, look through each index and find lowest cost split and its axis
// then split list there, and recurse
BoundingBox* Model::buildBVH(std::vector<Triangle*>::iterator begin, std::vector<Triangle*>::iterator end) {
	GLuint triangleCount = 4;
	size_t size = end - begin;

	// leaf
	if (size <= triangleCount)
		return new BoundingBox(std::vector<Triangle*>(begin, end));

	std::vector<Triangle*> axisData[3];
	//for (int axis = 0; axis < 3; axis++)
		//axisData[axis] = std::vector<Triangle*>(begin, end);

	// async for best axis
	auto f0 = std::async(std::launch::async, &Model::computeForAxis, begin, end, std::ref(axisData[0]), 0);
	auto f1 = std::async(std::launch::async, &Model::computeForAxis, begin, end, std::ref(axisData[1]), 1);
	auto f2 = std::async(std::launch::async, &Model::computeForAxis, begin, end, std::ref(axisData[2]), 2);

	std::pair<float, int> r0 = f0.get();
	std::pair<float, int> r1 = f1.get();
	std::pair<float, int> r2 = f2.get();

	float best = r0.first;
	int bestAxis = 0;
	int bestIndex = r0.second;
	if (r1.first < best) { best = r1.first; bestAxis = 1; bestIndex = r1.second; }
	if (r2.first < best) { best = r2.first; bestAxis = 2; bestIndex = r2.second; }
	auto& sorted = axisData[bestAxis];

	// recurse
	BoundingBox* left = nullptr;
	BoundingBox* right = nullptr;

	int threshold = 10000;
	if (size > threshold) {
		auto leftFuture = std::async(std::launch::async, [&]() {
			return buildBVH(sorted.begin(), sorted.begin() + bestIndex);
		});
		right = buildBVH(sorted.begin() + bestIndex, sorted.end());
		left = leftFuture.get();
	} else {
		left = buildBVH(sorted.begin(), sorted.begin() + bestIndex);
		right = buildBVH(sorted.begin() + bestIndex, sorted.end());
	}

	// build bounding box parents
	return new BoundingBox(left, right);
}

GLuint Model::convertToGPU(BoundingBox* box, std::vector<GPUBoundingBox>& outList, std::unordered_map<Triangle*, int>& triangleMap) {
	GLuint currIdx = outList.size();
	outList.push_back(GPUBoundingBox(box));

	if (outList[currIdx].isLeaf) {
		outList[currIdx].children.x = (box->triangles.size() >= 1 && box->triangles[0]) ? triangleMap[box->triangles[0]] : -1;
		outList[currIdx].children.y = (box->triangles.size() >= 2 && box->triangles[1]) ? triangleMap[box->triangles[1]] : -1;
		outList[currIdx].children.z = (box->triangles.size() >= 3 && box->triangles[2]) ? triangleMap[box->triangles[2]] : -1;
		outList[currIdx].children.w = (box->triangles.size() >= 4 && box->triangles[3]) ? triangleMap[box->triangles[3]] : -1;
	} else {
		outList[currIdx].children.x = box->left ? convertToGPU(box->left, outList, triangleMap) : -1;
		outList[currIdx].children.y = box->right ? convertToGPU(box->right, outList, triangleMap) : -1;
	}

	return currIdx;
}

std::vector<GPUBoundingBox> Model::BVH() {
	std::vector<Triangle*> triangles;
	std::vector<Vertex> verticesList;
	std::vector<glm::mat4> meshMatrices;
	std::vector<MaterialData> materialList;

	// combine all objects vertices and indices
	std::vector<BoundingBox> boundingBoxes;
	GLuint indiceOffset = 0;
	GLuint meshOffset = 0;
	GLuint materialOffset = 0;

	for (Model* model : instances) {
		std::vector<MaterialData> materialData = model->getMaterialData();
		for (MaterialData& material : materialData) {
			if (material.hasBaseTexture) {
				material.baseColorTexture = glGetTextureHandleARB(material.baseColorTexture);
				glMakeTextureHandleResidentARB(material.baseColorTexture);
			}
			if (material.hasNormalTexture) {
				material.normalTexture = glGetTextureHandleARB(material.normalTexture);
				glMakeTextureHandleResidentARB(material.normalTexture);
			}
			if (material.hasMetallicRoughnessTexture) {
				material.metallicRoughnessTexture = glGetTextureHandleARB(material.metallicRoughnessTexture);
				glMakeTextureHandleResidentARB(material.metallicRoughnessTexture);
			}
		}
		materialList.insert(materialList.end(), materialData.begin(), materialData.end());

		std::vector<glm::mat4> _meshMatrices = model->getMatricesMeshes();
		meshMatrices.insert(meshMatrices.end(), _meshMatrices.begin(), _meshMatrices.end());

		std::cout << model->meshes.size() << "\n";
		for (int meshIdx = 0; meshIdx < model->meshes.size(); meshIdx++) {
			Mesh& mesh = model->meshes[meshIdx];
			verticesList.insert(verticesList.end(), mesh.vertices.begin(), mesh.vertices.end());
			triangles.reserve(triangles.size() + mesh.indices.size() / 3);
			for (int i = 0; i < mesh.indices.size(); i += 3) {
				glm::vec3 p1 = mesh.vertices[mesh.indices[i]].position;
				glm::vec3 p2 = mesh.vertices[mesh.indices[i+1]].position;
				glm::vec3 p3 = mesh.vertices[mesh.indices[i+2]].position;
				GLuint materialIdx = model->JSON["meshes"][meshIdx]["primitives"][0].value("material", 0) + materialOffset;
				triangles.push_back(new Triangle(p1, p2, p3, glm::vec3(mesh.indices[i], mesh.indices[i+1], mesh.indices[i+2]) + glm::vec3(indiceOffset), meshOffset + meshIdx, materialIdx));
			}

			indiceOffset += mesh.vertices.size();
		}
		meshOffset += model->meshes.size();
		materialOffset += materialData.size();
	}

	BoundingBox* box = buildBVH(triangles.begin(), triangles.end());

	std::unordered_map<Triangle*, int> triangleMap;
	triangleMap.reserve(triangles.size());
	for (int i = 0; i < triangles.size(); i++)
		triangleMap[triangles[i]] = i;
	
	// DFS to convert to GPUBoundingBox and list
	std::vector<GPUBoundingBox> BVHList;
	convertToGPU(box, BVHList, triangleMap);

	std::vector<Triangle> triangleList;
	triangleList.reserve(triangles.size());
	for (Triangle* triangle : triangles)
		triangleList.push_back(*triangle);

	// setup SSBOs
	SSBO::Bind(verticesList.data(), verticesList.size() * sizeof(Vertex), 0);
	SSBO::Bind(meshMatrices.data(), meshMatrices.size() * sizeof(glm::mat4), 1);
	SSBO::Bind(materialList.data(), materialList.size() * sizeof(MaterialData), 2);
	SSBO::Bind(BVHList.data(), BVHList.size() * sizeof(GPUBoundingBox), 3);
	SSBO::Bind(triangleList.data(), triangleList.size() * sizeof(Triangle), 4);

	// delete
	for (int i = 0; i < triangles.size(); i++)
		delete triangles[i];
	DeleteBoundingBox(box);

	return BVHList;
}

void Model::DeleteBoundingBox(BoundingBox* box) {
	if (!box->left && !box->right) {
		delete box;
		return;
	}
	if (box->left)
		DeleteBoundingBox(box->left);
	if (box->right)
		DeleteBoundingBox(box->right);
	delete box;
}

void Model::DeleteAll() {
	for (Model* model : instances) {
		delete model;
	}
	instances.clear();
}