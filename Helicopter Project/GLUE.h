#pragma once
/*/
	GLUE - (open)GL - Utilities - Extra
		This aims to abstract functions in freeGLUT to allow for easier programming.

		LAST UPDATED 23/07/24
	Author - Robin Nowlan 2024
*/
#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define PI 3.14159265359
#define SHAPE_RESOLUTION 50 //How many points make up a circle
#define COLOR_FALLOFF 0.8 //How much the edges darken compared to the center

#define MAX_PARTICLES 1000 //Number of particles allowed

//Key defines
#define KEY_EXIT 27 // Escape key.
#define KEY_Y 'y' //Y key
#define KEY_S 's' //S Key
#define KEY_Q 'q' //Q Key
#define KEY_J 'j' //J Key
#define KEY_K 'k' //K Key
#define KEY_L 'l' //L Key
#define KEY_I 'i' //I Key
#define KEY_H 'h' //H Key

//Tree constants
#define TREE_BRANCH_SCALAR 0.618033988 // How much the branch length changes in after every itteration
#define TREE_THICKNESS_SCALAR 0.61803399 //How the thickness of each branch changes on every itteration
#define TREE_ANGLE 25 //The angle between branches

#define OBJ_DELIM ' '
#define VERTEX_FLOAT_ACCURACY 9 //Number of digits in a vertex float

/// <summary>
/// Pass colours more easily
/// </summary>
typedef struct Colour {
	float r;
	float g;
	float b;
	float a;
} Colour;

typedef struct GLUE_Colour {
	GLfloat R;
	GLfloat G;
	GLfloat B;
	GLfloat A;

	const GLfloat* array[4];
} GLUE_Colour;

typedef struct GLUE_Position {
	GLfloat X;
	GLfloat Y;
	GLfloat Z;
} GLUE_Position;

/// <summary>
/// Stores x and y positions
/// </summary>
typedef struct Location {
	float x;
	float y;
	float z;
}Location;

/// <summary>
/// Velocity
/// </summary>
struct Velocity {
	float vx;
	float vy;
	float vz;
};

enum ParticleState
{
	ALIVE, DEAD
};

enum OBJ_Line {
	VERTEX, FACE
};
/*
typedef struct Vector3D {
	GLdouble x;
	GLdouble y;
	GLdouble z;
} Vector3D;*/

typedef struct Vector3D {
	float x;
	float y;
	float z;
} Vector3D;

struct Point {
	struct Location location;
	struct Colour colour;
};

struct Particle_2D {
	struct Location location;
	struct Colour colour;
	struct Velocity velocity;
	float size;
	enum ParticleState state;
	int id;
};

typedef struct DiffuseMaterial {
	struct Colour colour;
	float roughness;
} DiffuseMaterial;

typedef struct {
	struct Colour colour;
	float roughness;
} PhongMaterial;


typedef struct GLUE_Camera {
	GLUE_Position Position;
	GLUE_Position LookAt;
	Vector3D Rotation;
} GLUE_Camera;

typedef struct GLUE_Material {
	GLfloat* AmbientColour[4];
	GLfloat* DiffuseColour[4];
	GLfloat* SpecularColour[4];
	GLfloat Shininess;
} GLUE_Material;



typedef struct GLUE_Light{
	GLUE_Position Position;
	GLUE_Colour globalColour;
	GLUE_Colour ambientColour;
	GLUE_Colour diffuseColour;
	GLUE_Colour specularColour;
} Light;




typedef struct Vector2D{
	GLdouble x;
	GLdouble y;
} Vector2D;

typedef struct {
	int vertexIndex;	// Index of this vertex in the object's vertices array
	int texCoordIndex; // Index of the texture coordinate for this vertex in the object's texCoords array
	int normalIndex;	// Index of the normal for this vertex in the object's normals array
} meshObjectFacePoint;

typedef struct {
	int pointCount;
	meshObjectFacePoint* points;
} meshObjectFace;

typedef struct MeshOBJ {
	GLUE_Material* material;
	Vector3D* vertices;
	Vector3D* normals;
	meshObjectFace* faces;

	Vector2D* texCoords;

	Vector3D* scale;
	Vector3D* rotation;
    Vector3D* offset;
	int normalCount;
	int faceCount;
	int texCoordCount;
	int vertexCount;
} MeshOBJ;

struct Camera {
	Vector3D eye;
	Vector3D center;
	Vector3D up;
};


struct BoundingBox {
	Vector3D min;
	Vector3D max;
};

/// <summary>
/// Tints the "shadows" to a set colour (lower value mean more of that colour)
/// </summary>
struct Colour FallOffMultiplyer;

/// <summary>
/// Collection of particles
/// </summary>
struct Particle_2D GLUE_Particles[MAX_PARTICLES];

/// <summary>
/// Draws a circle with a gradient to simulate 3D
/// </summary>
/// <param name="l">Location of the center of the circle</param>
/// <param name="r">Radius of the circle</param>
/// <param name="c">Colour of the circle</param>
void GLUE_CIRCLE(struct Location l, int r, struct Colour c);

/// <summary>
/// Draws a particle to the screen
/// </summary>
/// <param name=""></param>
void GLUE_DrawParticle(struct Particle_2D* p);

/// <summary>
/// Updates the particles position based on its velocity
/// </summary>
/// <param name="p"></param>
void GLUE_MoveParticle(struct Particle_2D* p);

/// <summary>
/// Returns a point between two locations
/// </summary>
/// <param name="l1">First location</param>
/// <param name="l2">Second Location</param>
/// <returns>Returns a mid-point location</returns>
struct Location GLUE_LineCenter(struct Location l1, struct Location l2);

/// <summary>
/// !!! WIP DO NOT USE !!! 
/// Returns the center of a polygon array
/// </summary>
/// <param name="locations">Array of all polygon points</param>
/// <returns>Returns a mid-point location</returns>
struct Location GLUE_PolygonCenter(struct Location locations[]);

/// <summary>
/// 
/// </summary>
/// <param name="first"></param>
/// <param name="second"></param>
/// <returns></returns>
float GLUE_LocationDistances(struct Location first, struct Location second);

/// <summary>
/// Displays text on the screen
/// </summary>
/// <param name="str">Text to be written</param>
/// <param name="l">Location of the bottom left corner of the text</param>
/// <param name="c">Color of the text</param>
void GLUE_Text(char str[], struct Location l, struct Colour c);

/// <summary>
/// Takes in a number and returns a string with its float representation
/// </summary>
/// <param name="num"></param>
/// <returns>String</returns>
char* GLUE_toStringf(double num);

/// <summary>
/// Takes in a number and returns a string with its int representation
/// </summary>
/// <param name="num"></param>
/// <returns>String</returns>
char* GLUE_toStringi(double num);

/// <summary>
/// Takes in who strings and returns a combined string
/// </summary>
/// <param name="str1">: First String</param>
/// <param name="str2">: Second String</param>
/// <returns>String</returns>
char* GLUE_combineString(char str1[], char str2[]);

/// <summary>
/// Displays a string value of a float
/// </summary>
/// <param name="title"></param>
/// <param name="num"></param>
/// <returns></returns>
char* GLUE_stringValuef(char title[], double num);

/// <summary>
/// Displays a string value of an int
/// </summary>
/// <param name="title"></param>
/// <param name="num"></param>
/// <returns></returns>
char* GLUE_stringValuei(char title[], double num);

/// <summary>
/// Returns a random number between 0 and randMax
/// </summary>
/// <param name="c"></param>
int GLUE_RAND(int randMax);

/// <summary>
/// Adds a particle to the particle array
/// </summary>
void GLUE_AddParticle(struct Particle_2D p);

/// <summary>
/// Returns a new location which has been moved by an offset factor (x,y)
/// </summary>
struct Location GLUE_OffsetLocation(struct Location* loc, int x, int y);

/// <summary>
/// Draws a reccursive tree at a given location
/// </summary>
void GLUE_Tree(struct Location baseLocation, struct Colour baseColour, struct Colour tipColour, int depth);

/// <summary>
/// Draws a line between two points
/// </summary>
void GLUE_Line(struct Location start, struct Location end, struct Colour startColour, struct Colour endColour, int thickness);

/// <summary>
/// Simple way to set the colour from a struct Colour
/// </summary>
void GLUE_COLOUR(struct Colour c);

/// <summary>
/// Recursive call for drawing the branches on a tree, should only be used privatly 
/// </summary>
static void GLUE_TreeBranch(struct Location startPoint, struct Colour baseColour, struct Colour tipColour, float length, int depth, int itter, float angle);

/// <summary>
///  Returns the number of ALIVE particles on screen
/// </summary>
/// <param name="screenHeight"></param>
/// <returns></returns>
int getParticleCount(int screenHeight);

/// <summary>
/// Loads a mesh object from a file
/// </summary>
/// <param name="fileName"></param>
/// <returns></returns>
MeshOBJ* GLUE_loadMeshObject(char* fileName);

/// <summary>
/// Displays the mesh object
/// </summary>
/// <param name="object"></param>
void GLUE_renderMeshObject(MeshOBJ* object);

/// <summary>
/// Creates a mesh object face
/// </summary>
/// <param name="face"></param>
/// <param name="faceData"></param>
/// <param name="faceDataLength"></param>
void initMeshObjectFace(meshObjectFace* face, char* faceData, int faceDataLength);

/// <summary>
/// Deletes the mesh object to save memory
/// </summary>
/// <param name="object"></param>
void freeMeshObject(MeshOBJ* object);


void loadPPM();

//void computeBoundingBox(MeshOBJ* object, Vector3D* min, Vector3D* max);
//void drawBox(Vector3D* min, Vector3D* max);