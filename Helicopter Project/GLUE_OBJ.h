#pragma once

#include "GLUE_OBJ.h"
#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define OBJ_DELIM ' '
#define VERTEX_FLOAT_ACCURACY 9 //Number of digits in a vertex float

typedef struct Vector3D {
	GLfloat x;
	GLfloat y;
	GLfloat z;
} Vector3D;

typedef struct Vector2D {
	GLfloat x;
	GLfloat y;
} Vector2D;

typedef struct GLUE_Material {
	GLfloat* AmbientColour;
	GLfloat* DiffuseColour;
	GLfloat* SpecularColour;
	GLfloat Shininess;
} GLUE_Material;

typedef struct GLUE_OBJ_FacePoint{
	int vertexIndex;	// Index of this vertex in the object's vertices array
	int texCoordIndex; // Index of the texture coordinate for this vertex in the object's texCoords array
	int normalIndex;	// Index of the normal for this vertex in the object's normals array
} GLUE_OBJ_FacePoint;

typedef struct GLUE_OBJ_Face{
	int pointCount;
	GLUE_OBJ_FacePoint* points;
} GLUE_OBJ_Face;

typedef struct GLUE_OBJ {
	GLUE_Material* material;
	Vector3D* vertices;
	Vector3D* normals;
	GLUE_OBJ_Face* faces;

	Vector2D* texCoords;

	Vector3D* scale;
	Vector3D* rotation;
	Vector3D* offset;
	int normalCount;
	int faceCount;
	int texCoordCount;
	int vertexCount;
} GLUE_OBJ;

/// <summary>
/// Loads a mesh object from a file
/// </summary>
/// <param name="fileName"></param>
/// <returns></returns>
GLUE_OBJ* GLUE_loadMeshObject(char* fileName);

/// <summary>
/// Displays the mesh object
/// </summary>
/// <param name="object"></param>
void GLUE_renderMeshObject(GLUE_OBJ* object);

/// <summary>
/// Creates a mesh object face
/// </summary>
/// <param name="face"></param>
/// <param name="faceData"></param>
/// <param name="faceDataLength"></param>
void GLUE_initMeshObjectFace(GLUE_OBJ_Face* face, char* faceData, int faceDataLength);

/// <summary>
/// Deletes the mesh object to save memory
/// </summary>
/// <param name="object"></param>
void GLUE_freeMeshObject(GLUE_OBJ* object);

