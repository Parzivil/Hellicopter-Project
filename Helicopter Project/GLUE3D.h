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
#include "GLUE_OBJ.h"


#define PI 3.14159265359
#define SHAPE_RESOLUTION 50

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

///***Needs to be combined***//
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
///***^^^***///

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
typedef struct Velocity {
	float vx;
	float vy;
	float vz;
}Velocity;


typedef struct Point {
	Location location;
	Colour colour;
}Point;

typedef struct GLUE_Camera {
	GLUE_Position Position;
	GLUE_Position LookAt;
	Vector3D Rotation;
} GLUE_Camera;

typedef struct GLUE_Light{
	GLUE_Position Position;
	GLUE_Colour globalColour;
	GLUE_Colour ambientColour;
	GLUE_Colour diffuseColour;
	GLUE_Colour specularColour;
} Light;

typedef struct Camera {
	Vector3D eye;
	Vector3D center;
	Vector3D up;
}Camera;

/// <summary>
/// Draws a circle with a gradient to simulate 3D
/// </summary>
/// <param name="l">Location of the center of the circle</param>
/// <param name="r">Radius of the circle</param>
/// <param name="c">Colour of the circle</param>
void GLUE_CIRCLE(Location l, int r, Colour c);


/// <summary>
/// Returns a point between two locations
/// </summary>
/// <param name="l1">First location</param>
/// <param name="l2">Second Location</param>
/// <returns>Returns a mid-point location</returns>
Location GLUE_LineCenter(Location l1, Location l2);

/// <summary>
/// !!! WIP DO NOT USE !!! 
/// Returns the center of a polygon array
/// </summary>
/// <param name="locations">Array of all polygon points</param>
/// <returns>Returns a mid-point location</returns>
Location GLUE_PolygonCenter(Location locations[]);

/// <summary>
/// 
/// </summary>
/// <param name="first"></param>
/// <param name="second"></param>
/// <returns></returns>
float GLUE_LocationDistances(Location first, Location second);

/// <summary>
/// Displays text on the screen
/// </summary>
/// <param name="str">Text to be written</param>
/// <param name="l">Location of the bottom left corner of the text</param>
/// <param name="c">Color of the text</param>
void GLUE_Text(char str[], Location l, Colour c);

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
/// Returns a new location which has been moved by an offset factor (x,y)
/// </summary>
struct Location GLUE_OffsetLocation(Location* loc, int x, int y);

/// <summary>
/// Draws a line between two points
/// </summary>
void GLUE_Line(Location start, Location end, Colour startColour, Colour endColour, int thickness);

/// <summary>
/// Simple way to set the colour from a struct Colour
/// </summary>
void GLUE_COLOUR(Colour c);