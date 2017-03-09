#include <iostream>
#include <GL/glew.h>
#include "camera.h"

using namespace std;
using namespace glm;

//Create the Camera
Camera camera;

class Window {
public:
	Window() {
		this->interval = 1000 / 60;		//60 FPS
		this->window_handle = -1;
	}
	int window_handle, interval; // DECLAR INTEGERS 
	ivec2 size; // DECLARE 2 DIMENSION
	float window_aspect; // DECLARE FLOAT
} window; // CREATE CLASS WINDOW NAMED WINDOW

//Invalidate the window handle when window is closed
void CloseFunc() {
	window.window_handle = -1;
}
//Resize the window and properly update the camera viewport
void ReshapeFunc(int w, int h) {
	if (h > 0) {
		window.size = ivec2(w, h);
		window.window_aspect = float(w) / float(h);
	}
	camera.SetViewport(0, 0, window.size.x, window.size.y); // SET WINDOW
}

//Keyboard input for camera, also handles exit case
void KeyboardFunc(unsigned char c, int x, int y) { // USE OPENGL FUNCTION KEYBOARDFUNC
	switch (c) { // GO TO CASE OF WHICH KEY IS PRESSED
	case 'w': // GO HERE IF LOWER CASE W IS PRESSED
		camera.Move(FORWARD); // MOVE CAMERA FORWARD
		break; // EXIT SWITCH
	case 'a': //GO HERE IF LOWER CASE A IS PRESSED
		camera.Move(LEFT); // MOVE CAMERA LEFT
		break; // EXIT SWITCH
	case 's': // GO HERE IF LOWER CASE S IS PRESSED
		camera.Move(BACK); // MOVE CAMERA BACK
		break; // EXIT SWITCH
	case 'd': // GO HERE IF LOWER CASE D IS PRESSED
		camera.Move(RIGHT); // MOVE CAMERA RIGHT
		break; // EXIT SWITCH
	case 'q': // GO HERE IF LOWER CASE Q IS PRESSED
		camera.Move(DOWN); // MOVE CAMERA DOWN
		break; // EXIT SWITCH
	case 'e': // GO HERE IF LOWER CASE E IS PRESSED
		camera.Move(UP); // MOVE CAMERA UP
		break; // EXIT SWITCH
	case 'x': // GO HERE IF LOWER CASE X IS PRESSED
	case 27: // GO HERE IF ESCAPE KEY IS PRESSED
		exit(0); // END PROGRAM
		return;
	default: // GO HERE IS A KEY NOT SPECIFIED IS PRESSED
		break; // EXIT SWITCH
	}
}

void SpecialFunc(int c, int x, int y) {}
void CallBackPassiveFunc(int x, int y) {}
//Used when person clicks mouse
void CallBackMouseFunc(int button, int state, int x, int y) {
	camera.SetPos(button, state, x, y);
}
//Used when person drags mouse around
void CallBackMotionFunc(int x, int y) {
	camera.Move2D(x, y);
}
//Draw a wire cube! (nothing fancy here)
void DisplayFunc() {
	glEnable(GL_CULL_FACE); // REMOVE BACK POLYGONS 
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // SET CLEAR COLOR
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, window.size.x, window.size.y); // SET VIEWPORT

	glm::mat4 model, view, projection;
	camera.Update();
	camera.GetMatricies(projection, view, model);

	glm::mat4 mvp = projection* view * model;	//Compute the mvp matrix
	glLoadMatrixf(glm::value_ptr(mvp));
	glutWireCube(1);
	glutSwapBuffers();
}
//Redraw based on fps set for window
void TimerFunc(int value) {
	if (window.window_handle != -1) {
		glutTimerFunc(window.interval, TimerFunc, value);
		glutPostRedisplay();
	}
}

int main(int argc, char * argv[]) {
		//glut boilerplate
		glutInit(&argc, argv);
		glutInitWindowSize(1024, 512);
		glutInitWindowPosition(0, 0);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
		//Setup window and callbacks
		window.window_handle = glutCreateWindow("MODERN_GL_CAMERA");
		glutReshapeFunc(ReshapeFunc);
		glutDisplayFunc(DisplayFunc);
		glutKeyboardFunc(KeyboardFunc);
		glutSpecialFunc(SpecialFunc);
		glutMouseFunc(CallBackMouseFunc);
		glutMotionFunc(CallBackMotionFunc);
		glutPassiveMotionFunc(CallBackPassiveFunc);
		glutTimerFunc(window.interval, TimerFunc, 0);

		glewExperimental = GL_TRUE;

		if (glewInit() != GLEW_OK) {
			cerr << "GLEW failed to initialize." << endl;
			return 0;
		}
		//Setup camera
		camera.SetMode(FREE);
		camera.SetPosition(glm::vec3(0, 0, -1));
		camera.SetLookAt(glm::vec3(0, 0, 0));
		camera.SetClipping(.1, 1000);
		camera.SetFOV(45);
		//Start the glut loop!
		glutMainLoop();
return 0;
}
