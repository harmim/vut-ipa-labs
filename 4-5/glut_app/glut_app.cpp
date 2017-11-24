/*#IPA - lab5 solution
*Autor: Tomas Goldmann, igoldmann@fit.vutbr.cz, cast kodu prevzata z cviceni IPA od autora Davida Hermana
*
*LOGIN STUDENTA: igoldmann
*/

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <windows.h>
#include <stddef.h>
#include <malloc.h>
#include <xmmintrin.h>
#include <GL/freeglut.h>

#include "ipa_algorithm.h"
#include "ipa_tool.h"

#define M_PI 3.141592653589
//#define ASM_OPTIM
// Lab 5 a lab 6
#define SPHERS_COUNT 512

double L;								  // SIZE OF DIMENSION
double radius = 2.0;                      // radius of molecule
double minExtent[3], maxExtent[3];        // extent of system volume
int xWindowSize = 640, yWindowSize = 640; // window size in screen pixels
GLdouble aspectRatio;                     // window aspect ratio
GLdouble fovy, nearClip, farClip;         // variables for 3D projection
GLdouble eye[3], center[3], up[3];        // more projection variables
GLuint sphereID, configID;                // display list ID numbers
int phi, theta;                           // to rotate system using arrow keys
int angle = 5;                            // rotation angle in degrees

										  // Declarations.
void initView(double *minExtent, double *maxExtent);		// Init GL.
void display();												// Display.
void reshape(GLsizei width, GLsizei height);				// Reshape.
void takeStep();											// Take a step.
void createRenderList();										// Make particles.
void makeSphere(GLuint sphereID, float radius);				// Make sphere.

bool* keyStates = new bool[256]; // Create an array of boolean values of length 256 (0-255)  

__declspec(align(32))struct t_sphere_array {
	float x[SPHERS_COUNT];
	float y[SPHERS_COUNT];
	float z[SPHERS_COUNT];
	float r[SPHERS_COUNT];
	float m[SPHERS_COUNT];
	float vectorX[SPHERS_COUNT];
	float vectorY[SPHERS_COUNT];
	float vectorZ[SPHERS_COUNT];
	uint32_t color[SPHERS_COUNT];
};

extern "C" void optim_function(t_sphere_array * _spheres, int count, float dt);

enum Color { red=1, green, blue };

t_sphere_array spheres;

float dt = (float)0.08;
unsigned __int64 counter = 2;
unsigned __int64 average = 0;
unsigned __int64 average_c = 0;

void keyPressed(unsigned char key, int x, int y)
{
	keyStates[key] = true; // Set the state of the current key to pressed  
}

void keyUp(unsigned char key, int x, int y)
{
	keyStates[key] = false; // Set the state of the current key to pressed  
}
															// Main.
int main(int argc, char *argv[])
{
	L = 320;					// Size of the cube.

	phi = 0; theta = 0;

	for (int i = 0;i < SPHERS_COUNT;i++)
	{

		spheres.x[i] =(float) (rand() % 320) - 160;
		spheres.y[i] = (float)(rand() % 320) - 160;
		spheres.z[i] = (float)(rand() % 320) - 160;
	
		spheres.vectorX[i] = ((float)rand() / ((RAND_MAX) / 2)) - (float)1.0;
		spheres.vectorY[i] = ((float)rand() / ((RAND_MAX) / 2)) - (float)1.0;
		spheres.vectorZ[i] = ((float)rand() / ((RAND_MAX) / 2)) - (float)1.0;

		spheres.r[i] = (float)((rand() % 5) + 1);
		spheres.m[i] = spheres.r[i]*((rand() % 5) + 1);
	}

	spheres.x[SPHERS_COUNT - 1] = (float)10.0;
	spheres.y[SPHERS_COUNT - 1] = (float)15.0;
	spheres.z[SPHERS_COUNT - 1] = (float)10.0;
	spheres.r[SPHERS_COUNT - 1] = (float)2.0;

	spheres.x[SPHERS_COUNT - 2] = (float)9.2;
	spheres.y[SPHERS_COUNT - 2] = (float)10.2;
	spheres.z[SPHERS_COUNT - 2] = (float)12.2;
	spheres.r[SPHERS_COUNT - 2] = (float)2.0;

	spheres.x[SPHERS_COUNT - 3] = (float)10.5;
	spheres.y[SPHERS_COUNT - 3] = (float)10.0;
	spheres.z[SPHERS_COUNT - 3] = (float)13.1;
	spheres.r[SPHERS_COUNT - 3] = (float)2.0;

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(xWindowSize, yWindowSize);
	glutCreateWindow("IPA - lab 5 a lab 6");

	// Set cube boundaries.
	for (int i = 0; i < 3; i++) {
		minExtent[i] = -L / 2;
		maxExtent[i] = L / 2;
	}

	initView(minExtent, maxExtent);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(takeStep);
	glutKeyboardFunc(keyPressed); 
	glutKeyboardUpFunc(keyUp);

	sphereID = glGenLists(1);
	makeSphere(sphereID, 1.0);
	configID = glGenLists(1);

	glutMainLoop();
}

void initView(double *minExtent, double *maxExtent) {
	// use a single light source to illuminate the scene
	GLfloat lightDiffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat lightPosition[] = { 0.5, 0.5, 1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);              // use single light number 0
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);

	// compute the distance scale of the system
	double difExtent[3];
	for (int i = 0; i < 3; i++)
		difExtent[i] = maxExtent[i] - minExtent[i];
	double dist = 0;
	for (int i = 0; i < 3; i++)
		dist += difExtent[i] * difExtent[i];
	dist = sqrt(dist);

	// locate the center of the system, camera position, and orientation
	for (int i = 0; i < 3; i++)
		center[i] = minExtent[i] + difExtent[i] / 2;

	eye[0] = center[0];
	eye[1] = center[1];
	eye[2] = center[2] + dist;        // along z axis is towards viewer
	up[0] = 0;
	up[1] = 1;                        // y axis is up
	up[2] = 0;

	// set up clipping planes, field of view angle in degrees in y direction
	nearClip = (dist - difExtent[2] / 2.0) / 2.0;
	farClip = 2.0 * (dist + difExtent[2] / 2.0);

	fovy = difExtent[1] / (dist - difExtent[2] / 2.0) / 2.0;
	fovy = 2.0 * atan(fovy) / M_PI * 180.0;
	fovy *= 1.2;
}

// Display.
void display() {
	// Clear buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Set camera.
	gluLookAt(eye[0], eye[1], eye[2],			// Where the camera is.
		center[0], center[1], center[2],	// What the camera see (point).
		up[0], up[1], up[2]);				// Orientation of the camera.

											// Draw particles.
	glCallList(configID);
	// Swap buffers.
	glutSwapBuffers();
}

// Handler for window's re-size event.
void reshape(GLsizei width, GLsizei height)
{
	// Prevent divide by 0.
	if (height == 0) height = 1;

	// Compute new aspect ratio.
	aspectRatio = width / double(height);

	// Set the viewport to cover entire application window.
	// View port is in pixels in Window's coordinates (origin at top left corner).
	// Projection plane is transformed via Viewport transformation to viewport on screen.
	glViewport(0, 0, width, height);

	// Select the aspect ratio of the clipping area to mach the viewport.
	glMatrixMode(GL_PROJECTION);	// Select the projection matrix.
	glLoadIdentity();				// Reset the projection matrix.

									// fovy is the angle between the mottom and top of the projectors.
									// aspectRatio is the ratio of width and height of the front clipping plane.
									// zNear and zFar specify the front and back clipping planes.
									// Coordinates of the clipping volume are relative to the camera's eye position.
	gluPerspective(fovy, aspectRatio, nearClip, farClip);

	// Reset the Model View matrix.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void nbody()
{
	for (int i = 0;i < SPHERS_COUNT;i++)
	{
		for (int j = i + 1;j < SPHERS_COUNT;j++)
		{
			if (i != j)
			{

				float vector_length = sqrt((spheres.x[i] - spheres.x[j])*(spheres.x[i] - spheres.x[j]) + (spheres.y[i] - spheres.y[j])*(spheres.y[i] - spheres.y[j]) + (spheres.z[i] - spheres.z[j])*(spheres.z[i] - spheres.z[j]));

				if (vector_length< (spheres.r[i] + spheres.r[j]))
				{
					float n_x = spheres.x[i] - spheres.x[j];
					float n_y = spheres.y[i] - spheres.y[j];
					float n_z = spheres.z[i] - spheres.z[j];


					float n_x_norm = n_x / vector_length;
					float n_y_norm = n_y / vector_length;
					float n_z_norm = n_z / vector_length;


					float a1_dot = spheres.vectorX[i] * n_x_norm + spheres.vectorY[i] * n_y_norm + spheres.vectorZ[i] * n_z_norm;
					float a2_dot = spheres.vectorX[j] * n_x_norm + spheres.vectorY[j] * n_y_norm + spheres.vectorZ[j] * n_z_norm;

					float P = ((float)2.0*(a1_dot - a2_dot)) / (spheres.m[i] + spheres.m[j]);


					spheres.vectorX[i] = spheres.vectorX[i] - P*spheres.m[j] * n_x_norm;
					spheres.vectorY[i] = spheres.vectorY[i] - P*spheres.m[j] * n_y_norm;
					spheres.vectorZ[i] = spheres.vectorZ[i] - P*spheres.m[j] * n_z_norm;
					spheres.vectorX[j] = spheres.vectorX[j] + P*spheres.m[i] * n_x_norm;
					spheres.vectorY[j] = spheres.vectorY[j] + P*spheres.m[i] * n_y_norm;
					spheres.vectorZ[j] = spheres.vectorZ[j] + P*spheres.m[i] * n_z_norm;

					spheres.color[i] = green;

				}
			}
		}
		spheres.x[i] = spheres.x[i] + spheres.vectorX[i] * dt;
		spheres.y[i] = spheres.y[i] + spheres.vectorY[i] * dt;
		spheres.z[i] = spheres.z[i] + spheres.vectorZ[i] * dt;
	}
	return;
}

//DOPLNIT
// Take step.
void takeStep() {

	CycleCounter ticks;
	ticks.start();

//#ifdef ASM_OPTIM

	if(counter%2==0)
	{
		optim_function(&spheres, SPHERS_COUNT, dt);
		average += ticks.getCyclesCount();
	}
	else
	{
//#else
	nbody();
	//average_c += ticks.getCyclesCount();
	}
//#endif	
	counter++;
	double asm_frac = (double)average / (counter / 2.0);
	double c_frac = (double)average_c / (counter / 2.0);

	double frac = (c_frac) / (asm_frac);

	printf("ASM: %f   C: %f  %f\n", asm_frac,c_frac, frac);

	//ticks.print();
	createRenderList();
	glutPostRedisplay();

}


void createRenderList()
{
	// Create new list.
	glNewList(configID, GL_COMPILE);
	glColor3f(0.0, 1.0, 0.0);

	for (int i = 0;i < SPHERS_COUNT;i++)
	{
		glPushMatrix();
		if (spheres.color[i] == red)
		{
			glColor3f(1.0, 0.0, 0.0);

		}
		else if (spheres.color[i] == blue)
		{
			glColor3f(0.0, 0.0, 1.0);
		}
		else if (spheres.color[i] == green)
		{
			glColor3f(0.0, 1.0, 0.0);
		}
		else
		{
			glColor3f(1.0, 1.0, 1.0);
		}
		//
		glTranslated(spheres.x[i], spheres.y[i], spheres.z[i]); // Set position of sphere/particle.
		glScaled(spheres.r[i], spheres.r[i], spheres.r[i]);
		glCallList(sphereID);
		glPopMatrix();
	}

	// Space.
	glColor3ub(255, 255, 255);		// White color.
	glutWireCube(L);				// Cubical space.

	glEndList();
}

// Make sphere.
void makeSphere(GLuint sphereID, float radius)
{
	int nTheta = 9;                       // Number of polar angle slices.
	int nPhi = 18;                        // Number of azimuthal angle slices.
	glNewList(sphereID, GL_COMPILE);
	glutSolidSphere(radius, nPhi, nTheta);
	glEndList();
}