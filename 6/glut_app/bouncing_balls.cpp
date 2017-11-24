/*#IPA - lab6 template (boucing ball paralleization by AVX instruction)
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
#include <thread>
#include "ipa_tool.h"

#define M_PI 3.141592653589
#define SPHERS_COUNT 4096
#define G 9.82
#define ASM

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
void createRenderList();							
void createRenderList_th2();
void makeSphere(GLuint sphereID, float radius);				// Make sphere.

bool* keyStates = new bool[256]; // Create an array of boolean values of length 256 (0-255)  

float h0 = 160.0;
float vmax = sqrtf(2 * 160 * (float)G);

__declspec(align(64))struct t_sphere_array {
	float x[SPHERS_COUNT];
	float y[SPHERS_COUNT];
	float z[SPHERS_COUNT];
	float r[SPHERS_COUNT];
	float v[SPHERS_COUNT];
	float q[SPHERS_COUNT];
	
	uint32_t color[SPHERS_COUNT];
};


extern "C" void BouncingBall_ASM(t_sphere_array * _spheres, int count, float dt);
void thread_function();
enum Color { red=1, green, blue };
t_sphere_array spheres;
t_sphere_array spheres_th2;

float dt = (float)0.1;
unsigned __int64 counter = 0;
unsigned __int64 counter2 = 0;
unsigned __int64 average = 0;
unsigned __int64 average2 = 0;
int step_on = 0;
int end = 0;

void keyPressed(unsigned char key, int x, int y)
{
	keyStates[key] = true; // Set the state of the current key to pressed  
}

void keyUp(unsigned char key, int x, int y)
{
	keyStates[key] = false; // Set the state of the current key to pressed  
}

double Guassian(int x,int y, double sigma)
{
	double c = 2.0 * sigma * sigma;
	double r = sqrt(x*x + y*y);
	
	return  (exp(-(r*r) / c)) / (M_PI * c);;
}

void generateSpheres(t_sphere_array * _spheres,float y)
{
	for (int i = 0;i < SPHERS_COUNT;i++)
	{
		_spheres->x[i] = (float)((i * 5) % 320) - 160;
		_spheres->y[i] = y;
		_spheres->z[i] = (float)(int((i * 5) / 320)) * 5 - 160;
		//spheres.q[i] = -((double)rand() / (RAND_MAX))*0.75;
		//_spheres->q[i] = -(((float)((i * 5) % 320) / (float)330.0)*((int((i * 5) / 320)) * 5) / (float)330.0);
		_spheres->q[i] = -40000*(Guassian((((i * 5) % 320))-160, ((int((i * 5) / 320)) * 5) - 160, 90.0));
		_spheres->v[i] = 0.0;
		_spheres->color[i] = 1;
		_spheres->r[i] = (float)2.0;
	}
}
int main(int argc, char *argv[])
{
	//SetProcessAffinityMask(GetCurrentProcess(), 1);
	//std::thread threadObj(thread_function);
	
	L = 320;phi = 0; theta = 0;
	generateSpheres(&spheres,0);
	//generateSpheres(&spheres_th2, 50.0);

	//GLUT initi
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(xWindowSize, yWindowSize);
	glutCreateWindow("IPA - lab 6");

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
	end = 1;
	//threadObj.join();
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



void BouncingBall(t_sphere_array * _spheres, int count, float dt)
{
	for (int i = 0;i < SPHERS_COUNT;i++)
	{
		float v_old = _spheres->v[i];
		float speed_new= _spheres->v[i] + (float)G*dt;
		float y_new = _spheres->y[i] - v_old*dt + (float)0.5*(float)G*dt*dt;
		
		_spheres->y[i] = y_new;
		_spheres->v[i] = speed_new;

		if (y_new < -160.0)
		{
			_spheres->v[i] = spheres.v[i] * spheres.q[i];
			_spheres->y[i] = -160.0;
		}
		
	}

}

void takeStep() {

	CycleCounter ticks;
	ticks.start();
	step_on = 1;
#ifdef ASM
	BouncingBall_ASM(&spheres, SPHERS_COUNT, dt);
#else
	BouncingBall(&spheres, SPHERS_COUNT, dt);
#endif 
	counter++;
	average += ticks.getCyclesCount();
	if (counter % 100 == 0)
		printf("%" PRIu64 "\n", average/counter);
	createRenderList();
	glutPostRedisplay();
}


void thread_function()
{
	SetThreadAffinityMask(GetCurrentThread(), 2);
	CycleCounter ticks;
	ticks.start();
	while (!end)
	{
		while (!(step_on || end)) {             // wait until main() sets ready...
			std::this_thread::yield();
		}
		CycleCounter ticks;
		ticks.start();
		BouncingBall_ASM(&(spheres_th2), SPHERS_COUNT, dt);
		step_on = 0;

		counter2++;
		average2 += ticks.getCyclesCount();
		if (counter2 % 100 == 0)
		{
			printf("TH2: %" PRIu64 "\n", average2 / counter2);
		}
	}
	
}

void DrawCube(void)
{

	glMatrixMode(GL_MODELVIEW);
	// clear the drawing buffer.
	glClear(GL_COLOR_BUFFER_BIT);


	glBegin(GL_QUADS);        // Draw The Cube Using quads
	glColor3f(0.0f, 1.0f, 0.0f);    // Color Blue
	glVertex3f(1.0f, 1.0f, -1.0f);    // Top Right Of The Quad (Top)
	glVertex3f(-1.0f, 1.0f, -1.0f);    // Top Left Of The Quad (Top)
	glVertex3f(-1.0f, 1.0f, 1.0f);    // Bottom Left Of The Quad (Top)
	glVertex3f(1.0f, 1.0f, 1.0f);    // Bottom Right Of The Quad (Top)
	glColor3f(1.0f, 0.5f, 0.0f);    // Color Orange
	glVertex3f(1.0f, -1.0f, 1.0f);    // Top Right Of The Quad (Bottom)
	glVertex3f(-1.0f, -1.0f, 1.0f);    // Top Left Of The Quad (Bottom)
	glVertex3f(-1.0f, -1.0f, -1.0f);    // Bottom Left Of The Quad (Bottom)
	glVertex3f(1.0f, -1.0f, -1.0f);    // Bottom Right Of The Quad (Bottom)
	glColor3f(1.0f, 0.0f, 0.0f);    // Color Red    
	glVertex3f(1.0f, 1.0f, 1.0f);    // Top Right Of The Quad (Front)
	glVertex3f(-1.0f, 1.0f, 1.0f);    // Top Left Of The Quad (Front)
	glVertex3f(-1.0f, -1.0f, 1.0f);    // Bottom Left Of The Quad (Front)
	glVertex3f(1.0f, -1.0f, 1.0f);    // Bottom Right Of The Quad (Front)
	glColor3f(1.0f, 1.0f, 0.0f);    // Color Yellow
	glVertex3f(1.0f, -1.0f, -1.0f);    // Top Right Of The Quad (Back)
	glVertex3f(-1.0f, -1.0f, -1.0f);    // Top Left Of The Quad (Back)
	glVertex3f(-1.0f, 1.0f, -1.0f);    // Bottom Left Of The Quad (Back)
	glVertex3f(1.0f, 1.0f, -1.0f);    // Bottom Right Of The Quad (Back)
	glColor3f(0.0f, 0.0f, 1.0f);    // Color Blue
	glVertex3f(-1.0f, 1.0f, 1.0f);    // Top Right Of The Quad (Left)
	glVertex3f(-1.0f, 1.0f, -1.0f);    // Top Left Of The Quad (Left)
	glVertex3f(-1.0f, -1.0f, -1.0f);    // Bottom Left Of The Quad (Left)
	glVertex3f(-1.0f, -1.0f, 1.0f);    // Bottom Right Of The Quad (Left)
	glColor3f(1.0f, 0.0f, 1.0f);    // Color Violet
	glVertex3f(1.0f, 1.0f, -1.0f);    // Top Right Of The Quad (Right)
	glVertex3f(1.0f, 1.0f, 1.0f);    // Top Left Of The Quad (Right)
	glVertex3f(1.0f, -1.0f, 1.0f);    // Bottom Left Of The Quad (Right)
	glVertex3f(1.0f, -1.0f, -1.0f);    // Bottom Right Of The Quad (Right)
	glEnd();            // End Drawing The Cube
	glFlush();
}

void createRenderList()
{
	// Create new list.
	glNewList(configID, GL_COMPILE);
	glColor3f(0.0, 1.0, 0.0);

	/*for (int i = 0;i < SPHERS_COUNT;i++)
	{
		glPushMatrix();
		glTranslated(spheres_th2.x[i], spheres_th2.y[i], spheres_th2.z[i]); // Set position of sphere/particle.
		glScaled(spheres_th2.r[i], spheres_th2.r[i], spheres_th2.r[i]);
		glCallList(sphereID);
		//DrawCube();
		glPopMatrix();
	}*/


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


void createRenderList_th2()
{

}



// Make sphere.
void makeSphere(GLuint sphereID, float radius)
{
	int nTheta = 3;                       // Number of polar angle slices.
	int nPhi = 3;                        // Number of azimuthal angle slices.
	glNewList(sphereID, GL_COMPILE);
	glutSolidSphere(radius, nPhi, nTheta);
	glEndList();
}