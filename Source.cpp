#include <math.h>
#include <vector>
#include <time.h>
#include <glutil.h>
#include <figures.h>
#include <camera.h>

#include <files.hpp>
#include <model.hpp>
//Windows things
const u32 FSIZE = sizeof(f32);
const u32 ISIZE = sizeof(u32);
const u32 SCRWIDTH = 1280;
const u32 SCRHEIGHT = 720;
const f32 ASPECT = (f32)SCRWIDTH / (f32)SCRHEIGHT;
//Ligths things
glm::vec3 lightPos(0.0f, 2.0f, -0.0f);
glm::vec3 lightColor = glm::vec3(1.0f);
//Camera things
Cam* cam;
f32 lastx;
f32 lasty;
f32 deltaTime = 0.0f;
f32 lastFrame = 0.0f;
bool firstmouse = true;
bool wireframe = false;

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cam->processKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cam->processKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cam->processKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cam->processKeyboard(RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		wireframe = true;
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		wireframe = false;
	}
}
//Movement of Camera 
void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (firstmouse) {
			lastx = xpos;
			lasty = ypos;
			firstmouse = false;
			return;
		}
		cam->processMouse((f32)(xpos - lastx), (f32)(lasty - ypos));
		lastx = xpos;
		lasty = ypos;
	}
	else {
		firstmouse = true;
	}
}

void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset) {
	cam->processScroll((f32)yoffset);
}

i32 main() {	
	srand(time(NULL));
	GLFWwindow* window = glutilInit(3, 3, SCRWIDTH, SCRHEIGHT, "TF_CG");
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	Shader* lightingShader = new Shader("shader.vert", "shader.frag");
	Shader* lightCubeShader = new Shader("shader2.vert", "shader2.frag");
	cam = new Cam();
	Cube* cubex = new Cube();
	Files* files = new Files("bin", "resources/textures", "resources/objects");
	ShaderModel* shader = new ShaderModel(files, "shader_modelo.vert", "shader_modelo.frag");
	Model* modelo = new Model(files, "dog/dog.obj");

	//Land creation
	i32 n = 50;
	std::vector<glm::vec3> positions(n * n);
	for (u32 i = 0; i < n; ++i) {
		for (u32 j = 0; j < n; ++j) {
			f32 x = i - n / 2.0f;
			f32 z = j - n / 2.0f;
			f32 y = 0.0;
			positions[i * n + j] = glm::vec3(x, y, z);
		}
	}
	//Set up buffers on the GPU
	u32 cubeVao, lightCubeVao, vbo, ebo;
	glGenVertexArrays(1, &cubeVao);
	glGenVertexArrays(1, &lightCubeVao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(cubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBufferData(GL_ARRAY_BUFFER, cubex->getVSize() * FSIZE,
		cubex->getVertices(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubex->getISize() * ISIZE,
		cubex->getIndices(), GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(6));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(9));
	glEnableVertexAttribArray(2);

	glBindVertexArray(lightCubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);

	unsigned int texture = lightingShader->loadTexture("texture2.jpg");
	//Setup Rendering
	while (!glfwWindowShouldClose(window)) {
		f32 currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::perspective(cam->getZoom(), ASPECT, 0.1f, 100.0f);
		lightingShader->useProgram();
		glm::mat4 model = glm::mat4(1.0f);
		glBindVertexArray(cubeVao);

		for (u32 i = 0; i < positions.size(); ++i) {

			glBindTexture(GL_TEXTURE_2D, texture);
			lightingShader->setVec3("xyzMat.specular", 0.19f, 0.19f, 0.19f);
			lightingShader->setF32("xyzMat.shininess", 4.0f);

			lightingShader->setVec3("xyzLht.position", lightPos);
			lightingShader->setVec3("xyz", cam->getPos());
			lightingShader->setVec3("xyzLht.ambient", 0.2f, 0.2f, 0.2f);
			lightingShader->setVec3("xyzLht.diffuse", 0.5f, 0.5f, 0.5f);
			lightingShader->setVec3("xyzLht.specular", 1.0f, 1.0f, 1.0f);

			lightingShader->setMat4("proj", proj);
			lightingShader->setMat4("view", cam->getViewM4());

			model = glm::mat4(1.0f);
			model = glm::translate(model, positions[i]);
			lightingShader->setMat4("model", model);
			glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);
		}

		shader->use();
		shader->setVec3("xyz", lightPos);
		shader->setVec3("xyzColor", lightColor);
		shader->setVec3("xyzView", cam->getPos());
		shader->setMat4("proj", proj);
		shader->setMat4("view", cam->getViewM4());
		model = glm::mat4(1.0f);
		model = translate(model, glm::vec3(sin(currentFrame), 5, cos(currentFrame)));//Movement
		model = rotate(model, currentFrame, glm::vec3(0.0f, 0.50f, 0.0f));//Rotation
		model = glm::scale(model, glm::vec3(0.3));
		shader->setMat4("model", model);
		modelo->Draw(shader);

		lightCubeShader->useProgram();
		lightCubeShader->setMat4("proj", proj);
		lightCubeShader->setMat4("view", cam->getViewM4());
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.05));
		lightCubeShader->setMat4("model", model);

		glBindVertexArray(lightCubeVao);
		glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	};
	//Delete buffers for GPU
	glDeleteVertexArrays(1, &cubeVao);
	glDeleteVertexArrays(1, &lightCubeVao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	//Delete the rest
	delete lightingShader;
	delete lightCubeShader;
	delete cubex;
	delete cam;
	delete shader;
	delete modelo;
	return 0;
}