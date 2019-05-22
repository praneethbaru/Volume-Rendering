#define GLEW_STATIC
//#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <nanogui/nanogui.h>

#include <nanogui/common.h>
#include "Camera.h"
#include "Shader.h"
#include "ObjectR.h"
#include "stb_image.h"
#include "../externals/GLSLShader.h"



GLFWwindow* window;
nanogui::Screen* screen = nullptr;
//nanogui::VectorXf &func;
Camera* camera = new Camera();

//bool
bool enabled = true;
bool keys[1024];
bool bvar = true;
bool reset = true;
bool tfs = false;

//int
int ivar = 12345678;
int num_slices = 2000;
int indices[] = { 0,1,2, 0,2,3, 0,3,4, 0,4,5 };
const int MAX_SLICES = 2000;
int XDIM = 256;
int YDIM = 256;
int ZDIM = 178;
unsigned int texture;
GLuint VBO, VAO, EBO;
GLuint textureID;
GLuint colorbarMap;
GLuint tfTexID;

//float
float angleX = 0, angleY = 0;
float rX = 4, rY = 50, dist = -2;
float sampling = 0.0;
const float EPSILON = 0.0001f;
GLfloat angle, radians;
GLfloat delta_time = 0.0;
GLfloat last_frame = 0.0;
float s0 = 0.0f, s1 = 0.0f, s2 = 0.0f, s3 = 0.0f, s4 = 0.0f, s5 = 0.0f, s6 = 0.0f, s7 = 0.0f, s8 = 0.0f;
float alpha, a0, s = 5;

//string
std::string strval = "A string";
std::string colorbar = "./objs/colorbar.png";
std::string volume_file = "./objs/BostonTeapot_256_256_178.raw";
std::string color_bar_path = "./objs/colorbar.png";

//vectors
glm::vec3 viewDir;
glm::vec3 vTextureSlices[MAX_SLICES * 12];
glm::vec4 bg = glm::vec4(0.0, 0.0, 0.0, 1);


//shader
GLSLShader shader;

//color
nanogui::Color colval(0.5f, 0.5f, 0.5f, 1.f);

//enum
enum Render_type {
	points,
	lines,
	solid
};
enum Model_name {
	TEAPOT,
	BUCKY,
	BONSAI,
	HEAD,
};
Render_type render_val = solid;
Model_name model_name = TEAPOT;
Model_name old_model_name = model_name;

//Function declarations
bool loadVolume();
void draw_scene(GLSLShader& shader);
void SliceVolume(glm::vec3 viewDir);
int FindAbsMax(glm::vec3 v);
void setupNanogui();
void init();
void cameraMove();
void loadColorBar();
void loadColorBar2();
void getModelData(Model_name model_name);
void checkIfModelChanged();

//Cube data
GLfloat cube_vertices[24] = {
0.0, 0.0, 0.0,
0.0, 0.0, 1.0,
0.0, 1.0, 0.0,
0.0, 1.0, 1.0,
1.0, 0.0, 0.0,
1.0, 0.0, 1.0,
1.0, 1.0, 0.0,
1.0, 1.0, 1.0
};

GLuint cube_indices[36] = {
	1,5,7,
	3,6,7,
	0,2,6,
	2,4,0,
	0,1,3,
	3,2,1,
	7,5,6,
	4,6,7,
	1,3,7,
	7,5,1,
	1,0,4,
	4,5,0
};

GLfloat cube_tex_coords[] = {
	1.0, 1.0, 1.0,    //0
	1.0, 1.0, 0.0,    //1
	1.0, 0.0, 1.0,    //2
	1.0, 0.0, 0.0,    //3
	0.0, 1.0, 1.0,    //4
	0.0, 1.0, 0.0,    //5
	0.0, 0.0, 1.0,    //6
	0.0, 0.0, 0.0    //7
};

glm::vec3 vertexList[8] = { glm::vec3(-0.5,-0.5,-0.5),
						   glm::vec3(0.5,-0.5,-0.5),
						   glm::vec3(0.5, 0.5,-0.5),
						   glm::vec3(-0.5, 0.5,-0.5),
						   glm::vec3(-0.5,-0.5, 0.5),
						   glm::vec3(0.5,-0.5, 0.5),
						   glm::vec3(0.5, 0.5, 0.5),
						   glm::vec3(-0.5, 0.5, 0.5) };

//unit cube edges
int edgeList[8][12] = {
	{ 0,1,5,6,   4,8,11,9,  3,7,2,10 }, // v0 is front
	{ 0,4,3,11,  1,2,6,7,   5,9,8,10 }, // v1 is front
	{ 1,5,0,8,   2,3,7,4,   6,10,9,11}, // v2 is front
	{ 7,11,10,8, 2,6,1,9,   3,0,4,5  }, // v3 is front
	{ 8,5,9,1,   11,10,7,6, 4,3,0,2  }, // v4 is front
	{ 9,6,10,2,  8,11,4,7,  5,0,1,3  }, // v5 is front
	{ 9,8,5,4,   6,1,2,0,   10,7,11,3}, // v6 is front
	{ 10,9,6,5,  7,2,3,1,   11,4,8,0 }  // v7 is front
};
const int edges[12][2] = { {0,1},{1,2},{2,3},{3,0},{0,4},{1,5},{2,6},{3,7},{4,5},{5,6},{6,7},{7,4} };

//transfer function (lookup table) colour values
const glm::vec4 jet_values[9] = {
								glm::vec4(0,0,0.5,0),
								glm::vec4(0,0,1,0.1),
								glm::vec4(0,0.5,1,0.3),
								glm::vec4(0,1,1,0.5),
								glm::vec4(0.5,1,0.5,0.75),
								glm::vec4(1,1,0,0.8),
								glm::vec4(1,0.5,0,0.6),
								glm::vec4(1,0,0,0.5),
								glm::vec4(0.5,0,0,0.0) };

//main function starts
int main() {

	setupNanogui();
	init();



	while (!glfwWindowShouldClose(window))
	{
		//Listen to GUI inputs
		glfwPollEvents();

		//check if a new model has been selected
		checkIfModelChanged();
		//checkIfSliderChanged();

		//camera movement
		cameraMove();

		//Render
		draw_scene(shader);
		glBindVertexArray(VAO);

		//draw nanogui
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		screen->drawWidgets();

		//glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}

void draw_scene(GLSLShader& shader)
{
	//set background colour
	glClearColor(bg.r, bg.g, bg.b, bg.a);

	glm::mat4 M = glm::mat4();
	glm::mat4 V = camera->get_view_mat();
	glm::mat4 P = camera->get_projection_mat();

	//glDeleteBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//get the viewing direction
	viewDir = camera->front;
	SliceVolume(viewDir);

	camera->get_projection_mat();

	//clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//get the combined modelview projection matrix
	glm::mat4 MVP = P * V * M;
	glDisable(GL_DEPTH_TEST);

	//enable alpha blending (use over operator)
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//bind volume vertex array object
	glBindVertexArray(VAO);


	loadColorBar();
	shader.Use();

	//pass the shader uniform
	glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform3f(glGetUniformLocation(shader._program, "color"), colval[0], colval[1], colval[2]);
	glUniform1f(glGetUniformLocation(shader._program, "tfs"), tfs);

	//for TFS true
	if (tfs)
	{

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, sizeof(vTextureSlices) / sizeof(vTextureSlices[0]));
	}

	//for TFS false
	if (!tfs)
	{
		//points
		if (render_val == 0) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			glDrawArrays(GL_TRIANGLES, 0, sizeof(vTextureSlices) / sizeof(vTextureSlices[0]));
		}

		//lines
		if (render_val == 1) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawArrays(GL_TRIANGLES, 0, sizeof(vTextureSlices) / sizeof(vTextureSlices[0]));
		}

		//triangles
		if (render_val == 2) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, sizeof(vTextureSlices) / sizeof(vTextureSlices[0]));
		}

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glDrawArrays(GL_TRIANGLES, 0, sizeof(vTextureSlices) / sizeof(vTextureSlices[0]));
	}

	//unbind the shader
	shader.UnUse();

	//disable blending
	glDisable(GL_BLEND);
}

//function that load a volume from the given raw data file and
//generates an OpenGL 3D texture from it
bool loadVolume() {
	std::ifstream infile(volume_file.c_str(), std::ios_base::binary);

	if (infile.good()) {
		//read the volume data file
		GLubyte* pData = new GLubyte[XDIM*YDIM*ZDIM];
		infile.read(reinterpret_cast<char*>(pData), XDIM*YDIM*ZDIM * sizeof(GLubyte));
		infile.close();

		//generate OpenGL texture
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_3D, textureID);

		// set the texture parameters
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//set the mipmap levels (base and max)
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);

		//allocate data with internal format and foramt as (GL_RED)       
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, XDIM, YDIM, ZDIM, 0, GL_RED, GL_UNSIGNED_BYTE, pData);

		//generate mipmaps
		glGenerateMipmap(GL_TEXTURE_3D);
		glFinish();

		//delete the volume data allocated on heap
		delete[] pData;
		return true;
	}
	else
	{
		return false;
	}
}

//main slicing function
void SliceVolume(glm::vec3 viewDir) {
	//get the max and min distance of each vertex of the unit cube
	//in the viewing direction
	float max_dist = glm::dot(viewDir, vertexList[0]);
	float min_dist = max_dist;
	int max_index = 0;
	int count = 0;

	for (int i = 1; i < 8; i++) {
		//get the distance between the current unit cube vertex and
		//the view vector by dot product
		float dist = glm::dot(viewDir, vertexList[i]);

		//if distance is > max_dist, store the value and index
		if (dist > max_dist) {
			max_dist = dist;
			max_index = i;
		}

		//if distance is < min_dist, store the value
		if (dist < min_dist)
			min_dist = dist;
	}
	//find tha abs maximum of the view direction vector
	int max_dim = FindAbsMax(viewDir);

	//expand it a little bit
	min_dist -= EPSILON;
	max_dist += EPSILON;

	//local variables to store the start, direction vectors,
	//lambda intersection values
	glm::vec3 vecStart[12];
	glm::vec3 vecDir[12];
	float lambda[12];
	float lambda_inc[12];
	float denom = 0;

	//set the minimum distance as the plane_dist
	//subtract the max and min distances and divide by the
	//total number of slices to get the plane increment
	float plane_dist = min_dist;
	float plane_dist_inc = (max_dist - min_dist) / float(num_slices);

	//for all edges
	for (int i = 0; i < 12; i++) {
		//get the start position vertex by table lookup
		vecStart[i] = vertexList[edges[edgeList[max_index][i]][0]];

		//get the direction by table lookup
		vecDir[i] = vertexList[edges[edgeList[max_index][i]][1]] - vecStart[i];

		//do a dot of vecDir with the view direction vector
		denom = glm::dot(vecDir[i], viewDir);

		//determine the plane intersection parameter (lambda) and
		//plane intersection parameter increment (lambda_inc)
		if (1.0 + denom != 1.0) {
			lambda_inc[i] = plane_dist_inc / denom;
			lambda[i] = (plane_dist - glm::dot(vecStart[i], viewDir)) / denom;
		}
		else {
			lambda[i] = -1.0;
			lambda_inc[i] = 0.0;
		}
	}

	//local variables to store the intesected points
	//note that for a plane and sub intersection, we can have
	//a minimum of 3 and a maximum of 6 vertex polygon
	glm::vec3 intersection[6];
	float dL[12];

	for (int i = 0; i < MAX_SLICES * 12; i++)
	{
		vTextureSlices[i].x = 0.0f;
		vTextureSlices[i].y = 0.0f;
		vTextureSlices[i].z = 0.0f;
	}


	//loop through all slices
	for (int i = num_slices - 1; i >= 0; i--)
	{

		//determine the lambda value for all edges
		for (int e = 0; e < 12; e++)
		{
			dL[e] = lambda[e] + i * lambda_inc[e];
		}

		//if the values are between 0-1, we have an intersection at the current edge
		//repeat the same for all 12 edges
		if ((dL[0] >= 0.0) && (dL[0] < 1.0)) {
			intersection[0] = vecStart[0] + dL[0] * vecDir[0];
		}
		else if ((dL[1] >= 0.0) && (dL[1] < 1.0)) {
			intersection[0] = vecStart[1] + dL[1] * vecDir[1];
		}
		else if ((dL[3] >= 0.0) && (dL[3] < 1.0)) {
			intersection[0] = vecStart[3] + dL[3] * vecDir[3];
		}
		else continue;

		if ((dL[2] >= 0.0) && (dL[2] < 1.0)) {
			intersection[1] = vecStart[2] + dL[2] * vecDir[2];
		}
		else if ((dL[0] >= 0.0) && (dL[0] < 1.0)) {
			intersection[1] = vecStart[0] + dL[0] * vecDir[0];
		}
		else if ((dL[1] >= 0.0) && (dL[1] < 1.0)) {
			intersection[1] = vecStart[1] + dL[1] * vecDir[1];
		}
		else {
			intersection[1] = vecStart[3] + dL[3] * vecDir[3];
		}

		if ((dL[4] >= 0.0) && (dL[4] < 1.0)) {
			intersection[2] = vecStart[4] + dL[4] * vecDir[4];
		}
		else if ((dL[5] >= 0.0) && (dL[5] < 1.0)) {
			intersection[2] = vecStart[5] + dL[5] * vecDir[5];
		}
		else {
			intersection[2] = vecStart[7] + dL[7] * vecDir[7];
		}
		if ((dL[6] >= 0.0) && (dL[6] < 1.0)) {
			intersection[3] = vecStart[6] + dL[6] * vecDir[6];
		}
		else if ((dL[4] >= 0.0) && (dL[4] < 1.0)) {
			intersection[3] = vecStart[4] + dL[4] * vecDir[4];
		}
		else if ((dL[5] >= 0.0) && (dL[5] < 1.0)) {
			intersection[3] = vecStart[5] + dL[5] * vecDir[5];
		}
		else {
			intersection[3] = vecStart[7] + dL[7] * vecDir[7];
		}
		if ((dL[8] >= 0.0) && (dL[8] < 1.0)) {
			intersection[4] = vecStart[8] + dL[8] * vecDir[8];
		}
		else if ((dL[9] >= 0.0) && (dL[9] < 1.0)) {
			intersection[4] = vecStart[9] + dL[9] * vecDir[9];
		}
		else {
			intersection[4] = vecStart[11] + dL[11] * vecDir[11];
		}

		if ((dL[10] >= 0.0) && (dL[10] < 1.0)) {
			intersection[5] = vecStart[10] + dL[10] * vecDir[10];
		}
		else if ((dL[8] >= 0.0) && (dL[8] < 1.0)) {
			intersection[5] = vecStart[8] + dL[8] * vecDir[8];
		}
		else if ((dL[9] >= 0.0) && (dL[9] < 1.0)) {
			intersection[5] = vecStart[9] + dL[9] * vecDir[9];
		}
		else {
			intersection[5] = vecStart[11] + dL[11] * vecDir[11];
		}

		//after all 6 possible intersection vertices are obtained,
		//we calculated the proper polygon indices by using indices of a triangular fan
		int indices[] = { 0,1,2, 0,2,3, 0,3,4, 0,4,5 };

		//Using the indices, pass the intersection vertices to the vTextureSlices vector
		for (int j = 0; j < 12; j++)
		{
			vTextureSlices[count++] = intersection[indices[j]];
			int c = count;
			//printf("%f, %f, %f\n", intersection[indices[j]].x, intersection[indices[j]].y, intersection[indices[j]].z);
		}
		//printf("----------------------------------------------------------------\n");
	}

	//update buffer object with the new vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vTextureSlices), &(vTextureSlices[0].x));
}

void loadColorBar()
{
	float pData[256][4];
	int indices[9];

	alpha = 1 - (glm::pow((1 - 0.7), (s / num_slices)));
	//slider 0
	for (int i = 0; i < 32; i++)
	{
		pData[i][0] = 0.0f;
		pData[i][1] = 0.0f;
		pData[i][2] = s0;
		pData[i][3] = s0*alpha;
	}
	 
	//slider 1
	for (int i = 32; i < 64; i++)
	{
		//alpha = 1 - (glm::pow((1 - s1), (s / num_slices)));
		pData[i][0] = s1 * 0.5;
		pData[i][1] = s1 * 0.5;
		pData[i][2] = s1  ;
		pData[i][3] = s1 * alpha;
	}

	//slider 2
	for (int i = 64; i < 96; i++)
	{
		pData[i][0] = s2 * 0.6;
		pData[i][1] = s2 * 0.7;
		pData[i][2] = s2 * 1.5;
		pData[i][3] = alpha * s2;
	}

	//slider 3
	for (int i = 96; i < 128; i++)
	{
		pData[i][0] = s3 * 0.8;
		pData[i][1] = s3 * 0.8;
		pData[i][2] = s3 * 1.0;
		pData[i][3] = alpha * s3;
	}

	//slider 4
	for (int i = 128; i < 160; i++)
	{
		pData[i][0] = s4;
		pData[i][1] = s4 * 0.7;
		pData[i][2] = s4 * 0.7;
		pData[i][3] = alpha * s4;
	}

	//slider 5
	for (int i = 160; i < 192; i++)
	{
		pData[i][0] = s5;
		pData[i][1] = 0.2;
		pData[i][2] =  0.1;
		pData[i][3] = alpha * s5;
	}

	//slider 6
	for (int i = 192; i < 224; i++)
	{
		pData[i][0] = s6;
		pData[i][1] = 0.75* s6;
		pData[i][2] = 0.75 * s6;
		pData[i][3] = alpha * s6 ;
	}

	//slider 7
	for (int i = 224; i < 256; i++)
	{
		pData[i][0] = s7;
		pData[i][1] = 0.0f;
		pData[i][2] = 0.0f;
		pData[i][3] = alpha * s7;
	}

	//generate the OpenGL texture
	glGenTextures(1, &tfTexID);
	//bind this texture to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, tfTexID);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//allocate the data to texture memory. Since pData is on stack, we donot delete it
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, pData);

}

//loading the color bar
void loadColorBar2()
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, textureID);

	int width, height, nrComponents;

	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(colorbar.c_str(), &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		// set the texture parameters
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		//allocate the data to texture memory. Since pData is on stack, we donot delete it
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 3 * sizeof(float) * 4, GL_RGBA, GL_FLOAT, data);
		float pdata = data[256];
		printf("%f \n", pdata);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << std::endl;
		stbi_image_free(data);
	}
}

//function to get the max (abs) dimension of the given vertex v
int FindAbsMax(glm::vec3 v) {
	v = glm::abs(v);
	int max_dim = 0;
	float val = v.x;
	if (v.y > val) {
		val = v.y;
		max_dim = 1;
	}
	if (v.z > val) {
		val = v.z;
		max_dim = 2;
	}
	return max_dim;
}



void init()
{

	//Load the texture slicing shader
	shader.LoadFromFile(GL_VERTEX_SHADER, "shader/textureSlicer.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shader/textureSlicer.frag");

	//compile and link the shader
	shader.CreateAndLinkProgram();
	shader.Use();

	//add attributes and uniforms
	shader.AddAttribute("vVertex");
	shader.AddUniform("MVP");
	shader.AddUniform("volume");
	shader.AddUniform("color_tex");
	shader.AddUniform("tfs");

	//pass constant uniforms at initialization
	glUniform1i(shader("volume"), 0);
	glUniform1i(shader("color_tex"), 1);
	glUniform1i(shader("tfs"), tfs);
	shader.UnUse();

	//slice the volume dataset initially
	loadVolume();
	loadColorBar();

	//set background colour
	glClearColor(bg.r, bg.g, bg.b, bg.a);

	//setup the current camera transform and get the view direction vector
	glm::mat4 M = glm::mat4();
	glm::mat4 V = camera->get_view_mat();
	glm::mat4 P = camera->get_projection_mat();

	//get the viewing direction
	viewDir = camera->front;

	//setup the vertex array and buffer objects
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	//pass the sliced vertices vector to buffer object memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vTextureSlices), 0, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), 0, GL_STATIC_DRAW);

	//enable vertex attribute array for position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	SliceVolume(viewDir);
}

void cameraMove()
{
	GLfloat current_frame = glfwGetTime();
	delta_time = current_frame - last_frame;
	last_frame = current_frame;
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera->process_keyboard(FORWARD, delta_time);
	if (keys[GLFW_KEY_S])
		camera->process_keyboard(BACKWARD, delta_time);
	if (keys[GLFW_KEY_A])
		camera->process_keyboard(LEFT, delta_time);
	if (keys[GLFW_KEY_D])
		camera->process_keyboard(RIGHT, delta_time);
	if (keys[GLFW_KEY_Q])
		camera->process_keyboard(UP, delta_time);
	if (keys[GLFW_KEY_E])
		camera->process_keyboard(DOWN, delta_time);
	if (keys[GLFW_KEY_I])
		camera->process_keyboard(ROTATE_X_UP, delta_time);
	if (keys[GLFW_KEY_K])
		camera->process_keyboard(ROTATE_X_DOWN, delta_time);
	if (keys[GLFW_KEY_J])
		camera->process_keyboard(ROTATE_Y_UP, delta_time);
	if (keys[GLFW_KEY_L])
		camera->process_keyboard(ROTATE_Y_DOWN, delta_time);
	if (keys[GLFW_KEY_U])
		camera->process_keyboard(ROTATE_Z_UP, delta_time);
	if (keys[GLFW_KEY_O])
		camera->process_keyboard(ROTATE_Z_DOWN, delta_time);
}

void setupNanogui()
{
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	camera->init();

	window = glfwCreateWindow(1200, 700, "Assignment 1", nullptr, nullptr);

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();


	//NANOGUI PART
	screen = new nanogui::Screen();
	screen->initialize(window, true);

	glViewport(0, 0, camera->width, camera->height);

	nanogui::FormHelper *gui_1 = new nanogui::FormHelper(screen);
	nanogui::ref<nanogui::Window> nanoguiWindow_1 = gui_1->addWindow(Eigen::Vector2i(0, 0), "Nanogui control bar_1");

	gui_1->addGroup("Position");
	static auto cameraX = gui_1->addVariable("X", camera->position.x);
	cameraX->setSpinnable(true);
	static auto cameraY = gui_1->addVariable("Y", camera->position.y);
	cameraY->setSpinnable(true);
	static auto cameraZ = gui_1->addVariable("Z", camera->position.z);
	cameraZ->setSpinnable(true);

	gui_1->addGroup("Rotate");
	gui_1->addVariable("Rotate Value", radians)->setSpinnable(true);

	gui_1->addButton("Rotate right +", []()
	{
		camera->rotate_x(glm::radians(radians));
	});
	gui_1->addButton("Rotate right -", []()
	{
		camera->rotate_x(-glm::radians(radians));
	});
	gui_1->addButton("Rotate Up +", []()
	{
		camera->rotate_y(-glm::radians(radians));
	});
	gui_1->addButton("Rotate Up -", []()
	{
		camera->rotate_y(glm::radians(radians));
	});
	gui_1->addButton("Rotate front +", []()
	{
		camera->rotate_z(-glm::radians(radians));
	});
	gui_1->addButton("Rotate front-", []()
	{
		camera->rotate_z(glm::radians(radians));
	});

	gui_1->addVariable("Model name", model_name, enabled)->setItems({ "TEAPOT", "BUCKY", "BONSAI", "HEAD" });
	gui_1->addVariable("Render type", render_val, enabled)->setItems({ "points", "lines", "solid" });
	gui_1->addVariable("Colorbar image path", color_bar_path);

	gui_1->addButton("Reload model", []()
	{
		camera->reset();
	});
	gui_1->addButton("Reset", []()
	{
		camera->reset();
	});

	gui_1->addGroup("VOLUME RENDERING");
	gui_1->addVariable("Object Color", colval);
	gui_1->addVariable("TFS", tfs);

	gui_1->addVariable("Sampling rate", num_slices);

	//window 2
	nanogui::FormHelper *gui_2 = new nanogui::FormHelper(screen);
	nanogui::ref<nanogui::Window> nanoguiWindow_2 = gui_2->addWindow(Eigen::Vector2i(850, 100), "Nanogui control bar_2");

	nanogui::Widget *panel1 = new nanogui::Widget(nanoguiWindow_2);
	gui_2->addWidget("View Slider", panel1);
	panel1->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider *slider1 = new nanogui::Slider(panel1);
	slider1->setValue(camera->near);
	slider1->setFixedWidth(80);
	slider1->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox *textBox1 = new nanogui::TextBox(panel1);
	textBox1->setFixedSize(Eigen::Vector2i(100, 25));
	textBox1->setValue("0.0");
	slider1->setCallback([textBox1](float value) {
		textBox1->setValue(std::to_string((float)(value)));
		camera->near = 2.0f - value*0.5f;
		camera->far = 2.0f + value*0.5f;
	});

	//graph
	nanogui::Widget *graphWidget = new nanogui::Widget(nanoguiWindow_2);
	graphWidget->setHeight(100);

	gui_2->addWidget("Transfer function", graphWidget);
	nanogui::Graph *graph = graphWidget->add<nanogui::Graph>("Alpha");
	graph->setFixedHeight(100);
	nanogui::VectorXf &func = graph->values();
	func.resize(256);
	graph->setFooter("Intensity");

	nanogui::Widget *panel2 = new nanogui::Widget(nanoguiWindow_2);
	gui_2->addWidget("Slider 0", panel2);
	panel2->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider *slider2 = new nanogui::Slider(panel2);
	slider2->setValue(s0);
	slider2->setFixedWidth(80);
	slider2->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox *textBox2 = new nanogui::TextBox(panel2);
	textBox2->setFixedSize(Eigen::Vector2i(100, 25));
	textBox2->setValue("0.0");
	slider2->setCallback([textBox2, &func](float value) {
		textBox2->setValue(std::to_string((float)(value * 1)));
		s0 = value;
		func[0] = s0;
		func[32] = s1;

		for (int i = 1; i < 32; i++)
		{
			func[i] = s0 + (s1 - s0)*i / 32;
		}

	});


	nanogui::Widget *panel3 = new nanogui::Widget(nanoguiWindow_2);
	gui_2->addWidget("Slider 1", panel3);
	panel3->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider *slider3 = new nanogui::Slider(panel3);
	slider3->setValue(s1);
	slider3->setFixedWidth(80);
	slider3->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox *textBox3 = new nanogui::TextBox(panel3);
	textBox3->setFixedSize(Eigen::Vector2i(100, 25));
	textBox3->setValue("0.0");
	slider3->setCallback([textBox3, &func](float value) {
		textBox3->setValue(std::to_string((float)(value * 1)));
		s1 = value;
		func[0] = s0;
		func[32] = s1;
		func[64] = s2;
		
		//s0-s1
		for (int i = 1; i < 32; i++)
		{
			func[i] = s0 + (s1 - s0)*i / 32;
		}
		
		//s1-s2
		for (int i = 33; i < 63; i++)
		{
			func[i] = s1 + (s2 - s1)*(i - 32) / 32;
		}
		
	});
	nanogui::Widget *panel4 = new nanogui::Widget(nanoguiWindow_2);
	gui_2->addWidget("Slider 2", panel4);
	panel4->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider *slider4 = new nanogui::Slider(panel4);
	slider4->setValue(s2);
	slider4->setFixedWidth(80);
	slider4->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox *textBox4 = new nanogui::TextBox(panel4);
	textBox4->setFixedSize(Eigen::Vector2i(100, 25));
	textBox4->setValue("0.0");
	slider4->setCallback([textBox4, &func](float value) {
		textBox4->setValue(std::to_string((float)(value * 1)));
		s2 = value;
		func[32] = s1;
		func[64] = s2;
		func[96] = s3;

		//s1-s2
		for (int i = 33; i < 64; i++)
		{
			func[i] = s1 + (s2 - s1)*(i - 32) / 32;
		}

		//s2-s3
		for (int i = 65; i < 95; i++)
		{
			func[i] = s2 + (s3 - s2)*(i - 64) / 32;
		}

	});
	nanogui::Widget *panel5 = new nanogui::Widget(nanoguiWindow_2);
	gui_2->addWidget("Slider 3", panel5);
	panel5->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider *slider5 = new nanogui::Slider(panel5);
	slider5->setValue(s3);
	slider5->setFixedWidth(80);
	slider5->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox *textBox5 = new nanogui::TextBox(panel5);
	textBox5->setFixedSize(Eigen::Vector2i(100, 25));
	textBox5->setValue("0.0");
	slider5->setCallback([textBox5, &func](float value) {
		textBox5->setValue(std::to_string((float)(value * 1)));
		s3 = value;
		func[64] = s2;
		func[96] = s3;
		func[128] = s4;

		
		//s2-s3
		for (int i = 65; i < 96; i++)
		{
			func[i] = s2 + (s3 - s2)*(i - 64) / 32;
		}

		//s3-s4
		for (int i = 97; i < 128; i++)
		{
			func[i] = s3 + (s4 - s3)*(i - 96) / 32;
		}

	});

	nanogui::Widget *panel6 = new nanogui::Widget(nanoguiWindow_2);
	gui_2->addWidget("Slider 4", panel6);
	panel6->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider *slider6 = new nanogui::Slider(panel6);
	slider6->setValue(s4);
	slider6->setFixedWidth(80);
	slider6->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox *textBox6 = new nanogui::TextBox(panel6);
	textBox6->setFixedSize(Eigen::Vector2i(100, 25));
	textBox6->setValue("0.0");
	slider6->setCallback([textBox6, &func](float value) {
		textBox6->setValue(std::to_string((float)(value * 1)));
		s4 = value;
		func[96] = s3;
		func[128] = s4;
		func[160] = s5;

		//s3-s4
		for (int i = 97; i < 128; i++)
		{
			func[i] = s3 + (s4 - s3)*(i - 96) / 32;
		}

		//s4-s5
		for (int i = 129; i < 160; i++)
		{
			func[i] = s4 + (s5 - s4)*(i - 128) / 32;
		}
	});

	nanogui::Widget *panel7 = new nanogui::Widget(nanoguiWindow_2);
	gui_2->addWidget("Slider 5", panel7);
	panel7->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider *slider7 = new nanogui::Slider(panel7);
	slider7->setValue(s5);
	slider7->setFixedWidth(80);
	slider7->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox *textBox7 = new nanogui::TextBox(panel7);
	textBox7->setFixedSize(Eigen::Vector2i(100, 25));
	textBox7->setValue("0.0");
	slider7->setCallback([textBox7, &func](float value) {
		textBox7->setValue(std::to_string((float)(value * 1)));
		s5 = value;
		func[128] = s4;
		func[160] = s5;
		func[192] = s6;

		//s4-s5
		for (int i = 129; i < 160; i++)
		{
			func[i] = s4 + (s5 - s4)*(i - 128) / 32;
		}

		//s5-s6
		for (int i = 161; i < 192; i++)
		{
			func[i] = s5 + (s6 - s5)*(i - 160) / 32;
		}

	});
	nanogui::Widget *panel8 = new nanogui::Widget(nanoguiWindow_2);
	gui_2->addWidget("Slider 6", panel8);
	panel8->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider *slider8 = new nanogui::Slider(panel8);
	slider8->setValue(s6);
	slider8->setFixedWidth(80);
	slider8->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox *textBox8 = new nanogui::TextBox(panel8);
	textBox8->setFixedSize(Eigen::Vector2i(100, 25));
	textBox8->setValue("0.0");
	slider8->setCallback([textBox8, &func](float value) {
		textBox8->setValue(std::to_string((float)(value * 1)));
		s6 = value;
		func[160] = s5;
		func[192] = s6;
		func[225] = s7;
		
		//s5-s6
		for (int i = 161; i < 192; i++)
		{
			func[i] = s5 + (s6 - s5)*(i - 160) / 32;
		}

		//s6-s7
		for (int i = 193; i < 225; i++)
		{
			func[i] = s6 + (s7 - s6)*(i - 192) / 32;
		}
		

	});

	//slider 9
	nanogui::Widget *panel9 = new nanogui::Widget(nanoguiWindow_2);
	gui_2->addWidget("Slider 7", panel9);
	panel9->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 20));

	nanogui::Slider *slider9 = new nanogui::Slider(panel9);
	slider9->setValue(s7);
	slider9->setFixedWidth(80);
	slider9->setPosition(Eigen::Vector2i(250, 100));

	nanogui::TextBox *textBox9 = new nanogui::TextBox(panel9);
	textBox9->setFixedSize(Eigen::Vector2i(100, 25));
	textBox9->setValue("0.0");
	slider9->setCallback([textBox9, &func](float value) {
		textBox9->setValue(std::to_string((float)(value * 1)));
		s7 = value;
		func[192] = s6;
		func[225] = s7;
		func[255] = s8;
		
		//s6-s7
		for (int i = 193; i < 225; i++)
		{
			func[i] = s6 + (s7 - s6)*(i - 192) / 32;
		}

		//s7-s8
		for (int i = 226; i < 256; i++)
		{
			func[i] = s7 + (s8 - s7)*(i - 225) / 32;
		}


	});

	screen->setVisible(true);
	screen->performLayout();

	glfwSetCursorPosCallback(window,
		[](GLFWwindow *window, double x, double y) {
		screen->cursorPosCallbackEvent(x, y);
	}
	);

	glfwSetMouseButtonCallback(window,
		[](GLFWwindow *, int button, int action, int modifiers) {
		screen->mouseButtonCallbackEvent(button, action, modifiers);
		cameraX->setValue(camera->position[0]);
		cameraY->setValue(camera->position[1]);
		cameraZ->setValue(camera->position[2]);
	}
	);

	glfwSetKeyCallback(window,
		[](GLFWwindow *window, int key, int scancode, int action, int mods) {
		//screen->keyCallbackEvent(key, scancode, action, mods);

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		if (key >= 0 && key < 1024)
		{
			if (action == GLFW_PRESS)
			{
				keys[key] = true;

			}
			else if (action == GLFW_RELEASE)
				keys[key] = false;
		}
		cameraX->setValue(camera->position[0]);
		cameraY->setValue(camera->position[1]);
		cameraZ->setValue(camera->position[2]);

	}
	);

	glfwSetCharCallback(window,
		[](GLFWwindow *, unsigned int codepoint) {
		screen->charCallbackEvent(codepoint);
	}
	);

	glfwSetDropCallback(window,
		[](GLFWwindow *, int count, const char **filenames) {
		screen->dropCallbackEvent(count, filenames);
	}
	);

	glfwSetScrollCallback(window,
		[](GLFWwindow *, double x, double y) {
		screen->scrollCallbackEvent(x, y);

	}
	);

	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow *, int width, int height) {
		screen->resizeCallbackEvent(width, height);
	}
	);

}


void checkIfModelChanged()
{
	if (model_name != old_model_name)
	{
		old_model_name = model_name;
		getModelData(model_name);
		loadVolume();
	}
}

void getModelData(Model_name model_name)
{
	//if model is changed
	if (model_name == HEAD)            //head
	{
		volume_file = "./objs/Head_256_256_225.raw";
		XDIM = 256; YDIM = 256; ZDIM = 225;
		loadVolume();

	}

	else if (model_name == TEAPOT)        //teapot
	{
		volume_file = "./objs/BostonTeapot_256_256_178.raw";
		XDIM = 256; YDIM = 256; ZDIM = 178;
		loadVolume();
	}

	else if (model_name == BUCKY)        // bucky
	{
		volume_file = "./objs/Bucky_32_32_32.raw";
		XDIM = 32; YDIM = 32; ZDIM = 32;
		loadVolume();
	}

	else                            //bonsai
	{
		volume_file = "./objs/Bonsai_512_512_154.raw";
		XDIM = 512; YDIM = 512; ZDIM = 154;
		loadVolume();
	}
}
