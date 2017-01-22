/*
 * Santiago Balladares
 * Dissertation Project
 * Texture Space Subsurface Scattering
 * Newcastle University
 * Computer Game Programming MSc
 * 2011 - 2012
 */
#include "Renderer.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent)
{
#pragma region camera
	camera = new Camera(4.660f, 37.680f, Vector3(0.364f, -0.030f, 0.482f));
#pragma endregion

#pragma region meshes
	// Quad
	quad = Mesh::GenerateQuad();

	// head
	OBJMesh *mHead = new OBJMesh("../Meshes/head.obj");
	mHead->SetTexture(SOIL_load_OGL_texture("../Textures/head_col.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	mHead->SetBumpMap(SOIL_load_OGL_texture("../Textures/head_normal.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	headMesh = mHead;
#pragma endregion


#pragma region light
	// light position
	lightPos = Vector3(2.0099986f, 0.0f, 2.6999984f);

	// light target
	lightTarget = Vector3(0.0f, 0.0f, 0.0f);

	// new light
	light = new Light(lightPos, Vector4(1.0f, 1.0f, 1.0f, 1.0f), 10.0f);

	// light mesh
	OBJMesh *mSphere = new OBJMesh();
	mSphere->LoadOBJMesh("../Meshes/ico.obj");
	lightMesh = mSphere;
#pragma endregion


#pragma region shaders
	basicShader = new Shader("Shaders/basicVert.glsl", "Shaders/basicFrag.glsl");
	if ( !basicShader->LinkProgram() )
	{
		return;
	}

	lightShader = new Shader("Shaders/lightVert.glsl", "Shaders/lightFrag.glsl");
	if ( !lightShader->LinkProgram() )
	{
		return;
	}

	beckmannShader = new Shader( "Shaders/beckmannVert.glsl", "Shaders/beckmannFrag.glsl" );
	if ( !beckmannShader->LinkProgram() )
	{
		return;
	}

	shadowShader = new Shader("Shaders/shadowVert.glsl", "Shaders/shadowFrag.glsl");
	if ( !shadowShader->LinkProgram() )
	{
		return;
	}

	stretchShader = new Shader("Shaders/stretchVert.glsl", "Shaders/stretchFrag.glsl");
	if ( !stretchShader->LinkProgram() )
	{
		return;
	}

	unwrapShader = new Shader("Shaders/unwrapVert.glsl", "Shaders/unwrapFrag.glsl");
	if ( !unwrapShader->LinkProgram() )
	{
		return;
	}

	blurShader = new Shader("Shaders/blurVert.glsl", "Shaders/blurFrag.glsl");
	if ( !blurShader->LinkProgram() )
	{
		return;
	}

	mainShader = new Shader("Shaders/mainVert.glsl", "Shaders/mainFrag.glsl");
	if ( !mainShader->LinkProgram() )
	{
		return;
	}
#pragma endregion


#pragma region beckmann texture buffer
	// diffuse texture
	generateTexture(beckmannTex, MAP_SIZE, MAP_SIZE);

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


#pragma region shadow map buffer
	// depth texture
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, MAP_SIZE, MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glBindTexture(GL_TEXTURE_2D, 0);

	// frame buffer
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,	GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion


#pragma region 1 non-convolved & 5 convolved irradiance textures
	generateTexture(nonBlurredTexture, MAP_SIZE, MAP_SIZE);

	for (int i = 0; i < 5; ++i)
	{
		generateTexture(blurredTexture[i], MAP_SIZE, MAP_SIZE);
	}
#pragma endregion


#pragma region stretch map buffer
	// depth texture
	generateTexture(stretchDepthTex, MAP_SIZE, MAP_SIZE, true);

	// colour texture
	generateTexture(stretchColourTex, MAP_SIZE, MAP_SIZE);
		
	// frame buffer
	glGenFramebuffers(1, &stretchFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, stretchFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,	GL_TEXTURE_2D, stretchDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stretchDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, stretchColourTex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion


#pragma region unwrap buffer
	// depth texture
	generateTexture(unwrapDepthTex, MAP_SIZE, MAP_SIZE, true);

	// frame buffer
	glGenFramebuffers(1, &unwrapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, unwrapFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,	GL_TEXTURE_2D, unwrapDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, unwrapDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nonBlurredTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion


#pragma region blur buffer
	// colour texture
	generateTexture(tempColourTex, MAP_SIZE, MAP_SIZE);

	// frame buffer
	glGenFramebuffers(1, &blurFBO);
#pragma endregion


#pragma region OpenGL & others variables
	glEnable(GL_DEPTH_TEST);

	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, FOV);

	firstFrame = true;
	useBlur = true;
	useStretch = true;
	init = true;
#pragma endregion
}

Renderer::~Renderer(void)
{
	// Shaders
	delete basicShader;
	delete lightShader;
	delete beckmannShader;
	delete stretchShader;
	delete unwrapShader;
	delete blurShader;
	delete mainShader;
	currentShader = NULL;


	// Beckmann texture buffer
	glDeleteTextures(1, &beckmannTex);
	glDeleteFramebuffers(1, &beckmannFBO);


	// Shadow map buffer
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);


	// Textures
	glDeleteTextures(1, &nonBlurredTexture);
	glDeleteTextures(5, blurredTexture);


	// stretch buffer
	glDeleteTextures(1, &stretchColourTex);
	glDeleteTextures(1, &stretchDepthTex);
	glDeleteFramebuffers(1, &stretchFBO);


	// unwrap buffer
	glDeleteTextures(1, &unwrapDepthTex);
	glDeleteFramebuffers(1, &unwrapFBO);


	// blur buffer
	glDeleteTextures(1, &tempColourTex);
	glDeleteFramebuffers(1, &blurFBO);


	// Meshes
	delete quad;
	delete headMesh;


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

	glTexImage2D(GL_TEXTURE_2D, 0, 
				 depth_stencil ? GL_DEPTH24_STENCIL8	: GL_RGBA8, 
				 width, height, 0, 
				 depth_stencil ? GL_DEPTH_STENCIL		: GL_RGBA, 
				 depth_stencil ? GL_UNSIGNED_INT_24_8	: GL_UNSIGNED_BYTE, 
				 NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::drawQuad(GLuint &texture, Vector2 &pos, float w, float h)
{
	// set texture to draw
	quad->SetTexture(texture);
	
	// shader
	SetCurrentShader(basicShader);

	// shader textures
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	// matrices
	viewMatrix.ToIdentity();
	projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, (float)width, 0.0f, (float)height, 0.0f);
	modelMatrix = Matrix4::Translation(Vector3(pos.x + w, pos.y + h, 0.0f)) * 
					Matrix4::Scale(Vector3(w, h, 1)) * 
					Matrix4::Rotation(180, Vector3(0, 0, 1)) * 
					Matrix4::Rotation(180, Vector3(0, 1, 0));
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
	if ( Window::GetKeyboard()->KeyTriggered(KEYBOARD_R) )
	{
		camera->SetPitch(4.660f);
		camera->SetYaw(37.680f);
		camera->SetPosition(Vector3(0.364f, -0.030f, 0.482f));
	}

	// enable SSS
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))
	{
		useBlur = !useBlur;
	}

	// enable stretch map correction
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
	{
		useStretch = !useStretch;
	}

#pragma region light movement
	if(Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
		lightPos.z += 0.006f;
	if(Window::GetKeyboard()->KeyDown(KEYBOARD_UP))
		lightPos.z -= 0.006f;

	if(Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT))
		lightPos.x += 0.006f;
	if(Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))
		lightPos.x -= 0.006f;

	if(Window::GetKeyboard()->KeyDown(KEYBOARD_O))
		lightPos.y += 0.006f;
	if(Window::GetKeyboard()->KeyDown(KEYBOARD_L))
		lightPos.y -= 0.006f;

	light->SetPosition(lightPos);
#pragma endregion
}

void Renderer::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (firstFrame)
	{
		computeBeckmannTex();
		computeStretchMap();

		firstFrame = false;
	}

	shadowPass();
	unwrapMesh();
	blurPass();
	mainPass();

	GL_BREAKPOINT
	SwapBuffers();
	glUseProgram(0);
}

void Renderer::computeBeckmannTex()
{
	// set up
	glBindFramebuffer(GL_FRAMEBUFFER, beckmannFBO);
	glViewport(0.0f, 0.0f, MAP_SIZE, MAP_SIZE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

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

void Renderer::computeStretchMap()
{
	// set up
	glBindFramebuffer(GL_FRAMEBUFFER, stretchFBO);
	glViewport(0.0f, 0.0f, MAP_SIZE, MAP_SIZE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// shader
	SetCurrentShader(stretchShader);

	// matrices
	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, 1.0, FOV);
	viewMatrix = camera->BuildViewMatrix();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	// draw calls
	drawMesh();

	// clean up
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0.0f, 0.0f, (float)width, (float)height);
}

void Renderer::shadowPass()
{
	// set up
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glViewport(0, 0, MAP_SIZE, MAP_SIZE);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// shader
	SetCurrentShader(shadowShader);

	// matrices
	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, 1.0, FOV);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), lightTarget);
	modelMatrix.ToIdentity();
	shadowMatrix = biasMatrix * (projMatrix * viewMatrix);
	UpdateShaderMatrices();

	// draw calls
	drawMesh();

	// clean up
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

void Renderer::unwrapMesh()
{
	// set up
	glBindFramebuffer(GL_FRAMEBUFFER, unwrapFBO);
	glViewport(0.0f, 0.0f, MAP_SIZE, MAP_SIZE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// shader
	SetCurrentShader(unwrapShader);
	
	// shader textures
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "shadowTex"), 2);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	// shader light variables
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	// matrices
	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, 1.0, FOV);
	viewMatrix = camera->BuildViewMatrix();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	// draw calls
	drawMesh();

	// clean up
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0.0f, 0.0f, (float)width, (float)height);
}

void Renderer::blurPass()
{
	// set up
	glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glViewport(0.0f, 0.0f, MAP_SIZE, MAP_SIZE);

	// shader
	SetCurrentShader(blurShader);

	// shader textures
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "stretchTex"), 2);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, stretchColourTex);

	// shader variables
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"), 1.0f/MAP_SIZE, 1.0f/MAP_SIZE);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "useStretch"), useStretch);

	// matrices
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f);
	UpdateShaderMatrices();

	// blur 1
	uvPass(nonBlurredTexture, blurredTexture[0]);
	// blur 2
	uvPass(blurredTexture[0], blurredTexture[1]);
	// blur 3
	uvPass(blurredTexture[1], blurredTexture[2]);
	// blur 4
	uvPass(blurredTexture[2], blurredTexture[3]);
	// blur 5
	uvPass(blurredTexture[3], blurredTexture[4]);

	// clean up
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
	glViewport(0.0f, 0.0f, (float)width, (float)height);
}

void Renderer::uvPass(GLuint &sourceTex, GLuint &targetTex)
{
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "xAxis"), true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempColourTex, 0);

	quad->SetTexture(sourceTex);
	quad->Draw();

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "xAxis"), false);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTex, 0);

	quad->SetTexture(tempColourTex);
	quad->Draw();
}

void Renderer::mainPass()
{
	// shader
	SetCurrentShader(mainShader);
	
	// shader textures
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "shadowTex"), 2);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "beckmannTex"), 3);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex1"), 5);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex2"), 6);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex3"), 7);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex4"), 8);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex5"), 9);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "blurredTex6"), 10);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, beckmannTex);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, nonBlurredTexture);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, blurredTexture[0]);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, blurredTexture[1]);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, blurredTexture[2]);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, blurredTexture[3]);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, blurredTexture[4]);

	// shader variables
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "useBlur"), useBlur);

	// shader light variables
	SetShaderLight(*light);

	// matrices
	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, FOV);
	viewMatrix = camera->BuildViewMatrix();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	// draw calls
	drawMesh();
	drawLight();

	// clean up
	glUseProgram(0);
}

void Renderer::drawMesh()
{
	modelMatrix = Matrix4::Translation(Vector3(0.0f, 0.0f, 0.0f)) * Matrix4::Scale(Vector3(1.0f, 1.0f, 1.0f));

	Matrix4 tempMatrix = shadowMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "shadowMatrix"), 1, false, *&tempMatrix.values);

	UpdateShaderMatrices();

	headMesh->Draw();
}

void Renderer::drawLight()
{
	modelMatrix = Matrix4::Translation(light->GetPosition()) * Matrix4::Scale(Vector3(0.01f, 0.01f, 0.01f));
	UpdateShaderMatrices();

	lightMesh->Draw();
}