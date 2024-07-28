#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "cglm/include/cglm/affine.h"
#include "cglm/include/cglm/cglm.h"
#include "cglm/include/cglm/mat4.h"
#include <assert.h>
#include <math.h>

void window_resize(GLFWwindow * window, int idth, int height);
GLFWwindow * create_window();
void render_loop_start(GLFWwindow * window);
void render_loop_end(GLFWwindow * window);
void process_input(GLFWwindow * window);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);

vec3 eye = {0.0,0.0,3.0};
vec3 up = {0.0, 1.0, 0.0};
vec3 front = {0.0, 0.0, -1.0};
mat4 look_at;
mat4 perspective;
vec3 target;
mat4 identity;
mat4 translate; 

int main() {
	glm_mat4_identity(identity);;
	glm_mat4_copy(identity, translate);
	glm_translate(translate, (vec3){4.0,2.0,-2.0});
	glm_scale_uni(translate, 0.5);	

	GLFWwindow * window = create_window();

	//for now just render something. That way you can start worrying about the blocks, textures, cameras etc. one at at a time.
	unsigned int VAO;
	glGenVertexArrays(1,&VAO);
	glBindVertexArray(VAO);

	float vertices [] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	unsigned int VBO;
	glGenBuffers(1,&VBO);	
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void *)0);
	glEnableVertexAttribArray(0);

	int success;

	const char * vertex_shader_source= "#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"uniform mat4 transform;\n"
	"uniform mat4 translate;\n"
	"uniform vec4 input_color;\n"
	"out vec4 material_color;\n"
	"void main() {\n"
	"material_color = input_color;\n"
	"gl_Position = transform*translate*vec4(aPos, 1.0);\n"
	"}\0";

	const char * fragment_shader_source = "#version 330 core\n"
	"in vec4 material_color;\n"
	"out vec4 FragColor;\n"
	"void main() {\n"
	"FragColor = vec4(material_color);}\0";

	unsigned int vertex_shader  = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader  , 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader,GL_COMPILE_STATUS,&success);
	assert(success);
	
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader,GL_COMPILE_STATUS,&success);
	assert(success);

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, fragment_shader);
	glAttachShader(shader_program, vertex_shader);
	glLinkProgram(shader_program);
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	assert(success);

	glDeleteShader(vertex_shader);	
	glDeleteShader(fragment_shader);	
	
	glUseProgram(shader_program);
	int transform_loc = glGetUniformLocation(shader_program,"transform");
	int translate_loc = glGetUniformLocation(shader_program,"translate");
	int color_loc = glGetUniformLocation(shader_program,"input_color");
	
	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window)) {
		render_loop_start(window);
		process_input(window);
		
		glBindVertexArray(VAO);
		glUseProgram(shader_program);

		//draw the first cube 
		glUniformMatrix4fv(transform_loc,1,GL_FALSE, (float *)look_at);
		glUniformMatrix4fv(translate_loc,1,GL_FALSE, (float *)identity);
		glUniform4f(color_loc, 0.0,0.2,0.6,0.8);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		glUniformMatrix4fv(transform_loc,1,GL_FALSE, (float *)look_at);
		glUniformMatrix4fv(translate_loc,1,GL_FALSE, (float *)translate);
		glUniform4f(color_loc, 0.8,0.2,0.2,0.8);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		render_loop_end(window);
	}
}

void mouse_callback(GLFWwindow * window, double xpos, double ypos) {
	static int first_mouse = true;

	static float lastX = 400;
	static float lastY = 400;
	static float pitch = 0;
	static float yaw = 0;
	float xoffset = (xpos - lastX) * 0.01;
	float yoffset = (ypos - lastY) * 0.01;

	if (first_mouse) {
		lastX = xpos;
		lastY = ypos;
		first_mouse = false;
	}

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0)
		pitch = 89.0;
	if (pitch < -89.0)
		pitch = -89.0;

	vec3 direction;
	direction[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
	direction[1] = -glm_rad(pitch);
	direction[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
	glm_normalize_to(direction, front);

	lastX = xpos;
	lastY = ypos;
}

void process_input(GLFWwindow * window) {
	glfwPollEvents();

	float current_time = glfwGetTime();
	static float previous_time = 0.0;

	float delta_time = current_time- previous_time;
	previous_time = current_time;
	
	float camera_speed = 2.5f * delta_time;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, 1);

	vec3 forwards = {front[0],0,front[2]};
	glm_normalize(forwards);
	
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		glm_vec3_scale(forwards, camera_speed, forwards);
		glm_vec3_add(eye,forwards,eye);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		glm_vec3_scale(forwards, -camera_speed, forwards);
		glm_vec3_add(eye,forwards,eye);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		glm_vec3_scale(forwards, camera_speed, forwards);
		glm_cross(forwards, (vec3) {0.0,1.0,0.0}, forwards);
		glm_vec3_add(eye,forwards,eye);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		glm_vec3_scale(forwards, -camera_speed, forwards);
		glm_cross(forwards, (vec3) {0.0,1.0,0.0}, forwards);
		glm_vec3_add(eye,forwards,eye);
	}
	
	glm_vec3_add(eye, front, target);
	glm_lookat(eye, target, up, look_at);
	glm_perspective(glm_rad(45.0), (float)16/(float)9, 0.1, 100.0f, perspective);
	glm_mul(perspective, look_at, look_at);
}

void render_loop_end(GLFWwindow * window) {	
	glfwSwapBuffers(window);
}

void render_loop_start(GLFWwindow * window) {
	process_input(window);
	glClearColor(0.0, 0.0, 0.0, 0.9);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void window_resize(GLFWwindow * window, int width, int height) {
	glViewport(0, 0, width, height);	
}

GLFWwindow * create_window() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	
	GLFWwindow* window = glfwCreateWindow(1920,1080,"Main", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, window_resize);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwPollEvents();
	glViewport(0, 0, 1920, 1080);	

	glewExperimental = GL_TRUE;
	glewInit();

	return window;
}
