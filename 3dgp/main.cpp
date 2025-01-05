#include <iostream>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D Terrains
C3dglTerrain terrain, road;

// 3D Models
C3dglModel cristmasTree;
C3dglModel skyDom;


//textures
GLuint idTexSnow;
GLuint idTexRoad;
GLuint idTexSkyBox;
GLuint idTexNone; // null TExture


// The View Matrix
mat4 matrixView;

// Camera & navigation
float maxspeed = 28.f;	// camera max speed
float accel = 8.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)

// GLSL Program
C3dglProgram program;

//For Rotation  
float angle = 0;
float dayNightTimer = 0;
bool rotatinEnabled = true;


//***************** TEXTURES *****************
bool TextureSetup(const char textureName[], GLuint &textureId, GLuint textureNr)
{
	C3dglBitmap bm;
	bm.load(textureName, GL_RGBA);
	if (!bm.getBits()) return false;

	glActiveTexture(GL_TEXTURE + textureNr);
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.getBits());

	return true;
}

//NULL TEXture
void NullTexture()
{
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);
}

void SetTexture(const char textureName[], GLuint textureId, GLuint textureNr)
{
	program.sendUniform(textureName, textureNr);
	glBindTexture(GL_TEXTURE_2D, textureId);
	//If you use more than one texture unit(multitexturing), this should be preceded by a glActiveTexture call1 :
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, idTexWood);
}

bool init()
{
	//SHADERS CREATION
	C3dglShader vertexShader;
	C3dglShader fragmentShader;

	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/basic.vert.shader")) return false;
	if (!vertexShader.compile()) return false;
	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/basic.frag.shader")) return false;
	if (!fragmentShader.compile()) return false;

	if (!program.create()) return false;
	if (!program.attach(vertexShader)) return false;
	if (!program.attach(fragmentShader)) return false;
	if (!program.link()) return false;
	if (!program.use(true)) return false;

	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// load your 3D models here!
	if (!terrain.load("models\\terrain\\worldHM.png", 25)) return false;
	if (!road.load("models\\terrain\\roadHM.png", 25)) return false;

	//EASY TEXTURE LOAD
	if (!cristmasTree.load("models\\christmas_tree\\christmas_tree.obj")) return false;
	cristmasTree.loadMaterials("models\\christmas_tree\\");

	if (!skyDom.load("models\\skySphere\\sphere.obj")) return false;
	//skyDom.loadMaterials("models\\skySphere\\");

	//textures Setup
	if (!TextureSetup("models/PaintTextures/snow2.jpg", idTexSnow, 0))
		return false;
	if (!TextureSetup("models/PaintTextures/road2.jpg", idTexRoad, 1))
		return false;
	if (!TextureSetup("models/skySphere/skyDom.jpg", idTexSkyBox, 2))
		return false;
 

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(4.0, 1.5, 5.0),
		vec3(4.0, 1.5, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	//glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky background



	return true;
}


//SETUP FRO MATRIX EG OBJECTS IN THE WORLD
mat4 setMatrix(vec3 Translate, float RotAngle, vec3 Rotate, vec3 Scale, vec3 objColour)
{
	mat4 m = matrixView;
	m = translate(m, Translate);
	m = rotate(m, radians(RotAngle), Rotate);
	m = scale(m, Scale);

	//LIGHT - TO MOVE
	//Ambient Light & Material to calculate correct colour
	program.sendUniform("lightAmbient.color", vec3(0.2, 0.2, 0.2));
	program.sendUniform("materialAmbient", vec3(objColour));
	
	//diffuse?
	program.sendUniform("lightDir.diffuse", vec3(0.9, 0.9, 0.9)); // set "lightDir.diffuse", vec3(0.0, 0.0, 0.0) to switch off
	program.sendUniform("materialDiffuse", vec3(objColour));

	//direction
	program.sendUniform("lightDir.direction", vec3(-1.0, 0.5, -1.0));

	return m; 
}

//LIGHT SETUP
void LightManager()
{

	//gpt + me
	float oneDayTimeInSec = 24.0f; // Total seconds representing one full day cycle
	float elapsedTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Time in seconds since the program started
	float dayFraction = fmod(elapsedTime, oneDayTimeInSec) / oneDayTimeInSec; // Fraction of the day

 
	//point light
	if (!rotatinEnabled   )
	{
		program.sendUniform("lightPoint.position", vec3(0.5f, 2.5f, 1.4f));
		program.sendUniform("lightPoint.diffuse", vec3(dayFraction * 5 , dayFraction * 5, dayFraction * 5));
		program.sendUniform("lightPoint.specular", vec3(1.0, 1.0, 1.0));
	}
	else
	{
		program.sendUniform("lightPoint.diffuse", vec3(0, 0, 0));
		program.sendUniform("lightPoint.specular", vec3(0, 0, 0));
	}

	//shine
	program.sendUniform("shininess", 10.0);
	program.sendUniform("materialSpecular", vec3(0.6, 0.6, 1.0));

	
}



//*********** RENDER SCENE ***********
void renderScene(mat4& matrixView, float time, float deltaTime)
{
 
	LightManager();

	angle += 0.003f;
	// render the terrain
	SetTexture("texture0", idTexSnow, 0);
	terrain.render(setMatrix({ 0.f, 0.f, 0.f },  0.f, { 0.0f, 1.0f, 0.0f }, {  1.f,  1.f ,  1.f }, { 1.f, 1.f, 1.f }));

	// render the road
	SetTexture("texture1", idTexRoad, 1);
	road.render(setMatrix({ 0.f, 0.15f, 0.f }, 0.f, { 0.0f, 1.0f, 0.0f }, { 1.f,  1.f ,  1.f }, { 1.f, 1.f, 1.f }));

	//cristmasTree
	cristmasTree.render(setMatrix({ 0.f, 8.5f, 0.f }, 0.f, { 1.0f, 1.0f, 0.0f }, { 7.f,  7.f ,  7.f }, { 0.f, 1.f, 0.f }));
	SetTexture("texture2", idTexSkyBox, 2);
	//skydom
	skyDom.render(setMatrix({ 0.f, 15.f, 0.f }, angle, { 0.5f, 1.0f, 0.0f }, { 20.f,  20.f ,  20.f }, { 1.f, 1.f, 1.f }));
}

//*********** RENDER GLUT ***********
void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1),
		pitch, vec3(1, 0, 0)),	// switch the pitch off
		_vel * deltaTime),		// animate camera motion (controlled by WASD keys)
		-pitch, vec3(1, 0, 0))	// switch the pitch on
		* matrixView;

	// move the camera up following the profile of terrain (Y coordinate of the terrain)
	float terrainY = -terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]);
	matrixView = translate(matrixView, vec3(0, terrainY, 0));

	renderScene(matrixView, time, deltaTime); // render the scene objects

	// the camera must be moved down by terrainY to avoid unwanted effects
	matrixView = translate(matrixView, vec3(0, -terrainY, 0));

	glutSwapBuffers(); 	// essential for double-buffering technique
	glutPostRedisplay();// proceed the animation
}


//*********** RESHAPE***********
// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(_fov), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	program.sendUniform("matrixProjection", matrixProjection);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; break;
	case 's': _acc.z = -accel; break;
	case 'a': _acc.x = accel; break;
	case 'd': _acc.x = -accel; break;
	case 'e': _acc.y = accel; break;
	case 'q': _acc.y = -accel; break;
	case 't': rotatinEnabled = !rotatinEnabled; break;
	}
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': _acc.z = _vel.z = 0; break;
	case 'a':
	case 'd': _acc.x = _vel.x = 0; break;
	case 'q':
	case 'e': _acc.y = _vel.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

// handle mouse move
void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	// find delta (change to) pan & pitch
	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;	// avoid warping side-effects

	// View = Pitch * DeltaPitch * DeltaYaw * Pitch^-1 * View;
	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)), 
		-pitch, vec3(1.f, 0.f, 0.f)) 
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene: First Terrain");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
		return 0;

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	if (!init())
		return 0;

	glutMainLoop();	// enter GLUT event processing cycle

	return 1;
}