#pragma once

#include "../Framework/OGLRenderer.h"
#include "../Framework/Camera.h"
#include "../Framework/OBJMesh.h"

#define ZNEAR		0.1f
#define ZFAR		10.0f
#define FOV			25.0f

#define MAP_SIZE	(1024)

class Renderer : public OGLRenderer
{
public:
	Renderer(Window &parent);
	virtual ~Renderer(void);

	virtual void RenderScene();
	virtual void UpdateScene(float msec);

protected:
	void generateTexture(GLuint &into, float width, float height, bool depth_stencil = false);
	void drawQuad(GLuint &texture, Vector2 &pos, float w, float h);
	void drawMesh();
	void drawLight();
	
	void computeBeckmannTex();
	void computeStretchMap();
	void shadowPass();
	void unwrapMesh();
	void blurPass();
	void uvPass(GLuint &sourceTex, GLuint &targetTex);
	void mainPass();


	// Meshes
	Mesh *quad;
	OBJMesh *headMesh;
	OBJMesh *lightMesh;


	// Camera & light
	Camera *camera;
	Light *light;
	Vector3 lightPos;
	Vector3 lightTarget;


	// Shaders
	Shader *basicShader;
	Shader *lightShader;

	Matrix4 lightMatrix;

	Shader *beckmannShader;
	Shader *shadowShader;
	Shader *stretchShader;
	Shader *unwrapShader;
	Shader *blurShader;
	Shader *mainShader;

	
	// Beckmann Texture buffer
	GLuint beckmannFBO;
	GLuint beckmannTex;


	// stretch buffer
	GLuint stretchFBO;
	GLuint stretchColourTex;
	GLuint stretchDepthTex;


	// Shadow map
	GLuint shadowFBO;
	GLuint shadowTex;
	Matrix4 shadowMatrix;


	// 1 non-convolved & 5 convolved irradiance textures
	GLuint nonBlurredTexture;
	GLuint blurredTexture[5];


	// unwrap buffer
	GLuint unwrapFBO;
	GLuint unwrapDepthTex;


	// blur buffer
	GLuint blurFBO;
	GLuint tempColourTex;


	// bool variables
	bool firstFrame;
	bool useBlur;
	bool useStretch;
};
