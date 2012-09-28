#include <stdlib.h>
#include <stdio.h>
#include <gl/glut.h>
#include "glm/glm.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include "common.h"
using namespace std;
using namespace Raytracer;

#define APP_NAME "Assignment 2"

int winW = 256;
int winH = 256;

float d = 0;

# define MODE_OPEN_GL 0
# define MODE_RAY_CAST 1

int mode = MODE_RAY_CAST;

GLfloat light_position[] = {0.0, 5.0, 0.0};

GLMmodel *model;

void glutDisplay();
void glutKeyboard(unsigned char key, int x, int y);
void glutMouse(int button, int state, int x, int y);
void glutResize(int width, int height);
//void parseObjFile();
void getObjModel();
int doesRayIntersectTriangle(vector3 rayPoint, vector3 rayDirection, vector3 p0, vector3 p1, vector3 p2);

void main() {
	getObjModel();

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(winW, winH);
	glutCreateWindow(APP_NAME);
	glutDisplayFunc(glutDisplay);
	glutReshapeFunc(glutResize);
	glutKeyboardFunc(glutKeyboard);
	glutMouseFunc(glutMouse);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_COLOR_MATERIAL);

	glutMainLoop();
}

void glutResize(int width, int height) {
	winW = width;
	winH = height;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, winW, winH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(90, winW / winH, -1, 1);

	glMatrixMode(GL_MODELVIEW);

	gluLookAt(0, 0, -2, // eye position
			  0, 0, 1,  // look at vector
			  0, 1, 0); // up vector

	glutPostRedisplay();
}

void glutKeyboard(unsigned char key, int x, int y) {
	if (key == ' ') {
		mode = !mode;
	} else if (key == 'a') {
		d += .1;
		if (d > 2)
			d = -2;
	}

	glutPostRedisplay();
}

void glutMouse(int button, int state, int x, int y) {
	printf("clicked at (%d, %d)\n", x, y);
}

void glutDisplay() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glColor3f(255, 255, 255);

	glPushMatrix();
	if (mode == MODE_OPEN_GL) {
		int i, j;
		glTranslatef(0, 0, 2);
		glBegin(GL_TRIANGLES);
		//printf("num vertices: %d\n", model->numvertices);
		for (i=0; i<model->numtriangles; i++) {
			for (j=0; j<3; j++) {
				int index = model->triangles[i].vindices[j];
				vector3 vertex(model->vertices[3*index],		// x
								model->vertices[3*index+1],		// y
								model->vertices[3*index+2]);	// z
				printf("printing vertex (%f, %f, %f)\n", vertex.x, vertex.y, vertex.z);
				glVertex3f(vertex.x, vertex.y, vertex.z);
			}
		}
		glEnd();
	} else {
		float i, j;
		int k, l;
		glBegin(GL_POINTS);
		vector3 eye(0, 0, -2);

		/*int numtriangles = model->numtriangles;
		vector3** triangles = new vector3*[numtriangles];
		for (k=0; k<numtriangles; k++) {
			triangles[k] = new vector3[3];
			for (l=0; l<3; l++) {
				int index = model->triangles[k].vindices[l];
				triangles[k][l] = vector3(model->vertices[3*index],		// x
									model->vertices[3*index+1],		// y
									model->vertices[3*index+2]);	// z
			} 
		}*/

		for (i=-1.0f; i<=1.01f; i+= (1.0f/255.0f)) {
			//printf("i: %f\n", i);
			for (j=-1.0f; j<=1.01f; j+= (1.0f/255.0f)) {
				//printf("j: %f\n", j);
				for (k=0; k<model->numtriangles; k++) {
					vector3 vertices[3];
					for (l=0; l<3; l++) {
						int index = model->triangles[k].vindices[l];
						vertices[l] = vector3(model->vertices[3*index],		// x
										model->vertices[3*index+1],		// y
										model->vertices[3*index+2]);	// z
					}
					vector3 direction = vector3(i,j,-1)-eye;
					direction.Normalize();
					//if (i>-0.01 && i<0.01 && j>0.47 && j<0.5)
						//printf("corner");
					int doesIntersect = doesRayIntersectTriangle(eye, direction, vertices[0], vertices[1], vertices[2]);
					if (doesIntersect) {
						glVertex2f(i, j);
						break;
					}
				}
			}
		}
		//glVertex2f(0, 0.5);
		glEnd();
	}

	glPopMatrix();

	glutSwapBuffers();
}

void getObjModel() {
	model = glmReadOBJ("input.obj");
}

// source modified from http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
int doesRayIntersectTriangle(vector3 rayPoint, vector3 rayDirection, vector3 p0, vector3 p1, vector3 p2) {
	vector3 edge1 = p1 - p0;
	vector3 edge2 = p2 - p0;

	vector3 crossOfDirectionAndEdge2 = rayDirection.Cross(edge2);
	float a = edge1.Dot(crossOfDirectionAndEdge2);

	if (a > -0.00001 && a < 0.00001)
		return 0;

	float f = 1.0f/a;
	vector3 s = rayPoint - p0;
	float u = f * (s.Dot(crossOfDirectionAndEdge2));

	if (u < 0.0 || u > 1.0)
		return 0;

	vector3 q = s.Cross(edge1);
	float v = f * (rayDirection.Dot(q));

	if (v < 0.0 || u + v > 1.0)
		return 0;

	float t = f * (edge2.Dot(q));

	if (t > 0.00001)
		return 1;
	else
		return 0;
}

// This parser code was adapted from http://codepad.org/rqZIKFfQ
/*void parseObjFile() {
	string line, type;
	ifstream file("input.obj");
	// For each line in the file
	while (getline(file, line)) {
		// Skip every line we don't know how to handle
		if (line[0] != 'v')
			continue;

		// Create a line stream from the line
		istringstream lineStream(line);
		// Get the type of item this line defines
		lineStream >> type;

		// If this line is a vertex
		if (type == "v") {
			struct vertex vertex;
			// Get the 3 or 4 vertices from the line
			// The line format is: 'v x y z (w)' - w is optional and defaults to 1.0 (TODO should I support w?)
			sscanf(line.c_str(), "%*s %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			//printf("found vertex (%f, %f, %f)\n", vertex.x, vertex.y, vertex.z);
			// Put the vertex array into our vertex vector
			vertices.push_back(vertex);
		}
	}
}*/