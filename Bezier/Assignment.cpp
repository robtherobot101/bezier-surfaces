//  ========================================================================
//  COSC422: Advanced Computer Graphics (2016);  University of Canterbury.
//
//  FILE NAME: GS_SurfRevln.cpp
//  See  Ex05_SurfaceRevln.pdf  for details

//	Demonstrates the use of geometry shader in geometry amplification
//  Files required:  Vase.dat, GeomSR.vert, GeomSR.frag
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

GLuint vao[2];
float CDR = 3.14159265/180.0;   //Degrees to radians conversion
int nvert, nvert2 = 0;
GLuint matrixLoc1, matrixLoc2, matrixLoc3, vecLoc1, zLoc, tLoc, floorMatrixLoc1, program, floorProgram;
glm::mat4 mvpMatrix, proj, view, invMatrix;
float angle = 0.0;
float camz = 200.001;
float camSpeed = 0;
GLenum wireFrame = GL_FILL;

int GRIDLINES = 50;
float GRIDSIZE = 5.0;

bool exploding = false;
int t = 0;

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

void initialise()
{
	GLuint shaderv = loadShader(GL_VERTEX_SHADER, "Assignment.vert");
	GLuint shadertc = loadShader(GL_TESS_CONTROL_SHADER, "Assignment.cont");
	GLuint shaderte = loadShader(GL_TESS_EVALUATION_SHADER, "Assignment.eval");
	GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "Assignment.frag");
	GLuint shaderg = loadShader(GL_GEOMETRY_SHADER, "Assignment.geom");

	program = glCreateProgram();
	glAttachShader(program, shaderv);
	glAttachShader(program, shadertc);
	glAttachShader(program, shaderte);
	glAttachShader(program, shaderf);
	glAttachShader(program, shaderg);
	glLinkProgram(program);
	
	GLuint floorShaderv = loadShader(GL_VERTEX_SHADER, "floor.vert");
	GLuint floorShaderf = loadShader(GL_FRAGMENT_SHADER, "floor.frag");
	
	floorProgram = glCreateProgram();
	glAttachShader(floorProgram, floorShaderv);
	glAttachShader(floorProgram, floorShaderf);
	glLinkProgram(floorProgram);
	

	GLint status;
	GLint floorStatus;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	glGetProgramiv (floorProgram, GL_LINK_STATUS, &floorStatus);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
	
	if (floorStatus == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(floorProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(floorProgram, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
	glUseProgram(program);

	matrixLoc1 = glGetUniformLocation(program, "mvpMatrix");
	matrixLoc2 = glGetUniformLocation(program, "mvMatrix");
	matrixLoc3 = glGetUniformLocation(program, "norMatrix");
	vecLoc1    = glGetUniformLocation(program, "lightpos");
	zLoc	   = glGetUniformLocation(program, "camz");
	tLoc	   = glGetUniformLocation(program, "t");
	

    ifstream infile;
    infile.open("PatchVerts_Gumbo.txt", ios::in);
	infile >> nvert;
	float verts[nvert*3];
	for(int i = 0; i < nvert; i++)
	{
	glUseProgram(floorProgram);
		infile >> verts[3*i] >> verts[3*i+1] >> verts[3*i+2];
	}
	GLuint vbo[2];

	glGenVertexArrays(2, vao);
    glBindVertexArray(vao[0]);

    glGenVertexArrays(2, vbo);
    
    glPatchParameteri(GL_PATCH_VERTICES, 16);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    
	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    
    
    
    
    
    glUseProgram(floorProgram);

	floorMatrixLoc1 = glGetUniformLocation(floorProgram, "mvpMatrix");
	
	float floorVerts[8 * GRIDLINES];
	for(int i = 0; i < GRIDLINES; i++)
	{
		floorVerts[8 * i + 0] = (i - (GRIDLINES - 1) / 2.0) * GRIDSIZE;
		floorVerts[8 * i + 1] = -(GRIDLINES - 1) / 2.0 * GRIDSIZE;
		
		floorVerts[8 * i + 2] = (i - (GRIDLINES - 1) / 2.0) * GRIDSIZE;
		floorVerts[8 * i + 3] = (GRIDLINES - 1) / 2.0 * GRIDSIZE;
		
		floorVerts[8 * i + 5] = (i - (GRIDLINES - 1) / 2.0) * GRIDSIZE;
		floorVerts[8 * i + 4] = -(GRIDLINES - 1) / 2.0 * GRIDSIZE;
		
		floorVerts[8 * i + 7] = (i - (GRIDLINES - 1) / 2.0) * GRIDSIZE;
		floorVerts[8 * i + 6] = (GRIDLINES - 1) / 2.0 * GRIDSIZE;
	}
	
    glBindVertexArray(vao[1]);
    

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVerts), floorVerts, GL_STATIC_DRAW);
    
	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
    

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
}

void display()
{
	glUseProgram(program);
	glm::vec4 light = glm::vec4{50.0, -1000.0, 50.0, 1.0};
	
	proj = glm::perspective(20.0f*CDR, 1.0f, 10.0f, 1000.0f);  //perspective projection matrix
	view = glm::lookAt(glm::vec3(0.0, 50.0, camz), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)); //view matrix
	view = glm::rotate(view, angle*CDR, glm::vec3(0.0, 1.0, 0.0));  //rotation matrix
	glm::vec4 lightEye = view * light;
	invMatrix = glm::inverse(view);
	mvpMatrix = proj * view;        //Model-view matrix
	
	///////////////////// DRAW GUMBO ////////////////////////////
	glUniformMatrix4fv(matrixLoc1, 1, GL_FALSE, &mvpMatrix[0][0]);
	glUniformMatrix4fv(matrixLoc2, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(matrixLoc3, 1, GL_TRUE,  &invMatrix[0][0]);
	glUniform4fv(vecLoc1, 1, &lightEye[0]);
	glUniform1f(zLoc, camz);
	glUniform1i(tLoc, t);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_PATCHES, 0, nvert);
	
	///////////////////// DRAW FLOOR ////////////////////////////
	glUseProgram(floorProgram);
	glUniformMatrix4fv(floorMatrixLoc1, 1, GL_FALSE, &mvpMatrix[0][0]);
	
	glBindVertexArray(vao[1]);
	glDrawArrays(GL_LINES, 0, 8 * GRIDLINES);
	
	
	glFlush();
}

void update(int value)
{
	angle++;
	camz -= camSpeed;
	if(exploding)
	{
		t++;
	}
	glutTimerFunc(50, update, 0);
	glutPostRedisplay();
}



void specialKeyPressed(int key, int x, int y)
{
	if(key == GLUT_KEY_UP)
	{
		camSpeed = 5;
	}
	else if(key == GLUT_KEY_DOWN)
	{
		camSpeed = -5;
	}
}

void specialKeyUp(int key, int x, int y)
{
	if(key == GLUT_KEY_UP || key == GLUT_KEY_DOWN)
	{
		camSpeed = 0;
	}
}

void keyPressed(unsigned char key, int x, int y)
{
	if(key == 'w')
	{
		wireFrame = (wireFrame == GL_LINE) ? GL_FILL : GL_LINE; 
	}
	else if(key == ' ')
	{
		exploding = !exploding;
		t = 0;
	}
	glPolygonMode(GL_FRONT_AND_BACK, wireFrame);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Assignment");
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
	glutSpecialFunc(specialKeyPressed);
	glutSpecialUpFunc(specialKeyUp);
	glutKeyboardFunc(keyPressed);
	glutTimerFunc(50, update, 0);
	glutDisplayFunc(display);
	glutMainLoop();
}

