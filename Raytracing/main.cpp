#include "main.h"
#include "Camera.h"
#include "model.h"
#include "computeShader.h"

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

	std::vector<glm::vec4> positionList;
	std::vector<GLuint> indiceList = cube->getMeshes()[0].indices;
	for (const Vertex vertex : cube->getMeshes()[0].vertices) {
		positionList.push_back(glm::vec4(vertex.position, 1.f));
	}

	// setup SSBOs
	GLuint vertexSSBO;
	glGenBuffers(1, &vertexSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
	// allocate and upload data
	glBufferData(GL_SHADER_STORAGE_BUFFER, positionList.size() * sizeof(glm::vec4), positionList.data(), GL_STATIC_DRAW);
	// bind ssbo to binding point 0
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	GLuint indicesSSBO;
	glGenBuffers(1, &indicesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indicesSSBO);
	// allocate and upload data
	glBufferData(GL_SHADER_STORAGE_BUFFER, indiceList.size() * sizeof(unsigned int), indiceList.data(), GL_STATIC_DRAW);
	// bind ssbo to binding point 0
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indicesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

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
			computeShader->setMat4("model", cube->getMatricesMeshes()[0]);
			computeShader->setVec3("camPos", camera->Position);
			computeShader->setVec3("camOrientation", camera->Orientation);
			computeShader->setMat4("view", camera->view);
			computeShader->setMat4("invView", glm::inverse(camera->view));
			computeShader->setFloat("aspect", stuff::screenSize.x / stuff::screenSize.y);
			computeShader->setFloat("fov", camera->FOVdeg);
			computeShader->setVec3("lightDir", glm::normalize(glm::vec3(1, 0.75, -.5f)));

			glDispatchCompute((TEXTURE_WIDTH + 15) / 16, (TEXTURE_HEIGHT + 15) / 16, 1); // simple strat to make somethiung like width = 500 into 512 which is divisible by 16
			// make sure writing to image has finished before read
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			quadShader->Activate();
			quadShader->setInt("tex", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			renderQuad();
		} else {
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shader->Activate();
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

	GLint data[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &data[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &data[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &data[2]);
	std::cout << "data: " << data[0] << ", " << data[1] << ", " << data[2] << "\n";

	//camera = new Camera(stuff::screenSize.x, stuff::screenSize.y, glm::vec3(6, 5, 6));
	camera = new Camera(stuff::screenSize.x, stuff::screenSize.y, glm::vec3(0, 0, 5));
	//cube = new Model("models/bunny/scene.gltf");
	cube = new Model("models/cube.gltf");
}

bool RayIntersectsTriangle(const glm::vec3& ray_origin,
	const glm::vec3& ray_vector,
	glm::vec3& p1,
	glm::vec3& p2,
	glm::vec3& p3,
	glm::vec3& hitLoc)
{
	p1 = -p1;
	p2 = -p2;
	p3 = -p3;

	constexpr float epsilon = std::numeric_limits<float>::epsilon();

	glm::vec3 edge1 = p2 - p1;
	glm::vec3 edge2 = p3 - p1;
	glm::vec3 ray_cross_e2 = cross(ray_vector, edge2);
	float det = glm::dot(edge1, ray_cross_e2);

	if (det > -epsilon && det < epsilon)
		return false;    // This ray is parallel to this triangle.

	float inv_det = 1.0 / det;
	glm::vec3 s = ray_origin - p1;
	float u = inv_det * glm::dot(s, ray_cross_e2);

	if ((u < 0 && abs(u) > epsilon) || (u > 1 && abs(u - 1) > epsilon))
		return false;

	glm::vec3 s_cross_e1 = cross(s, edge1);
	float v = inv_det * glm::dot(ray_vector, s_cross_e1);

	if ((v < 0 && abs(v) > epsilon) || (u + v > 1 && abs(u + v - 1) > epsilon))
		return false;

	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = inv_det * glm::dot(edge2, s_cross_e1);

	if (t > epsilon) { // ray intersection
		hitLoc = glm::vec3(ray_origin + ray_vector * t);
		return true;
	} else // This means that there is a line intersection but not a ray intersection.
		return false;
}

void Main::Update(float deltaTime) {
	camera->Update(deltaTime, window, shader);
}

void Main::Draw(Shader* shader) {
	shader->Activate();

	shader->setVec3("lightDir", glm::normalize(glm::vec3(1, 0.75, -.5f)));
	cube->Draw(shader, camera);
}