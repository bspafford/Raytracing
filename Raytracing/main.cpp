#include "main.h"
#include "Camera.h"
#include "model.h"
#include "computeShader.h"
#include "SSBO.h"

int main() {
	Main* _main = new Main();
	return _main->createWindow();
}

void Main::setupQuad() {
	float quadVertices[] = {
		// positions        // texcoords
		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,   // top right
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,   // bottom right
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,   // bottom left

		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,   // bottom left
		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,   // top left
		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f    // top right
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
}

void Main::renderQuad() {
	quadShader->Activate();
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

int Main::createWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(stuff::screenSize.x, stuff::screenSize.y, "Raytracing", nullptr, nullptr);

	if (window == nullptr) {
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		return -1;
	}

	glViewport(0, 0, stuff::screenSize.x, stuff::screenSize.y);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Start();

	// setup quad texture for Compute shader to write on
	const unsigned int TEXTURE_WIDTH = stuff::screenSize.x, TEXTURE_HEIGHT = stuff::screenSize.y;
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindTexture(GL_TEXTURE_2D, 0);

	setupQuad();

	std::vector<Vertex> vertexList;
	std::vector<GLuint> indiceList;
	std::vector<glm::mat4> meshMatrices;
	std::vector<GLuint> meshStartLoc;
	std::vector<MaterialData> materialList;
	GLuint texCount = 0;

	computeShader->Activate();
	// combine all objects vertices and indices
	GLuint indiceOffset = 0;
	GLuint meshOffset = 0;
	for (Model* model : Model::instances) {
		// only binds first texture
		Texture tex = model->getLoadedTex()[0];
		glActiveTexture(GL_TEXTURE1 + texCount);
		glBindTexture(GL_TEXTURE_2D, tex.ID);
		glUniform1i(glGetUniformLocation(computeShader->ID, ("textures[" + std::to_string(texCount) + "]").c_str()), texCount + 1);
		texCount++;

		std::vector<glm::mat4> _meshMatrices = model->getMatricesMeshes();
		meshMatrices.insert(meshMatrices.end(), _meshMatrices.begin(), _meshMatrices.end());

		std::vector<MaterialData> materialData = model->getMaterialData();
		materialList.insert(materialList.end(), materialData.begin(), materialData.end());
		for (Mesh& mesh : model->getMeshes()) {
			vertexList.insert(vertexList.end(), mesh.vertices.begin(), mesh.vertices.end());
			for (GLuint& indice : mesh.indices)
				indiceList.push_back(indice + indiceOffset);

			meshStartLoc.push_back(meshOffset);
			meshOffset += mesh.indices.size();
			indiceOffset += mesh.vertices.size();
		}
	}

	for (int i = 0; i < indiceList.size(); i++) {
		if (indiceList[i] >= 233)
			std::cout << "pause!\n";
	}

	// setup SSBOs
	SSBO::Bind(vertexList.data(), vertexList.size() * sizeof(Vertex));
	SSBO::Bind(indiceList.data(), indiceList.size() * sizeof(GLuint));
	SSBO::Bind(meshMatrices.data(), meshMatrices.size() * sizeof(glm::mat4));
	SSBO::Bind(meshStartLoc.data(), meshStartLoc.size() * sizeof(GLuint));
	SSBO::Bind(materialList.data(), materialList.size() * sizeof(MaterialData));

	auto lastTime = std::chrono::steady_clock::now();
	float time = 0;
	while (!glfwWindowShouldClose(window)) {
		auto currentTime = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;
		time += deltaTime;

		glfwPollEvents();

		Update(deltaTime);

		if (true) { // whether or not should render using computeShader
			computeShader->Activate();
			computeShader->setMat4("projection", camera->cameraMatrix);
			computeShader->setMat4("invProj", glm::inverse(camera->cameraMatrix));
			computeShader->setVec3("camPos", camera->Position);
			computeShader->setVec3("camOrientation", camera->Orientation);
			computeShader->setMat4("view", camera->view);
			computeShader->setMat4("invView", glm::inverse(camera->view));
			computeShader->setFloat("aspect", stuff::screenSize.x / stuff::screenSize.y);
			computeShader->setFloat("fov", camera->FOVdeg);
			//computeShader->setVec3("lightDir", glm::normalize(glm::vec3(cos(time), 0.75, sin(time))));
			computeShader->setVec3("lightDir", glm::normalize(glm::vec3(1.f, 0.75f, -0.5f)));

			glDispatchCompute((TEXTURE_WIDTH + 15) / 16, (TEXTURE_HEIGHT + 15) / 16, 1); // simple strat to make somethiung like width = 500 into 512 which is divisible by 16
			// make sure writing to image has finished before read
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			quadShader->Activate();
			quadShader->setInt("tex", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			renderQuad();
		} else {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shader->Activate();
			shader->setVec3("lightDir", glm::normalize(glm::vec3(cos(time), 0.75, sin(time))));
			Draw(shader);
		}

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void Main::Start() {
	shader = new Shader("shader.vert", "shader.frag");
	quadShader = new Shader("quadShader.vert", "quadShader.frag");
	computeShader = new ComputeShader("shader.comp");

	camera = new Camera(stuff::screenSize.x, stuff::screenSize.y, glm::vec3(0, 0, 5));
	sphere = new Model("models/sphere/sphere.gltf");
	cube = new Model("models/cube/cube.gltf");
	sphere->setPos(glm::vec3(2, 2, 2));
	sphereFlat = new Model("models/sphereFlat/sphere.gltf");
	sphereFlat->setPos(glm::vec3(2, 2, -2));
}

void Main::Update(float deltaTime) {
	camera->Update(deltaTime, window, shader);
}

void Main::Draw(Shader* shader) {
	shader->Activate();
	cube->Draw(shader, camera);
	sphere->Draw(shader, camera);
}