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

	//Leg Setup
	GLUE_OBJ* Legs;
	Vector3D LegScale;
	Vector3D LegOffset;

	//Propeller Setup
	GLUE_OBJ* Propeller;
	Vector3D PropellerScale;
	Vector3D PropellerOffset;

	//Rotor Setup
	GLUE_OBJ* Rotor;
	Vector3D RotorScale;
	Vector3D RotorOffset;

	Vector3D Velocity;
} HelicopterModel;

typedef struct PartPosistion {
	Vector3D location;
	Vector3D offset;
	Vector3D scale;
	Vector3D rotation;
} PartPosistion;

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

char TerrainPath[] = "Terrain.obj"; //Path for the helicopter model

// the degrees of shinnines (size of the specular highlight, bigger number means smaller highlight)
GLfloat noShininess = 0.0;
GLfloat highShininess = 100.0;

GLfloat lightPosition[] = { 10.0, 10.0, 10.0, 1.0 }; //  position the light source 

GLfloat fogColor[4] = { 0.582,0.589,0.539,0.5 };
GLfloat zeroMaterial[] = { 0.0, 0.0, 0.0, 1.0 }; // a material that is all zeros
GLfloat redAmbient[] = { 0.5, 0.0, 0.0, 1.0 }; // a red ambient material
GLfloat blueDiffuse[] = { 0.1f, 0.5f, 0.8f, 1.0f }; // a blue diffuse material
GLfloat redDiffuse[] = { 1.0, 0.0, 0.0, 1.0 }; // a red diffuse material
GLfloat whiteSpecular[] = { 1.0, 1.0, 1.0, 1.0 }; // a white specular material

const GLfloat skyColour[] = { 0.527, 0.804, 0.917, 1.0 };

const GLfloat DiffuseColour[] = { 0.1f, 0.5f, 0.8f, 1.0f };
const GLfloat SpecularColour[] = { 1.0, 1.0, 1.0, 1.0 };
GLUE_Material blueMaterial = { &blueDiffuse, &DiffuseColour, &SpecularColour, &highShininess };

const GLfloat TDiffuseColour[] = { 0.5, 0.5, 1, 1.0f };
const GLfloat TSpecularColour[] = { 1.0, 1.0, 1.0, 1.0 };
GLUE_Material terrainMaterial = { &skyColour, &TDiffuseColour, &TSpecularColour, &highShininess };

GLUE_OBJ* terrain;

//Helicopter component obj
GLUE_OBJ* HelicopterBodyOBJ;
GLUE_OBJ* HelicopterLegsOBJ;
GLUE_OBJ* HelicopterPropellerOBJ;
GLUE_OBJ* HelicopterRotorOBJ;

HelicopterModel helicopter; //Model for the helicopter

//Helicopter global parameters
Vector3D helicopterRotation = { 0, 0, 0 };
Vector3D helicopterScale = { 1, 1, 1 };
Vector3D helicopterLocation = { 0, 0, 0 };
Vector3D helicopterVeclocity = { 0, 0, 0 };

//Legs
Vector3D helicopterLegsRotation = { 0, 0, 0 };
Vector3D helicopterLegsScale = { 0.5, 0.5, 0.5 };
Vector3D helicopterLegsLocation = { 0, 0, 0 };
Vector3D helicopterLegsOffset = { 0, -0.25, 0 };
GLUE_Material helicopterLegsMaterial = { &redDiffuse, &blueDiffuse, &SpecularColour, &highShininess };

//Rotor
Vector3D helicopterRotorRotation = { 0, 0, 0 };
Vector3D helicopterRotorScale = { 1, 1, 1 };
Vector3D helicopterRotorLocation = { 0, 0, 0 };
Vector3D helicopterRotorOffset = { 0, 0, 0 };
GLUE_Material helicopterRotorMaterial;

//Prop
Vector3D helicopterPropRotation = { 0, 0, 0 };
Vector3D helicopterPropScale = { 1, 1, 1 };
Vector3D helicopterPropLocation = { 0.1, 0.2, 0 };
Vector3D helicopterPropOffset = { 0.1, 0.2, 0 };
GLUE_Material helicopterPropMaterial;

//Terrain
Vector3D TerrainRotation = { 0, 0, 0 };
Vector3D TerrainScale = { 25, 25, 25 };
Vector3D TerrainLocation = { 0, 0, 0 };
Vector3D TerrainrVeclocity = { 0, 0, 0 };

//Scene configuration variables
int smoothOn = 1; //Smoothing of the entire scene

GLint windowWidth = 800;
GLint windowHeight = 400;

GLuint terrainTexture;

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
void loadChopper(HelicopterModel* heli);
void updateHelicopter(HelicopterModel* heli);
void loadTerrain();

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
	glEnable(GL_FOG);


	// set the color of the fog
	glFogfv(GL_FOG_COLOR, fogColor);

	glFogf(GL_FOG_MODE, GL_EXP);
	glFogf(GL_FOG_DENSITY, 0.1);

	loadChopper(&helicopter); //Load the helicopter and its components
	loadTerrain();
}

void loadTerrain() {
	terrain = GLUE_loadMeshObject(TerrainPath);
	terrain->material = &terrainMaterial;
	terrain->location = &TerrainLocation;
	terrain->scale = &TerrainScale;
	terrain->rotation = &TerrainRotation;
	terrain->textureID = GLUE_loadTexture("TerrainTex.ppm");
}

void initLights(void)
{
	// define the light color and intensity
	GLfloat ambientLight[] = { 0.0, 0.0, 1.0, 1.0 };  // relying on global ambient
	GLfloat diffuseLight[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat sunLight[] = { 1, 1, 1, 1 };

	// set the global ambient light level
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, skyColour);

	// define the color and intensity for light 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

	GLfloat direction[] = { 0, 1, 0 };
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction);
	GLfloat theta = 15.0;
	glLightfv(GL_LIGHT1, GL_SPOT_CUTOFF, &theta);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight);

	// enable lighting 
	glEnable(GL_LIGHTING);
	// enable lights
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
}

void display(void)
{

	if (smoothOn) glShadeModel(GL_SMOOTH);
	else glShadeModel(GL_FLAT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen and depth buffer

	glLoadIdentity(); // load the identity matrix into the model view matrix

	// position light 0
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);


	GLfloat spotlightPosition[] = { helicopter.Body->location->x, helicopter.Body->location->y, helicopter.Body->location->z, 1 };
	glLightfv(GL_LIGHT1, GL_POSITION, spotlightPosition);

	//Set Camera Alignment
	GLUE_SetCameraToObject(helicopter.Body, 3, 270, 20); //Link the camera and helicopter

	//Render the helicopter
	GLUE_renderMeshObject(helicopter.Body);
	GLUE_renderMeshObject(helicopter.Legs);
	GLUE_renderMeshObject(helicopter.Propeller);
	GLUE_renderMeshObject(helicopter.Rotor);

	//Render the Terrain
	if (renderFillEnabled) GLUE_renderMeshObject(terrain);
	else GLUE_renderWireframeObject(terrain);

	//Draw Skybox
	glColor4b(skyColour[0], skyColour[1], skyColour[2], skyColour[3]);
	glutSolidCube((terrain->scale->x * 2) -1 );


	glutSolidCube(2);

	// swap the drawing buffers
	glutSwapBuffers();
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
	} 

	//A/D
	if (keyboardMotion.Sway != MOTION_NONE) {
		helicopter.Velocity.z -= keyboardMotion.Sway * 0.01; //Move forward backward
	}
	//Up arrow / down arrow
	if (keyboardMotion.Heave != MOTION_NONE) {
		helicopter.Velocity.y = keyboardMotion.Heave * 0.2; //Move up and down
	}

	updateHelicopter(&helicopter);
}

void updateHelicopter(HelicopterModel* heli) {
	// Move forward based on the helicopter's orientation and velocity
	heli->Body->location->x += heli->Velocity.z * sin((PI / 180) * heli->Body->rotation->y);  // Side movement based on forward velocity
	heli->Body->location->z += heli->Velocity.z * cos((PI / 180) * heli->Body->rotation->y);  // Forward movement

	heli->Body->location->x += heli->Velocity.x * cos((PI / 180) * heli->Body->rotation->y);  // Right/Left movement
	heli->Body->location->z -= heli->Velocity.x * sin((PI / 180) * heli->Body->rotation->y);  // Right/Left movement

	heli->Body->location->y += heli->Velocity.y;  // Vertical movement

	//Set legs to offset Position
	heli->Legs->location->x = (heli->Body->location->x + helicopterLegsOffset.x) * 1/helicopterLegsScale.x;
	heli->Legs->location->y = (heli->Body->location->y + helicopterLegsOffset.y) * 1/helicopterLegsScale.y;
	heli->Legs->location->z = (heli->Body->location->z + helicopterLegsOffset.z) * 1/helicopterLegsScale.y;

	heli->Propeller->location->x = (heli->Body->location->x + helicopterPropOffset.x) * 1 / helicopterPropScale.x;
	heli->Propeller->location->y = (heli->Body->location->y + helicopterPropOffset.y) * 1 / helicopterPropScale.y;
	heli->Propeller->location->z = (heli->Body->location->z + helicopterPropOffset.z) * 1 / helicopterPropScale.y;

	heli->Propeller->rotation->x += heli->Body->rotation->x ;
	heli->Propeller->rotation->z += heli->Body->rotation->z ;
	heli->Propeller->rotation->y += (10 * heli->Velocity.z) + 20; //Spin the Propeller

	heli->Rotor->location->x = (heli->Body->location->x + helicopterRotorOffset.x) * 1 / helicopterRotorScale.x;
	heli->Rotor->location->y = (heli->Body->location->y + helicopterRotorOffset.y) * 1 / helicopterRotorScale.y;
	heli->Rotor->location->z = (heli->Body->location->z + helicopterRotorOffset.z) * 1 / helicopterRotorScale.y;

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

	//Config Body
	heli->Body->rotation = &helicopterRotation;
	heli->Body->scale = &helicopterScale;
	heli->Body->location = &helicopterLocation;
	heli->Body->material = &blueMaterial;

	//Congfig Legs
	heli->Legs->rotation = heli->Body->rotation;
	heli->Legs->scale = &helicopterLegsScale;
	heli->Legs->location = &helicopterLegsLocation;
	heli->Legs->material = &helicopterLegsMaterial;

	//Config Propeller
	heli->Propeller->rotation = &helicopterPropRotation;
	heli->Propeller->scale = &helicopterPropScale;
	heli->Propeller->location = &helicopterPropLocation;
	heli->Propeller->material = heli->Body->material;

	//Config Rotor
	heli->Rotor->rotation = &helicopterRotorRotation;
	heli->Rotor->scale = &helicopterRotorScale;
	heli->Rotor->location = &helicopterRotorLocation;
	heli->Rotor->material = heli->Body->material;
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
	int diagonalScale = sqrt(pow(terrain->scale->x, 2) + pow(terrain->scale->y, 2) + pow(terrain->scale->z, 2));
	gluPerspective(60, (float)windowWidth / (float)windowHeight, 0.1, diagonalScale);

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


