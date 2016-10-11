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
#include <fstream>
#include <math.h>


float eyex, eyey, eyez;

double theta, phi;
double r;

GLuint program;

glm::mat4 projection;

GLuint objVAO;
int triangles;

int horiz = 5;
int vert = 5;

GLfloat* map;

int getIndNum(int width, int height) {
	return (2 * height) * (width - 1);
}

void  findNormal(GLfloat* p1, GLfloat* p2, GLfloat* p3, GLfloat(&n)[3]) {
	GLfloat v[3], w[3];

	v[0] = p2[0] - p1[0];
	v[1] = p2[1] - p1[1];
	v[2] = p2[2] - p1[2];

	w[0] = p3[0] - p1[0];
	w[1] = p3[1] - p1[1];
	w[2] = p3[2] - p1[2];

	n[0] = (v[1] * w[2]) - (v[2] * w[1]);
	n[1] = (v[2] * w[0]) - (v[0] * w[2]);
	n[2] = (v[0] * w[1]) - (v[1] * w[0]);

	GLfloat div = (abs(n[0]) + abs(n[1]) + abs(n[2]));

	if (div != 0) {
		n[0] = n[0] / div;
		n[1] = n[1] / div;
		n[2] = n[2] / div;
	}
}

/*
* This version of the init procedure produces the
* data for drawing a cube.  The vertex and index
* data are stored in constant arrays which are copied
* into the buffers.  This code should be familiar
* from class.
*/
void init() {
	GLuint vbuffer;
	GLuint ibuffer;
	GLint vPosition;
	GLint vNormal;

	glGenVertexArrays(1, &objVAO);
	glBindVertexArray(objVAO);

	GLfloat vertices [25][4];
	GLfloat v1[81][4];  // (horiz + (horiz - 1)) * (vert + (vert - 1))

	int newHoriz = (horiz + (horiz - 1));
	int newVert = (vert + (vert - 1));

	int nv = horiz * vert;
	int nv1 = newHoriz * newVert;

	//vertices = new GLfloat*[nv];

	//for (int i = 0; i < nv; i++) {
	//	vertices[i] = new GLfloat[4];
	//}

	for (int i = 0; i < vert * horiz; i++) {
		vertices[i][0] = i % horiz; // count from 0 to horiz size for x value
		vertices[i][1] = (int)(i / 5); // Only increase y value when we change to a new line
		vertices[i][2] = map[i]; // z value = map value
		vertices[i][3] = 1.0; 
	}


	for (int j = 0; j < vert - 1; j++) {
		for (int i = 0; i < horiz - 1; i++) {
			GLfloat avg[3];

			avg[0] = (vertices[i + j*horiz][0] + vertices[i + 1 + j*horiz][0] + vertices[i + (j + 1)*horiz][0] + vertices[i + 1 + (j + 1)*horiz][0]) / 4;
			avg[1] = (vertices[i + j*horiz][1] + vertices[i + 1 + j*horiz][1] + vertices[i + (j + 1)*horiz][1] + vertices[i + 1 + (j + 1)*horiz][1]) / 4;
			avg[2] = (vertices[i + j*horiz][2] + vertices[i + 1 + j*horiz][2] + vertices[i + (j + 1)*horiz][2] + vertices[i + 1 + (j + 1)*horiz][2]) / 4;

			int pos = (j+1) * (2*horiz) + j*(2*(horiz - 1)) + (2*i);

			v1[pos][0] = avg[0];
			v1[pos][1] = avg[1];
			v1[pos][2] = avg[2];
			v1[pos][3] = 1.0;			
		}
	}

	for (int j = 0; j < vert; j++) {
		for (int i = 0; i < horiz; i++) {
			int pos = (j) * (2 * horiz) + (2 * i) + j*(2 * (horiz - 1));
			pos = pos;

			v1[pos][0] = vertices[i + j*horiz][0];
			v1[pos][1] = vertices[i + j*horiz][1];
			v1[pos][2] = vertices[i + j*horiz][2];
			v1[pos][3] = 1.0;

		}
	}

	for (int j = 0; j < newVert; j++) {
		for (int i = 0; i < newHoriz; i++) {

			int pos = i + j*newHoriz;
			int count = 0;

			if (v1[pos][0] < (-1e5)) {
				
				v1[pos][0] = 0;
				v1[pos][1] = 0;
				v1[pos][2] = 0;
				v1[pos][3] = 1.0;

				if (i - 1 >= 0) {
					v1[pos][0] += v1[pos - 1][0];
					v1[pos][1] += v1[pos - 1][1];
					v1[pos][2] += v1[pos - 1][2];
					count++;
				}

				if (i + 1 < newHoriz) {
					v1[pos][0] += v1[pos + 1][0];
					v1[pos][1] += v1[pos + 1][1];
					v1[pos][2] += v1[pos + 1][2];
					count++;
				}
				
				if (j - 1 >= 0) {
					v1[pos][0] += v1[pos - newHoriz][0];
					v1[pos][1] += v1[pos - newHoriz][1];
					v1[pos][2] += v1[pos - newHoriz][2];
					count++;
				}

				if (j + 1 < newHoriz) {
					v1[pos][0] += v1[pos + newHoriz][0];
					v1[pos][1] += v1[pos + newHoriz][1];
					v1[pos][2] += v1[pos + newHoriz][2];
					count++;
				}

				v1[pos][0] /= count;
				v1[pos][1] /= count;
				v1[pos][2] /= count;

			}
		}
	}


	GLfloat fnormals[38][3]; // face normal for every face (38 faces)
	GLfloat normals[25][3]; // number of vertices (25 vertices)

	GLuint indexes[40]; 


	GLuint nIndexes[144];

	int i = 0;

	for (int row = 0; row < vert - 1; row++) {
		if ((row & 1) == 0) { // even rows
			for (int col = 0; col<vert; col++) { // modified version of the formula from one of the examples you gave me.
				indexes[i++] = row + col * vert; // It seems to work correctly.
				indexes[i++] = (row + 1) + col * vert;		
			}
		}
		else { // odd rows
			for (int col = horiz - 1; col >= 0; col--) {
				indexes[i++] = (row) + col * horiz;
				indexes[i++] = (row + 1) + col * horiz;
			}
		}
	}

	for (int row = 0; row < newVert - 1; row++) {
		if ((row & 1) == 0) { // even rows
			for (int col = 0; col<newVert; col++) { // modified version of the formula from one of the examples you gave me.
				nIndexes[i++] = row + col * newVert; // It seems to work correctly.
				nIndexes[i++] = (row + 1) + col * newVert;
			}
		}
		else { // odd rows
			for (int col = newHoriz - 1; col >= 0; col--) {
				nIndexes[i++] = (row)+col * newHoriz;
				nIndexes[i++] = (row + 1) + col * newHoriz;
			}
		}
	}

	
	GLfloat arrHolder[3];
	GLfloat test1, test2, test3;

	bool toggle = false;

	for (int i = 0; i < 38; i++) { // for every index...

		if (toggle == false) {
			findNormal(vertices[indexes[i]], vertices[indexes[i + 1]], vertices[indexes[i + 2]], arrHolder); // find the normal of the face
			toggle = true;
		}
		else {
			findNormal(vertices[indexes[i]], vertices[indexes[i + 2]], vertices[indexes[i + 1]], arrHolder); // alternate the direction of the normal to 
			toggle = false;																					 // compensate for the direction change in the indices formula
		}
		

		fnormals[i][0] = arrHolder[0];
		fnormals[i][1] = arrHolder[1];
		fnormals[i][2] = arrHolder[2];
	}

	int count;
	for (int i = 0; i < 25; i++) { // for every vertex...
		count = 0;

		normals[i][0] = 0;
		normals[i][1] = 0; // set vertex normals to 0
		normals[i][2] = 0;

		for (int j = 0; j < 38; j++) { // for every face...
			if (indexes[j] == i || indexes[j + 1] == i || indexes[j + 2] == i) {  // if the face contains the vertex
				count++; // increment the count
				normals[i][0] += fnormals[j][0]; // add the face normal to the vertex normal 
				normals[i][1] += fnormals[j][1];
				normals[i][2] += fnormals[j][2];
			}
		}

		normals[i][0] /= count; // divide the normal by the count to average
		normals[i][1] /= count;
		normals[i][2] /= count;
	}

	//normals[1][0] = 1;
	//normals[1][1] = 1;  // used for debugging to figure out what normals were mapped to where in the image
	//normals[1][2] = 1;

	triangles = 38;

	int size = nv * 4;
	int size1 = nv1 * 4;

	//std::cout << sizeof(test) << std::endl;
	std::cout << size * sizeof(GLfloat) << std::endl;

	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, size1 * sizeof(GLfloat) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size1 * sizeof(GLfloat), v1);
	glBufferSubData(GL_ARRAY_BUFFER, size1 * sizeof(GLfloat), sizeof(normals), normals);

	glGenBuffers(1, &ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(nIndexes), nIndexes, GL_STATIC_DRAW);

	glUseProgram(program);
	vPosition = glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(program, "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*) sizeof(v1));
	glEnableVertexAttribArray(vNormal);

}

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).

	if (h == 0)
		h = 1;

	float ratio = 1.0 * w / h;

	glViewport(0, 0, w, h);

	projection = glm::perspective(45.0f, ratio, 1.0f, 100.0f);

}

void displayFunc(void) {
	glm::mat4 view;
	int viewLoc;
	int projLoc;
	int colourLoc;
	int eyeLoc;
	int lightLoc;
	int materialLoc;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);

	view = glm::lookAt(glm::vec3(eyex, eyey, eyez),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f));

	viewLoc = glGetUniformLocation(program, "modelView");
	glUniformMatrix4fv(viewLoc, 1, 0, glm::value_ptr(view));
	projLoc = glGetUniformLocation(program, "projection");
	glUniformMatrix4fv(projLoc, 1, 0, glm::value_ptr(projection));

	colourLoc = glGetUniformLocation(program, "colour");
	glUniform4f(colourLoc, 1.0, 0.0, 0.0, 1.0);
	eyeLoc = glGetUniformLocation(program, "Eye");
	glUniform3f(eyeLoc, eyex, eyey, eyez);
	lightLoc = glGetUniformLocation(program, "light");
	glUniform3f(lightLoc, 1.0, 1.0, 1.0);
	materialLoc = glGetUniformLocation(program, "material");
	glUniform4f(materialLoc, 0.3, 0.7, 0.7, 150.0);

	glBindVertexArray(objVAO);
	glDrawElements(GL_TRIANGLE_STRIP, 3 * triangles, GL_UNSIGNED_INT, NULL);

	glutSwapBuffers();
}

void keyboardFunc(unsigned char key, int x, int y) {

	switch (key) {
	case 'a':
		phi -= 0.1;
		break;
	case 'd':
		phi += 0.1;
		break;
	case 'w':
		theta += 0.1;
		break;
	case 's':
		theta -= 0.1;
		break;
	}

	eyex = r*sin(theta)*cos(phi);
	eyey = r*sin(theta)*sin(phi);
	eyez = r*cos(theta);

	glutPostRedisplay();

}

int main(int argc, char **argv) {
	int fs;
	int vs;
	int user;

	std::string in;
	std::fstream file("heightmap.txt");

	map = new GLfloat[25];//[horiz*vert];

	for (int i = 0; i < horiz * vert; i++) {
		file >> map[i];
	}

	file.close();


	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(320, 320);
	glutCreateWindow("Viewer");
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		printf("Error starting GLEW: %s\n", glewGetErrorString(error));
		exit(0);
	}

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keyboardFunc);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	vs = buildShader(GL_VERTEX_SHADER, "lab2.vs");
	fs = buildShader(GL_FRAGMENT_SHADER, "lab2.fs");
	program = buildProgram(vs, fs, 0);
	dumpProgram(program, "Lab 2 shader program");
	init();

	eyex = 0.0;
	eyez = 5.0;
	eyey = 0.0;

	theta = 1.5;
	phi = 1.5;
	r = 10.0;

	glutMainLoop();

}
