#include "GLUE.h"


struct Colour FallOffMultiplyer = { 0.7f, 0.6f, 0.3f };
const struct Colour WHITE = { 1.0f, 1.0f, 1.0f };

void GLUE_COLOUR(struct Colour c) {
	glColor3f(c.r, c.g, c.b); //Sets the GL colour using the colour struct
}

void GLUE_CIRCLE(struct Location l, int r, struct Colour c) {
	glBegin(GL_TRIANGLE_FAN); //Set drawing mode to triangle fan
	glColor3f(c.r, c.g, c.b); //Set center colour
	glVertex2f(l.x, l.y); //Draw center point

	//Calculate the colour of the outter points using the falloff conststant
	glColor3f(c.r - (COLOR_FALLOFF * FallOffMultiplyer.r), c.g - (COLOR_FALLOFF * FallOffMultiplyer.g), c.b - (COLOR_FALLOFF * FallOffMultiplyer.b));

	//Loop for the number of points that make up the circle
	for (int i = 0; i <= SHAPE_RESOLUTION; i++) {
		double angle = (2 * PI / SHAPE_RESOLUTION) * i; //Calculate the current angle

		float px = r * cos(angle); //Convert polar point to cartisian x
		float py = r * sin(angle); //Convert polar point to cartisian y

		glVertex2f(px + l.x, py + l.y); //Set vertex point
	}
	glEnd(); //Finish drawing the circle
}

struct Location GLUE_LineCenter(struct Location firstLocation, struct Location secondLocation) {
	struct Location middle = { (firstLocation.x + secondLocation.x) / 2, (firstLocation.y + secondLocation.y) / 2, };
	return middle;
}

struct Location GLUE_PolygonCenter(struct Location locations[]) {
	int xSum = 0;
	int ySum = 0;

	int count = sizeof(locations) / sizeof(struct Location);

	for (int i = 0; i < count; i++) {
		xSum += locations[i].x;

		ySum += locations[i].y;
	}

	struct Location middle = { xSum / count, ySum / count };
	return middle;
}

float GLUE_LocationDistances(struct Location first, struct Location second) {
	return sqrt(pow(first.x - second.x, 2) + pow(first.y - second.y, 2));
}

void GLUE_Text(char str[], struct Location l, struct Colour c) {
	glColor3f(c.r, c.g, c.b); //Set text colour
	for (int i = 0; i < strlen(str); i++) { //Loop through each character
		glRasterPos2f(l.x, l.y); //Set position of the character
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, str[i]); //Draw the character

		l.x += 10;//Shift the character 10px to the right
	}
}

char* GLUE_toStringf(double num) {
	char* str = (char*)malloc(50 * sizeof(char)); //Assign memory location for the converted value

	//Ensure that the string is not null
	if (str == NULL) return NULL; // Allocation failed, return NULL

	sprintf_s(str, 50, "%f", num); //Convert float to string using printf

	return str; //Return the new string value
}

char* GLUE_toStringi(double num) {
	char* str = (char*)malloc(50 * sizeof(char)); //Assign memory location for the converted value

	//Ensure that the string is not null
	if (str == NULL) return NULL; // Allocation failed, return NULL

	sprintf_s(str, 50, "%.0f", num); //Convert integer to string using printf

	return str; //Return the new string value
}

char* GLUE_combineString(char str1[], char str2[]) {
	// Calculate the length of the first string
	int len1 = 0;
	while (str1[len1] != '\0') {
		len1++;
	}

	// Calculate the length of the second string
	int len2 = 0;
	while (str2[len2] != '\0') {
		len2++;
	}

	// Allocate memory for the combined string (including null terminator)
	char* combinedStr = (char*)malloc((len1 + len2 + 1) * sizeof(char));

	// Copy characters from the first string
	for (int i = 0; i < len1; i++) {
		combinedStr[i] = str1[i];
	}

	// Copy characters from the second string
	for (int i = 0; i < len2; i++) {
		combinedStr[len1 + i] = str2[i];
	}

	// Add the null terminator
	combinedStr[len1 + len2] = '\0';

	return combinedStr;
}

char* GLUE_stringValuef(char title[], double num) {
	return GLUE_combineString(title, GLUE_toStringf(num));
}

char* GLUE_stringValuei(char title[], double num) {
	return GLUE_combineString(title, GLUE_toStringi(num));
}

int GLUE_RAND(int maxVal) {
	return (int)(((double)(maxVal + 1) / RAND_MAX) * rand()); //Clamp rand value betwen 0 and randMax
}

int GLUE_RAND_MIN_MAX(int minVal, int maxVal) {
	return (int)(((double)rand()) * (maxVal - minVal) / (RAND_MAX)+minVal);
}

//Redraw the particle
void GLUE_DrawParticle(struct Particle_2D* p) {
	GLUE_CIRCLE(p->location, p->size, p->colour);
}

//Move the particle acoording to its velocity
void GLUE_MoveParticle(struct Particle_2D* p) {
	p->location.x -= p->velocity.vx;
	p->location.y -= p->velocity.vy;
}

void GLUE_AddParticle(struct Particle_2D p) {
	int hash = p.id % MAX_PARTICLES; //Use a hash function to set its array location
	GLUE_Particles[hash] = p;
}

void GLUE_MakeParticle(struct Location location,
	struct Colour colour,
	struct Velocity velocity,
	float size,
	enum ParticleState state,
	int id) {

	//Create a new particle
	struct Particle_2D p = { location, colour, velocity,size, state, id };

	int hash = p.id % MAX_PARTICLES; //Use a hash function to set its array location
	GLUE_Particles[hash] = p; //Set the array position depending on hash location
}

//Function to offset a previously set location
struct Location GLUE_OffsetLocation(struct Location* loc, int x, int y) {
	struct Location newLocation = { loc->x + x, loc->y + y };

	return newLocation;
}

void GLUE_Line(struct Location start, struct Location end, struct Colour startColour, struct Colour endColour, int thickness) {
	glBegin(GL_QUADS);


	// Calculate direction vector
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	float length = sqrt(dx * dx + dy * dy);

	// Calculate perpendicular vector (normal)
	float nx = -dy / length * thickness / 2;
	float ny = dx / length * thickness / 2;

	GLUE_COLOUR(startColour); //Set the line colour
	// Draw the thick line as a quadrilateral
	glVertex2f(start.x + nx, start.y + ny);
	glVertex2f(start.x - nx, start.y - ny);

	GLUE_COLOUR(endColour); //Set the line colour
	glVertex2f(end.x - nx, end.y - ny);
	glVertex2f(end.x + nx, end.y + ny);

	glEnd();
}


void GLUE_Tree(struct Location baseLocation, struct Colour baseColour, struct Colour tipColour, int depth) {
	int maxSize = 500; //Max size of tree

	//Begin branch recursion, 
	GLUE_TreeBranch(baseLocation, baseColour, tipColour, maxSize - baseLocation.y, depth, 0.01 * (300 - baseLocation.y), (90) * (PI / 180));
}

static void GLUE_TreeBranch(struct Location startPoint, struct Colour baseColour, struct Colour tipColour, float length, int depth, int itter, float angle) {
	//Ensure that recursion has a limit
	if (itter <= depth) {
		//Move recursivly here
		length *= TREE_BRANCH_SCALAR; //+ GLUE_RAND_MIN_MAX(-1* TREE_VARIABILITY, TREE_VARIABILITY); //Shrink the length of the branch
		length += itter;

		struct Colour sectionColour; //Create a new colour for this specific branch

		//Calculat the colour of the branch section
		sectionColour.r = baseColour.r + (tipColour.r - baseColour.r) / (depth - itter);
		sectionColour.g = baseColour.g + (tipColour.g - baseColour.g) / (depth - itter);
		sectionColour.b = baseColour.b + (tipColour.b - baseColour.b) / (depth - itter);

		//Calculate the end point of the branch to draw to
		struct Location endPoint = GLUE_OffsetLocation(&startPoint, length * cos(angle), length * sin(angle));

		//Draw the branch line with reletive thickness
		GLUE_Line(startPoint, endPoint, baseColour, sectionColour, TREE_THICKNESS_SCALAR * (depth - itter));
		GLUE_TreeBranch(endPoint, sectionColour, tipColour, length, depth, itter + 1, angle + (TREE_ANGLE * (PI / 180))); //Recursivly draw Right branch
		GLUE_TreeBranch(endPoint, sectionColour, tipColour, length, depth, itter + 1, angle - (TREE_ANGLE * (PI / 180))); //Recursivly draw Left Branch
	}
	//Otherwise the tree has reached max depth
}

int getParticleCount(int screenHeight) {
	int count = 0;
	for (int i = 0; i < MAX_PARTICLES; i++) {
		if (GLUE_Particles[i].state == ALIVE) count++;
	}
	return count;
}

MeshOBJ* GLUE_loadMeshObject(char* fileName) {
	Vector3D vertSum = {0, 0, 0};
	FILE* inFile = NULL;
	MeshOBJ* object;
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
	object = malloc(sizeof(MeshOBJ));
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
	if (object->faceCount > 0) object->faces = malloc(sizeof(meshObjectFace) * object->faceCount);

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
			else if (strcmp(keyword, "vn") == 0) {
				Vector3D normal = { 0, 0, 0 };
				sscanf_s(line, "%*s %f %f %f", &normal.x, &normal.y, &normal.z);
				memcpy_s(&object->normals[currentNormalIndex], sizeof(Vector3D), &normal, sizeof(Vector3D));
				currentNormalIndex++;
			}
			else if (strcmp(keyword, "f") == 0) {
				initMeshObjectFace(&(object->faces[currentFaceIndex]), line, _countof(line));
				currentFaceIndex++;
			}
		}
	}

	fclose(inFile);

	Vector3D centerPoint = { vertSum.x / object->vertexCount,
						vertSum.y / object->vertexCount,
						vertSum.z / object->vertexCount };
	Vector3D zero = { -1 * centerPoint.x, -1 * centerPoint.y, -1 * centerPoint.z };
	object->offset = &zero;

	return object;
}


/*
	Initialise the specified Mesh Object Face from a string of face data in the Wavefront OBJ file format.
*/
void initMeshObjectFace(meshObjectFace* face, char* faceData, int maxFaceDataLength) {
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
		face->points = malloc(sizeof(meshObjectFacePoint) * maxPoints);

		token = strtok_s(faceData, delimiter, &context);
		while ((token != NULL) && (face->pointCount < maxPoints)) {
			meshObjectFacePoint parsedPoint = { 0, 0, 0 }; // At this point we're working with 1-based indices from the OBJ file.

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

					memcpy_s(&face->points[face->pointCount], sizeof(meshObjectFacePoint), &parsedPoint, sizeof(meshObjectFacePoint));
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
			realloc(face->points, sizeof(meshObjectFacePoint) * face->pointCount);
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
void GLUE_renderMeshObject(MeshOBJ* object) {

	GLfloat ambient[] = { object->material.AmbientColour.R, object->material.AmbientColour.G, object->material.AmbientColour.B, object->material.AmbientColour.A };
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, &object->material.DiffuseColour);
	glMaterialfv(GL_FRONT, GL_SPECULAR, &object->material.SpecularColour);
	glMaterialf(GL_FRONT, GL_SHININESS, object->material.Shininess);
	// Set up model transformations
	glPushMatrix();

	// Apply scale
	if (object->scale != NULL) {
		glScalef(object->scale->x, object->scale->y, object->scale->z);
	}

	// Apply translation (offset)
	if (object->offset != NULL) {
		glTranslatef(object->offset->x, object->offset->y, object->offset->z);
	}

	//Itterate through the faces
	for (int faceNo = 0; faceNo < object->faceCount; faceNo++) {
		meshObjectFace face = object->faces[faceNo]; //Select one face

		//Ensure there are enough faces to draw a closed shape
		if (face.pointCount >= 3) {
			glBegin(GL_POLYGON);

			for (int pointNo = 0; pointNo < face.pointCount; pointNo++) {
				meshObjectFacePoint point = face.points[pointNo];

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
void freeMeshObject(MeshOBJ* object)
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

//I AM SO LOST WHAT THIS DOES
void loadPPM()
{

	//Set the texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void computeBoundingBox(MeshOBJ* object, Vector3D* min, Vector3D* max) {
	//if (object == NULL || object->vertices == NULL) return;

	min->x = min->y = min->z = 0xFFFFFF;
	max->x = max->y = max->z = -0xFFFFFF;

	for (int i = 0; i < object->vertexCount; i++) {
		Vector3D* v = &object->vertices[i];
		if (v->x < min->x) min->x = v->x;
		if (v->y < min->y) min->y = v->y;
		if (v->z < min->z) min->z = v->z;
		if (v->x > max->x) max->x = v->x;
		if (v->y > max->y) max->y = v->y;
		if (v->z > max->z) max->z = v->z;
	}

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