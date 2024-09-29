#include "GLUE3D.h"

const  Colour WHITE = { 1.0f, 1.0f, 1.0f };

void GLUE_COLOUR( Colour c) {
	glColor3f(c.r, c.g, c.b); //Sets the GL colour using the colour struct
}

void GLUE_CIRCLE(Location l, int r, Colour c) {
	glBegin(GL_TRIANGLE_FAN); //Set drawing mode to triangle fan
	glColor3f(c.r, c.g, c.b); //Set center colour
	glVertex2f(l.x, l.y); //Draw center point

	//Calculate the colour of the outter points using the falloff conststant
	glColor3f(c.r, c.g, c.b);

	//Loop for the number of points that make up the circle
	for (int i = 0; i <= SHAPE_RESOLUTION; i++) {
		double angle = (2 * PI / SHAPE_RESOLUTION) * i; //Calculate the current angle

		float px = r * cos(angle); //Convert polar point to cartisian x
		float py = r * sin(angle); //Convert polar point to cartisian y

		glVertex2f(px + l.x, py + l.y); //Set vertex point
	}
	glEnd(); //Finish drawing the circle
}

struct Location GLUE_LineCenter(Location firstLocation, Location secondLocation) {
	Location middle = { (firstLocation.x + secondLocation.x) / 2, (firstLocation.y + secondLocation.y) / 2, };
	return middle;
}

struct Location GLUE_PolygonCenter(Location locations[]) {
	int xSum = 0;
	int ySum = 0;

	int count = sizeof(locations) / sizeof(Location);

	for (int i = 0; i < count; i++) {
		xSum += locations[i].x;

		ySum += locations[i].y;
	}

	Location middle = { xSum / count, ySum / count };
	return middle;
}

float GLUE_LocationDistances(Location first, Location second) {
	return sqrt(pow(first.x - second.x, 2) + pow(first.y - second.y, 2));
}

void GLUE_Text(char str[], Location l, Colour c) {
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

//Function to offset a previously set location
Location GLUE_OffsetLocation(Location* loc, int x, int y) {
	Location newLocation = { loc->x + x, loc->y + y };
	return newLocation;
}

void GLUE_Line(Location start, Location end, Colour startColour, Colour endColour, int thickness) {
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
