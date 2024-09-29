#include "GLUE_OBJ.h"


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
	if (object->normalCount > 0) object->normals = malloc(sizeof(Vector3D) * object->normalCount);
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
				printf("%i: %f, %f, %f\n", currentVertexIndex, vertex.x, vertex.y, vertex.z);
			}
			else if (strcmp(keyword, "vt") == 0) {
				Vector2D texCoord = { 0, 0 };
				sscanf_s(line, "%*s %f %f", &texCoord.x, &texCoord.y);
				memcpy_s(&object->texCoords[currentTexCoordIndex], sizeof(Vector2D), &texCoord, sizeof(Vector2D));
				currentTexCoordIndex++;
			}
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
	glMaterialf(GL_FRONT, GL_SHININESS, object->material->Shininess);

	// Set up model transformations
	glPushMatrix();

	glScalef(object->scale->x, object->scale->y, object->scale->z);

	glTranslatef(object->offset->x, object->offset->y, object->offset->z);

	glRotatef(object->rotation->x, 0, 1, 0);

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


///***WIP***///
void computeBoundingBox(GLUE_OBJ* object, Vector3D* min, Vector3D* max) {
	//if (object == NULL || object->vertices == NULL) return;

	min->x = min->y = min->z = 0xFFFFFF;
	max->x = max->y = max->z = -0xFFFFFF;

	/*for (int i = 0; i < object->vertexCount; i++) {
		Vector3D* v = &object->vertices[i];
		if (v->x < min->x) min->x = v->x;
		if (v->y < min->y) min->y = v->y;
		if (v->z < min->z) min->z = v->z;
		if (v->x > max->x) max->x = v->x;
		if (v->y > max->y) max->y = v->y;
		if (v->z > max->z) max->z = v->z;
	}*/

	printf("Min: %d, %d, %d\n", min->x, min->y, min->z);
	printf("Max: %d, %d, %d\n", max->x, max->y, max->z);
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