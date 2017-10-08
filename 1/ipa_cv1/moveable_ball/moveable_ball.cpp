#include <iostream>
#include <windows.h>
#include "GL/glut.h"

#define PI 3.14159265f

typedef void (*Update)(GLfloat a, GLfloat b, GLfloat c, GLfloat d);
typedef GLfloat (*GetBallY)();
typedef GLfloat (*GetBallX)();

int windowWidth = 640;     // Windowed mode's width
int windowHeight = 480;     // Windowed mode's height
int windowPosX = 50;      // Windowed mode's top-left corner x
int windowPosY = 50;      // Windowed mode's top-left corner y

GLfloat ballRadius = 0.1f;   // Radius of the bouncing ball
GLfloat ballXMax, ballXMin, ballYMax, ballYMin; // Ball's center (x, y) bounds
int refreshMillis = 20;      // Refresh period in milliseconds

Update _Update;
GetBallY _GetBallY;
GetBallX _GetBallX;
// Projection clipping area
GLdouble clipAreaXLeft, clipAreaXRight, clipAreaYBottom, clipAreaYTop;


void display()
{
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer
	glMatrixMode(GL_MODELVIEW); // To operate on the model-view matrix
	glLoadIdentity(); // Reset model-view matrix

	GLfloat y = _GetBallY();
	GLfloat x = _GetBallX();
	std::cout << "X: " << x << " Y: " << y << std::endl;
	glTranslatef(x, y, 0.0f);  // Translate to (xPos, yPos)

	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.0f, 0.0f, 1.0f);  // Blue
	glVertex2f(0.0f, 0.0f);       // Center of circle
	int numSegments = 100;
	GLfloat angle;
	for (int i = 0; i <= numSegments; i++) { // Last vertex same as first vertex
		angle = i * 2.0f * PI / numSegments;  // 360 deg for all segments
		glVertex2f(cos(angle) * ballRadius, sin(angle) * ballRadius);
	}
	glEnd();

	glutSwapBuffers(); // Swap front and back buffers (of double buffered mode)
}


void Timer(int value) 
{
	glutPostRedisplay(); // Post a paint request to activate display()
	glutTimerFunc(refreshMillis, Timer, 0); // subsequent timer call at milliseconds
}


/* Call back when the windows is re-sized */
void reshape(GLsizei width, GLsizei height) 
{
	// Compute aspect ratio of the new window
	if (height == 0) 
	{
		height = 1; // To prevent divide by 0
	}
	GLfloat aspect = (GLfloat) width / (GLfloat) height;

	// Set the viewport to cover the new window
	glViewport(0, 0, width, height);

	// Set the aspect ratio of the clipping area to match the viewport
	glMatrixMode(GL_PROJECTION); // To operate on the Projection matrix
	glLoadIdentity(); // Reset the projection matrix
	if (width >= height) 
	{
		clipAreaXLeft = -1.0 * aspect;
		clipAreaXRight = 1.0 * aspect;
		clipAreaYBottom = -1.0;
		clipAreaYTop = 1.0;
	}
	else 
	{
		clipAreaXLeft = -1.0;
		clipAreaXRight = 1.0;
		clipAreaYBottom = -1.0 / aspect;
		clipAreaYTop = 1.0 / aspect;
	}
	gluOrtho2D(clipAreaXLeft, clipAreaXRight, clipAreaYBottom, clipAreaYTop);
	ballXMin = (GLfloat) (clipAreaXLeft + ballRadius);
	ballXMax = (GLfloat) (clipAreaXRight - ballRadius);
	ballYMin = (GLfloat)(clipAreaYBottom + ballRadius);
	ballYMax = (GLfloat) (clipAreaYTop - ballRadius);
	std::cout << ballYMax << std::endl;

	_Update(ballXMin, ballYMin, ballXMax, ballYMax);
}


int main()
{
	HINSTANCE hInstLibrary = LoadLibrary("IPA_DLL.dll");

	if (!hInstLibrary)
	{
		std::cout << "DLL Failed To Load!" << std::endl;
	}
	else
	{
		_Update = (Update) GetProcAddress(hInstLibrary, "update");
		_GetBallY = (GetBallY) GetProcAddress(hInstLibrary, "getY");
		_GetBallX = (GetBallX)GetProcAddress(hInstLibrary, "getX");
		if (_Update && _GetBallY && _GetBallX)
		{
			int argc = 1;
			char *argv[1] = { (char*) "IPA" };
			glutInit(&argc, argv); // Initialize GLUT
			glutCreateWindow("IPA cv1"); // Create a window with the given title
			glutInitDisplayMode(GLUT_DOUBLE); // Enable double buffered mode
			glutReshapeFunc(reshape); // Register callback handler for window re-shape
			glutInitWindowSize(windowWidth, windowHeight); // Initial window width and height
			glutInitWindowPosition(windowPosX, windowPosY); // Initial window top-left corner (x, y)
			glutTimerFunc(0, Timer, 0); // First timer call immediately
			glutDisplayFunc(display); // Register display callback handler for window re-paint
			glutMainLoop(); // Enter the event-processing loop
		}

		FreeLibrary(hInstLibrary);
	}

	return 0;
}
