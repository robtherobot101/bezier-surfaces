//  ========================================================================
//  COSC422: Computer Graphics (2018);  University of Canterbury.
//
//  FILE NAME: Terrain.cpp
//  This is part of Assignment1 files.
//
//	The program generates and loads the mesh data for a terrain floor (100 verts, 81 elems).
//  Required files:  Terrain.vert (vertex shader), Terrain.frag (fragment shader), HeightMap1.tga  (height map)
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h"
using namespace std;

GLuint vaoID;
GLuint theProgram;
GLuint mvMatrixLoc, lightLoc, normLoc, mvpMatrixLoc, zLoc, boundsLoc, waterLoc, mapLoc, tLoc;

glm::mat4 proj, view;   //Projection and view matrices
glm::vec3 camPos = glm::vec3(0.0, 20.0, 30.0);

float lookAngle = 0;
int camSpeed = 0;
int camaSpeed = 0;
float waterLevel = 20;
float snowLevel = 16;
float waterSpeed = -0.2;
float snowSpeed = 0;
bool draining = true;
int heightMap = 0;
GLuint texLoc, tex2Loc;
GLuint wireFrame = GL_FILL;
int t = 0;

float CDR = 3.14159265/180.0;     //Conversion from degrees to rad (required in GLM 0.9.6)

float verts[100*3];       //10x10 grid (100 vertices)
GLushort elems[81*4];       //Element array for 81 quad patches

glm::mat4 projView;

glm::vec4 texBounds = glm::vec4(9, 12, 15, 20);

//Generate vertex and element data for the terrain floor
void generateData()
{
	int indx, start;
	//verts array
	for(int i = 0; i < 10; i++)   //100 vertices on a 10x10 grid
	{
		for(int j = 0; j < 10; j++)
		{
			indx = 10*i + j;
			verts[3*indx] = 10*i - 45;		//x  varies from -45 to +45
			verts[3*indx+1] = 0;			//y  is set to 0 (ground plane)
			verts[3*indx+2] = -10*j;		//z  varies from 0 to -100
		}
	}

	//elems array
	for(int i = 0; i < 9; i++)
	{
		for(int j = 0; j < 9; j++)
		{
			indx = 9*i +j;
			start = 10*i + j;
			elems[4*indx] = start;
			elems[4*indx+1] = start+10;
			elems[4*indx+2] = start+11;
			elems[4*indx+3] = start+1;			
		}
	}
}

//Loads terrain texture
void loadTextures()
{
	GLuint texID[6];
    glGenTextures(6, texID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID[0]);
	loadTGA("HeightMap1.tga");
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texID[1]);
	loadTGA("grassy.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
   	glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texID[2]);
	loadTGA("rocky.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
   	glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texID[3]);
	loadTGA("snowy.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, texID[4]);
	loadTGA("HeightMap2.tga");
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texID[5]);
	loadTGA("ice.tga");
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}


//Loads a shader file and returns the reference to a shader object
GLuint loadShader(GLenum shaderType, string filename)
{
	ifstream shaderFile(filename.c_str());
	if(!shaderFile.good()) cout << "Error opening shader file." << endl;
	stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();
	string shaderStr = shaderData.str();
	const char* shaderTxt = shaderStr.c_str();

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderTxt, NULL);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		//const char *strShaderType = NULL;
		cerr <<  "Compile failure in shader: " << strInfoLog << endl;
		delete[] strInfoLog;
	}
	return shader;
}

//Initialise the shader program, create and load buffer data
void initialise()
{
//--------Load terrain height map-----------
	loadTextures();
//--------Load shaders----------------------
	GLuint shaderv = loadShader(GL_VERTEX_SHADER, "Terrain.vert");
	GLuint shadertc = loadShader(GL_TESS_CONTROL_SHADER, "Terrain.cont");
	GLuint shaderte = loadShader(GL_TESS_EVALUATION_SHADER, "Terrain.eval");
	GLuint shaderg = loadShader(GL_GEOMETRY_SHADER, "Terrain.geom");
	GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "Terrain.frag");

	GLuint program = glCreateProgram();
	glAttachShader(program, shaderv);
	glAttachShader(program, shadertc);
	glAttachShader(program, shaderte);
	glAttachShader(program, shaderg);
	glAttachShader(program, shaderf);
	glLinkProgram(program);

	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
	glUseProgram(program);

	mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
	texLoc = glGetUniformLocation(program, "heightMap");
	GLuint grassLoc = glGetUniformLocation(program, "grassTex");
	GLuint rockLoc = glGetUniformLocation(program, "rockTex");
	GLuint snowLoc = glGetUniformLocation(program, "snowTex");
	mvMatrixLoc = glGetUniformLocation(program, "mvMatrix");
	lightLoc = glGetUniformLocation(program, "lightPos");
	normLoc = glGetUniformLocation(program, "norMatrix");
	boundsLoc = glGetUniformLocation(program, "texBounds");
	waterLoc = glGetUniformLocation(program, "waterLevel");
	GLuint tex2Loc = glGetUniformLocation(program, "heightMap2");
	GLuint iceLoc = glGetUniformLocation(program, "iceTex");
	mapLoc = glGetUniformLocation(program, "map");
	tLoc = glGetUniformLocation(program, "t");
	
	
	
	zLoc = glGetUniformLocation(program, "camz");
	glUniform1i(texLoc, 0);
	glUniform1i(grassLoc, 1);
	glUniform1i(rockLoc, 2);
	glUniform1i(snowLoc, 3);
	glUniform1i(tex2Loc, 4);
	glUniform1i(iceLoc, 5);
	glPatchParameteri(GL_PATCH_VERTICES, 4);

//--------Compute matrices----------------------
	proj = glm::perspective(30.0f*CDR, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
	float xlook = -100.0 * sin(lookAngle * CDR) + camPos[0];
    float zlook = -100.0 * cos(lookAngle * CDR) + camPos[2];
	view = glm::lookAt(camPos, glm::vec3(xlook, camPos[1] - 10, zlook), glm::vec3(0.0, 1.0, 0.0)); //view matrix
	projView = proj * view;  //Product (mvp) matrix

//---------Load buffer data-----------------------
	generateData();

	GLuint vboID[2];
	glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glGenBuffers(2, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
}

//Display function to compute uniform values based on transformation parameters and to draw the scene
void display()
{
	glm::vec4 light = glm::vec4{50.0, -1000.0, 50.0, 1.0};
	float xlook = -100.0 * sin(lookAngle * CDR) + camPos[0];
    float zlook = -100.0 * cos(lookAngle * CDR) + camPos[2];
	proj = glm::perspective(30.0f*CDR, 1.25f, 2.0f, 500.0f);  //perspective projection matrix
	view = glm::lookAt(camPos, glm::vec3(xlook, camPos[1] - 10, zlook), glm::vec3(0.0, 1.0, 0.0)); //view matrix
	light = view * light;
	projView = proj * view;  //Product (mvp) matrix
	glm::mat4 invMatrix = glm::inverse(view);
	
	glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
	glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(normLoc, 1, GL_TRUE,  &invMatrix[0][0]);
	glUniform1f(zLoc, camPos[2]);
	glUniform4fv(lightLoc, 1, &light[0]);
	glUniform4fv(boundsLoc, 1, &texBounds[0]);
	glUniform1f(waterLoc, waterLevel);
	glUniform1i(mapLoc, heightMap);
	glUniform1i(tLoc, t);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vaoID);
	glDrawElements(GL_PATCHES, 81*4, GL_UNSIGNED_SHORT, NULL);
	glFlush();
}

void update(int value)
{
	t++;
    camPos[0] -= sin(lookAngle * CDR) * camSpeed;
    camPos[2] -= cos(lookAngle * CDR) * camSpeed;
    lookAngle += camaSpeed;
    waterLevel += waterSpeed;
    texBounds[2] -= snowSpeed;
    texBounds[1] = glm::min(12.0f, texBounds[2]);
    if(draining && waterLevel <= 10) waterSpeed = draining = false;
    	
	glutTimerFunc(50, update, 0);
	glutPostRedisplay();
}

void special(int key, int x, int y)
{
	if(key == GLUT_KEY_UP) {
		camSpeed = 1;
	}
	else if(key == GLUT_KEY_DOWN) {
		camSpeed = -1;
	}
    else if(key==GLUT_KEY_LEFT) camaSpeed = 2;        //Turn left
    else if(key==GLUT_KEY_RIGHT) camaSpeed = -2;   //Turn right
 
    glutPostRedisplay();
}

void specialUp (int key, int x, int y)
{
    if(key == GLUT_KEY_UP || key == GLUT_KEY_DOWN) {
        camSpeed = 0;
    }
    else if(key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT) {
        camaSpeed = 0;
 
    }
}
 
void keyPressed(unsigned char key, int x, int y)
{
	if(key == '+')
	{
		waterSpeed = 0.1; 
	}
	else if(key == '-')
	{
		waterSpeed = -0.1;
	}
	else if(key == 'w') 
	
	{
		wireFrame = (wireFrame == GL_LINE) ? GL_FILL : GL_LINE; 
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame);
	}
	else if(key == '1')
	{
		heightMap = 0;
	}
	else if(key == '2')
	{
		heightMap = 1;
	}
	else if(key == 's')
	{
		snowSpeed = 0.1;
	}
	else if(key == 'S')
	{
		snowSpeed = -0.1;
	}
}

void keyReleased(unsigned char key, int x, int y)
{
	if(key == '+' || key == '-')
	{
		waterSpeed = 0; 
	}
	else if(key == 's' || key == 'S')
	{
		snowSpeed = 0; 
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Terrain");
	glutInitContextVersion (4, 2);
	glutInitContextProfile ( GLUT_CORE_PROFILE );

	if(glewInit() == GLEW_OK)
	{
		cout << "GLEW initialization successful! " << endl;
		cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
	}
	else
	{
		cerr << "Unable to initialize GLEW  ...exiting." << endl;
		exit(EXIT_FAILURE);
	}

	initialise();
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);
	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutTimerFunc(50, update, 0);
	glutDisplayFunc(display);
	glutMainLoop();
}

