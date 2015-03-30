// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
#include <math.h>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

const int window_width = 1024, window_height = 768;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLushort* &, int);
void createVAOs(Vertex[], GLushort[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void normalDraw(glm::mat4x4);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);
char state = 'u'; //u for unknown
#define PI 3.14159265
// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 30;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects] = { 0 };
GLuint VertexBufferId[NumObjects] = { 0 };
GLuint IndexBufferId[NumObjects] = { 0 };

size_t NumIndices[NumObjects] = { 0 };
size_t VertexBufferSize[NumObjects] = { 0 };
size_t IndexBufferSize[NumObjects] = { 0 };

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID;
GLuint LightID2;

GLint gX = 0.0;
GLint gZ = 0.0;

float camx = 10.0;
float camy = 10.0;
float camz = 10.0f;
float alphaH = 45;
float alphaV = 45;
float r_h = sqrt(200.0);
// animation control
bool animation = false;
GLfloat phi = 0.0;

//Definte here some variables which will used when calculating the matricies
//They will be modifiable by commands and are global

typedef struct treenode {
	//GLuint m[7]; // transformation
	glm::mat4x4 m[7];
	int sequenceNum = 0;
	treenode *next;
};
//define the root
treenode *stages[7];
float testStage2 = 0.0;
float stage3rot = 0.0;
float stage4rot = 0.0;
float stage5lat = 0.0;
float stage5long = 0.0;
float stage5pen = 0.0;

void defineStages(){
	for (int i = 0; i < 7; i++){
		stages[i] = new treenode();
		stages[i]->sequenceNum = i;
		if (i > 0){
			stages[i - 1]->next = stages[i];
		}
	}
}

void traverse(treenode *node){

	glm::mat4x4 ModelMatrix = node->m[node->sequenceNum];
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glBindVertexArray(VertexArrayId[node->sequenceNum + 2]);
	glDrawElements(GL_TRIANGLES, NumIndices[node->sequenceNum + 2], GL_UNSIGNED_SHORT, (void*)0);

	if (node == stages[6]) {
		return;
	}
	traverse(node->next);
}

void initTransformations(){
	for (int i = 0; i < 7; i++){
		stages[i]->m[i] = glm::mat4(1.0);
	}
	//m[0] covers the entire body translation
	for (int i = 0; i < 7; i++){
		stages[i]->m[0] = glm::translate(stages[i]->m[0], glm::vec3(0, .4, 0));
	}
	//m[1] covers every transformation, usually a rotation around the y axis
	for (int i = 1; i < 7; i++){
		stages[i]->m[1] = glm::translate(stages[i]->m[0], glm::vec3(0, .8, 0));
	}
	for (int i = 2; i < 7; i++){
		stages[i]->m[2] = glm::rotate(stages[i]->m[1], GLfloat(45), glm::vec3(0, 0, 1));
		stages[i]->m[2] = glm::translate(stages[i]->m[2], glm::vec3(0.5, 0, -0.15));
	}
	for (int i = 3; i < 7; i++){
		stages[i]->m[3] = glm::scale(stages[i]->m[2], glm::vec3(0.75, 0.75, 0.75));
		//stages[i]->m[3] = glm::rotate(stages[i]->m[1], GLfloat(45), glm::vec3(0, 0, 1));
		stages[i]->m[3] = glm::translate(stages[i]->m[3], glm::vec3(1.8, 0.7, 0.2));
	}
	for (int i = 4; i < 7; i++){
		stages[i]->m[4] = glm::scale(stages[i]->m[3], glm::vec3(1.5, 1.5, 1.5));
		//stages[i]->m[3] = glm::rotate(stages[i]->m[1], GLfloat(45), glm::vec3(0, 0, 1));
		stages[i]->m[4] = glm::translate(stages[i]->m[4], glm::vec3(0.18, -0.5, 0));
	}
	for (int i = 5; i < 7; i++){
		stages[i]->m[5] = glm::scale(stages[i]->m[4], glm::vec3(.75, .75, .75));
		stages[i]->m[5] = glm::translate(stages[i]->m[5], glm::vec3(0, -0.9, 0));
		stages[i]->m[5] = glm::rotate(stages[i]->m[5], GLfloat(-45), glm::vec3(0, 0, 1));
	}
	for (int i = 6; i < 7; i++){
		//stages[i]->m[5] = glm::scale(stages[i]->m[4], glm::vec3(.75, .75, .75));
		//stages[i]->m[5] = glm::rotate(stages[i]->m[5], GLfloat(-45), glm::vec3(0, 0, 1));
		stages[i]->m[6] = glm::translate(stages[i]->m[5], glm::vec3(0.1, .5, 0));
	}
}

void updateTransformations(){
	//m[0] covers the entire body translation
	for (int i = 0; i < 7; i++){
		stages[i]->m[0] = glm::translate(stages[i]->m[0], glm::vec3(0, 0, 0));
	}
	//m[1] covers every transformation, usually a rotation around the y axis
	for (int i = 1; i < 7; i++){
		stages[i]->m[1] = glm::rotate(stages[i]->m[0], GLfloat(testStage2), glm::vec3(0, 1, 0));
		stages[i]->m[1] = glm::translate(stages[i]->m[1], glm::vec3(0, 0.8, 0));
	}
	for (int i = 2; i < 7; i++){
		stages[i]->m[2] = glm::rotate(stages[i]->m[1], GLfloat(stage3rot), glm::vec3(0, 0, 1));
		stages[i]->m[2] = glm::rotate(stages[i]->m[2], GLfloat(45), glm::vec3(0, 0, 1));
		stages[i]->m[2] = glm::translate(stages[i]->m[2], glm::vec3(0.5, 0, -0.15));
	}
	for (int i = 3; i < 7; i++){

		stages[i]->m[3] = glm::scale(stages[i]->m[2], glm::vec3(0.75, 0.75, 0.75));
		//stages[i]->m[3] = glm::rotate(stages[i]->m[1], GLfloat(45), glm::vec3(0, 0, 1));
		stages[i]->m[3] = glm::translate(stages[i]->m[3], glm::vec3(1.8, 0.7, 0.2));
	}
	for (int i = 4; i < 7; i++){

		stages[i]->m[4] = glm::scale(stages[i]->m[3], glm::vec3(1.5, 1.5, 1.5));
		stages[i]->m[4] = glm::rotate(stages[i]->m[4], GLfloat(stage4rot), glm::vec3(0, 0, 1));
		//stages[i]->m[3] = glm::rotate(stages[i]->m[1], GLfloat(45), glm::vec3(0, 0, 1));
		stages[i]->m[4] = glm::translate(stages[i]->m[4], glm::vec3(0.18, -0.5, 0));
	}
	for (int i = 5; i < 7; i++){

		stages[i]->m[5] = glm::scale(stages[i]->m[4], glm::vec3(.75, .75, .75));
		stages[i]->m[5] = glm::translate(stages[i]->m[5], glm::vec3(0, -0.9, 0));
		stages[i]->m[5] = glm::rotate(stages[i]->m[5], GLfloat(-45), glm::vec3(0, 0, 1));
		stages[i]->m[5] = glm::rotate(stages[i]->m[5], GLfloat(stage5lat), glm::vec3(1, 0, 0));
		stages[i]->m[5] = glm::rotate(stages[i]->m[5], GLfloat(stage5long), glm::vec3(0, 0, 1));
		stages[i]->m[5] = glm::rotate(stages[i]->m[5], GLfloat(stage5pen), glm::vec3(0, 1, 0));
		
	}
	for (int i = 6; i < 7; i++){

		//stages[i]->m[5] = glm::scale(stages[i]->m[4], glm::vec3(.75, .75, .75));
		//stages[i]->m[5] = glm::rotate(stages[i]->m[5], GLfloat(-45), glm::vec3(0, 0, 1));
		stages[i]->m[6] = glm::translate(stages[i]->m[5], glm::vec3(0.1, 0.5, 0));
	}
}

void moveObject(){

	if (state == '1'){
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
			for (int i = 0; i < 7; i++){
				//so I also need to update the m's down the list without increasing the values
				stages[i]->m[0] = glm::translate(stages[i]->m[0], glm::vec3(-0.1, 0, 0));
				updateTransformations();
			}
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
			for (int i = 0; i < 7; i++){
				//so I also need to update the m's down the list without increasing the values
				stages[i]->m[0] = glm::translate(stages[i]->m[0], glm::vec3(0.1, 0, 0));
				updateTransformations();
			}
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
			for (int i = 0; i < 7; i++){
				//so I also need to update the m's down the list without increasing the values
				stages[i]->m[0] = glm::translate(stages[i]->m[0], glm::vec3(0, 0, 0.1));
				updateTransformations();
			}
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
			for (int i = 0; i < 7; i++){
				//so I also need to update the m's down the list without increasing the values
				stages[i]->m[0] = glm::translate(stages[i]->m[0], glm::vec3(0, 0, -0.1));
				updateTransformations();
			}
		}

	}

	if (state == '2'){
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
			for (int i = 0; i < 7; i++){
				//so I also need to update the m's down the list without increasing the values
				//stages[i]->m[0] = glm::rotate(stages[i]->m[0], GLfloat(2), glm::vec3(0, 1, 0));
				testStage2 = testStage2 + 1;
				updateTransformations();
			}
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
			for (int i = 0; i < 7; i++){
				//so I also need to update the m's down the list without increasing the values
				//stages[i]->m[0] = glm::rotate(stages[i]->m[0], GLfloat(-2), glm::vec3(0, 1, 0));
				testStage2 = testStage2 - 1;
				updateTransformations();
			}
		}
	}

	if (state == '3'){
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
			//so I also need to update the m's down the list without increasing the values
			//stages[i]->m[0] = glm::rotate(stages[i]->m[0], GLfloat(stage3rot), glm::vec3(0, 0, 1));
			if (stage3rot < 104)
				stage3rot = stage3rot + 3;
			updateTransformations();
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
			//so I also need to update the m's down the list without increasing the values
			//stages[i]->m[2] = glm::rotate(stages[i]->m[2], GLfloat(-2), glm::vec3(0, 0, 1));
			if (stage3rot > -77.0)
				stage3rot = stage3rot - 3;
			updateTransformations();
		}
	}
	if (state == '4'){
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
			//so I also need to update the m's down the list without increasing the values
			//stages[i]->m[0] = glm::rotate(stages[i]->m[0], GLfloat(stage3rot), glm::vec3(0, 0, 1));
			if (stage4rot < 215)
				stage4rot = stage4rot + 3;
			updateTransformations();
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
			//so I also need to update the m's down the list without increasing the values
			//stages[i]->m[2] = glm::rotate(stages[i]->m[2], GLfloat(-2), glm::vec3(0, 0, 1));
			if (stage4rot > -52.0)
				stage4rot = stage4rot - 3;
			updateTransformations();
		}
	}
	if (state == '5'){
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
			stage5lat = stage5lat + 5;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
			stage5lat = stage5lat - 5;
			//for (int i = 5; i < 7; i++)
			///	stages[i]->m[5] = glm::rotate(stages[i]->m[5], GLfloat(1), glm::vec3(1, 0, 0));
			//updateTransformations();
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && !(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)){
			stage5long = stage5long + 5;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)){
			stage5long = stage5long - 5;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)){
			stage5pen = stage5pen + 5;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)){
			stage5pen = stage5pen - 5;
		}
		updateTransformations();
	}
}

void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLushort* &out_Indices, int ObjectId)
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIndices[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}


void createObjects(void)
{
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);

	//-- GRID --//

	// ATTN: create your grid vertices here!
	//Martin I am so sorry that I can't figure out how to create verticies in a non-naive way.
	Vertex GridVerts[] = {
		//10
		{ { 5.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 4.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 4.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 3.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 3.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 2.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 2.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 1.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 1.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 0.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		//12
		{ { 5.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 4.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 4.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 3.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 3.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 2.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 2.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 1.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 1.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		//12
		{ { -5.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -4.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -4.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -3.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -3.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -2.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -2.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -1.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -1.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		//10
		{ { -5.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -4.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -4.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -3.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -3.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -2.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -2.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -1.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -1.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
	};
	//generate grid vertices

	/*Vertex GridVerts[44];
	float x = 5.0;
	float z = 5.0;
	for (int i = 0; i < 44; i+2){
	if (z >= 0) {
	GridVerts[i] = { { 0.0 + x, 0.0, 0.0 + z, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	GridVerts[i + 1] = { { (-1.0)*(0.0 + x), 0.0, 0.0 + z, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	z -= 1.0;
	}
	else if (x >= 0) {
	GridVerts[i] = { { 0.0 + x, 0.0, 0.0 + z, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	GridVerts[i + 1] = { { 0.0 + x, 0.0, (-1.0)*(0.0 + z), 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	x -= 1.0;
	}
	}*/


	//create VAO for grid here
	VertexBufferSize[1] = sizeof(GridVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(GridVerts, NULL, 1);


	//-- .OBJs --//

	// ATTN: load your models here
	Vertex* Verts;
	GLushort* Idcs;
	loadObject("base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 2);
	createVAOs(Verts, Idcs, 2);
	loadObject("top.obj", glm::vec4(0.0, 1.0, 0.0, 1.0), Verts, Idcs, 3);
	createVAOs(Verts, Idcs, 3);
	loadObject("arm1.obj", glm::vec4(0.0, 0.0, 1.0, 1.0), Verts, Idcs, 4);
	createVAOs(Verts, Idcs, 4);
	loadObject("joint.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Verts, Idcs, 5);
	createVAOs(Verts, Idcs, 5);
	loadObject("arm2.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Verts, Idcs, 6);
	createVAOs(Verts, Idcs, 6);
	loadObject("pen.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Verts, Idcs, 7);
	createVAOs(Verts, Idcs, 7);
	loadObject("button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 8);
	createVAOs(Verts, Idcs, 8);
	loadObject("pen.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts, Idcs, 9);
	createVAOs(Verts, Idcs, 9);
}

void renderScene(void)
{
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!


	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glm::vec3 lightPos2 = glm::vec3(-4, 4, 4);
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);


		glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 6);

		glBindVertexArray(VertexArrayId[1]); //draw grid
		glDrawArrays(GL_LINES, 0, 46);

		traverse(stages[0]);

		glBindVertexArray(0);

	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void pickObject(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		//// draw Base
		//glBindVertexArray(X);	
		//	glUniform1f(pickingColorID, Y / 255.0f); // here we pass in the picking marker
		//	glDrawElements(Z);
		//// draw Top
		//glBindVertexArray(XX);	
		//	glUniform1f(pickingColorID, YY / 255.0f); // here we pass in the picking marker
		//	glDrawElements(ZZ);
		//glBindVertexArray(0);
	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);

	if (gPickedIndex == 255){ // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Frazee,William, 13555441", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	camx = 10.0;
	camy = 10.0;
	camz = 10.0f;

	gViewMatrix = glm::lookAt(glm::vec3(camx, camy, camz),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	LightID2 = glGetUniformLocation(programID, "LightPosition_worldspace2");
	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
			);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

void redoView(){
	if (state == 'c'){
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
		//vertical camera movement
			gViewMatrix = glm::rotate(gViewMatrix, GLfloat(1.0), glm::vec3(1, 0, 0));
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
			gViewMatrix = glm::rotate(gViewMatrix, GLfloat(-1.0), glm::vec3(1, 0, 0));
		}
		//horizontal movement
		else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
			gViewMatrix = glm::rotate(gViewMatrix, GLfloat(-2.0), glm::vec3(0, 1, 0));
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
			gViewMatrix = glm::rotate(gViewMatrix, GLfloat(2.0), glm::vec3(0, 1, 0));
		}
	}
}

void changeState(){
	Vertex* Verts;
	GLushort* Idcs;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
		state = 'c';
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS){
		state = '1';
		//change base color
		loadObject("base.obj", glm::vec4(0.5, 0.5, 0.5, 1.0), Verts, Idcs, 2);
		createVAOs(Verts, Idcs, 2);
		loadObject("top.obj", glm::vec4(0.0, 1.0, 0.0, 1.0), Verts, Idcs, 3);
		createVAOs(Verts, Idcs, 3);
		loadObject("arm1.obj", glm::vec4(0.0, 0.0, 1.0, 1.0), Verts, Idcs, 4);
		createVAOs(Verts, Idcs, 4);
		loadObject("joint.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Verts, Idcs, 5);
		createVAOs(Verts, Idcs, 5);
		loadObject("arm2.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Verts, Idcs, 6);
		createVAOs(Verts, Idcs, 6);
		loadObject("pen.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Verts, Idcs, 7);
		createVAOs(Verts, Idcs, 7);
		loadObject("button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 8);
		createVAOs(Verts, Idcs, 8);
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS){
		state = '2';
		loadObject("base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 2);
		createVAOs(Verts, Idcs, 2);
		//change top color
		loadObject("top.obj", glm::vec4(0.5, 0.5, 0.5, 1.0), Verts, Idcs, 3);
		createVAOs(Verts, Idcs, 3);
		loadObject("arm1.obj", glm::vec4(0.0, 0.0, 1.0, 1.0), Verts, Idcs, 4);
		createVAOs(Verts, Idcs, 4);
		loadObject("joint.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Verts, Idcs, 5);
		createVAOs(Verts, Idcs, 5);
		loadObject("arm2.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Verts, Idcs, 6);
		createVAOs(Verts, Idcs, 6);
		loadObject("pen.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Verts, Idcs, 7);
		createVAOs(Verts, Idcs, 7);
		loadObject("button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 8);
		createVAOs(Verts, Idcs, 8);
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS){
		state = '3';
		loadObject("base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 2);
		createVAOs(Verts, Idcs, 2);
		loadObject("top.obj", glm::vec4(0.0, 1.0, 0.0, 1.0), Verts, Idcs, 3);
		createVAOs(Verts, Idcs, 3);
		//change arm1 color
		loadObject("arm1.obj", glm::vec4(0.5, 0.5, 0.5, 1.0), Verts, Idcs, 4);
		createVAOs(Verts, Idcs, 4);
		loadObject("joint.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Verts, Idcs, 5);
		createVAOs(Verts, Idcs, 5);
		loadObject("arm2.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Verts, Idcs, 6);
		createVAOs(Verts, Idcs, 6);
		loadObject("pen.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Verts, Idcs, 7);
		createVAOs(Verts, Idcs, 7);
		loadObject("button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 8);
		createVAOs(Verts, Idcs, 8);
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS){
		state = '4';
		loadObject("base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 2);
		createVAOs(Verts, Idcs, 2);
		loadObject("top.obj", glm::vec4(0.0, 1.0, 0.0, 1.0), Verts, Idcs, 3);
		createVAOs(Verts, Idcs, 3);
		loadObject("arm1.obj", glm::vec4(0.0, 0.0, 1.0, 1.0), Verts, Idcs, 4);
		createVAOs(Verts, Idcs, 4);
		loadObject("joint.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Verts, Idcs, 5);
		createVAOs(Verts, Idcs, 5);
		//change arm2 color
		loadObject("arm2.obj", glm::vec4(0.5, 0.5, 0.5, 1.0), Verts, Idcs, 6);
		createVAOs(Verts, Idcs, 6);
		loadObject("pen.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Verts, Idcs, 7);
		createVAOs(Verts, Idcs, 7);
		loadObject("button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 8);
		createVAOs(Verts, Idcs, 8);
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS){
		state = '5';
		loadObject("base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 2);
		createVAOs(Verts, Idcs, 2);
		loadObject("top.obj", glm::vec4(0.0, 1.0, 0.0, 1.0), Verts, Idcs, 3);
		createVAOs(Verts, Idcs, 3);
		loadObject("arm1.obj", glm::vec4(0.0, 0.0, 1.0, 1.0), Verts, Idcs, 4);
		createVAOs(Verts, Idcs, 4);
		loadObject("joint.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Verts, Idcs, 5);
		createVAOs(Verts, Idcs, 5);
		loadObject("arm2.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Verts, Idcs, 6);
		createVAOs(Verts, Idcs, 6);
		//change pen color
		loadObject("pen.obj", glm::vec4(0.5, 0.5, 0.5, 1.0), Verts, Idcs, 7);
		createVAOs(Verts, Idcs, 7);
		loadObject("button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, 8);
		createVAOs(Verts, Idcs, 8);
	}
}


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_A:
			break;
		case GLFW_KEY_D:
			break;
		case GLFW_KEY_W:
			break;
		case GLFW_KEY_S:
			break;
		case GLFW_KEY_SPACE:
			break;
		default:
			break;
		}
	}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}



int main(void)
{
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// initialize OpenGL pipeline
	initOpenGL();
	defineStages();
	initTransformations();
	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		//// Measure speed
		//double currentTime = glfwGetTime();
		//nbFrames++;
		//if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
		//	// printf and reset
		//	printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		//	nbFrames = 0;
		//	lastTime += 1.0;
		//}

		if (animation){
			phi += 0.01;
			if (phi > 360)
				phi -= 360;
		}
		changeState();
		redoView();
		moveObject();
		// DRAWING POINTS
		renderScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}