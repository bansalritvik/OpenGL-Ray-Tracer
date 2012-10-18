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

# define MODE_OPEN_GL 0
# define MODE_RAY_CAST 1

int mode = MODE_RAY_CAST;
int showShadows = 0;
int showReflectionColor = 0;
int showRefractedColor = 0;
int lightOverhead = 1;

GLfloat light_ambient[] = {1.0, 1.0, 1.0};
GLfloat light_specular[] = {1.0, 1.0, 1.0};
GLfloat light_diffuse[] = {1.0, 1.0, 1.0};
GLfloat light_position[] = {0.0, 5.0, 0.0, 1};

GLMmodel *model;

float angle_v = 0;
float angle_h = 0;

void glutDisplay();
void glutKeyboard(unsigned char key, int x, int y);
void glutSpecial(int key, int xx, int yy);
void glutResize(int width, int height);
void getObjModel();
int findClosestTriangleThatRayIntersects(vector3 rayPoint, vector3 rayDirection, GLMtriangle *intersectedTriangle, GLMgroup *groupForTriangle, vector3 *p0, vector3 *p1, vector3 *p2, vector3 *baryCoords, float *mindepth);
vector3 doesRayIntersectTriangle(vector3 rayPoint, vector3 rayDirection, vector3 p0, vector3 p1, vector3 p2);
int isLightVisibleFromPoint(vector3 point);
vector3 calculateColorForPoint(vector3 normal, vector3 point, vector3 direction, GLMtriangle trianglePointIsOn, GLMmaterial materialOfTriangle, vector3 p0, vector3 p1, vector3 p2, vector3 bary);

void main() {
	getObjModel();

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(winW, winH);
	glutCreateWindow(APP_NAME);
	glutDisplayFunc(glutDisplay);
	glutReshapeFunc(glutResize);
	glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecial);

	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_SMOOTH);
	glDepthFunc(GL_LEQUAL);

	glutMainLoop();
}

void glutResize(int width, int height) {
	winW = width;
	winH = height;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, winW, winH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glFrustum(-1.1, 1.1, -1.1, 1.1, .9, 3.1);
	glMatrixMode(GL_MODELVIEW);
}

void glutKeyboard(unsigned char key, int x, int y) {
	if (key == ' ') {
		mode = !mode;
		if (mode == MODE_OPEN_GL)
			printf("Using OpenGL for rendering\n");
		else
			printf("Using ray casting for rendering\n");
	} else if (key == 's' || key == 'S') {
		showShadows = !showShadows;
		printf("Shadows on? %d\n", showShadows);
	} else if (key == 'r' || key == 'R') {
		showReflectionColor = !showReflectionColor;
		printf("Reflection on? %d\n", showReflectionColor);
	} else if (key == 'f' || key == 'F') {
		showRefractedColor = !showRefractedColor;
		printf("Refraction on? %d\n", showRefractedColor);
	} else if (key == 'l' || key == 'L') {
		lightOverhead = !lightOverhead;
		if (lightOverhead)
			light_position[2] = 0.0f;
		else
			light_position[2] = -5.0f;
		printf("Light overhead? %d\n", lightOverhead);
	} else if (key == 'u' || key == 'U') {
		printf("Unitizing models\n");
		glmUnitize(model);
	}

	glutPostRedisplay();
}

void glutSpecial(int key, int xx, int yy) {
	switch (key) {
	case GLUT_KEY_LEFT:
		angle_h -= 5;
		break;
	case GLUT_KEY_RIGHT:
		angle_h += 5;
		break;
	case GLUT_KEY_UP:
		angle_v += 5;
		break;
	case GLUT_KEY_DOWN:
		angle_v -= 5;
		break;
	}

	glutPostRedisplay();
}

void glutDisplay() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glLoadIdentity();

	gluLookAt(0, 0, -2, // eye position
			  0, 0, 1,  // look at vector
			  0, 1, 0); // up vector

	if (mode == MODE_OPEN_GL) {
		glEnable(GL_LIGHTING);
		int h, i, j;
		glBegin(GL_TRIANGLES);
		GLMgroup *group = model->groups;
		while (group) {
			GLMmaterial material = model->materials[group->material];
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material.ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material.diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material.specular);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.shininess);
			for (i=0; i<group->numtriangles; i++) {
				int triangle = group->triangles[i];
				glNormal3fv(&model->facetnorms[3*model->triangles[triangle].findex]);
				for (j=0; j<3; j++) {
					int index = model->triangles[triangle].vindices[j];
					vector3 vertex(model->vertices[3*index],		// x
									model->vertices[3*index+1],		// y
									model->vertices[3*index+2]);	// z
					glVertex3f(vertex.x, vertex.y, vertex.z);
				}
			}
			group = group->next;
		}
		glEnd();
	} else {
		glDisable(GL_LIGHTING);
		float i, j;
		int k, l;
		glBegin(GL_POINTS);
		vector3 eye(0, 0, -2);

		for (i=-1.0f; i<=1.01f; i+= (1.0f/winW)) {
			for (j=-1.0f; j<=1.01f; j+= (1.0f/winH)) {
				float mindepth = FLT_MAX;
				GLMgroup mingroup;
				GLMtriangle mintriangle;
				vector3 minresult;
				vector3 baryCoords;
				vector3 minp0, minp1, minp2;
				vector3 direction = vector3(i,j,-1)-eye;
						direction.Normalize();
				
				if (findClosestTriangleThatRayIntersects(eye, direction, &mintriangle, &mingroup, &minp0, &minp1, &minp2, &baryCoords, &mindepth)) {
					vector3 normal = (minp1-minp0).Cross(minp2-minp0);
					normal.Normalize();
					vector3 intersection = eye + mindepth*direction;

					//if (i>.02 && i<.05 && j>-.01 && j<.01) // blue
 					//	printf("break\n");
					//else if (i>.05 && i<.1 && j>-.01 && j<.01) // red
					//	printf("break\n");

					vector3 combined = calculateColorForPoint(normal, intersection, direction, mintriangle, model->materials[mingroup.material], minp0, minp1, minp2, baryCoords);

					vector3 reflectedColor;
					if (showReflectionColor) {
						//if (i>.25 && i<.26 && j>-.3 && j<-.29)
						//	printf("break\n");
						float minReflectionDepth = FLT_MAX;
						GLMgroup reflectionGroup;
						GLMtriangle reflectionTriangle;
						vector3 minReflectionResult;
						vector3 reflectionBaryCoords;
						vector3 minrp0, minrp1, minrp2;
						
						vector3 directionToLight = intersection-light_position;//light_position - intersection;
						directionToLight.Normalize();
						vector3 reflection = -2*(directionToLight.Dot(normal))*normal+directionToLight;
						reflection.Normalize();

						if (findClosestTriangleThatRayIntersects(intersection, reflection, &reflectionTriangle, &reflectionGroup, &minrp0, &minrp1, &minrp2, &reflectionBaryCoords, &minReflectionDepth)) {
							vector3 reflectedTriangleNormal = (minrp1-minrp0).Cross(minrp2-minrp0);
							reflectedColor = calculateColorForPoint(reflectedTriangleNormal, intersection, reflection, reflectionTriangle, model->materials[reflectionGroup.material], minrp0, minrp1, minrp2, reflectionBaryCoords);
						}
					}

					vector3 refractedColor;
					if (showRefractedColor) {
						//if (i>.25 && i<.26 && j>-.3 && j<-.29)
						//	printf("break\n");
						float minRefractionDepth = FLT_MAX;
						GLMgroup refractionGroup;
						GLMtriangle refractionTriangle;
						vector3 minrefractionResult;
						vector3 refractionBaryCoords;
						vector3 minrp0, minrp1, minrp2;
						
						vector3 directionFromLight = intersection-light_position;//light_position - intersection;
						directionFromLight.Normalize();
						// refraction code help from http://www.cs.unc.edu/~rademach/xroads-RT/RTarticle.html
						float c1 = -normal.Dot(directionFromLight);
						float n = 1/1.52; // refraction indicies: air/glass
						float c2 = sqrt(1-pow(n,2)*(1-pow(c1,2)));
						vector3 refraction = (n*directionFromLight)+(n*c1-c2)*normal;
						refraction.Normalize();


						if (findClosestTriangleThatRayIntersects(intersection, refraction, &refractionTriangle, &refractionGroup, &minrp0, &minrp1, &minrp2, &refractionBaryCoords, &minRefractionDepth)) {
							vector3 reflectedTriangleNormal = (minrp1-minrp0).Cross(minrp2-minrp0);
							refractedColor = calculateColorForPoint(reflectedTriangleNormal, intersection, refraction, refractionTriangle, model->materials[refractionGroup.material], minrp0, minrp1, minrp2, refractionBaryCoords);
						}
					}

					combined = combined + (.2*reflectedColor) + (.2*refractedColor);
					//if (i>.25 && i<.26 && j>-.3 && j<-.29)
					//	glColor3f(0,0,1);
					//else if (i>.05 && i<.1 && j>-.01 && j<.01)
					//	glColor3f(1,0,0);
					//else
					glColor3f(combined.x, combined.y, combined.z);
					glVertex3f(i, j, -1);
				}
			}
		}
		glEnd();
	}

	glPopMatrix();

	glutSwapBuffers();
}

void getObjModel() {
	model = glmReadOBJ("input.obj");
	glmFacetNormals(model);
	glmVertexNormals(model, 89); // 89 because 90 creates weird shadows on flat surfaces
}

int findClosestTriangleThatRayIntersects(vector3 rayPoint, vector3 rayDirection, GLMtriangle *intersectedTriangle, GLMgroup *groupForTriangle, vector3 *p0, vector3 *p1, vector3 *p2, vector3 *baryCoords, float *mindepth) {
	int k, l;
	*mindepth = FLT_MAX;

	GLMgroup *group = model->groups;
	while (group) {
		for (k=0; k<group->numtriangles; k++) {
			vector3 vertices[3];
			int triangle = group->triangles[k];
			for (l=0; l<3; l++) {
				int index = model->triangles[triangle].vindices[l];
				vertices[l] = vector3(model->vertices[3*index], // x
								model->vertices[3*index+1],		// y
								model->vertices[3*index+2]);	// z
			}
			vector3 result = doesRayIntersectTriangle(rayPoint, rayDirection, vertices[0], vertices[1], vertices[2]);
			if (result.x == FLT_MAX)
				continue;

			float depth = result.x;
			if (depth < *mindepth) {
				*baryCoords = vector3(result);
				*groupForTriangle = *group;
				*intersectedTriangle = model->triangles[triangle];
				*p0 = vector3(vertices[0]);
				*p1 = vector3(vertices[1]);
				*p2 = vector3(vertices[2]);
				*mindepth = depth;
			}
		}
		group = group->next;
	}
	if (*mindepth < FLT_MAX)
		return 1;
	else
		return 0;
}

// source modified from http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
vector3 doesRayIntersectTriangle(vector3 rayPoint, vector3 rayDirection, vector3 p0, vector3 p1, vector3 p2) {
	vector3 edge1 = p1 - p0;
	vector3 edge2 = p2 - p0;

	vector3 crossOfDirectionAndEdge2 = rayDirection.Cross(edge2);
	float a = edge1.Dot(crossOfDirectionAndEdge2);

	if (a > -0.00001 && a < 0.00001)
		return vector3(FLT_MAX, FLT_MAX, FLT_MAX);

	float f = 1.0f/a;
	vector3 s = rayPoint - p0;
	float u = f * (s.Dot(crossOfDirectionAndEdge2));

	if (u < 0.0 || u > 1.0)
		return vector3(FLT_MAX, FLT_MAX, FLT_MAX);

	vector3 q = s.Cross(edge1);
	float v = f * (rayDirection.Dot(q));

	if (v < 0.0 || u + v > 1.0)
		return vector3(FLT_MAX, FLT_MAX, FLT_MAX);

	float t = f * (edge2.Dot(q));

	if (t > 0.00001)
		return vector3(t, u, v);
	else
		return vector3(FLT_MAX, FLT_MAX, FLT_MAX);
}

int isLightVisibleFromPoint(vector3 point) {
	vector3 toLight = light_position - point;
	GLMgroup *group = model->groups;
	while (group) {
		for (int k=0; k<group->numtriangles; k++) {
			vector3 vertices[3];
			int triangle = group->triangles[k];
			for (int l=0; l<3; l++) {
				int index = model->triangles[triangle].vindices[l];
				vertices[l] = vector3(model->vertices[3*index], // x
								model->vertices[3*index+1],		// y
								model->vertices[3*index+2]);	// z
			}
			vector3 result = doesRayIntersectTriangle(point, toLight, vertices[0], vertices[1], vertices[2]);
			if (result.x != FLT_MAX)
				return 0;
		}
		group = group->next;
	}
	return 1;
}

vector3 calculateColorForPoint(vector3 normal, vector3 point, vector3 direction, GLMtriangle trianglePointIsOn, GLMmaterial materialOfTriangle, vector3 p0, vector3 p1, vector3 p2, vector3 bary) {
	// AMBIENT
	vector3 ambient = vector3(light_ambient)*vector3(materialOfTriangle.ambient);
	// DIFFUSE & SPECULAR
	vector3 diffuse, specular;
	if (!showShadows || (showShadows && isLightVisibleFromPoint(point))) {
		vector3 directionToLight = light_position - point;
		directionToLight.Normalize();
		vector3 reflection = -2*(directionToLight.Dot(normal))*normal+directionToLight;
		reflection.Normalize();

		//float denominator = (((p1-p0).Cross(p2-p0)).Dot(normal));
		//float alpha = (((p2-p1).Cross(point-p1)).Dot(normal))/denominator;
		//float beta  = (((p0-p2).Cross(point-p2)).Dot(normal))/denominator;
		//float gamma = (((p1-p0).Cross(point-p0)).Dot(normal))/denominator;

		vector3 p0Norm, p1Norm, p2Norm;
		p0Norm = vector3(&model->normals[3*trianglePointIsOn.nindices[0]]);
		p1Norm = vector3(&model->normals[3*trianglePointIsOn.nindices[1]]);
		p2Norm = vector3(&model->normals[3*trianglePointIsOn.nindices[2]]);

		//normal = alpha*p0Norm + beta*p1Norm + gamma*p2Norm;
		normal = (1-bary.y-bary.z)*p0Norm+bary.y*p1Norm+bary.z*p2Norm;
		normal.Normalize();

		// Add diffuse lighting, if the normal is in line with the directionToLight
		float temp = normal.Dot(directionToLight);
		if (temp >= 0) {
			diffuse = vector3(light_diffuse)*vector3(materialOfTriangle.diffuse)*
				normal.Dot(directionToLight);
		}
		// Add specular lighting, if the reflection is in line with the direction to the eye
		temp = reflection.Dot(direction);
		if (temp >=0) {
			float factor = pow(temp, (materialOfTriangle.shininess*1000/128)); // *1000/128 because glm converts shininess
																			   // to OpenGL's scale, but we don't want that
			specular = vector3(light_specular)*vector3(materialOfTriangle.specular)*factor;
		}
	}

	return ambient+diffuse+specular;
}