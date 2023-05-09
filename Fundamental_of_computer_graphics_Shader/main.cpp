//#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <glew.h>
#include <freeglut.h>
#include <string>
#include <fstream>
#include <sstream>

#include "algebra.h"
#include "shaders.h"
#include "mesh.h"

#define LINE 1
#define PROJECTION 2
#define FILL 2
#define CAMERA 1
#define MODEL 2

int LAB;
int state = CAMERA;
int screen_width = 1024;
int screen_height = 768;

Mesh *meshList = NULL; // Global pointer to linked list of triangle meshes
Mesh* model = NULL;

Camera cam = {{0.0f,0.0f,20.0f}, {0.0f,0.0f,0.0f}, 120, 1, 10000}; // Setup the global camera parameters

GLuint shprg; // Shader program id

// Global transform matrices
// V is the view transform
// P is the projection transform
// PV = P * V is the combined view-projection transform

Matrix a, V, P, PV, W, M;

void CleanBuffer()
{
	while ((getchar()) != '\n');
}
void shaderError(GLuint shader)
{
	int maxLength = 2048;
	int actualLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
	char infoLog[2048];
	glGetShaderInfoLog(shader, maxLength, &actualLength, infoLog);
	printf("Shader info log for GL index %i:\n%s\n", shader, infoLog);
	glDeleteShader(shader);
}
void linkerError(GLuint linker)
{
	int maxLength = 2048;
	int actualLength = 0;
	glGetProgramiv(linker, GL_INFO_LOG_LENGTH, &maxLength);
	char infoLog[2048];
	glGetProgramInfoLog(linker, maxLength, &actualLength, infoLog);
	printf("Shader info log for GL index %i:\n%s\n", linker, infoLog);
	glDeleteShader(linker);
}
void prepareShaderProgram(const char * vs_src, const char * fs_src) {
	GLint success = GL_FALSE;
	glEnable(GL_DEPTH_TEST);//used to enable Z-buffer
	shprg = glCreateProgram();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_src, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);	
	//if (!success) printf("Error in vertex shader!\n");
	if (success == GL_FALSE)
		shaderError(vs);
	else printf("Vertex shader compiled successfully!\n");

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_src, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);	
	//if (!success) printf("Error in fragment shader!\n");
	if (success == GL_FALSE)
		shaderError(fs);
	else printf("Fragment shader compiled successfully!\n");
	
	glAttachShader(shprg, vs);
	glAttachShader(shprg, fs);
	glLinkProgram(shprg);
	GLint isLinked = GL_FALSE;
	glGetProgramiv(shprg, GL_LINK_STATUS, &isLinked);
	//if (!isLinked) printf("Link error in shader program!\n");
	if (isLinked == GL_FALSE)
		linkerError(shprg);
	else printf("Shader program linked successfully!\n");

	
}
void prepareMesh(Mesh *mesh) {
	int sizeVerts = mesh->nv * 3 * sizeof(float);
	int sizeCols  = mesh->nv * 3 * sizeof(float);
	int sizeTris = mesh->nt * 3 * sizeof(int);
	
	// For storage of state and other buffer objects needed for vertex specification
	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	// Allocate VBO and load mesh data (vertices and normals)
	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeVerts + sizeCols, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVerts, (void *)mesh->vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeVerts, sizeCols, (void *)mesh->vnorms);

	// Allocate index buffer and load mesh indices
	glGenBuffers(1, &mesh->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeTris, (void *)mesh->triangles, GL_STATIC_DRAW);

	// Define the format of the vertex data
	GLint vPos = glGetAttribLocation(shprg, "vPos");
	glEnableVertexAttribArray(vPos);
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Define the format of the vertex data 
	GLint vNorm = glGetAttribLocation(shprg, "vNorm");
	glEnableVertexAttribArray(vNorm);
	glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, (void *)(mesh->nv * 3 *sizeof(float)));

	glBindVertexArray(0);

}
void renderMesh(Mesh *mesh) {
	
	// Assignment 1: Apply the transforms from local mesh coordinates to world coordinates here
	// Combine it with the viewing transform that is passed to the shader below
	if (LAB != LINE || LAB != (LINE+PROJECTION))//Used only in Lab 1.4 and 1.5 to interact and transform chosen model
	{
		W = MatMatMul(MatMatMul(MatMatMul(MatMatMul(Translate(mesh->position), Rotate_x(mesh->orientation.x)), Rotate_y(mesh->orientation.y)), Rotate_z(mesh->orientation.z)), Scale(mesh->size));
		M = MatMatMul(V, W);
		PV = MatMatMul(P, M);
	}
	
	// Pass the viewing transform to the shader
	GLint loc_PV = glGetUniformLocation(shprg, "PV");
	glUniformMatrix4fv(loc_PV, 1, GL_FALSE, PV.e);


	// Select current resources 
	glBindVertexArray(mesh->vao);
	
	// To accomplish wireframe rendering (can be removed to get filled triangles)
	if(LAB == LINE || LAB == LINE+PROJECTION)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else if(LAB == FILL)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Draw all triangles
	glDrawElements(GL_TRIANGLES, mesh->nt * 3, GL_UNSIGNED_INT, NULL); 
	glBindVertexArray(0);
}
void display(void) {
	Mesh *mesh;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	V = MatMatMul(MatMatMul(MatMatMul(Rotate_z(-cam.rotation.z), Rotate_y(-cam.rotation.y)), Rotate_x(-cam.rotation.x)), Translate(ScalarVecMul(-1.0f, cam.position)));
	// This finds the combined view-projection matrix
	//printf("LAB is: %i", LAB);
	if (LAB == LINE)//Used only in Lab 1.2
	{
		/*
		P.e[0] = 1.299038f; P.e[4] = 0.000000f; P.e[8] = 0.000000f; P.e[12] = 0.0f;
		P.e[1] = 0.000000f; P.e[5] = 1.732051f; P.e[9] = 0.000000f; P.e[13] = 0.0f;
		P.e[2] = 0.000000f; P.e[6] = 0.000000f; P.e[10] = -1.000200f; P.e[14] = -2.000200f;
		P.e[3] = 0.000000f; P.e[7] = 0.000000f; P.e[11] = -1.000000f; P.e[15] = 0.0f;
		*/
		PV = MatMatMul(P, V);
	}
	else if (LAB == LINE + PROJECTION)//Used only in Lab 1.3
		PV = MatMatMul(P, V);
		
	// Select the shader program to be used during rendering 
	glUseProgram(shprg);
	// Render all meshes in the scene
	mesh = meshList;
		
	while (mesh != NULL) {
		renderMesh(mesh);
		mesh = mesh->next;
	}

	glFlush();
}
void changeSize(int w, int h) {
	screen_width = w;
	screen_height = h;
	glViewport(0, 0, screen_width, screen_height);

}
void keypress(unsigned char key, int x, int y) {
	switch(key) {
	case 'c': //enable camera mode that will allow to change the position of the camera relative to the scene
		system("cls");
		printf("\nCamera mode\n");
		printf("\nTo change Model mode press m");
		state = CAMERA;
		break;
	case 'm'://enable seperate model mode that will allow to change the position or size of the model
		model = meshList;
		int id;
		system("cls");
		printf("\nYou have chosen to transform one model.\nWhich model do you want to transform?\n");
		printf("Model ID is: "); scanf_s("%i", &id);
		CleanBuffer();
		while (model != NULL) {
			if (model->ID == id)
			{
				printf("%s model mode", (id == 1) ? "First": (id == 2)? "Second": (id == 3) ? "Third": "Unknown");
				printf("\nTo change Camera mode press c");
				state = MODEL;
				break;
			}
			model = model->next;
		}
		if (model == NULL)
			printf("Model do not exist!");
		break;
	case 'b': //moves camera to begining or start point
		cam.position.x = 0;		cam.rotation.x = 0;
		cam.position.y = 0;		cam.rotation.y = 0;
		cam.position.z = 20;	cam.rotation.z = 0;
		break;
	case 'f'://Perspective projection
		P = FovProjection(cam.fov, 1024.0/768.0, cam.nearPlane, cam.farPlane);
		break;
	case 'o'://Parallel projection
		P = OrthogonalProjection(-20.0, 20.0, -10.0, 10.0, 1.0, 10000.0);
		break;
	case 'p'://perspective projection
		P = FrustumProjection(-20.0f, 20.0f, -10.0f, 10.0f, 1.0f, 10000.0f);
		break;
	case 'x':
		if (state == CAMERA)
			cam.position.x -= 0.7f;
		else
			model->position.x -= 0.7f;
		break;
	case 'X':
		if (state == CAMERA)
			cam.position.x += 0.7f;
		else
			model->position.x += 0.7f;
		break;
	case 'y':
		if(state == CAMERA)
			cam.position.y -= 0.7f;
		else
			model->position.y -= 0.7f;
		break;
	case 'Y':
		if (state == CAMERA)
			cam.position.y += 0.7f;
		else
			model->position.y += 0.7f;
		break;
	case 'z':
		if (state == CAMERA)
			cam.position.z -= 0.7f;
		else
			model->position.z -= 0.7f;
		break;
	case 'Z':
		if (state == CAMERA)
			cam.position.z += 0.7f;
		else
			model->position.z += 0.7f;
		break;
	case 'i':
		if (state == CAMERA)
			cam.rotation.x -= 0.7f;
		else
			model->orientation.x -= 0.7f;
		break;
	case 'I':
		if (state == CAMERA)
			cam.rotation.x += 0.7f;
		else
			model->orientation.x += 0.7f;
		break;
	case 'j':
		if (state == CAMERA)
			cam.rotation.y -= 0.7f;
		else
			model->orientation.y -= 0.7f;
		break;
	case 'J':
		if (state == CAMERA)
			cam.rotation.y += 0.7f;
		else
			model->orientation.y += 0.7f;
		break;
	case 'k':
		if (state == CAMERA)
			cam.rotation.z -= 0.7f;
		else
			model->orientation.z -= 0.7f;
		break;
	case 'K':
		if (state == CAMERA)
			cam.rotation.z += 0.7f;
		else
			model->orientation.z += 0.7f;
		break;
	case 'w':
		if (state == MODEL)
			model->size.x -= 0.7f;
		break;
	case 'W':
		if (state == MODEL)
			model->size.x += 0.7f;
		break;
	case 'h':
		if (state == MODEL)
			model->size.y -= 0.7f;
		break;
	case 'H':
		if (state == MODEL)
			model->size.y += 0.7f;
		break;
	case 'l':
		if (state == MODEL)
			model->size.z -= 0.7f;
		break;
	case'L':
		if (state == MODEL)
			model->size.z += 0.7f;
		break;
	case 'Q':
	case 'q':		
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}
std::string loadFileContent(const char* fileName)
{
	std::ifstream file(fileName);
	if (!file)
	{
		printf("Cant find file");
		return std::string();
	}
		
	std::stringstream sstr;
	sstr << file.rdbuf();
	file.close();

	return sstr.str();
}

void init(void) {
	
	//used to read in vertex shader and fragment shader for lab 1

	std::string vertex_source = loadFileContent("vertexShader.txt");
	std::string fragment_source = loadFileContent("fragmentShader.txt");

	//Used in case of error
	//printf("\nVertex Shader\n%s\n\n", vertex_source);
	//printf("\nFragment Shader\n%s\n\n", fragment_source);
	
	// Compile and link the given shader program (vertex shader and fragment shader)

	prepareShaderProgram(vertex_source.c_str(), fragment_source.c_str()); 

	// Setup OpenGL buffers for rendering of the meshes
	Mesh * mesh = meshList;
	while (mesh != NULL) {
		prepareMesh(mesh);
		mesh = mesh->next;
	}	
}
void cleanUp(void) {	
	printf("Running cleanUp function... ");
	// Free openGL resources
	// ...

	// Free meshes
	// ...
	printf("Done!\n\n");
}
void printMenu(int state) {
	
	if (state == 1)
	{
		printf("\n-------------------------------------------------------\n");
		printf("\n1:  Cow");
		printf("\n2:  Triceratops");
		printf("\n3:  Bunny");
		printf("\n4:  Cube");
		printf("\n5:  Frog");
		printf("\n6:  Knot");
		printf("\n7:  Sphere");
		printf("\n8:  Teapot");
		printf("\n9:  Lab 1.2 Viewing Transformation");
		printf("\n10: Lab 1.2 Viewing Transformation + Rotation");
		printf("\n11: Lab 1.3 Projection");
		printf("\n12: Lab 1.4 Generating Vertex Normals");
		printf("\n13: Lab 1.5 Scene Composition");
		printf("\n14: Continue to created scene");
		printf("\n0:  Quit program");
		printf("\n\n-------------------------------------------------------\n");
	}
	else if (state == 2)
	{
		printf("\n-------------------------------------------------------\n");
		printf("\n1: Lab 2.1 Gouroud shading");
		printf("\n2: Lab 2.2 Phong shading");
		printf("\n3: Lab 2.3 Geometry fur shader");
		//printf("\n4: ");
		printf("\n0: Quit program");
		printf("\n\n-------------------------------------------------------\n");
	}

}
//Function which is used to set model transformation vectors to default values
void defaultTransformation(Vector *translation, Vector *rotation, Vector *scaling)
{
	translation->x = 0.0f;
	translation->y = 0.0f;
	translation->z = 0.0f;
	rotation->x = 0.0f;
	rotation->y = 0.0f;
	rotation->z = 0.0f;
	scaling->x = 1.0f;
	scaling->y = 1.0f;
	scaling->z = 1.0f;
}
//Is used in case of manual model addition. Allow user to choose transformation values for models. 
void userInputTransformation(Vector *translation, Vector *rotation, Vector *scaling)
{
	printf("\nGive following input values: \n");
	printf("Position x:"); scanf_s("%f", &(translation->x));
	printf("Position y: "); scanf_s("%f", &(translation->y));
	printf("Position z: "); scanf_s("%f", &(translation->z));
	printf("\nRotation x: "); scanf_s("%f", &(rotation->x));
	printf("Rotation y: "); scanf_s("%f", &(rotation->y));
	printf("Rotation z: "); scanf_s("%f", &(rotation->z));
	printf("\nScaling x: "); scanf_s("%f", &(scaling->x));
	printf("Scaling y: "); scanf_s("%f", &(scaling->y));
	printf("Scaling z: "); scanf_s("%f", &(scaling->z));
	system("cls");
}
// Include data for some triangle meshes (hard coded in struct variables)
#include "./models/mesh_bunny.h"
#include "./models/mesh_cow.h"
#include "./models/mesh_cube.h"
#include "./models/mesh_frog.h"
#include "./models/mesh_knot.h"
#include "./models/mesh_sphere.h"
#include "./models/mesh_teapot.h"
#include "./models/mesh_triceratops.h"





int main(int argc, char **argv) {
	
	// Setup freeGLUT	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(screen_width, screen_height);
	glutCreateWindow("DVA338 Programming Assignments");
	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keypress);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// Specify your preferred OpenGL version and profile
	glutInitContextVersion(4, 5);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);	
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// Uses GLEW as OpenGL Loading Library to get access to modern core features as well as extensions
	GLenum err = glewInit(); 
	if (GLEW_OK != err) { fprintf(stdout, "Error: %s\n", glewGetErrorString(err)); return 1; }

	// Output OpenGL version info
	fprintf(stdout, "GLEW version: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, "OpenGL version: %s\n", (const char *)glGetString(GL_VERSION));
	fprintf(stdout, "OpenGL vendor: %s\n\n", glGetString(GL_VENDOR));
	
	//Vectors some is send in to and saved in every model mesh. Those are used later to transform model.
	Vector translation;
	Vector rotation;
	Vector scaling;

	LAB = FILL;
	int choice;
	//int LabState = 1;
	int LabState = 2;
	int ID = 1;
	//Create console menu where user is able to choose thether whether to add manualy models in scene or create Lab scenes
	if (LabState == 1)
	{
		printf("\nPress the appropriate button to \nselect an object, Lab setup or quit");
		do {
			printMenu(LabState);
			defaultTransformation(&translation, &rotation, &scaling);
			scanf_s("%i", &choice);
			system("cls");
			switch (choice) {
			case 1:
				printf("You chose model Cow!\n\n");
				userInputTransformation(&translation, &rotation, &scaling);
				insertModel(&meshList, cow.nov, cow.verts, cow.nof, cow.faces, 20.0, translation, rotation, scaling, ID, LAB);
				ID++;
				break;
			case 2:
				printf("You chose model Triceratops!\n\n");
				userInputTransformation(&translation, &rotation, &scaling);
				insertModel(&meshList, triceratops.nov, triceratops.verts, triceratops.nof, triceratops.faces, 3.0, translation, rotation, scaling, ID, LAB);
				ID++;
				break;
			case 3:
				printf("You chose model Bunny!\n\n");
				userInputTransformation(&translation, &rotation, &scaling);
				insertModel(&meshList, bunny.nov, bunny.verts, bunny.nof, bunny.faces, 60.0, translation, rotation, scaling, ID, LAB);
				ID++;
				break;
			case 4:
				printf("You chose model Cube!\n\n");
				userInputTransformation(&translation, &rotation, &scaling);
				insertModel(&meshList, cube.nov, cube.verts, cube.nof, cube.faces, 5.0, translation, rotation, scaling, ID, LAB);
				ID++;
				break;
			case 5:
				printf("You chose model Frog!\n\n");
				userInputTransformation(&translation, &rotation, &scaling);
				insertModel(&meshList, frog.nov, frog.verts, frog.nof, frog.faces, 2.5, translation, rotation, scaling, ID, LAB);
				ID++;
				break;
			case 6:
				printf("You chose model Knot!\n\n");
				userInputTransformation(&translation, &rotation, &scaling);
				insertModel(&meshList, knot.nov, knot.verts, knot.nof, knot.faces, 1.0, translation, rotation, scaling, ID, LAB);
				ID++;
				break;
			case 7:
				printf("You chose model Sphere!\n\n");
				userInputTransformation(&translation, &rotation, &scaling);
				insertModel(&meshList, sphere.nov, sphere.verts, sphere.nof, sphere.faces, 12.0, translation, rotation, scaling, ID, LAB);
				ID++;
				break;
			case 8:
				printf("You chose model Teapot!\n\n");
				userInputTransformation(&translation, &rotation, &scaling);
				insertModel(&meshList, teapot.nov, teapot.verts, teapot.nof, teapot.faces, 3.0, translation, rotation, scaling, ID, LAB);
				ID++;
				break;
			case 9:
				cam.fov = 60;
				LAB = LINE;
				printf("\nYou chose setup for Lab 1.2 Viewing Transformation\n\n");
				printf("\nCamera mode\n");
				printf("To change Model mode press m\n\n");
				insertModel(&meshList, cow.nov, cow.verts, cow.nof, cow.faces, 20.0, translation, rotation, scaling, 1, LAB);
				insertModel(&meshList, triceratops.nov, triceratops.verts, triceratops.nof, triceratops.faces, 3.0, translation, rotation, scaling, 1, LAB);
				choice = 14;
				break;
			case 10:
				cam = { {-5.0f,-5.0f,20.0f}, {-10.0f,-30.0f,-45.0f}, 60, 1, 10000 };
				LAB = LINE;
				printf("\nYou chose setup for Lab 1.2 Viewing Transformation + Rotation\n\n");
				printf("\nCamera mode\n");
				printf("To change Model mode press m\n\n");
				insertModel(&meshList, cow.nov, cow.verts, cow.nof, cow.faces, 20.0, translation, rotation, scaling, 1, LAB);
				insertModel(&meshList, triceratops.nov, triceratops.verts, triceratops.nof, triceratops.faces, 3.0, translation, rotation, scaling, 1, LAB);
				choice = 14;
				break;
			case 11:
				cam.fov = 60;
				LAB = LINE + PROJECTION;
				printf("\nYou chose setup for Lab 1.3 Projection\n\n");
				printf("\nCamera mode\n");
				printf("To change Model mode press m\n\n");
				insertModel(&meshList, knot.nov, knot.verts, knot.nof, knot.faces, 1.0, translation, rotation, scaling, 1, LAB);
				choice = 14;
				break;
			case 12:
				cam.fov = 60;
				printf("\nYou chose setup for Lab 1.4 Generating Vertex Normals\n\n");
				printf("\nCamera mode\n");
				printf("To change Model mode press m\n\n");
				insertModel(&meshList, knot.nov, knot.verts, knot.nof, knot.faces, 1.0, translation, rotation, scaling, 1, LAB);
				choice = 14;
				break;
			case 13:
				cam.fov = 120;
				printf("\nYou chose setup for Lab 1.5 Scene Composition!\n\n");
				printf("\nCamera mode\n");
				printf("To change Model mode press m\n\n");
				translation.x = 10.0f;
				rotation.y = 90.0f;
				insertModel(&meshList, cow.nov, cow.verts, cow.nof, cow.faces, 20.0, translation, rotation, scaling, ID, LAB);
				ID++;
				defaultTransformation(&translation, &rotation, &scaling);
				translation.y = 30.0f;
				translation.z = -40.0f;
				scaling.x = 3.0f; scaling.y = 1.0f; scaling.z = 1.0f;
				insertModel(&meshList, cow.nov, cow.verts, cow.nof, cow.faces, 20.0, translation, rotation, scaling, ID, LAB);
				ID++;
				defaultTransformation(&translation, &rotation, &scaling);
				translation.x = -25.0f;
				rotation.x = 60.0f;
				insertModel(&meshList, triceratops.nov, triceratops.verts, triceratops.nof, triceratops.faces, 3.0, translation, rotation, scaling, ID, LAB);
				ID++;
				choice = 14;
				break;
			case 14:
				choice = 14;
				break;
			case 0:
				return 0;
				break;
			default:
				printf("Wrong input!\nPlease select a number from the menu!");
				break;
			}
		} while (choice != 14);
	}
	else if (LabState == 2)
	{
			printf("\nPress the appropriate button to \nselect an object, Lab setup or quit");
			printMenu(LabState);
			defaultTransformation(&translation, &rotation, &scaling);
			scanf_s("%i", &choice);
			system("cls");
			switch (choice) {
			case 1:
				printf("You chose Lab 2.1 Gouraud shading!\n\n");
				cam = { {20.0f,20.0f,30.0f}, {-45.0f,30.0f,0.0f}, 60, 1, 10000 };
				rotation.x = -90.0f;
				insertModel(&meshList, teapot.nov, teapot.verts, teapot.nof, teapot.faces, 3.0, translation, rotation, scaling, ID, LAB);
				ID++;
				defaultTransformation(&translation, &rotation, &scaling);
				scaling.x = 4.0f; 
				scaling.y = 0.2f; 
				scaling.z = 4.0f;
				insertModel(&meshList, cube.nov, cube.verts, cube.nof, cube.faces, 5.0, translation, rotation, scaling, ID, LAB);
				ID++;
				/*
				defaultTransformation(&translation, &rotation, &scaling);
				translation.x = cam.position.x; scaling.x = 0.1f;
				translation.y = cam.position.y; scaling.y = 0.1f;
				translation.z = cam.position.z; scaling.z = 0.1f;
				insertModel(&meshList, sphere.nov, sphere.verts, sphere.nof, sphere.faces, 12.0, translation, rotation, scaling, ID, LAB);
				ID++;
				*/
				/*
				defaultTransformation(&translation, &rotation, &scaling);
				translation.x = 10.0f; scaling.x = 0.1f;
				translation.y = 10.0f; scaling.y = 0.1f;
				translation.z = 10.0f; scaling.z = 0.1f;
				insertModel(&meshList, sphere.nov, sphere.verts, sphere.nof, sphere.faces, 12.0, translation, rotation, scaling, ID, LAB);
				*/
				break;
			case 2:
				printf("You chose Lab 2.2 Phong shading!\n\n");
				cam = { {20.0f,20.0f,30.0f}, {-45.0f,30.0f,0.0f}, 60, 1, 10000 };
				rotation.x = -90.0f;
				insertModel(&meshList, teapot.nov, teapot.verts, teapot.nof, teapot.faces, 3.0, translation, rotation, scaling, ID, LAB);
				ID++;
				defaultTransformation(&translation, &rotation, &scaling);
				scaling.x = 4.0f;
				scaling.y = 0.2f;
				scaling.z = 4.0f;
				insertModel(&meshList, cube.nov, cube.verts, cube.nof, cube.faces, 5.0, translation, rotation, scaling, ID, LAB);
				ID++;
				break;
			case 3:
				printf("You chose Lab 2.3 Geometry fur shader!\n\n");
				insertModel(&meshList, sphere.nov, sphere.verts, sphere.nof, sphere.faces, 12.0, translation, rotation, scaling, ID, LAB);
				ID++;
				break;
			case 0:
				return 0;
				break;
			default:
				printf("Wrong input!\nPlease select a number from the menu!");
				break;
			}
	}

	
	//if(LAB != LINE)
		keypress('f',0,0);
	
	init();
	glutMainLoop();

	cleanUp();	
	return 0;
}
