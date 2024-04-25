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

//fix compiler errors
//void renderScene(mat4& matrixView, float time, float deltaTime);
//void onReshape(int w, int h);


// 3D models
C3dglModel camera;
C3dglModel table;
C3dglModel pomegranate;
C3dglModel vase;
C3dglModel lamp;
C3dglModel room;
C3dglModel ceilingLamp;
C3dglModel zomb;
C3dglModel logs;

//textures
C3dglBitmap wood;
C3dglBitmap fabric2;
C3dglBitmap brass;
GLuint idTexNormal;
GLuint idTexWood;
GLuint idTexNone;
GLuint idTexFabric2;
GLuint idTexBrass;
GLuint idTexShadowMap; // for shadow map
GLuint idFBO;

// The View Matrix
mat4 matrixView;

// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)

// shader init
C3dglProgram program;

//buffer vars
unsigned vertexBuffer = 0;
unsigned normalBuffer = 0;
unsigned indexBuffer = 0;

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// Initialise Shaders
	C3dglShader vertexShader;
	C3dglShader fragmentShader;
	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/basic.vert")) return false;
	if (!vertexShader.compile()) return false;
	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/basic.frag")) return false;
	if (!fragmentShader.compile()) return false;
	if (!program.create()) return false;
	if (!program.attach(vertexShader)) return false;
	if (!program.attach(fragmentShader)) return false;
	if (!program.link()) return false;
	if (!program.use(true)) return false;



	// glut additional setup
	glutSetVertexAttribCoord3(program.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(program.getAttribLocation("aNormal"));

	//pyramid 
	float vertices[] = {
		-4, 0, -4, 4, 0, -4, 0, 7, 0, -4, 0, 4, 4, 0, 4, 0, 7, 0,
		-4, 0, -4, -4, 0, 4, 0, 7, 0, 4, 0, -4, 4, 0, 4, 0, 7, 0,
		-4, 0, -4, -4, 0, 4, 4, 0, -4, 4, 0, 4 };
	float normals[] = {
		0, 4, -7, 0, 4, -7, 0, 4, -7, 0, 4, 7, 0, 4, 7, 0, 4, 7,
		-7, 4, 0, -7, 4, 0, -7, 4, 0, 7, 4, 0, 7, 4, 0, 7, 4, 0,
		0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0 };
	unsigned indices[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13, 14, 15 };

	// prepare vertex data
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// prepare normal data
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	// prepare indices array
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// load your 3D models here!
	if (!camera.load("models\\camera.3ds")) return false;
	if (!table.load("models\\table.obj")) return false;
	if (!pomegranate.load("models\\pomegranate.obj")) return false;
	if (!vase.load("models\\vase.obj")) return false;
	if (!lamp.load("models\\lamp.obj")) return false;
	if (!room.load("models\\Castle\\Castle OBJ.obj")) return false;
	//if (!room.load("models\\Castle\\Castle OBJ.obj")) return false;
	//room.loadMaterials("models\\Castle\\Castle OBJ");
	if (!ceilingLamp.load("models\\ceilinglamp.3ds")) return false;

	if (!zomb.load("models\\zomb.fbx")) return false;
	zomb.loadMaterials("models\\");
	zomb.loadAnimations();

	if (!logs.load("models\\Wood.obj")) return false;
	logs.loadMaterials("models\\");

	//load textures
	wood.load("models/oak.bmp", GL_RGBA);
	if (!wood.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wood.getWidth(), wood.getHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, wood.getBits());

	fabric2.load("models/fabric2.png", GL_RGBA);
	if (!fabric2.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexFabric2);
	glBindTexture(GL_TEXTURE_2D, idTexFabric2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fabric2.getWidth(), fabric2.getHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, fabric2.getBits());

	brass.load("models/brass.bmp", GL_RGBA);
	if (!brass.getBits()) return false;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexBrass);
	glBindTexture(GL_TEXTURE_2D, idTexBrass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, brass.getWidth(), brass.getHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, brass.getBits());

	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);

	//shadow map
	// Create shadow map texture
	glActiveTexture(GL_TEXTURE7);
	glGenTextures(1, &idTexShadowMap);
	glBindTexture(GL_TEXTURE_2D, idTexShadowMap);
	// Texture parameters - to get nice filtering & avoid artefact on the edges of the shadowmap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	// This will associate the texture with the depth component in the Z-buffer
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2], h = viewport[3];
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w * 2, h * 2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	// Send the texture info to the shaders
	program.sendUniform("shadowMap", 7);
	// revert to texture unit 0
	glActiveTexture(GL_TEXTURE0);

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(0.0, 20.0, 50.0),
		vec3(0.0, 20.0, 0.0),
		vec3(0.0, 1.0, 0.0));

	// frame buffer
	// Create a framebuffer object (FBO)
	glGenFramebuffers(1, &idFBO);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, idFBO);
	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	// attach the texture to FBO depth attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, idTexShadowMap, 0);
	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


	// setup the screen background colour
	glClearColor(0.3f, 0.5f, 0.8f, 1.0f);

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift to speed up your movement" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;


	return true;
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;
	float spin = 6.5f * time;

	std::vector<mat4> transform;
	zomb.getAnimData(0, time, transform);
	program.sendUniform("bones", &transform[0], transform.size());

	program.sendUniform("att_quadratic", 0.3f);

	//room
	// Setup NORMAL texturing
	program.sendUniform("useNormalMap", false);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idTexNormal);
	program.sendUniform("textureNormal", 1);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, idTexNone);

	program.sendUniform("texture0", 0);

	// setup materials - grey
	//program.sendUniform("material", vec3(0.6f, 0.6f, 0.6f));

	// camera
	m = matrixView;
	m = translate(m, vec3(2.0f, 0, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.04f, 0.04f, 0.04f));
	//camera.render(m);

	//ambient light
	program.sendUniform("materialAmbient", vec3(0.6f, 0.6f, 0.6f));
	program.sendUniform("lightAmbient.color", vec3(0.1, 0.1, 0.15));

	//diffuse light
	program.sendUniform("materialDiffuse", vec3(0.6, 0.6, 0.6));

	//directional diffuse light
	program.sendUniform("lightDir.direction", vec3(1.0, 0.5, 1.0));
	program.sendUniform("lightDir.diffuse", vec3(0.3, 0.3, 0.3)); //  white light

	//specular point light
	program.sendUniform("materialSpecular", vec3(0.6, 0.6, 1.0));
	program.sendUniform("shininess", (4.0f));

	// specular point light 1
	program.sendUniform("lightPoint0.position", vec3(28.0f, 17.5f, 4.0f));
	program.sendUniform("lightPoint0.diffuse", vec3(1.7, 1.7, 1.7));
	program.sendUniform("lightPoint0.specular", vec3(1.0, 1.0, 1.0));

	// specular point light 2
	program.sendUniform("lightPoint1.position", vec3(13.8f, 17.5f, -4.0f));
	program.sendUniform("lightPoint1.diffuse", vec3(1.7, 1.7, 1.7));
	program.sendUniform("lightPoint1.specular", vec3(1.0, 1.0, 1.0));

	//fog
	program.sendUniform("fogColour", vec3(0.1f, 0.19f, 0.28f));
	program.sendUniform("fogDensity", (0.0075f));

	//setup materials - blue
	//program.sendUniform("material", vec3(0.6f, 0.6f, 0.6f));

	// teapot
	//program.sendUniform("material", vec3(0.6f, 0.6f, 0.6f));
	m = matrixView;
	m = translate(m, vec3(22.0f, 13, 4.0f));
	m = rotate(m, radians(120.f), vec3(0.0f, 1.0f, 0.0f));
	program.sendUniform("matrixModelView", m);
	glutSolidTeapot(2.0);

	//pyramid
	//program.sendUniform("material", vec3(0.04f, 0.84f, 0.35f));
	m = matrixView;
	m = scale(m, vec3(0.3f, 0.3f, 0.3f));
	m = translate(m, vec3(90.0f, 45, 16.0f));
	m = rotate(m, radians(180.f), vec3(1.0f, 0.0f, 0.0f));
	m = rotate(m, radians(spin), vec3(0.0f, 1.0f, 0.0f));
	program.sendUniform("matrixModelView", m);
	// Get Attribute Locations
	GLuint attribVertex = program.getAttribLocation("aVertex");
	GLuint attribNormal = program.getAttribLocation("aNormal");
	// Enable vertex attribute arrays
	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribNormal);
	// Bind (activate) the vertex buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Bind (activate) the normal buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Draw triangles � using index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
	// Disable arrays
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribNormal);

	//table
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	m = matrixView;
	m = translate(m, vec3(25.0f, 0.0f, 0.0f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	table.render(1, m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	//chairs
	//program.sendUniform("material", vec3(0.74f, 0.34f, 0.65f));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexFabric2);
	m = matrixView;
	m = translate(m, vec3(25.0f, 0.0f, 0.0f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	table.render(0, m);

	m = matrixView;
	m = rotate(m, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	m = translate(m, vec3(0.0f, 0.0f, 30.0f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	table.render(0, m);

	m = matrixView;
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = translate(m, vec3(-25.0f, 0.0f, 0.0f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	table.render(0, m);

	m = matrixView;
	m = rotate(m, radians(270.f), vec3(0.0f, 1.0f, 0.0f));
	m = translate(m, vec3(2.0f, 0.0f, -18.0f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	table.render(0, m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	//pomegranate
	//program.sendUniform("material", vec3(1.00f, 0.1f, 0.1f));
	m = matrixView;
	//m = rotate(m, radians(90.f), vec3(1.0f, 0.0f, 1.0f));
	m = translate(m, vec3(26.4f, 14.0f, 5.4f));
	m = scale(m, vec3(1.2f, 1.2f, 1.2f));
	m = rotate(m, radians(-spin), vec3(0.0f, 1.0f, 0.0f));
	pomegranate.render(m);

	//vase
	m = matrixView;
	m = translate(m, vec3(28.0f, 11.2f, -2.0f));
	m = scale(m, vec3(0.35f, 0.35f, 0.35f));
	vase.render(m);

	//lamps
	//program.sendUniform("material", vec3(1.00f, 0.1f, 0.1f));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexBrass);

	m = matrixView;
	m = translate(m, vec3(31.5f, 11.5f, 4.0f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	lamp.render(m);

	m = matrixView;
	m = translate(m, vec3(17.5f, 11.5f, -4.0f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	lamp.render(m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	//bulb1
	m = matrixView;
	m = translate(m, vec3(28.0f, 17.5f, 4.0f));
	m = scale(m, vec3(0.35f, 0.35f, 0.35f));
	program.sendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	//bulb2
	m = matrixView;
	m = translate(m, vec3(13.8f, 17.5f, -4.0f));
	m = scale(m, vec3(0.35f, 0.35f, 0.35f));
	program.sendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	////ceiling lamp
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, idTexNone);

	//// Pendulum mechanics
	//static float alpha = 0; // angular position (swing)
	//static float omega = 0.7f; // angular velocity
	//deltaTime = glm::min(deltaTime, 0.2f); // remove time distortions (longer than 0.2s)
	//omega -= alpha * 0.05f * deltaTime; // Hooke's law: acceleration proportional to swing
	//alpha += omega * deltaTime * 50; // motion equation: swing += velocity * delta-time

	//m = matrixView;
	//m = scale(m, vec3(0.1f, 0.05f, 0.1f));
	//m = translate(m, vec3(250.0f, 610.0f, 0.0f));
	//m = rotate(m, radians(alpha), vec3(0.5, 0, 1));
	//ceilingLamp.render(m);

	//spotlight
	//program.sendUniform("lightSpot.matrix",m);
	//program.sendUniform("lightSpot.attenuation", float(1.2f));
	//program.sendUniform("lightSpot.position", vec3(24.f, 20.0f, 0.f));
	//program.sendUniform("lightSpot.diffuse", vec3(1.0, 1.0, 1.0));
	//program.sendUniform("lightSpot.direction", vec3(0, -1, 0));
	//program.sendUniform("lightSpot.cutoff", radians(25.0f));

	//char

	program.sendUniform("useNormalMap", false);
	m = matrixView;
	m = rotate(m, radians(40.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.06, 0.06, 0.06));
	m = translate(m, vec3(-50, 0, 50));
	zomb.render(m);

	
	//room
	m = matrixView;
	m = scale(m, vec3(10.f, 10.f, 10.f));
	m = translate(m, vec3(0, -0.28f, 0));
	room.render(m);

	//logs
	m = matrixView;
	m = rotate(m, radians(40.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(1.5f, 1.5f, 4.f));
	m = translate(m, vec3(-15, 1, 0));
	logs.render(m);

}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(_fov), ratio, 0.02f, 1000.f);

	program.sendUniform("matrixProjection", matrixProjection);
}

// Creates a shadow map and stores in idFBO
// lightTransform - lookAt transform corresponding to the light position predominant direction
void createShadowMap(mat4 lightTransform, float time, float deltaTime)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	// Store the current viewport in a safe place
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2], h = viewport[3];
	// setup the viewport to 2x2 the original and wide (120 degrees) FoV (Field of View)
	glViewport(0, 0, w * 2, h * 2);
	mat4 matrixProjection = perspective(radians(160.f), (float)w / (float)h, 0.5f, 50.0f);
	program.sendUniform("matrixProjection", matrixProjection);
	// prepare the camera
	mat4 matrixView = lightTransform;
	// send the View Matrix
	program.sendUniform("matrixView", matrixView);
	// Bind the Framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, idFBO);
	// OFF-SCREEN RENDERING FROM NOW!
	// Clear previous frame values - depth buffer only!
	glClear(GL_DEPTH_BUFFER_BIT);
	// Disable color rendering, we only want to write to the Z-Buffer (this is to speed-up)
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	// Prepare and send the Shadow Matrix - this is matrix transform every coordinate x,y,z
	//x = x * 0.5 + 0.5;
	//y = y * 0.5 + 0.5;
	//z = z * 0.5 + 0.5;
	// Moving from unit cube [-1,1] to [0,1] 
	const mat4 bias = {
	{ 0.5, 0.0, 0.0, 0.0 },
	{ 0.0, 0.5, 0.0, 0.0 },
	{ 0.0, 0.0, 0.5, 0.0 },
	{ 0.5, 0.5, 0.5, 1.0 }
	};
	program.sendUniform("matrixShadow", bias * matrixProjection * matrixView);
	// Render all objects in the scene
	renderScene(matrixView, time, deltaTime);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_CULL_FACE);
	onReshape(w, h);
}

void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime

	createShadowMap(lookAt(
		vec3(10, 10, 10), // coordinates of the source of the light
		//vec3(-2.55f, 4.24f, -1.0f), // coordinates of the source of the light
		vec3(.0f, 1.f, 1.0f), // coordinates of a point within or behind the scene
		vec3(0.0f, 1.0f, 0.0f)), // a reasonable "Up" vector
		time, deltaTime);

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

	// setup View Matrix
	program.sendUniform("matrixView", matrixView);

	// render the scene objects
	renderScene(matrixView, time, deltaTime);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
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
	glutCreateWindow("3DGL Scene: Level 3 (Hero Demo) - Mac (K2120853)");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

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

	C3dglLogger::log("Vendor: {}", (const char *)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char *)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	// init light and everything � not a GLUT or callback function!
	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}


	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

