#include "GLUE_OBJ.h"
#include <float.h>


GLUE_OBJ* GLUE_loadMeshObject(char* fileName) {
	Vector3D vertSum = { 0, 0, 0 };
	FILE* inFile = NULL;
	GLUE_OBJ* object;
	char line[512];					// Line currently being parsed
	char keyword[10];				// Keyword currently being parsed
	int currentVertexIndex = 0;		// 0-based index of the vertex currently being parsed
	int currentTexCoordIndex = 0;	// 0-based index of the texure coordinate currently being parsed
	int currentNormalIndex = 0;		// 0-based index of the normal currently being parsed
	int currentFaceIndex = 0;		// 0-based index of the face currently being parsed

	errno_t err = fopen_s(&inFile, fileName, "r");

	if (err != 0 || inFile == NULL) {
		return NULL;
	}

	// Allocate and initialize a new Mesh Object.
	object = malloc(sizeof(GLUE_OBJ));
	object->vertexCount = 0;
	object->vertices = NULL;
	object->texCoordCount = 0;
	object->texCoords = NULL;
	object->normalCount = 0;
	object->normals = NULL;
	object->faceCount = 0;
	object->faces = NULL;

	// Pre-parse the file to determine how many vertices, texture coordinates, normals, and faces we have.
	while (fgets(line, (unsigned)_countof(line), inFile))
	{
		if (sscanf_s(line, "%9s", keyword, (unsigned)_countof(keyword)) == 1) {
			if (strcmp(keyword, "v") == 0) {
				object->vertexCount++;
			}
			else if (strcmp(keyword, "vt") == 0) {
				object->texCoordCount++;
			}
			else if (strcmp(keyword, "vn") == 0) {
				object->normalCount++;
			}
			else if (strcmp(keyword, "f") == 0) {
				object->faceCount++;
			}
		}
	}

	if (object->vertexCount > 0)object->vertices = malloc(sizeof(Vector3D) * object->vertexCount);
	if (object->texCoordCount > 0) object->texCoords = malloc(sizeof(Vector2D) * object->texCoordCount);
	if (object->normalCount > 0) {
		object->normals = malloc(sizeof(Vector3D) * object->normalCount);
	}
	//Warn that the model imported does not contain normals
	else {
		printf("WARNING MODEL DOES NOT CONTAIN NORMALS!!\nLIGHTING WILL BE AFFECTED\n\n ");
	}
	if (object->faceCount > 0) object->faces = malloc(sizeof(GLUE_OBJ_Face) * object->faceCount);

	// Parse the file again, reading the actual vertices, texture coordinates, normals, and faces.
	rewind(inFile);

	while (fgets(line, (unsigned)_countof(line), inFile))
	{
		if (sscanf_s(line, "%9s", keyword, (unsigned)_countof(keyword)) == 1) {
			if (strcmp(keyword, "v") == 0) {
				Vector3D vertex = { 0, 0, 0 };
				sscanf_s(line, "%*s %f %f %f", &vertex.x, &vertex.y, &vertex.z);
				memcpy_s(&object->vertices[currentVertexIndex], sizeof(Vector3D), &vertex, sizeof(Vector3D));
				vertSum.x += vertex.x;
				vertSum.y += vertex.y;
				vertSum.z += vertex.z;
				currentVertexIndex++;
			}
			else if (strcmp(keyword, "vt") == 0) {
				Vector2D texCoord = { 0, 0 };
				sscanf_s(line, "%*s %f %f", &texCoord.x, &texCoord.y);
				memcpy_s(&object->texCoords[currentTexCoordIndex], sizeof(Vector2D), &texCoord, sizeof(Vector2D));
				currentTexCoordIndex++;
			}

			//SOME FILES DON'T INCLUDE NORMALS!!
			else if (strcmp(keyword, "vn") == 0) {
				Vector3D normal = { 0, 0, 0 };
				sscanf_s(line, "%*s %f %f %f", &normal.x, &normal.y, &normal.z);
				memcpy_s(&object->normals[currentNormalIndex], sizeof(Vector3D), &normal, sizeof(Vector3D));
				currentNormalIndex++;
			}
			else if (strcmp(keyword, "f") == 0) {
				GLUE_initMeshObjectFace(&(object->faces[currentFaceIndex]), line, _countof(line));
				currentFaceIndex++;
			}
		}
	}

	fclose(inFile);

	Vector3D centerPoint = { vertSum.x / object->vertexCount,
						vertSum.y / object->vertexCount,
						vertSum.z / object->vertexCount };
	Vector3D zero = { -1 * centerPoint.x, -1 * centerPoint.y, -1 * centerPoint.z };
	//object->offset = &zero;

	GLUE_NormalizeOBJ(object);

	return object;
}

/*
	Initialise the specified Mesh Object Face from a string of face data in the Wavefront OBJ file format.
*/
void GLUE_initMeshObjectFace(GLUE_OBJ_Face* face, char* faceData, int maxFaceDataLength) {
	int maxPoints = 0;
	int inWhitespace = 0;
	const char* delimiter = " ";
	char* token = NULL;
	char* context = NULL;

	// Do a quick scan of the input string to determine the maximum number of points in this face by counting
	// blocks of whitespace (each point must be preceded by at least one space). Note that we may end up with
	// fewer points than this if one or more prove to be invalid.
	for (int i = 0; i < maxFaceDataLength; i++)
	{
		char c = faceData[i];
		if (c == '\0') {
			break;
		}
		else if ((c == ' ') || (c == '\t')) {
			if (!inWhitespace) {
				inWhitespace = 1;
				maxPoints++;
			}
		}
		else {
			inWhitespace = 0;
		}
	}

	// Parse the input string to extract actual face points (if we're expecting any).
	face->pointCount = 0;
	if (maxPoints > 0) {
		face->points = malloc(sizeof(GLUE_OBJ_FacePoint) * maxPoints);

		token = strtok_s(faceData, delimiter, &context);
		while ((token != NULL) && (face->pointCount < maxPoints)) {
			GLUE_OBJ_FacePoint parsedPoint = { 0, 0, 0 }; // At this point we're working with 1-based indices from the OBJ file.

			if (strcmp(token, "f") != 0) {

				// Attempt to parse this face point in the format "v/t[/n]" (vertex, texture, and optional normal).
				if (sscanf_s(token, "%d/%d/%d", &parsedPoint.vertexIndex, &parsedPoint.texCoordIndex, &parsedPoint.normalIndex) < 2) {
					// That didn't work out: try parsing in the format "v[//n]" instead (vertex, no texture, and optional normal).
					sscanf_s(token, "%d//%d", &parsedPoint.vertexIndex, &parsedPoint.normalIndex);
				}

				// If we parsed a valid face point (i.e. one that at least contains the index of a vertex), add it.
				if (parsedPoint.vertexIndex > 0) {
					// Adjust all indices down by one: Wavefront OBJ uses 1-based indices, but our arrays are 0-based.
					parsedPoint.vertexIndex--;

					// Discard any negative texture coordinate or normal indices while adjusting them down by one.
					parsedPoint.texCoordIndex = (parsedPoint.texCoordIndex > 0) ? parsedPoint.texCoordIndex - 1 : -1;
					parsedPoint.normalIndex = (parsedPoint.normalIndex > 0) ? parsedPoint.normalIndex - 1 : -1;

					memcpy_s(&face->points[face->pointCount], sizeof(GLUE_OBJ_FacePoint), &parsedPoint, sizeof(GLUE_OBJ_FacePoint));
					face->pointCount++;
				}
			}

			token = strtok_s(NULL, delimiter, &context);
		}

		// If we have fewer points than expected, free the unused memory.
		if (face->pointCount == 0) {
			free(face->points);
			face->points = NULL;
		}
		else if (face->pointCount < maxPoints) {
			realloc(face->points, sizeof(GLUE_OBJ_FacePoint) * face->pointCount);
		}
	}
	else
	{
		face->points = NULL;
	}
}

/*
	Render the faces of the specified Mesh Object in OpenGL.
*/
void GLUE_renderMeshObject(GLUE_OBJ* object) {
	glMaterialfv(GL_FRONT, GL_AMBIENT, object->material->AmbientColour);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, object->material->DiffuseColour);
	glMaterialfv(GL_FRONT, GL_SPECULAR, object->material->SpecularColour);
	glMaterialf(GL_FRONT, GL_SHININESS, *object->material->Shininess);

	// Set up model transformations
	glPushMatrix();

	glScalef(object->scale->x, object->scale->y, object->scale->z);

	glTranslatef(object->location->x, object->location->y, object->location->z);

	glRotatef(object->rotation->x, 1, 0, 0);
	glRotatef(object->rotation->y, 0, 1, 0);
	glRotatef(object->rotation->z, 0, 0, 1);

	//Itterate through the faces
	for (int faceNo = 0; faceNo < object->faceCount; faceNo++) {
		GLUE_OBJ_Face face = object->faces[faceNo]; //Select one face

		//Ensure there are enough faces to draw a closed shape
		if (face.pointCount >= 3) {
			glBegin(GL_POLYGON);

			for (int pointNo = 0; pointNo < face.pointCount; pointNo++) {
				GLUE_OBJ_FacePoint point = face.points[pointNo];


				if (point.normalIndex >= 0) {
					Vector3D normal = object->normals[point.normalIndex];
					glNormal3d(normal.x, normal.y, normal.z);
				}


				if (point.texCoordIndex >= 0) {
					Vector2D texCoord = object->texCoords[point.texCoordIndex];
					glTexCoord2d(texCoord.x, texCoord.y);
				}

				Vector3D vertex = object->vertices[point.vertexIndex];
				glVertex3f(vertex.x, vertex.y, vertex.z);

			}

			glEnd();
		}
	}
	glPopMatrix();
}

/*
	Free the specified Mesh Object, including all of its vertices, texture coordinates, normals, and faces.
*/
void GLUE_freeMeshObject(GLUE_OBJ* object)
{
	if (object != NULL) {
		free(object->vertices);
		free(object->texCoords);
		free(object->normals);

		if (object->faces != NULL) {
			for (int i = 0; i < object->faceCount; i++) {
				free(object->faces[i].points);
			}

			free(object->faces);
		}

		free(object);
	}
}


void drawBox(Vector3D* min, Vector3D* max) {
	glBegin(GL_LINES);
	glColor3b(1, 0.5, 0);

	glVertex3f(min->x, min->y, min->z);
	glVertex3f(min->x, min->y, max->z);

	glVertex3f(min->x, min->y, min->z);
	glVertex3f(min->x, max->y, min->z);

	glVertex3f(min->x, min->y, min->z);
	glVertex3f(max->x, min->y, min->z);

	glEnd();
}


//WIP
void GLUE_SetCameraToObject(GLUE_OBJ* object, GLfloat distanceFromOBJ, GLfloat theta, GLfloat phi) {
    GLfloat cameraPosition[3];
    GLfloat center[3];

    // Convert angles to radians for calculations
    GLfloat thetaRad = (PI / 180) * theta;  // Vertical angle
    GLfloat phiRad = (PI / 180) * phi; // Horizontal angle
    GLfloat rotationYRad = -1*(PI / 180) * object->rotation->y; // Rotation around Y-axis

    // Calculate the camera's position relative to the object based on distance and angles
    cameraPosition[0] = distanceFromOBJ * sin(thetaRad) * cos(phiRad);
    cameraPosition[1] = distanceFromOBJ * sin(thetaRad) * sin(phiRad);
    cameraPosition[2] = distanceFromOBJ * cos(thetaRad);

    // Rotate the camera position around the Y-axis according to the object's rotation
    GLfloat rotatedCameraPosition[3];
    rotatedCameraPosition[0] = cameraPosition[0] * cos(rotationYRad) + cameraPosition[2] * sin(rotationYRad);
    rotatedCameraPosition[1] = cameraPosition[1]; // Y position remains the same
    rotatedCameraPosition[2] = -cameraPosition[0] * sin(rotationYRad) + cameraPosition[2] * cos(rotationYRad);

    // Update the camera position by adding the object's location
    cameraPosition[0] = object->location->x - rotatedCameraPosition[0];
    cameraPosition[1] = object->location->y - rotatedCameraPosition[1];
    cameraPosition[2] = object->location->z + rotatedCameraPosition[2];

    // Set the center of the camera to be the object's location
    center[0] = object->location->x;
    center[1] = object->location->y;
    center[2] = object->location->z;

    // Use gluLookAt to set the camera position and orientation
    gluLookAt(cameraPosition[0], cameraPosition[1], cameraPosition[2], 
              center[0], center[1], center[2], 
              0, 1, 0);  // Assuming the up vector is along Y
}

void GLUE_NormalizeOBJ(GLUE_OBJ* object) {

	GLfloat sum[3] = {0, 0, 0}; //Store the sum of all verticies
	GLfloat min[3] = { FLT_MAX, FLT_MAX, FLT_MAX }; //Store max float value
	GLfloat max[3] = { FLT_MIN, FLT_MIN, FLT_MIN }; //Store min float value

	double scaleFactor = 0; //How much to scale the OBJ by

	//Loop through all vertex to find min, max and average verticies
	for (int i = 0; i < object->vertexCount; i++) {
		GLfloat x = object->vertices[i].x;
		GLfloat y = object->vertices[i].y;
		GLfloat z = object->vertices[i].z;

		//Add to sum of verticies
		sum[0] += x;
		sum[1] += y;
		sum[2] += z;

		//Find min vertex
		if (x < min[0]) min[0] = x;
		if (y < min[1]) min[1] = y;
		if (z < min[2]) min[2] = z;

		//Find max vertex
		if (x > max[0]) max[0] = x;
		if (y > max[1]) max[1] = y;
		if (z > max[2]) max[2] = z;
	}

	for (int i = 0; i < 3; i++) if (fabs(min[i]) > scaleFactor) scaleFactor = fabs(min[i]);
	for (int i = 0; i < 3; i++) if (fabs(max[i]) > scaleFactor) scaleFactor = fabs(max[i]);
	

	GLfloat averages[3] = {sum[0] / object->vertexCount, sum[1] / object->vertexCount, sum[2] / object->vertexCount };

	//Used for debuging
		printf("Before:\n");
		printf("Average: %f, %f, %f\n", averages[0], averages[1], averages[2]);
		printf("Min: %f, %f, %f\n", min[0], min[1], min[2]);
		printf("Max: %f, %f, %f\n", max[0], max[1], max[2]);
		printf("Scale Factor: %f\n", scaleFactor);
		printf("---\n");
	//

	scaleFactor = 1 / scaleFactor;

	//Set the new vertex postions
	for (int i = 0; i < object->vertexCount; i++) {
		//Set center location to 0,0,0
		object->vertices[i].x += averages[0] * -1;
		object->vertices[i].y += averages[1] * -1;
		object->vertices[i].z += averages[2] * -1;

		object->vertices[i].x *= scaleFactor;
		object->vertices[i].y *= scaleFactor;
		object->vertices[i].z *= scaleFactor;
	}

	//Used for debuging
		GLfloat nsum[3] = { 0, 0, 0 }; //Store the sum of all verticies
		GLfloat nmin[3] = { FLT_MAX, FLT_MAX, FLT_MAX }; //Store max float value
		GLfloat nmax[3] = { FLT_MIN, FLT_MIN, FLT_MIN }; //Store min float value

		for (int i = 0; i < object->vertexCount; i++) {
			GLfloat x = object->vertices[i].x;
			GLfloat y = object->vertices[i].y;
			GLfloat z = object->vertices[i].z;

			//Add to sum of verticies
			nsum[0] += x;
			nsum[1] += y;
			nsum[2] += z;

			//Find min vertex
			if (x < nmin[0]) nmin[0] = x;
			if (y < nmin[1]) nmin[1] = y;
			if (z < nmin[2]) nmin[2] = z;

			//Find max vertex
			if (x > nmax[0]) nmax[0] = x;
			if (y > nmax[1]) nmax[1] = y;
			if (z > nmax[2]) nmax[2] = z;
		}

		GLfloat naverages[3] = { nsum[0] / object->vertexCount, nsum[1] / object->vertexCount, nsum[2] / object->vertexCount };
		printf("After:\n");
		printf("Average: %f, %f, %f\n", naverages[0], naverages[1], naverages[2]);
		printf("Min: %f, %f, %f\n", nmin[0], nmin[1], nmin[2]);
		printf("Max: %f, %f, %f\n", nmax[0], nmax[1], nmax[2]);
		printf("1/Scale Factor: %f\n", scaleFactor);
	//
}