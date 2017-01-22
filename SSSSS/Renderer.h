#pragma once

#include "../Framework/OGLRenderer.h"
#include "../Framework/Camera.h"
#include "../Framework/OBJMesh.h"
#include "Gaussian.h"

#define ZNEAR		0.1f
#define ZFAR		10.0f
#define FOV			25.0f

#define BECKMANN	1024.0f
#define SHADOWMAP	2048.0f

class Renderer : public OGLRenderer
{
public:
	Renderer(Window &parent);
	virtual ~Renderer(void);

	virtual void RenderScene();
	virtual void UpdateScene(float msec);

protected:
	void generateTexture(GLuint &into, float width, float height, bool depth_stencil = false);
	void generateDepthTexture(GLuint &into, float width, float height);
	void drawQuad(GLuint &texture, Vector2 &pos, float w, float h);
	void drawMesh();
	void drawLight();

	void drawDepthmap(bool face);
	void computeBeckmannTex();
	void shadowMapPass();
	void mainPass();
	void sssPass(const std::vector<Gaussian> &gaussians);
	void blurPass(GLuint &sourceTex, GLuint &targetTex, GLuint &finalTarget, const Gaussian &gaussian);
	void accumulationPass();
	void presentScene();

	// Meshes
	Mesh *quad;
	OBJMesh *headMesh;
	OBJMesh *knightMesh;
	OBJMesh *lightMesh;


	// Camera & light
	Camera *camera;
	Light *light;
	Vector3 lightTarget;
	Vector3 lightPos;


	// Shaders
	Shader *basicShader;
	Shader *beckmannShader;
	Shader *shadowShader;
	Shader *mainShader;
	Shader *blurShader;
	Shader *accumSkinShader;
	Shader *accumMarbleShader;
	Shader *depthShader;


	// Beckmann Texture buffer
	GLuint beckmannFBO;
	GLuint beckmannTex;


	// Shadow map
	GLuint shadowMapFBO;
	GLuint shadowMapTex;
	GLuint shadowMapDepthTex;
	Matrix4 shadowMatrix;
	Matrix4 lightView;


	// gaussian blur buffer
	GLuint blurFBO;
	GLuint blurTempTex;
	GLuint blurredTexture[4];


	// main buffer
	GLuint bufferFBO;
	GLuint bufferColourTex;
	GLuint bufferDepthTex;
	GLuint bufferDepthStencilTex;


	// final buffer
	GLuint finalFBO;
	GLuint finalColourTex;


	// Gaussians
	vector<Gaussian> skin6Gaussians;
	const vector<Gaussian> *gaussiansSkin;
	const vector<Gaussian> *gaussians6Skin;
	const vector<Gaussian> *gaussiansMarble;
	float correction;


	// bool variables
	bool firstFrame;
	bool useSSS;
	bool useTransmittance;
	bool switchMesh;
	bool singleMesh;


	// --- two depth maps ---
	GLuint frontDepthFBO;
	GLuint backDepthFBO;

	GLuint frontDepthTex;
	GLuint backDepthTex;

	GLuint frontZValTex;
	GLuint backZValTex;
	// --- o ---
};