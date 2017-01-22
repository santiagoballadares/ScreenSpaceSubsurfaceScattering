/******************************************************************************
Class:MD5Mesh
Implements:Mesh, MD5MeshInstance
Author:Rich Davison
Description: Implementation of id Software's MD5 skeletal animation format. 

MD5Meshes are built up of a list of 'sub meshes', built up out of a centralised
list of vertices and triangles. These lists of vertices and triangles are stored
in the MD5Mesh as MD5SubMesh structures, which feed vertex info into Meshes. 
As far as this class is concerned, the first sub mesh in the list is 'this', and 
the rest are 'children' of 'this', with the child functionality provided by the 
ChildMeshInterface class. Calling the update or skinning functions on 'this'
will update and skin all of its children, too - the entire submesh 
functionality is a complete 'black box' as far as the rest of the classes are
concerned.

MD5Mesh supports multiple animations being attached to a MD5Mesh, but only
one animation can be ran at a time (it shouldn't be /that/ hard to extend it!)

If you're going to have lots of instances of a MD5Mesh, you'd be better off
creating an MD5MeshInstance SceneNode subclass, which has a pointer to a
MD5Mesh as a member variable. This is because every MD5Mesh instantiation
will need its own VBOs and animations, so it's going to rather quickly result
in a lot of memory being used.

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////


#pragma once

/*
As this tutorial series progresses, you'll learn how to generate normals, tangents,
and how to use bumpmaps. In order for this class to compile before these features
are introduced, 'advanced' functionality has been disabled using the preprocessor.

If you want to play around with MD5Meshes in the first real time lighting tutorial,
uncomment the MD5_USE_NORMALS define. If you want to use and MD5Mesh in the second real
time lighting tutorial, uncomment both MD5_USE_NORMALS and MD5_USE_TANGENTS_BUMPMAPS
*/
#define MD5_USE_NORMALS
#define MD5_USE_TANGENTS_BUMPMAPS

#include <fstream>
#include <string>
#include <map>

#include "ChildMeshInterface.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "Vector2.h"

#include "Mesh.h"
#include "MD5Anim.h"


/*
MD5 Files are plain text, with each section in the file marked with tags.
It's good practice to use defines to keep these tag strings, even though
most of them will only actually be used once.
*/

#define MD5_VERSION_TAG			"MD5Version"
#define MD5_COMMANDLINE_TAG		"commandline"
#define MD5_NUMJOINTS_TAG		"numJoints"
#define MD5_NUMMESHES_TAG		"numMeshes"
#define MD5_JOINTS_TAG			"joints"
#define MD5_MESH_TAG			"mesh"
#define MD5_SUBMESH_NUMVERTS	"numverts"
#define MD5_SUBMESH_NUMTRIS		"numtris"
#define MD5_SUBMESH_NUMWEIGHTS  "numweights"
#define MD5_SUBMESH_SHADER		"shader"
#define MD5_SUBMESH_VERT		"vert"
#define MD5_SUBMESH_TRI			"tri"
#define MD5_SUBMESH_WEIGHT		"weight"

/*
An MD5Mesh is formed from various data structures, all of which are loaded
in from the file and stored, as they're used for skinning the mesh. 

*/

/*
Each MD5Mesh has an array of triangles, which together form the submeshes
that the MD5Mesh is built up out of.
*/
struct MD5Tri {
	int triIndex;	//Which triangle of the mesh this is
	unsigned int a;	//Index to first vertex
	unsigned int b;	//Index to second vertex
	unsigned int c;	//Index to third vertex
};



/*
Each MD5Skeleton has an array of joints, which are arranged in a parent/child
hierarchy, forming the skeleton. Each joint is given a name, so it can be
targeted by scripting ('GetTransformOfJoint("Hand") etc). Joint names are
stored separately, to avoid an annoying string copying problem with using
memcpy on an array of MD5Joints. MD5Joints have their transform twice - 
once as separate Vector3 and Quaternion (loaded from the MD5Mesh file) and
once as a Matrix4, generated at run-time. This is to make it more obvious
that joints really are just scenenode transforms like you are used to, as
well as making it slightly more efficient to transform the mesh.
*/
struct MD5Joint {
	string*		name;			//Pointer to the name of this joint
	int			parent;			//Index into the joint array of parent
	Vector3		position;		//Position relative to parent joint
	Quaternion	orientation;	//Orientation relative to parent joint
	Matrix4		transform;		//World transform of this joint
};


/*
Each MD5SubMesh has an array of MD5Verts. These are the vertices for the
submesh. As you an see, they are defined in the 'multiple weight position'
variation of vertex skinning, and so do not have a position themselves.
Instead, they have an index into an array of MD5Weights, and a number of
weights to use from this array, from which the MD5Vert's final position
will be calculated.
*/
struct MD5Vert {
	int		vertIndex;		//Which vertex of the MD5SubMesh this is
	Vector2 texCoords;		//Texture coordinates of this vertex
	int		weightIndex;	//Array index of first weighting influence
	int		weightElements; //Number of MD5Weights that influence this vertex
};

/*
Each MD5SubMesh also has an array of MD5Weights. These store an 'anchor'
position, and a weighting value (between 0.0 and 1.0). They also have an
index into the MD5Skeleton joint array, to determine which MD5Joint this
MD5Weight is relative to.
*/
struct MD5Weight {
	int		weightIndex;	//Which weight of the MD5SubMesh this is
	int		jointIndex;		//Which joint of the MD5Skeleton this weight is relative to
	float	weightValue;	//How much influence this MD5Weight has on the MD5Vert
	Vector3 position;		//Anchor position of this MD5Weight
};

/*
Each MD5Mesh has a skeleton, built up out of an array of MD5Joints.
*/
struct MD5Skeleton {
	MD5Joint*joints;	//Pointer to our array of MD5Joints
	int numJoints;		//Number of joints in the skeleton

	//Always initialise pointers to null!
	MD5Skeleton() {
		joints		= NULL;
		numJoints	= 0;
	}

	//MD5Skeleton's have heap memory, so we must free it!
	~MD5Skeleton() {
		delete[] joints;
	}
};

/*
Each MD5SubMesh has an array of MD5SubMeshes. Depending on the actual MD5Mesh
loaded there might only be one MD5SubMesh (the whole model) or a number of them
(one for each arm, leg, etc). Each has an array of MD5Tris, MD5Weights, and
MD5Verts. We also store the OpenGL names for the diffuse and bump textures 
here - don't worry if you don't know what a bump map is yet!
*/
struct MD5SubMesh {
	int numverts;			//How many vertices in this MD5SubMesh
	int numtris;			//How many triangles in this MD5SubMesh
	int numweights;			//How many weights in this MD5SubMesh
	unsigned int texIndex;	//OGL Name of the diffuse map (if any)

#ifdef	MD5_USE_TANGENTS_BUMPMAPS
	unsigned int bumpIndex;	//OGL Name of the bump map (if any)
#endif

	MD5Tri*		tris;		//Pointer to array of MD5Tris of this MD5SubMesh
	MD5Weight*	weights;	//Pointer to array of MD5Weights of this MD5SubMesh
	MD5Vert*	verts;		//Pointer to array of MD5Verts of this MD5SubMesh

	MD5SubMesh() {
		texIndex	= 0;
#ifdef	MD5_USE_TANGENTS_BUMPMAPS
		bumpIndex	= 0;
#endif
		tris		= NULL;
		weights		= NULL;
		verts		= NULL;
	}

	/*
	We should probably delete our heap memory...
	*/
	~MD5SubMesh() {
		delete[] tris;
		delete[] weights;
		delete[] verts;
	}
};

//Let the compiler know we should compile MD5Anim along with this class
class MD5Anim;


/*
Now for the actual class definition itself. We inherit the ability to store
Mesh instances as children from the ChildMeshInterface - this is so we don't
have to replicate code in the OBJMesh class, which can also have child meshes.

MD5Mesh is also a subclass of Mesh, meaning we get access to all of the usual
Mesh stuff you've been adding in as the tutorial series goes on.
*/
class MD5Mesh : public Mesh, public ChildMeshInterface	{
public:
	//The MD5Anim class works on the data of this class. We don't want any
	//other class messing around with its internal data though, so instead
	//of making accessor functions, we declare MD5Anim a friend class, meaning
	//we give it permission to fiddle with our internals. LOL etc.
	friend class MD5Anim;


	MD5Mesh(void);
	~MD5Mesh(void);

	/*
	Loads in an MD5Mesh from a given filename. Returns false if the loading
	failed, otherwise returns true.
	*/
	bool	LoadMD5Mesh(std::string filename);

	/*
	Draws the entire MD5Mesh, including its submeshes. Inherited from the 
	Mesh class, overloaded to support drawing of child meshes.
	*/
	virtual void Draw();

	/*
	Draws the underlying skeleton of the MD5Mesh, in its current pose. Points
	represent joints, lines represent the parent / child hierarchy - forming
	the 'bones' of the mesh.
	*/
	void	DrawSkeleton();	

	/*
	Adds an MD5Anim to the MD5Mesh's map of animations. This should probably
	return a bool to detect file loading success.
	*/
	void	AddAnim(std::string filename);

	/*
	Searches the map of animations for an MD5Anim with the passed in name, and
	starts applying it to the current MD5Mesh
	*/
	void	PlayAnim(std::string name);	


	/*
	Updates the skeleton according to the animation frames held in the current
	applied MD5Anim.
	*/
	void	UpdateAnim(float msec);	
				
protected:	
	/*
	Helper function used by LoadMD5Mesh to load in the joints for this mesh
	from an MD5Mesh file.
	*/
	int		LoadMD5Joints(std::ifstream &from);

	/*
	Helper function used by LoadMD5Mesh to load in the submeshes that make 
	up this mesh from an MD5Mesh file.
	*/
	void	LoadMD5SubMesh(std::ifstream &from, int &count);

	/*
	Once all of a MD5Mesh file's data has been loaded in, we can create the 
	various Mesh class instances required to render our new MD5Mesh, including
	setting up all of the VBOs and VAOs
	*/
	void	CreateMeshes();


	/*
	To draw an MD5Mesh in a pose, it must go through the process of vertex
	skinning. This function will skin the vertices according to the passed
	in MD5Skeleton, including skinning all of its submeshes.
	*/
	void	SkinVertices(MD5Skeleton &skel);	

	/*
	Once a skeleton has been moved to a new pose, the vertices must be
	skinned and transformed. This means we must rebuffer the VBOs to
	graphics memory.
	*/
	void	RebufferData();


	/*
	In Doom3 and other idTech4 games, each MD5Mesh has a 'shader' attached
	to it, containing all of the uniforms that should be sent to the game's
	active shader, and which textures should be used for which texture sampler.
	As we don't have anything quite so extravagant in this tutorial series,
	instead we have a series of shader 'proxy' files, containing two strings - 
	one for the diffuse map, and one for the bump map.
	*/
	void	LoadShaderProxy(std::string filename, MD5SubMesh &m);




	vector<string>	jointNames;			//Array of joint names for the skeleton

	unsigned int	currentAnimFrame;	//Current frame of animation
	unsigned int	numSubMeshes;		//How many submeshes this mesh has

	MD5Skeleton		bindPose;			//'Default' bindpose skeleton
	MD5Skeleton		currentSkeleton;	//'Working' skeleton, updated by anim

	MD5SubMesh*		subMeshes;			//array of MD5SubMeshes
	MD5Anim*		currentAnim;		//pointer to current active anim

	float	frameTime;					//How many msec until next frame change

	std::map<std::string, MD5Anim*>	animations;	//map of anims for this mesh


	/*
	idTech games (as well as Unreal engine games, and some others) don't use
	the standard axis' you'll be used to, pointing down positive X and having
	Y as 'up'. Instead, they point down negative X, and have Z as up. This means
	all of the data in the MD5Mesh file is 'incorrectly' rotated for the 
	standard OGL render space. We can transform everything to the 'correct' space
	using a simple transformation matrix. We only need one such matrix, and it
	never needs to change once created, so it is declared as static const.
	*/
	static const Matrix4 conversionMatrix;
};

