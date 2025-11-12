#include "main.h"
#include "Camera.h"
#include "model.h"
#include "computeShader.h"
#include "Box.h"

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

	window = glfwCreateWindow(screenSize.x, screenSize.y, "Raytracing", nullptr, nullptr);

	if (window == nullptr) {
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, Main::FramebufferSizeCallback);

	if (!gladLoadGL()) {
		return -1;
	}

	glViewport(0, 0, screenSize.x, screenSize.y);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Start();

	// setup quad texture for Compute shader to write on
	glGenTextures(1, &quadTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, quadTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, quadTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindTexture(GL_TEXTURE_2D, 0);

	setupQuad();

	computeShader->Activate();
	std::vector<GPUBoundingBox> BVHList = Model::BVH();

	// build box list for visual
	for (int i = 0; i < BVHList.size(); i++) {
		GPUBoundingBox& boundingBox = BVHList[i];
		Box* box = new Box(boundingBox.minLoc, boundingBox.maxLoc);
		if (boundingBox.isLeaf) // second half of list, meaning its last node
			box->setColor(glm::vec3(0, 1, 1));
	}

	auto lastTime = std::chrono::steady_clock::now();
	float time = 0;
	while (!glfwWindowShouldClose(window)) {
		auto currentTime = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;
		time += deltaTime;
		std::cout << "fps: " << 1.f / deltaTime << "\n"; // 300, 2000, 3000

		glfwPollEvents();

		Update(deltaTime);

		if (true) { // whether or not should render using computeShader
			computeShader->Activate();
			computeShader->setMat4("projection", camera->cameraMatrix);
			computeShader->setMat4("invProj", glm::inverse(camera->cameraMatrix));
			computeShader->setVec3("camPos", camera->Position);

			glm::vec3 camForward = glm::normalize(camera->Orientation);
			glm::vec3 camRight = glm::normalize(glm::cross(camForward, glm::vec3(0.f, 1.f, 0.f)));
			glm::vec3 camUp = glm::cross(camRight, camForward);
			computeShader->setVec3("camForward", camForward);
			computeShader->setVec3("camRight", camRight);
			computeShader->setVec3("camUp", camUp);
			
			computeShader->setMat4("view", camera->view);
			computeShader->setMat4("invView", glm::inverse(camera->view));
			computeShader->setFloat("aspect", screenSize.x / screenSize.y);
			computeShader->setFloat("fov", camera->FOVdeg);
			//computeShader->setVec3("lightDir", glm::normalize(glm::vec3(cos(time), 0.75, sin(time))));
			computeShader->setVec3("lightDir", glm::normalize(glm::vec3(1.f, 0.75f, -0.5f)));
			
			glDispatchCompute((screenSize.x + 15) / 16, (screenSize.y + 15) / 16, 1); // simple strat to make somethiung like width = 500 into 512 which is divisible by 16
			// make sure writing to image has finished before read
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			quadShader->Activate();
			quadShader->setInt("tex", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, quadTexture);
			renderQuad();

			//shader->Activate();
			//Draw(shader);
		} else {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shader->setVec3("lightDir", glm::normalize(glm::vec3(cos(time), 0.75, sin(time))));
			Draw(shader);
		}

		GLenum error;
		while ((error = glGetError()) != GL_NO_ERROR)
			std::cout << "OpenGL Error: " << error << std::endl;

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

	camera = new Camera(screenSize.x, screenSize.y, glm::vec3(0, 0, 5));
	sphere = new Model("models/sphere/sphere.gltf");
	//sphere = new Model("models/bunny/3k.gltf");
	cube = new Model("models/cube/cube.gltf");
	sphereFlat = new Model("models/sphere1/sphere.gltf");
	new Model("models/glassSphere/sphere.gltf");
}

void Main::Update(float deltaTime) {
	shader->Activate();
	camera->Update(deltaTime, window, shader, screenSize);
}

void Main::Draw(Shader* shader) {
	shader->Activate();
	camera->Matrix(shader, "camMatrix");
	shader->setMat4("model", glm::mat4(1));
	shader->setMat4("translation", glm::mat4(1));
	shader->setMat4("rotation", glm::mat4_cast(glm::quat(1, 0, 0, 0)));
	shader->setMat4("scale", glm::mat4(1));

	for (Box* box : Box::instances)
		box->draw(shader);
	//for (Model* model : Model::instances)
		//model->Draw(shader, camera);
}

void Main::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	screenSize = { float(width), float(height) };
	glfwSetWindowSize(window, screenSize.x, screenSize.y);

	// update texuture size
	glViewport(0, 0, screenSize.x, screenSize.y);
	glBindTexture(GL_TEXTURE_2D, quadTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindImageTexture(0, quadTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindTexture(GL_TEXTURE_2D, 0);
}