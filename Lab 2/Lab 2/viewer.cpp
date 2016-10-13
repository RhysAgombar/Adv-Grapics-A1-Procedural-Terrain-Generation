/************************************************************
*                   CSCI 4110 Lab 2
*
*  Basic OpenGL program that shows how to set up a
*  VAO and some basic shaders.  This program draws
*  a cube or sphere depending upon whether CUBE or
*  SPHERE is defined.
*
*/
#include <Windows.h>
#include <GL/glew.h>
#include <gl/glut.h>
#include <GL/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <stdio.h>
#include "Shaders.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <random>
#include <fstream>

float eyex, eyey, eyez;

double theta, phi;
double r;

GLuint program;

glm::mat4 projection;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

GLfloat lastX = 160; // center of window
GLfloat lastY = 160;
GLfloat pitch = 0; // center of window
GLfloat yaw = 0;

GLuint objVAO;
int triangles;
int k = 0;

std::default_random_engine generator;
std::normal_distribution<double> distribution(0.0,1.0);

float* map;

int vert = 5;
int horiz = 5;
int index_count = (2 * vert) * (horiz - 1);

int getIndNum(int width, int height) {
	return (2 * height) * (width - 1);
}

void findNormal(GLfloat* p1, GLfloat* p2, GLfloat* p3, GLfloat(&n)[3]) {
	GLfloat v[3], w[3];

	v[0] = p2[0] - p1[0];
	v[1] = p2[1] - p1[1];
	v[2] = p2[2] - p1[2];

	w[0] = p3[0] - p1[0];
	w[1] = p3[1] - p1[1];
	w[2] = p3[2] - p1[2];

	n[0] = abs((v[1] * w[2]) - (v[2] * w[1]));
	n[1] = abs((v[2] * w[0]) - (v[0] * w[2]));
	n[2] = abs((v[0] * w[1]) - (v[1] * w[0]));

	GLfloat div = (abs(n[0]) + abs(n[1]) + abs(n[2]));

	if (div != 0) {
		n[0] = n[0] / div;
		n[1] = n[1] / div;
		n[2] = n[2] / div;
	}
}

void expandMap(GLfloat*(&map)) {

	int newHoriz = (horiz + (horiz - 1));
	int newVert = (vert + (vert - 1));
	int t = 0;

	GLfloat * v1 = new GLfloat[(horiz + (horiz - 1)) * (vert + (vert - 1)) * 4];

	for (int j = 0; j < vert - 1; j++) {
		for (int i = 0; i < horiz - 1; i++) {
			GLfloat avg[3];
			int pos1 = (i + j*horiz) * 4;
			int pos2 = (i + 1 + j*horiz) * 4;
			int pos3 = (i + (j + 1)*horiz) * 4;
			int pos4 = (i + 1 + (j + 1)*horiz) * 4;

			avg[0] = (map[pos1] + map[pos2] + map[pos3] + map[pos4]) / 4;
			avg[1] = (map[pos1 + 1] + map[pos2 + 1] + map[pos3 + 1] + map[pos4 + 1]) / 4;
			avg[2] = (map[pos1 + 2] + map[pos2 + 2] + map[pos3 + 2] + map[pos4 + 2]) / 4;

			int vPos = ((j + 1) * (2 * horiz) + j*(2 * (horiz - 1)) + (2 * i)) * 4;

			v1[vPos] = avg[0];
			v1[vPos + 1] = avg[1];
			v1[vPos + 2] = avg[2];
			v1[vPos + 3] = 1.0;
		}
	}

	for (int j = 0; j < vert; j++) {
		for (int i = 0; i < horiz; i++) {
			int pos = ((j) * (2 * horiz) + (2 * i) + j*(2 * (horiz - 1))) * 4;
			int mPos = (i + j*horiz) * 4;
			pos = pos;

			v1[pos] = map[mPos];
			v1[pos + 1] = map[mPos + 1];
			v1[pos + 2] = map[mPos + 2];
			v1[pos + 3] = 1.0;

		}
	}


	for (int j = 0; j < newVert; j++) {
		for (int i = 0; i < newHoriz; i++) {

			int pos = (i + j*newHoriz) * 4;
			int count = 0;

			if (v1[pos] < (-1e5)) {

				v1[pos] = 0;
				v1[pos + 1] = 0;
				v1[pos + 2] = 0;
				v1[pos + 3] = 1.0;


				if ((pos - 1) >= 0) {
					v1[pos] += v1[pos - 4];
					v1[pos + 1] += v1[pos - 3];
					v1[pos + 2] += v1[pos - 2];
					count++;
				}

				if ((pos + 6) < (newHoriz * 4)) {
					v1[pos] += v1[pos + 4];
					v1[pos + 1] += v1[pos + 5];
					v1[pos + 2] += v1[pos + 6];
					count++;
				}

				if (j - 1 >= 0) {
					v1[pos] += v1[pos - newHoriz*4];
					v1[pos + 1] += v1[(pos - newHoriz*4) + 1];
					v1[pos + 2] += v1[(pos - newHoriz*4) + 2];
					count++;
				}

				if (j + 1 < newHoriz) {
					v1[pos] += v1[pos + newHoriz*4];
					v1[pos + 1] += v1[pos + newHoriz*4 + 1];
					v1[pos + 2] += v1[pos + newHoriz*4 + 2];
					count++;
				}

				v1[pos] /= count;
				v1[pos+1] /= count;
				
				double r = distribution(generator);

				v1[pos+2] /= count;
				v1[pos + 2] += (r * 0.1);

			}
		}
	}

	horiz = newHoriz;
	vert = newVert;

	index_count = getIndNum(horiz, vert);

	map = v1;

}

void init() {

	GLuint vbuffer;
	GLuint ibuffer;
	GLint vPosition;
	GLint vNormal;

	glGenVertexArrays(1, &objVAO);
	glBindVertexArray(objVAO);

	GLfloat * verts = new GLfloat[4 * vert*horiz];
	GLfloat * norms = new GLfloat[3 * vert*horiz];
	GLuint * inds = new GLuint[index_count];

	float heights[] = { 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1 };
	int vi = 0;
	int ni = 0;
	int ii = 0;
	int hi = 0;

	for (int i = 0; i < vert; ++i)
	{
		for (int j = 0; j < horiz; ++j)
		{
			verts[vi++] = i;
			verts[vi++] = heights[hi++];
			verts[vi++] = j;
			verts[vi++] = 1.0;
			norms[ni++] = 0.0;
			norms[ni++] = 1.0;
			norms[ni++] = 0.0;
			if (((j + 1) < horiz) && ((i + 1) < vert))
			{
				inds[ii++] = horiz * (i + 1) + j + 1;
				inds[ii++] = horiz * (i + 1) + j;
				inds[ii++] = horiz * i + j;
				inds[ii++] = horiz * (i + 1) + j + 1;
				inds[ii++] = horiz * i + j;
				inds[ii++] = horiz * i + j + 1;
			}
		}
	}

	int v_size = vert * horiz * 4 * sizeof(*verts);
	int n_size = vert * horiz * 3 * sizeof(*norms);
	int i_size = index_count * sizeof(*inds);

	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, v_size + n_size, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, v_size, verts);
	glBufferSubData(GL_ARRAY_BUFFER, v_size, n_size, norms);

	glGenBuffers(1, &ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_size, inds, GL_STATIC_DRAW);

	glUseProgram(program);
	vPosition = glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(program, "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)v_size);
	glEnableVertexAttribArray(vNormal);


}

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).

	if (h == 0)
		h = 1;

	float ratio = 1.0 * w / h;

	glViewport(0, 0, w, h);

	projection = glm::perspective(45.0f, ratio, 1.0f, 5000.0f);

}

void displayFunc(void) {
	glm::mat4 view;
	int viewLoc;
	int projLoc;
	int colourLoc;
	int camLoc;
	int lightLoc;
	int materialLoc;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);


	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	GLfloat radius = 10.0f;
	GLfloat camX = sin(k) * radius;
	GLfloat camZ = cos(k++) * radius;

	viewLoc = glGetUniformLocation(program, "modelView");
	glUniformMatrix4fv(viewLoc, 1, 0, glm::value_ptr(view));
	projLoc = glGetUniformLocation(program, "projection");
	glUniformMatrix4fv(projLoc, 1, 0, glm::value_ptr(projection));

	colourLoc = glGetUniformLocation(program, "colour");
	glUniform4f(colourLoc, 1.0, 0.0, 0.0, 1.0);
	camLoc = glGetUniformLocation(program, "Eye");
	glUniform3f(camLoc, cameraPos.x, 0.0f, cameraPos.z);
	lightLoc = glGetUniformLocation(program, "light");
	glUniform3f(lightLoc, 1.0, 1.0, 1.0);
	materialLoc = glGetUniformLocation(program, "material");
	glUniform4f(materialLoc, 0.3, 0.7, 0.7, 150.0);

	glBindVertexArray(objVAO);
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, NULL);

	glutSwapBuffers();
}

void keyboardFunc(unsigned char key, int x, int y) {

	GLfloat cameraSpeed = 1.5f;

	switch (key) {
	case 'a':
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		break;
	case 'd':
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		break;
	case 'w':
		cameraPos += cameraSpeed * cameraFront;
		break;
	case 's':
		cameraPos -= cameraSpeed * cameraFront;
		break;
	}

	glutPostRedisplay();

}

void mouseFunc(int button, int state, int x, int y) {

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		lastX = x;
		lastY = y;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {

		GLfloat xoffset = x - lastX;
		GLfloat yoffset = lastY - y;
		lastX = x;
		lastY = y;

		GLfloat sensitivity = 0.15;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = -sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = -sin(glm::radians(pitch));
		front.z = -cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
	}


}

int main(int argc, char **argv) {
	int fs;
	int vs;

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(700, 700);
	glutCreateWindow("Viewer");
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		printf("Error starting GLEW: %s\n", glewGetErrorString(error));
		exit(0);
	}

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);



	std::string in;
	std::fstream file("heightmap.txt");

	map = new GLfloat[horiz*vert];

	for (int i = 0; i < horiz * vert; i++) {
		file >> map[i];
	}

	file.close();


	vs = buildShader(GL_VERTEX_SHADER, "lab2.vs");
	fs = buildShader(GL_FRAGMENT_SHADER, "lab2.fs");
	program = buildProgram(vs, fs, 0);
	dumpProgram(program, "Lab 2 shader program");
	init();

	glutMainLoop();

}

#pragma region old code


//GLuint vbuffer;
//GLuint ibuffer;
//GLint vPosition;
//GLint vNormal;
//
//glGenVertexArrays(1, &objVAO);
//glBindVertexArray(objVAO);
//
//index_count = getIndNum(horiz,vert);
//
//GLfloat * vertices = new GLfloat[4 * horiz*vert];
//
//float heights[] = { 1, 1, 1, 1, 1,
//	1, 1, 2, 1, 1,
//	1, 2, 5, 2, 1,
//	1, 1, 2, 1, 1,
//	1, 1, 1, 1, 1 };
//
//int vi = 0;
//int ni = 0;
//int ni2 = 0;
//
//for (int i = 0; i < vert * horiz; i++) {
//	vertices[vi++] = i % horiz; // count from 0 to horiz size for x value
//	vertices[vi++] = (int)(i / 5); // Only increase y value when we change to a new line
//	vertices[vi++] = heights[i]; // z value = map value
//	vertices[vi++] = 1.0;
//}
//
//int i = 0;
//
////expandMap(vertices);
//
//int t = 0;
//std::cout << " ----------------------- " << std::endl;
//for (int i = 0; i < vert; i++) {
//	for (int j = 0; j < horiz; j++) {
//		std::cout << "{ " << vertices[t] << ", " << vertices[t + 1] << ", " << vertices[t + 2] << ", " << vertices[t + 3] << " }" << std::endl;
//		t = t + 4;
//	}
//	std::cout << " //// " << std::endl;
//}
//std::cout << " ----------------------- " << std::endl;
//
//GLfloat * fnormals = new GLfloat[index_count - 2];
//GLfloat * normals = new GLfloat[3 * horiz*vert];
//GLuint * indices = new GLuint[index_count];
//
//
//
//for (int row = 0; row < vert - 1; row++) {
//	if ((row & 1) == 0) { // even rows
//		for (int col = 0; col<vert; col++) { // modified version of the formula from one of the examples you gave me.
//			indices[i++] = row + col * vert; // It seems to work correctly.
//			indices[i++] = (row + 1) + col * vert;
//		}
//	}
//	else { // odd rows
//		for (int col = horiz - 1; col >= 0; col--) {
//			indices[i++] = (row)+col * horiz;
//			indices[i++] = (row + 1) + col * horiz;
//		}
//	}
//}
//
//GLfloat arrHolder[3];
//bool toggle = false;
//
//for (int i = 0; i < index_count - 2; i++) { // for every face...
//	int pos1, pos2, pos3;
//
//	pos1 = indices[i] * 4;
//	pos2 = indices[i+1] * 4;
//	pos3 = indices[i+2] * 4;
//
//	GLfloat p1[3] = { vertices[pos1] , vertices[pos1 + 1] , vertices[pos1 + 2] };
//	GLfloat p2[3] = { vertices[pos2] , vertices[pos2 + 1] , vertices[pos2 + 2] };
//	GLfloat p3[3] = { vertices[pos3] , vertices[pos3 + 1] , vertices[pos3 + 2] };
//
//
//	if (toggle == false) {
//		findNormal(p1, p2, p3, arrHolder); // find the normal of the face
//		toggle = true;
//	}
//	else {
//		findNormal(p1, p3, p2, arrHolder); // alternate the direction of the normal to 
//		toggle = false;					   // compensate for the direction change in the indices formula
//	}
//
//
//	fnormals[ni++] = arrHolder[0];
//	fnormals[ni++] = arrHolder[1];
//	fnormals[ni++] = arrHolder[2];
//}
//
//
//
//
//
//
////int count;
////int pos = 0;
////int pos1 = 0;
////for (int i = 0; i < horiz*vert; i++) { // for every vertex...
////	count = 0;
////
////	normals[pos] = 0;
////	normals[pos+1] = 0; // set vertex normals to 0
////	normals[pos+2] = 0;
////
////	for (int j = 0; j < index_count - 2; j++) { // for every face...
////		if (indices[j] == i || indices[j + 1] == i || indices[j + 2] == i) {  // if the face contains the vertex
////			count++; // increment the count
////			normals[pos] += fnormals[pos1++]; // add the face normal to the vertex normal 
////			normals[pos + 1] += fnormals[pos1++];
////			normals[pos + 2] += fnormals[pos1++];
////		}
////	}
////
////	GLfloat test = normals[pos];
////
////	normals[pos++] /= count; // divide the normal by the count to average
////	test = normals[pos];
////	normals[pos++] /= count;
////	test = normals[pos];
////	normals[pos++] /= count;
////}
//
//
//int v_size = horiz * vert * 4 * sizeof(*vertices);
//int n_size = 3 * (index_count - 2);
//
//for (int i = 0; i < 3 * (index_count - 2); i += 3) {
//	std::cout << fnormals[i] << ", " << fnormals[i + 1] << ", " << fnormals[i + 2] << std::endl;
//}
//
//int i_size = index_count * sizeof(*indices);
//
//glGenBuffers(1, &vbuffer);
//glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
//glBufferData(GL_ARRAY_BUFFER, v_size + n_size, NULL, GL_STATIC_DRAW);
//glBufferSubData(GL_ARRAY_BUFFER, 0, v_size, vertices);
//glBufferSubData(GL_ARRAY_BUFFER, v_size, n_size, normals);
//
//glGenBuffers(1, &ibuffer);
//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
//glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_size, indices, GL_STATIC_DRAW);
//
//glUseProgram(program);
//vPosition = glGetAttribLocation(program, "vPosition");
//glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
//glEnableVertexAttribArray(vPosition);
//vNormal = glGetAttribLocation(program, "vNormal");
//glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)v_size);
//glEnableVertexAttribArray(vNormal);

//GLuint vbuffer;
//GLuint ibuffer;
//GLint vPosition;
//GLint vNormal;
//
//glGenVertexArrays(1, &objVAO);
//glBindVertexArray(objVAO);
//
//GLfloat * vertices = new GLfloat[4 * vert*horiz];
//
////float heights[] = { 1, 10, 1, 10, 1,
////1, 20, 50, 10, 1,
////20, 20, 30, 20, 1,
////10, 40, 70, 50, 1,
////1, 1, 10, 1, 1 };
//
//
//int vi = 0;
//int ni = 0;
//int ii = 0;
//int hi = 0;
//
//for (int i = 0; i < horiz * vert; i++) {
//	vertices[vi++] = (i % horiz) * 100; // count from 0 to horiz size for x value
//	vertices[vi++] = ((int)(i / 5)) * 100; // Only increase y value when we change to a new line
//	vertices[vi++] = heights[i]; // z value = map value
//	vertices[vi++] = 1.0;
//}
//
////for (int i = 0; i < 4; i++) {
////	expandMap(vertices);
////}
//
////GLfloat * fnormals = new GLfloat[(index_count - 2) * 3];
//
//
//GLfloat * norms = new GLfloat[3 * vert*horiz];
//GLuint * indices = new GLuint[index_count];
//
//int i = 0;
//for (int row = 0; row < vert - 1; row++) {
//	if ((row & 1) == 0) { // even rows
//		for (int col = 0; col<vert; col++) { // modified version of the formula from one of the examples you gave me.
//			indices[i++] = row + col * vert; // It seems to work correctly.
//			indices[i++] = (row + 1) + col * vert;
//		}
//	}
//	else { // odd rows
//		for (int col = horiz - 1; col >= 0; col--) {
//			indices[i++] = (row)+col * horiz;
//			indices[i++] = (row + 1) + col * horiz;
//		}
//	}
//}
////
////GLfloat arrHolder[3];
////bool toggle = false;
////
////for (int i = 0; i < getIndNum(horiz,vert); i++) {
////	std::cout << indices[i] << std::endl;
////}
////
////GLfloat arrHolder[3];
////bool toggle = false;
////
////for (int i = 0; i < index_count - 2; i++) { // for every face...
////	int pos1, pos2, pos3;
////
////	pos1 = indices[i] * 4;
////	pos2 = indices[i+1] * 4;
////	pos3 = indices[i+2] * 4;
////
////	GLfloat p1[3] = { vertices[pos1] , vertices[pos1 + 1] , vertices[pos1 + 2] };
////	GLfloat p2[3] = { vertices[pos2] , vertices[pos2 + 1] , vertices[pos2 + 2] };
////	GLfloat p3[3] = { vertices[pos3] , vertices[pos3 + 1] , vertices[pos3 + 2] };
////
////
////	if (toggle == false) {
////		findNormal(p1, p2, p3, arrHolder); // find the normal of the face
////		toggle = true;
////	}
////	else {
////		findNormal(p1, p3, p2, arrHolder); // alternate the direction of the normal to 
////		toggle = false;					   // compensate for the direction change in the indices formula
////	}
////
////
////	fnormals[ni++] = arrHolder[0];
////	fnormals[ni++] = arrHolder[1];
////	fnormals[ni++] = arrHolder[2];
////}
//
//
////int count = 0;
////for (int i = 0; i < (index_count - 2); i++) {
////	std::cout << i << " - {" << fnormals[count++] << ", " << fnormals[count++] << ", " << fnormals[count++] << "}" << std::endl;
////}
//
//
//////int count;
////int pos = 0;
////int pos1 = 0;
////
////GLfloat * normals = new GLfloat[3 * vert*horiz];
////
////for (int i = 0; i < horiz*vert; i++) { // for every vertex...
////	count = 0;
////
////	normals[pos] = 0;
////	normals[pos+1] = 0; // set vertex normals to 0
////	normals[pos+2] = 0;
////
////	for (int j = 0; j < index_count - 2; j++) { // for every face...
////		if (indices[j] == i || indices[j + 1] == i || indices[j + 2] == i) {  // if the face contains the vertex
////			count++; // increment the count
////			normals[pos] += fnormals[pos1++]; // add the face normal to the vertex normal 
////			normals[pos + 1] += fnormals[pos1++];
////			normals[pos + 2] += fnormals[pos1++];
////		}
////	}
////
////	GLfloat test = normals[pos];
////
////	normals[pos++] /= count; // divide the normal by the count to average
////	test = normals[pos];
////	normals[pos++] /= count;
////	test = normals[pos];
////	normals[pos++] /= count;
////}
//
//ni = 0;
//
//for (int i = 0; i < vert; ++i)
//{
//	for (int j = 0; j < horiz; ++j)
//	{
//
//		norms[ni++] = 0.0;
//		norms[ni++] = 1.0;
//		norms[ni++] = 0.0;
//
//	}
//}
//
//int v_size = vert * horiz * 4 * sizeof(*vertices);
//int n_size = (index_count - 2) * 3 * sizeof(*fnormals);
//int i_size = index_count * sizeof(*indices);
//
//glGenBuffers(1, &vbuffer);
//glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
//glBufferData(GL_ARRAY_BUFFER, v_size + n_size, NULL, GL_STATIC_DRAW);
//glBufferSubData(GL_ARRAY_BUFFER, 0, v_size, vertices);
//glBufferSubData(GL_ARRAY_BUFFER, v_size, n_size, fnormals);
//
//glGenBuffers(1, &ibuffer);
//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
//glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_size, indices, GL_STATIC_DRAW);
//
//glUseProgram(program);
//vPosition = glGetAttribLocation(program, "vPosition");
//glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
//glEnableVertexAttribArray(vPosition);
//vNormal = glGetAttribLocation(program, "vNormal");
//glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)v_size);
//glEnableVertexAttribArray(vNormal);

#pragma endregion 
