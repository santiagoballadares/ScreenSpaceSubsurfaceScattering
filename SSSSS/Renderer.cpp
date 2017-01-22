/*
 * Santiago Balladares
 * Dissertation Project
 * Screen Space Subsurface Scattering
 * Newcastle University
 * Computer Game Programming MSc
 * 2011 - 2012
 */
#include "Renderer.h"

#define FRONT	true
#define BACK	false

Renderer::Renderer(Window &parent) : OGLRenderer( parent )
{
#pragma region camera
	camera = new Camera(4.660f, 37.680f, Vector3(0.364f, -0.030f, 0.482f));
#pragma endregion

#pragma region meshes
	// Quad
	quad = Mesh::GenerateQuad();

	// head
	OBJMesh *mHead = new OBJMesh( "../Meshes/head.obj" );
	mHead->SetTexture( SOIL_load_OGL_texture("../Textures/head_col.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS) );
	mHead->SetBumpMap( SOIL_load_OGL_texture("../Textures/head_normal.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS) );
	headMesh = mHead;

	// knight
	OBJMesh *mPiece = new OBJMesh("../Meshes/knight.obj");
	mPiece->SetTexture( SOIL_load_OGL_texture("../Textures/marble.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS) );
	mPiece->SetBumpMap( SOIL_load_OGL_texture("../Textures/basicBumpmap.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS) );
	knightMesh = mPiece;		
#pragma endregion


#pragma region light
	// light position
	lightPos = Vector3(2.0099986f, 0.0f, 2.6999984f);

	// light target
	lightTarget = Vector3(0.0f, 0.0f, 0.0f);

	// new light
	light = new Light( lightPos, Vector4(1.0f, 1.0f, 1.0f, 1.0f), 100.0f );

	// light mesh
	OBJMesh *mSphere = new OBJMesh();
	mSphere->LoadOBJMesh( "../Meshes/ico.obj" );
	lightMesh = mSphere;
#pragma endregion


#pragma region shaders
	basicShader = new Shader("Shaders/basicVert.glsl", "Shaders/basicFrag.glsl");
	if ( !basicShader->LinkProgram() ) 
	{
		return;
	}

	beckmannShader = new Shader( "Shaders/beckmannVert.glsl", "Shaders/beckmannFrag.glsl" );
	if ( !beckmannShader->LinkProgram() ) 
	{
		return;
	}

	shadowShader = new Shader( "Shaders/shadowVert.glsl", "Shaders/shadowFrag.glsl" );
	if ( !shadowShader->LinkProgram() )
	{
		return;
	}

	mainShader = new Shader( "Shaders/mainVert.glsl", "Shaders/mainFrag.glsl" );
	if ( !mainShader->LinkProgram() )
	{
		return;
	}

	blurShader = new Shader( "Shaders/blurVert.glsl", "Shaders/blurFrag.glsl" );
	if ( !blurShader->LinkProgram() )
	{
		return;
	}

	accumSkinShader = new Shader("Shaders/basicVert.glsl", "Shaders/accumSkinFrag.glsl");
	if ( !accumSkinShader->LinkProgram() )
	{
		return;
	}

	accumMarbleShader = new Shader("Shaders/basicVert.glsl", "Shaders/accumMarbleFrag.glsl");
	if ( !accumMarbleShader->LinkProgram() )
	{
		return;
	}

	depthShader = new Shader("Shaders/depthVert.glsl", "Shaders/depthFrag.glsl");
	if ( !depthShader->LinkProgram() )
	{
		return;
	}
#pragma endregion


#pragma region beckmann texture buffer
	// diffuse texture
	generateTexture(beckmannTex, BECKMANN, BECKMANN);

	// frame buffer
	glGenFramebuffers(1, &beckmannFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, beckmannFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, beckmannTex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !beckmannTex)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion


#pragma region front & back depth buffers
	generateTexture(frontZValTex, SHADOWMAP, SHADOWMAP);
	generateTexture(backZValTex, SHADOWMAP, SHADOWMAP);
	generateDepthTexture(frontDepthTex, SHADOWMAP, SHADOWMAP);
	generateDepthTexture(backDepthTex, SHADOWMAP, SHADOWMAP);

	glGenFramebuffers(1, &frontDepthFBO);
	glGenFramebuffers(1, &backDepthFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, frontDepthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, frontDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frontZValTex, 0);
	//glDrawBuffer( GL_NONE );

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, backDepthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, backDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backZValTex, 0);
	//glDrawBuffer( GL_NONE );

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion


#pragma region shadow map buffer
	// colour texture
	generateTexture(shadowMapTex, SHADOWMAP, SHADOWMAP);

	// depth texture
	glGenTextures(1, &shadowMapDepthTex);
	glBindTexture(GL_TEXTURE_2D, shadowMapDepthTex);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP, SHADOWMAP, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glBindTexture(GL_TEXTURE_2D, 0);

	// frame buffer
	glGenFramebuffers(1, &shadowMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowMapTex, 0);
	//glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE  || !shadowMapDepthTex)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion


#pragma region gaussian blur buffer
	// textures
	for (int i = 0; i < 5; ++i)
	{
		generateTexture(blurredTexture[i], width, height);
	}

	generateTexture(blurTempTex, width, height);

	if (!blurTempTex)
	{
		return;
	}

	// frame buffer
	glGenFramebuffers(1, &blurFBO);
#pragma endregion


#pragma region main buffer
	GLenum buffers[2];
	buffers[0] = GL_COLOR_ATTACHMENT0;
	buffers[1] = GL_COLOR_ATTACHMENT1;

	// diffuse texture
	generateTexture(bufferColourTex, width, height);

	// linear depth texture
	generateTexture(bufferDepthTex, width, height);

	// depth-stencil texture
	generateTexture(bufferDepthStencilTex, width, height, true);

	// frame buffer
	glGenFramebuffers(1, &bufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthStencilTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthStencilTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bufferDepthTex, 0);
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || 
		!bufferColourTex || !bufferDepthTex || !bufferDepthStencilTex)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion


#pragma region final buffer
	// diffuse texture
	generateTexture(finalColourTex, width, height);

	// frame buffer
	glGenFramebuffers(1, &finalFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalColourTex, 0);
		
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !finalColourTex)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion


#pragma region gaussians
	gaussiansSkin = &Gaussian::SKIN;
	gaussiansMarble = &Gaussian::MARBLE;

	// This 6-gaussian sum is included for comparison purposes
	float variances[] = { 0.0484f, 0.187f, 0.567f, 1.99f, 7.41f };
	Vector3 weights[] = {
							Vector3(0.233f, 0.455f, 0.649f),
							Vector3(0.100f, 0.336f, 0.344f),
							Vector3(0.118f, 0.198f, 0.0f),
							Vector3(0.113f, 0.007f, 0.007f),
							Vector3(0.358f, 0.004f, 0.0f),
							Vector3(0.078f, 0.0f, 0.0f)
						};

	skin6Gaussians = Gaussian::gaussianSum(variances, weights, 5);
	gaussians6Skin = &skin6Gaussians;

	correction = 800.0f;
#pragma endregion


#pragma region OpenGL & others variables
	glEnable(GL_DEPTH_TEST);

	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, FOV);
		
	useTransmittance = true;
	useSSS = true;
	singleMesh = true;
	switchMesh = true;

	firstFrame = true;
	init = true;
#pragma endregion
}

Renderer::~Renderer(void)
{
	// Shaders
	delete basicShader;
	delete beckmannShader;
	delete shadowShader;
	delete mainShader;
	delete blurShader;
	delete accumSkinShader;
	delete accumMarbleShader;
	delete depthShader;
	currentShader = NULL;


	// Beckmann texture buffer
	glDeleteTextures(1, &beckmannTex);
	glDeleteFramebuffers(1, &beckmannFBO);


	// --- two depth maps ---
	glDeleteTextures(1, &frontZValTex);
	glDeleteTextures(1, &backZValTex);
	glDeleteTextures(1, &frontDepthTex);
	glDeleteTextures(1, &backDepthTex);
	glDeleteFramebuffers(1, &frontDepthFBO);
	glDeleteFramebuffers(1, &backDepthFBO);
	// --- o ---


	// Shadow map buffer
	glDeleteTextures(1, &shadowMapTex);
	glDeleteTextures(1, &shadowMapDepthTex);
	glDeleteFramebuffers(1, &shadowMapFBO);


	// main buffer
	glDeleteTextures(1, &bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &bufferDepthStencilTex);
	glDeleteFramebuffers(1, &bufferFBO);


	// gaussian blur buffer
	glDeleteTextures(5, blurredTexture);
	glDeleteTextures(1, &blurTempTex);
	glDeleteFramebuffers(1, &blurFBO);


	// Meshes
	delete quad;
	delete headMesh;
	delete lightMesh;


	// Camera & light
	delete camera;
	delete light;
}

void Renderer::generateTexture(GLuint &into, float width, float height, bool depth_stencil)
{
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 
				 0, 
				 depth_stencil ? GL_DEPTH24_STENCIL8	: GL_RGBA8, 
				 width, 
				 height, 
				 0, 
				 depth_stencil ? GL_DEPTH_STENCIL		: GL_RGBA, 
				 depth_stencil ? GL_UNSIGNED_INT_24_8	: GL_UNSIGNED_BYTE, 
				 NULL
				);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::generateDepthTexture(GLuint &into, float width, float height)
{
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 
				 0, 
				 GL_DEPTH_COMPONENT24, 
				 width, 
				 height, 
				 0, 
				 GL_DEPTH_COMPONENT, 
				 GL_UNSIGNED_BYTE, 
				 NULL
				);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::drawQuad(GLuint &texture, Vector2 &pos, float w, float h)
{
	// set texture
	quad->SetTexture(texture);
	
	// shader
	SetCurrentShader(basicShader);

	// shader textures
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	// matrices
	viewMatrix.ToIdentity();
	projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, (float)width, 0.0f, (float)height, 0.0f);
	modelMatrix = Matrix4::Translation(Vector3(pos.x + w, pos.y + h, 0.0f)) * 
					Matrix4::Scale(Vector3(w, h, 1.0f)) * 
					Matrix4::Rotation(180.0f, Vector3(0.0f, 0.0f, 1.0f)) * 
					Matrix4::Rotation(180.0f, Vector3(0.0f, 1.0f, 0.0f));
	UpdateShaderMatrices();

	// draw call
	quad->Draw();

	// clean up
	glUseProgram(0);
	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, FOV);
}

void Renderer::UpdateScene(float msec)
{
	// update camera & view matrix
	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();
	
	// reset camera position
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R))
	{
		camera->SetPitch(4.660f);
		camera->SetYaw(37.680f);
		camera->SetPosition(Vector3(0.364f, -0.030f, 0.482f));
	}

	// enable SSS
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))
	{
		useSSS = !useSSS;
	}

	// enable light transmittance
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
	{
		useTransmittance = !useTransmittance;
	}

	// switch between meshes
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3))
	{
		switchMesh = !switchMesh;
	}

	// switch between a single mesh and multiple meshes
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4))
	{
		singleMesh = !singleMesh;
	}

	// light movement
	{
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
			lightPos.z += 0.009f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP))
			lightPos.z -= 0.009f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT))
			lightPos.x += 0.009f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))
			lightPos.x -= 0.009f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_O))
			lightPos.y += 0.009f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_L))
			lightPos.y -= 0.009f;

		light->SetPosition(lightPos);
	}
}

void Renderer::RenderScene()
{
	// Beckmann texture
	if (firstFrame) {
		computeBeckmannTex();

		firstFrame = false;
	}

	// Depth maps
	drawDepthmap(FRONT);
	drawDepthmap(BACK);

	// Shadow map pass
	shadowMapPass();

	// Main rendering pass
	mainPass();

	// SSS pass
	if ( switchMesh ) {
		sssPass(*gaussiansSkin);
	}
	else {
		sssPass(*gaussiansMarble);
	}

	// Final accumulation pass
	accumulationPass();

	// Render to screen
	presentScene();

	GL_BREAKPOINT
	SwapBuffers();
	glUseProgram(0);
}

void Renderer::computeBeckmannTex()
{
	// set up
	glBindFramebuffer(GL_FRAMEBUFFER, beckmannFBO);
	glViewport(0.0f, 0.0f, BECKMANN, BECKMANN);
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// shader
	SetCurrentShader(beckmannShader);

	// matrices
	projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f);
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	// draw call
	quad->Draw();

	// clean up
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0.0f, 0.0f, (float)width, (float)height);
}

void Renderer::drawDepthmap(bool face)
{
	// set up
	glBindFramebuffer(GL_FRAMEBUFFER, face ? frontDepthFBO : backDepthFBO);
	glViewport(0.0f, 0.0f, SHADOWMAP, SHADOWMAP);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(face ? GL_BACK : GL_FRONT);

	// shader
	SetCurrentShader(depthShader);

	// Shader variables
	glUniform1f( glGetUniformLocation(currentShader->GetProgram(), "zNear"), ZNEAR );
	glUniform1f( glGetUniformLocation(currentShader->GetProgram(), "zFar"), ZFAR );

	// matrices
	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, 1.0f, FOV);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), lightTarget);
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	// draw calls
	drawMesh();

	// clean up
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_CULL_FACE);
	glViewport(0, 0, width, height);
}

void Renderer::shadowMapPass()
{
	// set up
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glViewport(0.0f, 0.0f, SHADOWMAP, SHADOWMAP);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	// shader
	SetCurrentShader(shadowShader);

	// Shader variables
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "zNear"), ZNEAR);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "zFar"), ZFAR);

	// matrices
	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, 1.0f, FOV);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), lightTarget);
	modelMatrix.ToIdentity();
	shadowMatrix = biasMatrix * (projMatrix * viewMatrix);
	UpdateShaderMatrices();

	// draw
	drawMesh();

	// clean up
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
}

void Renderer::mainPass()
{
	//set up
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearStencil(0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	// shader
	SetCurrentShader(mainShader);

	// shader textures
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "shadowMapTex"), 2);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "beckmannTex"), 3);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "linearShadowMapTex"), 4);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowMapDepthTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, beckmannTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, shadowMapTex);

	// --- two depth maps ---
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "frontDepthTex"), 5);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "backDepthTex"), 6);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, frontDepthTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, backDepthTex);
	// --- o ---

	// shader variables
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "zNear"), ZNEAR);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "zFar"), ZFAR);

	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	SetShaderLight(*light);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "useTransmittance"), useTransmittance);

	// matrices
	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, FOV);
	viewMatrix = camera->BuildViewMatrix();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	// --- light proj matrix for transmittace (same as the one used for shadow mapping) ---
	Matrix4 lightViewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), lightTarget);
	Matrix4 lightProjMatrix = Matrix4::Perspective(ZNEAR, ZFAR, 1.0f, FOV);
	lightView = biasMatrix * (lightProjMatrix * lightViewMatrix);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "lightView"), 1,false, (float*)&lightView);
	// --- o ---

	// set stencil for geometry with SSS
	glStencilFunc(GL_ALWAYS, 1, ~0);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	// draw calls
	drawMesh();

	// set stencil for geometry without SSS
	glStencilFunc(GL_ALWAYS, 2, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// draw calls
	drawLight();

	// clean up
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::sssPass(const std::vector<Gaussian> &gaussians)
{
	// set up
	glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthStencilTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthStencilTex, 0);
/*
	// clear textures
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTempTex[0], 0);
	glClear( GL_COLOR_BUFFER_BIT );

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTempTex[1], 0);
	glClear( GL_COLOR_BUFFER_BIT );
*/
	// set up stencil test
	glStencilFunc(GL_EQUAL, 1, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// shader
	SetCurrentShader(blurShader);

	// shader textures
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "depthTex"), 5);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	// shader variables
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"), 1.0f/width, 1.0f/height);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "correction"), correction);

	// matrices
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f);
	UpdateShaderMatrices();

	//Call blur passes:
	// blur 1
	blurPass(bufferColourTex, blurredTexture[0], bufferColourTex, gaussians[0]);
	// blur 2
	blurPass(blurredTexture[0], blurredTexture[1], bufferColourTex, gaussians[1]);
	// blur 3
	blurPass(blurredTexture[1], blurredTexture[2], bufferColourTex, gaussians[2]);

	if (!switchMesh)
	{
		// blur 4
		blurPass(blurredTexture[2], blurredTexture[3], bufferColourTex, gaussians[3]);
	}

	// clean up
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

void Renderer::blurPass(GLuint &sourceTex, GLuint &targetTex, GLuint &finalTarget, const Gaussian &gaussian)
{
	// gaussian variables
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "gaussianWidth"), gaussian.getWidth());

#pragma region Horizontal Pass
	// set up render targets
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTempTex, 0);

	glClear(GL_COLOR_BUFFER_BIT);

	// shader variables
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "dir"), 1.0f, 0.0f);

	// draw
	quad->SetTexture(sourceTex);
	quad->Draw();
#pragma endregion

#pragma region Vertical Pass
	// set up render targets
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTex, 0);

	glClear(GL_COLOR_BUFFER_BIT);

	// shader variables
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "dir"), 0.0f, 1.0f);

	// draw
	quad->SetTexture(blurTempTex);
	quad->Draw();
#pragma endregion
}

void Renderer::accumulationPass()
{
	// Set up
	glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthStencilTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthStencilTex, 0);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Shader
	if (switchMesh)
	{
		SetCurrentShader(accumSkinShader);
	}
	else
	{
		SetCurrentShader(accumMarbleShader);
	}

	// Shader textures & variables
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "useSSS"), useSSS);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex1"), 5);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex2"), 6);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex3"), 7);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex4"), 8);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, blurredTexture[0]);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, blurredTexture[1]);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, blurredTexture[2]);

	if (!switchMesh)
	{
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex5"), 9);

		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, blurredTexture[3]);
	}

	// Matrices
	projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f);
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

#pragma region Draw call with SSS
	quad->SetTexture(bufferColourTex);

	quad->Draw();
#pragma endregion

#pragma region Draw geometry without SSS
	// set up stencil test
	glStencilFunc(GL_NOTEQUAL, 1, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// Shader
	SetCurrentShader(basicShader);

	// matrices
	UpdateShaderMatrices();

	// Draw call
	quad->SetTexture(bufferColourTex);
	quad->Draw();
#pragma endregion

	// Clean up
	glUseProgram(0);
	glDisable(GL_STENCIL_TEST);
}

void Renderer::presentScene()
{
	// Set up
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Shader
	SetCurrentShader(basicShader);

	// Matrices
	projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f);
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	// Draw call
	quad->SetTexture(finalColourTex);
	quad->Draw();

	// Clean up
	glUseProgram(0);
}

void Renderer::drawMesh()
{
	if (singleMesh)
	{
		if (switchMesh)
		{
			modelMatrix = Matrix4::Translation(Vector3(0.0f, 0.0f, 0.0f)) * Matrix4::Scale(Vector3(1.0f, 1.0f, 1.0f));
		}
		else
		{
			modelMatrix = Matrix4::Translation(Vector3(0.0f, -0.2f, 0.0f)) * Matrix4::Scale(Vector3(0.0051f, 0.005f, 0.005f));
		}

		Matrix4 tempMatrix = shadowMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "shadowMatrix"), 1, false, *&tempMatrix.values);

		UpdateShaderMatrices();

		if (switchMesh)
		{
			headMesh->Draw();
		}
		else
		{
			knightMesh->Draw();
		}
	}
	else
	{
		float xPos = 0.5f;
		float zPos = -0.5f;
		float count = 0.0f;

		for (int i = 0; i < 9; ++i)
		{
			if (i == 3)
			{
				zPos = 0.0f;
				count = 0.0f;
			}
			if (i == 6)
			{
				zPos = 0.5f;
				count = 0.0f;
			}

			if (switchMesh)
			{
				modelMatrix = Matrix4::Translation(Vector3(xPos * count - 0.5f, 0.0f, zPos)) * Matrix4::Scale(Vector3(1.0f, 1.0f, 1.0f));
			}
			else
			{
				modelMatrix = Matrix4::Translation(Vector3(xPos * count - 0.5f, -0.2f, zPos)) * Matrix4::Scale(Vector3(0.0051f, 0.005f, 0.005f));
			}
			++count;

			Matrix4 tempMatrix = shadowMatrix * modelMatrix;
			glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "shadowMatrix"), 1, false, *&tempMatrix.values);

			UpdateShaderMatrices();

			if (switchMesh)
			{
				headMesh->Draw();
			}
			else
			{
				knightMesh->Draw();
			}
		}
	}
}

void Renderer::drawLight()
{
	// origin
	modelMatrix = Matrix4::Translation(light->GetPosition()) * Matrix4::Scale(Vector3(0.01f, 0.01f, 0.01f));
	UpdateShaderMatrices();

	lightMesh->Draw();

	// target
	modelMatrix = Matrix4::Translation(lightTarget) * Matrix4::Scale(Vector3(0.01f, 0.01f, 0.01f));
	UpdateShaderMatrices();

	lightMesh->Draw();
}