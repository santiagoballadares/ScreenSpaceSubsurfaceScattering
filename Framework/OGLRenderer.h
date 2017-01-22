#pragma once
/*
Class:OGLRenderer
Author:Rich Davison
Description:Abstract base class for the graphics tutorials. Creates an OpenGL 
3.2 CORE PROFILE rendering context. Each lesson will create a renderer that 
inherits from this class - so all context creation is handled automatically,
but students still get to see HOW such a context is created.

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*/

#include <string>
#include <fstream>

#include "GL/glew.h"
#include "GL/wglew.h"

#include "SOIL.h"

#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Quaternion.h"
#include "Matrix4.h"
#include "Window.h"

#include "Shader.h"		//Students make this file...
#include "Mesh.h"		//And this one...
#include "Light.h"		//And this one too...

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")

#ifdef _DEBUG
#pragma comment(lib, "SOIL_D.lib")
#define GL_BREAKPOINT glUniform4uiv(0,0,0);//Invalid, but triggers gdebugger ;)
#else
#pragma comment(lib, "SOIL.lib")
#define GL_BREAKPOINT //
#endif

static const float biasValues[16] = {
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
};

static const Matrix4 biasMatrix(const_cast<float*>(biasValues));


class Shader;

class OGLRenderer	{
public:
	friend class Window;
	OGLRenderer(Window &parent);
	virtual ~OGLRenderer(void);

	virtual void	RenderScene() = 0;
	virtual void	UpdateScene(float msec);
	void			SwapBuffers() const;

	bool			HasInitialised() const;

protected:
	virtual void	Resize(int x, int y);	
	void			UpdateShaderMatrices();
	void			SetCurrentShader(Shader*s);

	void			SetTextureRepeating(GLuint target, bool state);

	void			SetShaderLight(const Light &l);
	void			SetShaderLights(Light **l, const int numLights);

	Shader *currentShader;

	Matrix4 projMatrix;		//Projection matrix
	Matrix4 modelMatrix;	//Model matrix. NOT MODELVIEW
	Matrix4 viewMatrix;		//View matrix
	Matrix4 textureMatrix;	//Texture matrix

	int		width;			//Render area width (not quite the same as window width)
	int		height;			//Render area height (not quite the same as window height)
	bool	init;			//Did the renderer initialise properly?

	HDC		deviceContext;	//...Device context?
	HGLRC	renderContext;	//Permanent Rendering Context
};