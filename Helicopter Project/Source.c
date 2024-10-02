#include "GLUE3D.h"


#define TARGET_FPS 60 // Target frame rate (number of Frames Per Second).

const unsigned int FRAME_TIME = 1000 / TARGET_FPS; // Ideal time each frame should be displayed for (in milliseconds).
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;
unsigned int frameStartTime = 0; // Time we started preparing the current frame (in milliseconds since GLUT was initialized).

#define MOTION_NONE 0				// No motion.
#define MOTION_CLOCKWISE -1			// Clockwise rotation.
#define MOTION_ANTICLOCKWISE 1		// Anticlockwise rotation.
#define MOTION_BACKWARD -1			// Backward motion.
#define MOTION_FORWARD 1			// Forward motion.
#define MOTION_LEFT -1				// Leftward motion.
#define MOTION_RIGHT 1				// Rightward motion.
#define MOTION_DOWN -1				// Downward motion.
#define MOTION_UP 1					// Upward motion.

#define HELICOPTER_DECELERATE_RATE 0.9 //Smaller is more responsive (must be > 0 < 1)

typedef struct {
	int Yaw;		// Turn about the Z axis	[<0 = Clockwise, 0 = Stop, >0 = Anticlockwise]
	int Surge;		// Move forward or back		[<0 = Backward,	0 = Stop, >0 = Forward]
	int Sway;		// Move sideways (strafe)	[<0 = Left, 0 = Stop, >0 = Right]
	int Heave;		// Move vertically			[<0 = Down, 0 = Stop, >0 = Up]
} motionstate4_t;

// Represents the state of a single keyboard key.Represents the state of a single keyboard key.
typedef enum {
	KEYSTATE_UP = 0,	// Key is not pressed.
	KEYSTATE_DOWN		// Key is pressed down.
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

typedef struct HelicopterModel {
	GLUE_OBJ* Body;
	GLUE_OBJ* Legs;
	GLUE_OBJ* Propeller;
	GLUE_OBJ* Rotor;
	Vector3D Velocity;
	float PropellerSpeed;

} HelicopterModel;


// Current state of all keys used to control our "player-controlled" object's motion.
motionkeys_t motionKeyStates = {
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP,
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP };

// How our "player-controlled" object should currently be moving, solely based on keyboard input.
//
// Note: this may not represent the actual motion of our object, which could be subject to
// other controls (e.g. mouse input) or other simulated forces (e.g. gravity).
motionstate4_t keyboardMotion = { MOTION_NONE, MOTION_NONE, MOTION_NONE, MOTION_NONE };

// Define all character keys used for input (add any new key definitions here).
#define KEY_MOVE_FORWARD	'w'
#define KEY_MOVE_BACKWARD	's'
#define KEY_MOVE_LEFT		'a'
#define KEY_MOVE_RIGHT		'd'
#define KEY_RENDER_FILL		'l'
#define KEY_EXIT			27 // Escape key.

// Define all GLUT special keys used for input (add any new key definitions here).
#define SP_KEY_MOVE_UP		GLUT_KEY_UP
#define SP_KEY_MOVE_DOWN	GLUT_KEY_DOWN
#define SP_KEY_TURN_LEFT	GLUT_KEY_LEFT
#define SP_KEY_TURN_RIGHT	GLUT_KEY_RIGHT

// Render objects as filled polygons (1) or wireframes (0). Default filled.
int renderFillEnabled = 1;

//Paths for the helicopter components
char HelicopterBodyOBJPath[] = "Helicopter_Body.obj"; //Path for the helicopter model
char HelicopterLegsOBJPath[] = "Helicopter_Legs.obj"; //Path for the helicopter model
char HelicopterPropellerOBJPath[] = "Helicopter_Propeller.obj"; //Path for the helicopter model
char HelicopterRotorOBJPath[] = "Helicopter_Rotor.obj"; //Path for the helicopter model

//Helicopter obj objects
GLUE_OBJ* HelicopterBodyOBJ;
GLUE_OBJ* HelicopterLegsOBJ;
GLUE_OBJ* HelicopterPropellerOBJ;
GLUE_OBJ* HelicopterRotorOBJ;

HelicopterModel helicopter; //Model for the helicopter

Vector3D helicopterVeclocity = { 0, 0, 0 };

Vector3D rot = { 0, 0, 0 };
Vector3D scale = { 1, 1, 1 };
Vector3D location = { 0, 0, 0 };

GLint windowWidth = 800;
GLint windowHeight = 400;

Vector3D offset = { 0, 0, 0 };

Vector3D min;
Vector3D max;

// the degrees of shinnines (size of the specular highlight, bigger number means smaller highlight)
GLfloat noShininess = 0.0;
GLfloat highShininess = 100.0;

const GLfloat AmbientColour[] = { 0.5, 0.0, 0.0, 1.0 };
const GLfloat DiffuseColour[] = { 0.1f, 0.5f, 0.8f, 1.0f };
const GLfloat SpecularColour[] = { 1.0, 1.0, 1.0, 1.0 };

GLUE_Material blueMaterial = { &AmbientColour, &DiffuseColour, &SpecularColour, &highShininess };

int smoothOn = 1; //Smoothing of the entire scene

GLfloat lightPosition[] = { 10.0, 10.0, 10.0, 1.0 }; //  position the light source 
GLfloat zeroMaterial[] = { 0.0, 0.0, 0.0, 1.0 }; // a material that is all zeros
GLfloat redAmbient[] = { 0.5, 0.0, 0.0, 1.0 }; // a red ambient material
GLfloat blueDiffuse[] = { 0.1f, 0.5f, 0.8f, 1.0f }; // a blue diffuse material
GLfloat redDiffuse[] = { 1.0, 0.0, 0.0, 1.0 }; // a red diffuse material
GLfloat whiteSpecular[] = { 1.0, 1.0, 1.0, 1.0 }; // a white specular material

//Function prototypes
void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void specialKeyPressed(int key, int x, int y);
void keyReleased(unsigned char key, int x, int y);
void specialKeyReleased(int key, int x, int y);
void idle(void);
void close(void);
void main(int argc, char** argv);
void init(void);
void think(void);
void initLights(void);
void moveVelocity(GLUE_OBJ* obj, Vector3D vel);
void loadChopper(HelicopterModel* heli);
void drawDemoScene(int resolution);
void updateHelicopter(HelicopterModel* heli);

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

void init(void)
{
	initLights();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE); // make sure the normals are unit vectors

	loadChopper(&helicopter); //Load the helicopter and its components
}

void think(void){
	//DECELERATE helicopter
	if(helicopter.Velocity.x != 0) helicopter.Velocity.x *= HELICOPTER_DECELERATE_RATE;
	if (helicopter.Velocity.y != 0) helicopter.Velocity.y *= HELICOPTER_DECELERATE_RATE;
	if (helicopter.Velocity.z != 0) helicopter.Velocity.z *= HELICOPTER_DECELERATE_RATE;

	//Left arrow / Right arrow
	if (keyboardMotion.Yaw != MOTION_NONE) {
		HelicopterBodyOBJ->rotation->y += keyboardMotion.Yaw; //Rotate around axis
	}

	//W/S
	if (keyboardMotion.Surge != MOTION_NONE) {
		helicopter.Velocity.x = keyboardMotion.Surge * -0.2;//Move left/Right
		HelicopterBodyOBJ->rotation->x = keyboardMotion.Surge * 5;
	} 

	//A/D
	if (keyboardMotion.Sway != MOTION_NONE) {
		helicopter.Velocity.z -= keyboardMotion.Sway * 0.01; //Move forward backward
		HelicopterBodyOBJ->rotation->z = -keyboardMotion.Sway * 5;
	}
	//Up arrow / down arrow
	if (keyboardMotion.Heave != MOTION_NONE) {
		helicopter.Velocity.y = keyboardMotion.Heave * 0.2; //Move up and down
	}

	updateHelicopter(&helicopter);

	//moveVelocity(helicopter.Body, helicopterVeclocity); //Adjust the helicopter position due to its velocity
}

void initLights(void)
{
	// define the light color and intensity
	GLfloat ambientLight[] = { 0.0, 0.0, 0.0, 1.0 };  // relying on global ambient
	GLfloat diffuseLight[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };

	//  the global ambient light level
	GLfloat globalAmbientLight[] = { 0.4f, 0.4f, 0.4f, 1.0f };

	// set the global ambient light level
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);

	// define the color and intensity for light 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, diffuseLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, specularLight);

	// enable lighting 
	glEnable(GL_LIGHTING);
	// enable light 0
	glEnable(GL_LIGHT0);
}

void display(void)
{
	int resolution = 50; // sphere resolution

	if (smoothOn) glShadeModel(GL_SMOOTH);
	else glShadeModel(GL_FLAT);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen and depth buffer
	
	glLoadIdentity(); // load the identity matrix into the model view matrix

	// position light 0
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	GLUE_SetCameraToObject(helicopter.Body, 3, 270, 30); //Link the camera and helicopter

	drawDemoScene(resolution); //Draw the demo spheres 

	GLUE_renderMeshObject(helicopter.Body);
	GLUE_renderMeshObject(helicopter.Legs);
	GLUE_renderMeshObject(helicopter.Propeller);
	GLUE_renderMeshObject(helicopter.Rotor);

	// swap the drawing buffers
	glutSwapBuffers();
}

/// <summary>
/// Moves an object by its velocity
/// </summary>
/// <param name="obj"></param>
/// <param name="vel">Velocity vector</param>
void moveVelocity(GLUE_OBJ* obj, Vector3D vel) {
	obj->location->x += vel.x;
	obj->location->y += vel.y;
	obj->location->z += vel.z;
}

void updateHelicopter(HelicopterModel* heli) {
	//Move by its velocity
	/*
	heli->Body->location->x += heli->Velocity.x * sin((PI/180) * heli->Body->rotation->y);
	heli->Body->location->y += heli->Velocity.y; //Vertical
	heli->Body->location->z += heli->Velocity.z * cos((PI / 180) * heli->Body->rotation->y);*/
	
	// Convert yaw (rotation around Y axis) to radians
	float yawRadians = (PI / 180) * heli->Body->rotation->y;

	// Move forward based on the helicopter's orientation and velocity
	heli->Body->location->x += heli->Velocity.z * sin(yawRadians);  // Side movement based on forward velocity
	heli->Body->location->z += heli->Velocity.z * cos(yawRadians);  // Forward movement

	// Move sideways based on the helicopter's sideways (X) velocity and orientation
	heli->Body->location->x += heli->Velocity.x * cos(yawRadians);  // Right/Left movement
	heli->Body->location->z -= heli->Velocity.x * sin(yawRadians);  // Right/Left movement

	// Apply vertical movement directly (Y axis remains unaffected by yaw)
	heli->Body->location->y += heli->Velocity.y;  // Vertical movement

	//Spin the rotors


}

void loadChopper(HelicopterModel* heli) {
	HelicopterBodyOBJ = GLUE_loadMeshObject(HelicopterBodyOBJPath); //Load object
	HelicopterLegsOBJ = GLUE_loadMeshObject(HelicopterLegsOBJPath); //Load object
	HelicopterPropellerOBJ = GLUE_loadMeshObject(HelicopterPropellerOBJPath); //Load object
	HelicopterRotorOBJ = GLUE_loadMeshObject(HelicopterRotorOBJPath); //Load object

	//Set helicopter struct to the individual components
	heli->Body = HelicopterBodyOBJ;
	heli->Legs = HelicopterLegsOBJ;
	heli->Propeller = HelicopterPropellerOBJ;
	heli->Rotor = HelicopterRotorOBJ;

	HelicopterBodyOBJ->rotation = &rot;
	HelicopterBodyOBJ->scale = &scale;
	HelicopterBodyOBJ->location = &offset;
	HelicopterBodyOBJ->material = &blueMaterial;

	HelicopterLegsOBJ->rotation = heli->Body->rotation;
	HelicopterLegsOBJ->scale = heli->Body->scale;
	HelicopterLegsOBJ->location = heli->Body->location;
	HelicopterLegsOBJ->material = heli->Body->material;

	HelicopterPropellerOBJ->rotation = heli->Body->rotation;
	HelicopterPropellerOBJ->scale = heli->Body->scale;
	HelicopterPropellerOBJ->location = heli->Body->location;
	HelicopterPropellerOBJ->material = heli->Body->material;

	HelicopterRotorOBJ->rotation = heli->Body->rotation;
	HelicopterRotorOBJ->scale = heli->Body->scale;
	HelicopterRotorOBJ->location = heli->Body->location;
	HelicopterRotorOBJ->material = heli->Body->material;
}

//Main setup of the program
void main(int argc, char** argv) {
	// Initialise the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
	// set window position on screen
	glutInitWindowPosition(100, 100);

	glutCreateWindow("Helicopter");

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

//Runs when window is resized
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

//Runs when window is closed
void close(void) {
	GLUE_freeMeshObject(HelicopterBodyOBJ);
	GLUE_freeMeshObject(HelicopterLegsOBJ);
	GLUE_freeMeshObject(HelicopterPropellerOBJ);
	GLUE_freeMeshObject(HelicopterRotorOBJ);
}

//***** Keyboard controls *****//
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
	}
}


void drawDemoScene(int resolution) {
	//Draw cube around area
	glPushMatrix();
	glutWireCube(25);
	glPopMatrix();

	// draw the left sphere, blue with no hightlight 
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, blueDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glPushMatrix();
	glTranslatef(-3.75, 0, 0);
	// glutSolidSphere(radius, slices - lines of longitude, stacks - lines of latitude);
	glutSolidSphere(1.0, resolution, resolution);  // glutSolidSphere is a convenience function that sets up a gluSphere,  
	//     and...automatically computes normals for us
	glPopMatrix();

	// draw the right sphere, blue with red ambient
	glMaterialfv(GL_FRONT, GL_AMBIENT, redAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, blueDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glPushMatrix();
	glTranslatef(3.75, 0, 0);
	glutSolidSphere(1.0, resolution, resolution);
	glPopMatrix();

	// draw a red floor
	glMaterialfv(GL_FRONT, GL_AMBIENT, redDiffuse);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, redDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glNormal3d(0, 1, 0);  // normal of the floor is pointing up

	glPushMatrix();
	glTranslatef(-0.5, -1, 0);
	glScalef(3, 0, 3);
	glBegin(GL_POLYGON);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glVertex3f(1, 0, 1);
	glVertex3f(1, 0, 0);
	glEnd();
	glPopMatrix();
}