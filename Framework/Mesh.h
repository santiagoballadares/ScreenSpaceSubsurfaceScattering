#pragma once
#include "OGLRenderer.h"

enum MeshBuffer
{
	VERTEX_BUFFER,
	COLOUR_BUFFER,
	TEXTURE_BUFFER,
	NORMAL_BUFFER,
	TANGENT_BUFFER,
	INDEX_BUFFER,
	MAX_BUFFER
};

class Mesh
{
public:
	Mesh();
	~Mesh();

	virtual void Draw();

	static Mesh * GenerateTriangle();
	static Mesh * GenerateQuad();
	static Mesh * GenerateQuad(float texCoordScale);

	void SetTexture(GLuint tex)		{ texture = tex;}
	GLuint GetTexture()				{ return texture; }

	void SetTexture2(GLuint tex)	{ texture2 = tex;}
	GLuint GetTexture2()			{ return texture2; }

	void SetTexture3(GLuint tex)	{ texture3 = tex;}
	GLuint GetTexture3()			{ return texture3; }

	void SetBumpMap(GLuint tex)		{ bumpTexture = tex; }
	GLuint GetBumpMap()				{ return bumpTexture; }

	void SetBumpMap2(GLuint tex)	{ bumpTexture2 = tex; }
	GLuint GetBumpMap2()			{ return bumpTexture2; }

	void SetBumpMap3(GLuint tex)	{ bumpTexture3 = tex; }
	GLuint GetBumpMap3()			{ return bumpTexture3; }

	unsigned int GetNumVertices()	{ return numVertices; }
	Vector3 * GetVertices()			{ return vertices; }
	Vector3 * GetNormals()			{ return normals; }

protected:
	void BufferData();

	float *colors;

	void GenerateNormals();
	void GenerateTangents();
	Vector3 GenerateTangent(const Vector3 &a, const Vector3 &b,	const Vector3 &c,
							const Vector2 &ta, const Vector2 &tb, const Vector2 &tc);

	Vector3 *vertices;
	Vector4 *colours;
	Vector2 *textureCoords;
	Vector3 *normals;
	Vector3 *tangents;

	GLuint bufferObject[MAX_BUFFER];

	GLuint arrayObject;
	GLuint numVertices;
	GLuint numIndices;
	GLuint texture;
	GLuint texture2;
	GLuint texture3;
	GLuint bumpTexture;
	GLuint bumpTexture2;
	GLuint bumpTexture3;
	
	unsigned int *indices;

	GLuint type;
};