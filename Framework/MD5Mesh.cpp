#include "MD5Mesh.h"
/*
http://www.modwiki.net/wiki/MD5MESH_%28file_format%29
*/

/*
This is the actual matrix data that will be used for the space
conversion matrix. As you can see, it's like an identity matrix,
but swaps the Y and Z axis, and negates the X axis.
*/
static float	matrixElements[16] = {
	-1,  0, 0, 0,
	0,  0, 1, 0,
	0,  1, 0, 0,
	0,  0, 0, 1
};

/*
static class variables must still be instantiated somewhere!!!
*/
const Matrix4 MD5Mesh::conversionMatrix = Matrix4(matrixElements);

MD5Mesh::MD5Mesh() {
	numSubMeshes	 = 0;
	currentAnimFrame = 0;
	subMeshes		 = NULL;
	currentAnim		 = NULL;
	frameTime	     = 0.0f;
}

MD5Mesh::~MD5Mesh(void)	{
	//Delete any MD5 animations we have loaded for this mesh
	for(std::map<std::string,MD5Anim*>::iterator i = animations.begin(); i != animations.end(); ++i) {
		delete i->second;
	}

	delete[]subMeshes; //Clean up our heap!
}

/*
Draws the current MD5Mesh. The handy thing about overloaded virtual functions
is that they can still run the code they have 'overridden', by calling the 
parent class function as you would a static function. So all of the new stuff
you've been building up in the Mesh class as the tutorials go on, will 
automatically be used by this overloaded function. Once 'this' has been drawn,
all of the children of 'this' will be drawn
*/
void MD5Mesh::Draw() {
	Mesh::Draw();
	for(unsigned int i = 0; i < children.size(); ++i) {
		children[i]->Draw();
	}
};

/*
This function loads in the texture filenames for the current MD5Mesh.

Each MD5Mesh submesh has a 'shader' applied to it, which in idTech4 games,
will contain all of the texture filenames, and uniform data etc required
for whatever effect has been applied to the mesh. For this tutorial series,
we don't really care about all that, we just want the filenames. So instead
of trying to parse the Doom3 shader files, instead there's some 'proxy' shaders
in the Meshes folder, containing a couple of filenames. Each proxy has the same
filename as a shader reference in the MD5Mesh file, so the MD5Mesh doesn't have
to be modified at all. If you want to add more MD5Meshes to your projects, you
are probably going to have to look in the MD5Mesh file and see what 'shaders'
it references, and write proxy shaders for them.
*/
void	MD5Mesh::LoadShaderProxy(std::string filename, MD5SubMesh &m) {
	//The filename passed to this function might be a relative path, or maybe even
	//an absolute path. So what what we do is cut the folder structure from the 
	//filename string entirely, leaving only the filename we want. We also remove
	//the file extension, and replace it with .proxy, the file extension I chose
	//to keep our 'pretend' texture filenames in.
	filename = filename.substr(1,filename.size()-2);
	int at = filename.find_last_of('/');

	std::ifstream f("../Meshes/" + filename.substr(at+1) + ".proxy",std::ios::in);

	if(!f) {	//Oh dear.
		return;
	}

	/*
	Proxy files have a couple of strings, referencing the relative path of a diffuse map
	and a bump map. If you have a gloss map for an MD5Mesh, it should be pretty trivial
	to add support to load them from a proxy file, too.
	*/

	string  diffuseMap;
	f >> diffuseMap;
#ifdef MD5_USE_TANGENTS_BUMPMAPS
	string	bumpMap;
	f >> bumpMap;
#endif

	f.close();	//That's all that's in the file, so we can close it.


	//Load in the textures using SOIL. As long as we actually delete the MD5Mesh, these
	//textures will eventually be deleted from the OGL context, as they'll end up applied
	//to the Mesh texture values.
	m.texIndex = SOIL_load_OGL_texture(
		diffuseMap.c_str(),
		SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS | SOIL_FLAG_COMPRESS_TO_DXT
	);

#ifdef MD5_USE_TANGENTS_BUMPMAPS
	m.bumpIndex = SOIL_load_OGL_texture(
		bumpMap.c_str(),
		SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS | 0
	);
#endif
}

/*
Loads MD5Mesh data in from a file with the given filename. Returns false if this fails,
otherwise returns true.
*/
bool	MD5Mesh::LoadMD5Mesh(std::string filename)	{
	std::ifstream f(filename,std::ios::in);	//MD5 files are text based, so don't make it an ios::binary ifstream...

	if(!f) {
		return false; //Oh dear!
	}

	//We have our MD5 file handle!
	int numExpectedJoints = 0;
	int numExpectedMeshes = 0;
	int md5Version		  = 0;
	int numLoadedMeshes   = 0;
	int numLoadedJoints   = 0;

	/*
	Now we simply load in the data, while there's still data to load
	*/
	while(!f.eof()) {
		std::string currentLine;	//A temporary string to keep the current line in
		f >> currentLine;			//stream the next line of the file in

		if(currentLine.empty()) {	//Actually, there's nothing in this line...usually
			continue;				//because we've hit the end of the file.
		}
		/*
		String::Find returns a value, equating to the position in the string the search 
		string is - or the special value 'npos' if the searchstring is not found at all
		*/

 		else if(currentLine.find(MD5_VERSION_TAG) != std::string::npos) {
			//We've found the MD5 version string!
			//ifstream allows us to stream ints,floats etc into variables
			f >> md5Version;
			std::cout << "MD5 File version is: " << md5Version << std::endl;
		}
		else if(currentLine.find(MD5_COMMANDLINE_TAG) != std::string::npos) {
			/*
			MD5Mesh files sometimes have a 'command line' value, used by the game
			toolchain to generate some data. We don't care about it!
			*/
			std::cout << "Ignoring commandline value" << std::endl;
			getline(f,currentLine);
		}
		else if(currentLine.find(MD5_NUMJOINTS_TAG) != std::string::npos) {
			f >> numExpectedJoints; //Load in the number of joints held in this MD5Mesh file
			std::cout << "Expecting file to have " << numExpectedJoints << " joints" << std::endl;
			//grab enough space for this number of joints
			bindPose.joints = new MD5Joint[numExpectedJoints];
		}
		else if(currentLine.find(MD5_NUMMESHES_TAG) != std::string::npos) {
			f >> numExpectedMeshes; //load in the number of submeshes held in this md5mesh
			std::cout << "Expecting file to have " << numExpectedMeshes << " meshes" << std::endl;

			subMeshes = new MD5SubMesh[numExpectedMeshes];
		}
		else if(currentLine.find(MD5_JOINTS_TAG) != std::string::npos) {
			numLoadedJoints += LoadMD5Joints(f); //Load in as many joints as possible
		}
		else if(currentLine.find(MD5_MESH_TAG) != std::string::npos) {
			LoadMD5SubMesh(f,numLoadedMeshes);	//Load in a submesh
			++numLoadedMeshes;
		}
	}

	bindPose.numJoints = numLoadedJoints;
	numSubMeshes	   = numLoadedMeshes;

	//If we get to here, we've loaded in everything from the file, so we can close it
	f.close();

	/*
	Now we have a 'bind pose' skeleton, which is handy. What we're also going to do is mempy
	this skeleton, so we have a 'scratch' skeleton we can pass around to MD5Anims to modify it,
	without losing the ability to draw the mesh in the bind pose.
	*/

	currentSkeleton.numJoints = bindPose.numJoints;
	currentSkeleton.joints = new MD5Joint[bindPose.numJoints];

	//Don't bother doing any file parsing, just memcpy the joints straight into the currentSkeleton!
	memcpy((void*)currentSkeleton.joints,(void*)bindPose.joints,sizeof(MD5Joint)*bindPose.numJoints);

	//If what we've loaded in does not equal what we /should/ have loaded in, we'll output an error
	//
	if(numLoadedJoints != numExpectedJoints) {
		std::cout << "Expected " << numExpectedJoints << " joints, but loaded " << numLoadedJoints << std::endl;
		return false;
	}

	if(numLoadedMeshes != numExpectedMeshes) {
		std::cout << "Expected " << numExpectedMeshes << " meshes, but loaded " << numLoadedMeshes << std::endl;
		return false;
	}

	//Everything is OK! let's create our submeshes :)
	CreateMeshes();

	return true;
}

/*
Loads in the MD5Mesh joint hierarchy. Uses the same ifstream as LoadMD5Mesh, passed
by reference. This function will return how many joints were loaded.
*/
int MD5Mesh::LoadMD5Joints( std::ifstream &from )	{
	/*
	The joints section of the file should look something like this...

	joints {
		"name"	parent ( pos.x pos.y pos.z ) ( quat.x quat.y quat.z )	//
		...more things
	}

	"joints" is parse by LoadMD5Mesh, so the first thing this function should see
	is a brace
	*/

	int loaded = 0;
	char skipChar;

	std::string tempLine; //Another temporary line to stream things into...

	do {
		from >> tempLine;	//Stream a line in
		
		if(tempLine == "{") {//In a well-behaved MD5 file, the first line will be '{'
		}
		else if(tempLine[0] == '"'){//Name of joint begins with a "
			//Substring cuts the name out from between the speech marks!
			//We keep the actual strings separate, and store a pointer to the string
			//in the joint (Avoids nasty string problems when using memcpy)
			jointNames.push_back(tempLine.substr(1,tempLine.find('"',1)-1)); 
			bindPose.joints[loaded].name = &jointNames.back();

			from >> bindPose.joints[loaded].parent;
			from >> skipChar; //first vec beginning bracket
			from >> bindPose.joints[loaded].position.x;
			from >> bindPose.joints[loaded].position.y;
			from >> bindPose.joints[loaded].position.z;
			from >> skipChar; //first vec end bracket
			from >> skipChar; //second vec beginning bracket
			from >> bindPose.joints[loaded].orientation.x;
			from >> bindPose.joints[loaded].orientation.y;
			from >> bindPose.joints[loaded].orientation.z;
			from >> skipChar; //second vec beginning bracket

			/*
			To save a tiny bit of space, the 4th component of the orientation
			quaternion is left out of the files. As we're dealing with unit length 
			quaternions (i.e they have a length of 1), the 4th component will be 
					sqrt of (1.0 - length of the other 3 components)
			*/

			bindPose.joints[loaded].orientation.GenerateW();
			bindPose.joints[loaded].orientation.Normalise();

			//Now we have the orientation and position, we can form the transformation matrix
			//for this joint. We need to further transform this matrix by the conversionmatrix
			//so that the rotation is in OpenGL space.

			bindPose.joints[loaded].transform = bindPose.joints[loaded].orientation.ToMatrix();
			bindPose.joints[loaded].transform.SetPositionVector(bindPose.joints[loaded].position);

			bindPose.joints[loaded].transform = conversionMatrix * bindPose.joints[loaded].transform;

			++loaded;	//...Just assume it worked ;)
		}
	}while(tempLine != "}");

	return loaded; //Return how many joints we loaded in
}

/*

*/
void MD5Mesh::LoadMD5SubMesh( std::ifstream &from, int &count )	{
	/*
	The submesh section of the file should look like this:

	mesh {
		shader shadername

		numverts numverts

		vert num ( tex.x tex.y ) weightindex weightcount
		...more verts

		numtris numtris
		tri trinum vertA vertB vertC
		...more tris

		numweights numweights
		weight weightnum jointnum weightval ( pos.x pos.y pos.z )
	}
	*/

	char skipChar;			//We skip the brackets by streaming them into this
	std::string tempLine;	//Another temporary line to stream things into...

		
	int vertsLoaded		= 0;				//Number of vertices actually loaded
	int trisLoaded		= 0;				//Number of tris actually loaded
	int weightsLoaded	= 0;				//number of weights actually loaded

	//We created the memory for the submeshes earlier, so we can just grab a reference
	//to the next available submesh...this'll go wrong if the MD5Mesh is invalid!
	MD5SubMesh& m		= subMeshes[count];

	do {
		from >> tempLine;	

		if(tempLine == MD5_SUBMESH_SHADER) {
			//If the line is a shader, we let the LoadShaderProxy function handle it
			std::string shaderName;
			from >> shaderName;
			LoadShaderProxy(shaderName,m);
		}
		else if(tempLine == MD5_SUBMESH_NUMVERTS) {
			//if the line tells us how many vertices to expect, initialise the memory for them
			from >> m.numverts;
			m.verts = new MD5Vert[m.numverts];
		}
		else if(tempLine == MD5_SUBMESH_NUMTRIS) {
			//if the line tells us how many tris to expect, initialise the memory for them
			from >> m.numtris;
			m.tris = new MD5Tri[m.numtris];
		}
		else if(tempLine == MD5_SUBMESH_NUMWEIGHTS) {
			//if the line tells us how many weights to expect, initialise the memory for them
			from >> m.numweights;
			m.weights = new MD5Weight[m.numweights];
		}
		else if(tempLine == MD5_SUBMESH_VERT) {
			//if the line is a vertex, load it in
			from >> m.verts[vertsLoaded].vertIndex;
			
			from >> skipChar; //vec beginning bracket
			from >> m.verts[vertsLoaded].texCoords.x;
			from >> m.verts[vertsLoaded].texCoords.y;
			from >> skipChar; //vec end bracket

			from >> m.verts[vertsLoaded].weightIndex;
			from >> m.verts[vertsLoaded].weightElements;

			vertsLoaded++;
		}
		else if(tempLine == MD5_SUBMESH_WEIGHT) {
			//if the line is a weight, load it in
			from >> m.weights[weightsLoaded].weightIndex;
			from >> m.weights[weightsLoaded].jointIndex;
			from >> m.weights[weightsLoaded].weightValue;

			from >> skipChar; //vec beginning bracket
			from >> m.weights[weightsLoaded].position.x;
			from >> m.weights[weightsLoaded].position.y;
			from >> m.weights[weightsLoaded].position.z;
			from >> skipChar; //vec end bracket

			weightsLoaded++;
		}
		else if(tempLine == MD5_SUBMESH_TRI) {
			//if the line is a triangle, load it in
			from >> m.tris[trisLoaded].triIndex;
			from >> m.tris[trisLoaded].a;
			from >> m.tris[trisLoaded].b;
			from >> m.tris[trisLoaded].c;

			trisLoaded++;
		}
		else{
			//Perhaps different MD5 files have other data? Or maybe the file is screwed...
			if(tempLine != "}" && tempLine != "{")	{
				std::cout << "Unknown MD5 file tag: " << tempLine << std::endl;
			}
		}
	}while(tempLine != "}");
}

/*
Create the child Mesh class instances from the loaded in MD5SubMeshes.
*/
void MD5Mesh::CreateMeshes()	{
	for(unsigned int i = 0; i < numSubMeshes; ++i) {
		MD5SubMesh& subMesh = subMeshes[i]; //Reference to the current submesh

		//What this does is this (lol): We want the first 'submesh' to put the 
		//vertex data in 'this', and all other submeshes in 'children' of 'this'
		//We could keep 'this' empty and put everything into child meshes,
		//but that seems slightly wasteful (and confusing when debugging)

		MD5Mesh*target = this;

		//Also, MD5Mesh can't access the protected member variables of Mesh...
		//But we don't really want to have MD5Meshes as children, as that's
		//a waste of resources (MD5Meshes have lots of extra crap in them)
		//so we cheat MASSIVELY by using a reinterpret cast to access its 
		//variables as if if the Mesh was an MD5Mesh (which we CAN access
		//protected variables of. This SHOULD be safe, as MD5Mesh is a subclass
		//of Mesh, and therefore will have the correct member variables, in
		//the place they should be. Still nasty, though.
		if(i != 0) {
			target = reinterpret_cast<MD5Mesh*>(new Mesh());
		}

		target->texture		  = subMesh.texIndex;	//Assign the diffuse map
		target->vertices	  = new Vector3[subMesh.numverts]; //Make vertex mem
		target->textureCoords = new Vector2[subMesh.numverts]; //Make texCoord mem

#ifdef MD5_USE_NORMALS
		//Create space for normals!
		target->normals		  = new Vector3[subMesh.numverts];
#endif 

#ifdef MD5_USE_TANGENTS_BUMPMAPS
		//Create space for tangents, and assign the bump texture
		target->bumpTexture	  = subMesh.bumpIndex;	
		target->tangents	  = new Vector3[subMesh.numverts];
#endif

		target->numIndices    = subMesh.numtris*3; //Each tri has 3 points....
		target->numVertices   = subMesh.numverts;

		target->indices		  = new unsigned int[target->numIndices]; //Make mem for indices

		/*
		Here we go through each tri, and put its indices in the Mesh index buffer. You'll see
		we have a weird ordering here, doing cba, rather than abc. MD5 triangles have a 
		/clockwise/ winding, whereas OGL expects anticlockwise to be 'forward facing' for
		a triangle. So, we simply reverse the order of indices to make the tri anticlockwise
		*/
		for(int j = 0; j < subMesh.numtris; ++j) {
			target->indices[(j*3)]   = subMesh.tris[j].c;
			target->indices[(j*3)+1] = subMesh.tris[j].b;
			target->indices[(j*3)+2] = subMesh.tris[j].a;
		}

		//If we added 'this' as a child of itself, we'd create an infinite loop
		//in the MD5Mesh::Draw function...that's not very good.
		if(target != this) {
			AddChild(target);
		}

		//Make the VAO and VBOs for this submesh!
		target->BufferData();
	}

	//Once all of the submeshes are created, we should skin the mesh into the bindpose
	SkinVertices(bindPose);
}

/*
Skins each vertex by its weightings, producing a final skinned mesh in the passed in
skeleton pose. 
*/
void	MD5Mesh::SkinVertices(MD5Skeleton &skel) {
	//For each submesh, we want to transform a position for each vertex
	for(unsigned int i = 0; i < numSubMeshes; ++i) {
		MD5SubMesh& subMesh = subMeshes[i];	//Get a reference to the current submesh
		/*
		Each MD5SubMesh targets a Mesh's data. The first submesh will target 'this', 
		while subsequent meshes will target the children of 'this'
		*/
		MD5Mesh*target		= this;
		if(i != 0) {
			target = (MD5Mesh*)children.at(i-1);
		}

		/*
		For each vertex in the submesh, we want to build up a final position, taking
		into account the various weighting anchors used.
		*/
		for(int j = 0; j < subMesh.numverts; ++j) {
			//UV coords can be copied straight over to the Mesh textureCoord array
			target->textureCoords[j]   = subMesh.verts[j].texCoords;

			//And we should start off with a Vector of 0,0,0
			target->vertices[j].ToZero();

			/*
			Each vertex has a number of weights, determined by weightElements. The first
			of these weights will be in the submesh weights array, at position weightIndex.

			Each of these weights has a joint it is in relation to, and a weighting value,
			which determines how much influence the weight has on the final vertex position

			
			*/

			for(int k = 0; k < subMesh.verts[j].weightElements; ++k) {
				MD5Weight& weight	= subMesh.weights[subMesh.verts[j].weightIndex + k];
				MD5Joint& joint		= skel.joints[weight.jointIndex];

				/*
				We can then transform the weight position by the joint's world transform, and multiply
				the result by the weightvalue. Finally, we add this value to the vertex position, eventually
				building up a weighted vertex position.
				*/

				target->vertices[j] += ((joint.transform * weight.position) * weight.weightValue);				
			}
		}

		/*
		As our vertices have moved, normals and tangents must be regenerated!
		*/
#ifdef MD5_USE_NORMALS
		target->GenerateNormals();
#endif

#ifdef MD5_USE_TANGENTS_BUMPMAPS
		target->GenerateTangents();
#endif
		/*
		Finally, as our vertex attributes data has changed, we must rebuffer the data to 
		graphics memory.
		*/
		target->RebufferData();
	}
}

/*
Rebuffers the vertex data on the graphics card. Now you know why we always keep hold of
our vertex data in system memory! This function is actually entirely covered in the 
skeletal animation tutorial text (unlike the other functions, which are kept as 
pseudocode). This should be in the Mesh class really, as it's a useful function to have.

*/
void MD5Mesh::RebufferData()	{
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[VERTEX_BUFFER]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices*sizeof(Vector3), (void*)vertices);

	if(textureCoords) {
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[TEXTURE_BUFFER]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices*sizeof(Vector2), (void*)textureCoords);
	}

	if (colours)	{
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[COLOUR_BUFFER]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices*sizeof(Vector4), (void*)colours);
	}

#ifdef MD5_USE_NORMALS
	if(normals) {
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[NORMAL_BUFFER]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices*sizeof(Vector3), (void*)normals);
	}
#endif

#ifdef MD5_USE_TANGENTS_BUMPMAPS
	if(tangents) {
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[TANGENT_BUFFER]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices*sizeof(Vector3), (void*)tangents);
	}
#endif

	if(indices) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject[INDEX_BUFFER]);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numVertices*sizeof(unsigned int), (void*)indices);
	}
}

/*
Draws the current skeleton for this MD5Mesh, using GL_LINES and GL_POINTS. Temporarily creates
a VBO and VAO, puts the joint positions in it, and draws it. An example of a use for
GL_STREAM_DRAW.

*/
void MD5Mesh::DrawSkeleton()	{	
	//Temporary VAO and VBO
	unsigned int skeletonArray;
	unsigned int skeletonBuffer;

	glGenVertexArrays(1, &skeletonArray);
	glGenBuffers(1, &skeletonBuffer);

	//Temporary chunk of memory to keep our joint positions in
	Vector3*	 skeletonVertices = new Vector3[bindPose.numJoints*2];


	/*
	Now for each joint we're going to have a pair of vertices - one at the joint position,
	and one at the joint's parent's position. This'll let us draw lines to show the skeletal
	shape. There'll be a bit of overdraw, which could be avoided by using indices. but this way
	is 'good enough'
	*/
	for(int i = 0; i < currentSkeleton.numJoints; ++i) {
		skeletonVertices[i*2] = conversionMatrix * currentSkeleton.joints[i].position;

		if(currentSkeleton.joints[i].parent == -1) {	//No parent, but to keep this simple we'll copy the position again...
			skeletonVertices[(i*2)+1] = conversionMatrix * currentSkeleton.joints[i].position;
		}
		else{
			skeletonVertices[(i*2)+1] = conversionMatrix * currentSkeleton.joints[currentSkeleton.joints[i].parent].position;
		}
	}

	//You should know what this all does by now, except we combine it with the draw operations in a single function
	glBindVertexArray(skeletonArray);
	glBindBuffer(GL_ARRAY_BUFFER, skeletonBuffer);
	glBufferData(GL_ARRAY_BUFFER, bindPose.numJoints*sizeof(Vector3)*2, skeletonVertices, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(0);

	glBindVertexArray(skeletonArray);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Draws the array twice, once as points, and once as lines. glLineWidth may or may not actually do anything
	//as it is deprecated functionality in OGL 3.2. 
	glPointSize(5.0f);
	glLineWidth(2.0f);
	glDrawArrays(GL_POINTS, 0, bindPose.numJoints*2);	// draw Joints
	glDrawArrays(GL_LINES,  0, bindPose.numJoints*2);	// draw Bones
	glPointSize(1.0f);
	glLineWidth(1.0f);

	glBindVertexArray(0);

	//Delete the VBO and VAO, and the heap memory we allocated earlier
	glDeleteVertexArrays(1, &skeletonArray);
	glDeleteBuffers(1, &skeletonBuffer);
	delete[]skeletonVertices;
}

/*
Adds an MD5Anim to this MD5Mesh, so we can animate it. We store each
MD5Anim in a map, using its filename as a key.
*/
void	MD5Mesh::AddAnim(std::string filename) {
	animations.insert(std::make_pair(filename,new MD5Anim(filename)));
}

/*
Swaps the currently used animation of this MD5Mesh. 
*/
void	MD5Mesh::PlayAnim(std::string name)	{
	/*
	We want to reset all of the animation details
	*/
	currentAnim		 = NULL;
	currentAnimFrame = 0;
	frameTime		 = 0.0f;

	//Go through the map and find the animation by its filename

	std::map<std::string, MD5Anim*>::iterator i =  animations.find(name);

	if(i != animations.end()) {
		currentAnim = (*i).second;	//We have an animation, play it!
	}
}

/*
Updates the MD5Mesh, and determines whether a new frame of animation
should be applied to the skeleton, and reskins the vertices.
*/
void	MD5Mesh::UpdateAnim(float msec) {
	//If we actually have an animation...
	if(currentAnim) {
		frameTime -= msec;

		bool reskin = false;

		//Just so we don't 'lose' frames by calculating a wrong frame when
		//msec is a large number (breakpoints / low fps / etc)
		while(frameTime < 0) {
			frameTime += 1000.0f / currentAnim->GetFrameRate();
			currentAnimFrame = currentAnimFrame++%(currentAnim->GetNumFrames());
			reskin = true;
		}
		//If a new animation frame is used, we should transform our 'working' skeleton
		//and reskin the vertices around it.
		if(reskin) {
			currentAnim->TransformSkeleton(currentSkeleton,currentAnimFrame-1);
			SkinVertices(currentSkeleton);	
		}
	}
}