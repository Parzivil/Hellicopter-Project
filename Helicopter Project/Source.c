/******************************************************************************
*
* Computer Graphics Programming 2020 Project Template v1.0 (11/04/2021)
*
* Based on: Animation Controller v1.0 (11/04/2021)
*
* This template provides a basic FPS-limited render loop for an animated scene,
* plus keyboard handling for smooth game-like control of an object such as a
* character or vehicle.
*
* A simple static lighting setup is provided via initLights(), which is not
* included in the animationalcontrol.c template. There are no other changes.
*
******************************************************************************/
#include "GLUE.h"
/******************************************************************************
* Animation & Timing Setup
******************************************************************************/
// Target frame rate (number of Frames Per Second).
#define TARGET_FPS 60
// Ideal time each frame should be displayed for (in milliseconds).
const unsigned int FRAME_TIME = 1000 / TARGET_FPS;
// Frame time in fractional seconds.
// Note: This is calculated to accurately reflect the truncated integer value of
// FRAME_TIME, which is used for timing, rather than the more accurate fractional
// value we'd get if we simply calculated "FRAME_TIME_SEC = 1.0f / TARGET_FPS".
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;
// Time we started preparing the current frame (in milliseconds since GLUT was initialized).
unsigned int frameStartTime = 0;
/******************************************************************************
* Some Simple Definitions of Motion
******************************************************************************/
#define MOTION_NONE 0 // No motion.
#define MOTION_CLOCKWISE -1 // Clockwise rotation.
#define MOTION_ANTICLOCKWISE 1 // Anticlockwise rotation.
#define MOTION_BACKWARD -1 // Backward motion.
#define MOTION_FORWARD 1 // Forward motion.
#define MOTION_LEFT -1 // Leftward motion.
#define MOTION_RIGHT 1 // Rightward motion.
#define MOTION_DOWN -1 // Downward motion.
#define MOTION_UP 1 // Upward motion.



// Represents the motion of an object on four axes (Yaw, Surge, Sway, and Heave).
//
// You can use any numeric values, as specified in the comments for each axis. However,
// the MOTION_ definitions offer an easy way to define a "unit" movement without using
// magic numbers (e.g. instead of setting Surge = 1, you can set Surge = MOTION_FORWARD).
//
typedef struct {
	int Yaw; // Turn about the Z axis [<0 = Clockwise, 0 = Stop, >0 = Anticlockwise]
	int Surge; // Move forward or back [<0 = Backward, 0 = Stop, >0 = Forward]
	int Sway; // Move sideways (strafe) [<0 = Left, 0 = Stop, >0 = Right]
	int Heave; // Move vertically [<0 = Down, 0 = Stop, >0 = Up]
} motionstate4_t;
/******************************************************************************
* Keyboard Input Handling Setup
******************************************************************************/
// Represents the state of a single keyboard key.Represents the state of a single keyboard key.
typedef enum {
	KEYSTATE_UP = 0, // Key is not pressed.
	KEYSTATE_DOWN // Key is pressed down.
} keystate_t;
// Represents the states of a set of keys used to control an object's motion.
typedef struct {
	keystate_t MoveForward;
	keystate_t MoveBackward;
	keystate_t MoveLeft;
	keystate_t MoveRight;
	keystate_t MoveUp;
	keystate_t MoveDown;
	keystate_t TurnLeft;
	keystate_t TurnRight;
} motionkeys_t;
// Current state of all keys used to control our "player-controlled" object's motion.
motionkeys_t motionKeyStates = {
KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP,
KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP };
// How our "player-controlled" object should currently be moving, solely based on keyboard input.
//
// Note: this may not represent the actual motion of our object, which could be subject to
// other controls (e.g. mouse input) or other simulated forces (e.g. gravity).
motionstate4_t keyboardMotion = { MOTION_NONE, MOTION_NONE, MOTION_NONE,
MOTION_NONE };
// Define all character keys used for input (add any new key definitions here).
// Note: USE ONLY LOWERCASE CHARACTERS HERE. The keyboard handler provided converts all
// characters typed by the user to lowercase, so the SHIFT key is ignored.
#define KEY_MOVE_FORWARD 'w'
#define KEY_MOVE_BACKWARD 's'
#define KEY_MOVE_LEFT 'a'
#define KEY_MOVE_RIGHT 'd'
#define KEY_RENDER_FILL 'l'
#define KEY_EXIT 27 // Escape key.
// Define all GLUT special keys used for input (add any new key definitions here).
#define SP_KEY_MOVE_UP GLUT_KEY_UP
#define SP_KEY_MOVE_DOWN GLUT_KEY_DOWN
#define SP_KEY_TURN_LEFT GLUT_KEY_LEFT
#define SP_KEY_TURN_RIGHT GLUT_KEY_RIGHT
/******************************************************************************
* GLUT Callback Prototypes
******************************************************************************/
void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void specialKeyPressed(int key, int x, int y);
void keyReleased(unsigned char key, int x, int y);
void specialKeyReleased(int key, int x, int y);
void idle(void);
void close(void);
/******************************************************************************
* Animation-Specific Function Prototypes (add your own here)
******************************************************************************/
void main(int argc, char** argv);
void init(void);
void think(void);
void initLights(void);
/******************************************************************************
* Animation-Specific Setup (Add your own definitions, constants, and globals here)
******************************************************************************/
// Render objects as filled polygons (1) or wireframes (0). Default filled.
int renderFillEnabled = 1;
MeshOBJ* cubeMesh;

struct Camera camera = { { 4, 4, 2 } , { 0, 0, 0 } , { 0, 1, 0 } };

GLint windowWidth = 800;
GLint windowHeight = 400;

Vector3D offset = { 0, 0, -2 };

Vector3D min;
Vector3D max;


GLUE_Material blueMaterial = { { 0.0, 0.0, 0.0, 1.0},{0.1f, 0.5f, 0.8f, 1.0f},{ 0.0, 0.0, 0.0, 1.0},0 };


/******************************************************************************
* Entry Point (don't put anything except the main function here)
******************************************************************************/
void main(int argc, char** argv)
{
	// Initialise the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
	// set window position on screen
	glutInitWindowPosition(100, 100);

	glutCreateWindow("Animation");

	// Set up the scene.
	init();


	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// Register GLUT callbacks.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(specialKeyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutSpecialUpFunc(specialKeyReleased);
	glutIdleFunc(idle);
	glutCloseFunc(close);

	// Record when we started rendering the very first frame (which should happen after we call glutMainLoop).
	frameStartTime = (unsigned int)glutGet(GLUT_ELAPSED_TIME);

	// Enter the main drawing loop (this will never return).
	glutMainLoop();
}
/******************************************************************************
* GLUT Callbacks (don't add any other functions here)
******************************************************************************/

void display(void)
{
	// clear the screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// load the identity matrix into the model view matrix
	glLoadIdentity();

	glColor3f(1.0f, 1.0f, 1.0f);

	glEnable(GL_COLOR_MATERIAL);

	GLUE_renderMeshObject(cubeMesh); //untextured
	computeBoundingBox(&cubeMesh, &min, &max);

	glColor3f(0.0f, 1.0f, 1.0f);
	glutWireSphere(0.5, 10, 10);
	glBegin(GL_LINES);

	//x axis -red
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(2.0f, 0.0f, 0.0f);

	//y axis -green
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);

	//z axis - blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 2.0f);

	glEnd();


	drawBox(&min, &max);

	// swap the drawing buffers
	glutSwapBuffers();
}

void reshape(int width, int h)
{
	// update the new width
	windowWidth = width;
	// update the new height
	windowHeight = h;

	// update the viewport to still be all of the window
	glViewport(0, 0, windowWidth, windowHeight);

	// change into projection mode so that we can change the camera properties
	glMatrixMode(GL_PROJECTION);

	// load the identity matrix into the projection matrix
	glLoadIdentity();

	// gluPerspective(fovy, aspect, near, far)
	gluPerspective(45, (float)windowWidth / (float)windowHeight, 1, 20);

	// change into model-view mode so that we can change the object positions
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void close(void) {
	freeMeshObject(cubeMesh);
}

void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {
		case KEY_MOVE_FORWARD:
			motionKeyStates.MoveForward = KEYSTATE_DOWN;
			keyboardMotion.Surge = MOTION_FORWARD;

			break;
		case KEY_MOVE_BACKWARD:
			motionKeyStates.MoveBackward = KEYSTATE_DOWN;
			keyboardMotion.Surge = MOTION_BACKWARD;

			break;
		case KEY_MOVE_LEFT:
			motionKeyStates.MoveLeft = KEYSTATE_DOWN;
			keyboardMotion.Sway = MOTION_LEFT;

			break;
		case KEY_MOVE_RIGHT:
			motionKeyStates.MoveRight = KEYSTATE_DOWN;
			keyboardMotion.Sway = MOTION_RIGHT;
			break;

		case KEY_RENDER_FILL:
			renderFillEnabled = !renderFillEnabled;
			break;
		case KEY_EXIT:
			exit(0);
			break;
	}
}
/*
Called each time a "special" key (e.g. an arrow key) is pressed.
*/
void specialKeyPressed(int key, int x, int y)
{
	switch (key) {
		case SP_KEY_MOVE_UP:
			motionKeyStates.MoveUp = KEYSTATE_DOWN;
			keyboardMotion.Heave = MOTION_UP;
			break;
		case SP_KEY_MOVE_DOWN:
			motionKeyStates.MoveDown = KEYSTATE_DOWN;
			keyboardMotion.Heave = MOTION_DOWN;
			break;
		case SP_KEY_TURN_LEFT:
			motionKeyStates.TurnLeft = KEYSTATE_DOWN;
			keyboardMotion.Yaw = MOTION_ANTICLOCKWISE;
			break;
		case SP_KEY_TURN_RIGHT:
			motionKeyStates.TurnRight = KEYSTATE_DOWN;
			keyboardMotion.Yaw = MOTION_CLOCKWISE;
			break;
	}
}

void keyReleased(unsigned char key, int x, int y)
{
	switch (tolower(key)) {
		case KEY_MOVE_FORWARD:
			motionKeyStates.MoveForward = KEYSTATE_UP;
			keyboardMotion.Surge = (motionKeyStates.MoveBackward ==
				KEYSTATE_DOWN) ? MOTION_BACKWARD : MOTION_NONE;
			break;
		case KEY_MOVE_BACKWARD:
			motionKeyStates.MoveBackward = KEYSTATE_UP;
			keyboardMotion.Surge = (motionKeyStates.MoveForward == KEYSTATE_DOWN) ?
				MOTION_FORWARD : MOTION_NONE;
			break;
		case KEY_MOVE_LEFT:
			motionKeyStates.MoveLeft = KEYSTATE_UP;
			keyboardMotion.Sway = (motionKeyStates.MoveRight == KEYSTATE_DOWN) ?
				MOTION_RIGHT : MOTION_NONE;
			break;
		case KEY_MOVE_RIGHT:
			motionKeyStates.MoveRight = KEYSTATE_UP;
			keyboardMotion.Sway = (motionKeyStates.MoveLeft == KEYSTATE_DOWN) ?
				MOTION_LEFT : MOTION_NONE;
			break;

	}
}
/*
Called each time a "special" key (e.g. an arrow key) is released.
*/
void specialKeyReleased(int key, int x, int y)
{
	switch (key) {
		/*
		Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION
		This works as per the motion keys in keyReleased.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_UP;
		keyboardMotion.Heave = (motionKeyStates.MoveDown == KEYSTATE_DOWN) ?
			MOTION_DOWN : MOTION_NONE;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_UP;
		keyboardMotion.Heave = (motionKeyStates.MoveUp == KEYSTATE_DOWN) ?
			MOTION_UP : MOTION_NONE;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnRight == KEYSTATE_DOWN) ?
			MOTION_CLOCKWISE : MOTION_NONE;
		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnLeft == KEYSTATE_DOWN) ?
			MOTION_ANTICLOCKWISE : MOTION_NONE;
		break;
		/*
		Other Keyboard Functions (add any new special key controls here)
		As per keyReleased, you only need to handle the key here if you want
		something
		to happen when the user lets go. If you just want something to happen
		when the
		key is first pressed, add you code to specialKeyPressed instead.
		*/
	}
}

void idle(void)
{
	// Wait until it's time to render the next frame.
	unsigned int frameTimeElapsed = (unsigned int)glutGet(GLUT_ELAPSED_TIME) -
		frameStartTime;
	if (frameTimeElapsed < FRAME_TIME)
	{
		// This frame took less time to render than the ideal FRAME_TIME: we'll suspend this thread for the remaining time,
			// so we're not taking up the CPU until we need to render another frame.
		unsigned int timeLeft = FRAME_TIME - frameTimeElapsed;
		Sleep(timeLeft);
	}
	// Begin processing the next frame.
	frameStartTime = glutGet(GLUT_ELAPSED_TIME); // Record when we started work on the new frame.
	think(); // Update our simulated world before the next call to display().
	glutPostRedisplay(); // Tell OpenGL there's a new frame ready to be drawn.
}
/******************************************************************************
* Animation-Specific Functions (Add your own functions at the end of this section)
******************************************************************************/

void init(void)
{

	initLights();
	
	//load assets
	cubeMesh = GLUE_loadMeshObject("pumpkin.obj");
	cubeMesh->material = blueMaterial;
	Vector3D scale = { 0.05, 0.05, 0.05 };
	cubeMesh->scale = &scale;

	cubeMesh->offset = &offset;

	loadPPM();

}

void think(void)
{
	if (keyboardMotion.Yaw != MOTION_NONE) {
		/* TEMPLATE: Turn your object right (clockwise) if .Yaw < 0, or left
		(anticlockwise) if .Yaw > 0 */
	}
	if (keyboardMotion.Surge != MOTION_NONE) {
		/* TEMPLATE: Move your object backward if .Surge < 0, or forward
		if .Surge > 0 */
		offset.x += keyboardMotion.Surge;
		cubeMesh->offset = &offset;
	}
	if (keyboardMotion.Sway != MOTION_NONE) {
		/* TEMPLATE: Move (strafe) your object left if .Sway < 0, or right
		if .Sway > 0 */
		offset.z += keyboardMotion.Sway;
		cubeMesh->offset = &offset;
	}
	if (keyboardMotion.Heave != MOTION_NONE) {
		/* TEMPLATE: Move your object down if .Heav e < 0, or up if .Heave > 0
		*/
		offset.y += keyboardMotion.Heave;
		cubeMesh->offset = &offset;
	}
}

void initLights(void)
{
	// Simple lighting setup
	GLfloat globalAmbient[] = { 0.4f, 0.4f, 0.4f, 1 };
	GLfloat lightPosition[] = { 5.0f, 5.0f, 5.0f, 1.0f };
	GLfloat ambientLight[] = { 0, 0, 0, 1 };
	GLfloat diffuseLight[] = { 1, 1, 1, 1 };
	GLfloat specularLight[] = { 1, 1, 1, 1 };
	// Configure global ambient lighting.
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
	// Configure Light 0.
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	// Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	// Make GL normalize the normal vectors we supply.
	glEnable(GL_NORMALIZE);

	// Turn on depth testing so that polygons are drawn in the correct order
	glEnable(GL_DEPTH_TEST);
	// Enable use of simple GL colours as materials.
	glEnable(GL_COLOR_MATERIAL);
}