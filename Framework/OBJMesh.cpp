#include "OBJMesh.h"

/*
OBJ files look generally something like this:

v xpos ypos zpos
..more vertices

vt xtex ytex
...more texcoords

vn xnorm ynorm znorm
...more normals

f vert index / tex index / norm index  vert index / tex index / norm index  vert index / tex index / norm index
...more faces

(i.e there's a set of float/float/float for each vertex of a face)

OBJ files can also be split up into a number of submeshes, making loading them
in even more annoying. 
*/
bool OBJMesh::LoadOBJMesh(std::string filename)	{
	std::ifstream f(filename.c_str(), std::ios::in);

	if (!f) { //Oh dear, it can't find the file
		return false;
	}

	/*
	Stores the loaded in vertex attributes
	*/
	std::vector<Vector2>inputTexCoords;
	std::vector<Vector3>inputVertices;
	std::vector<Vector3>inputNormals;

	/*
	SubMeshes temporarily get kept in here
	*/
	std::vector<OBJSubMesh*> inputSubMeshes;

	OBJSubMesh* currentMesh = new OBJSubMesh();
	inputSubMeshes.push_back(currentMesh);	//It's safe to assume our OBJ will have a mesh in it ;)

	while (!f.eof()) {
		std::string currentLine;
		f >> currentLine;

		if (currentLine == OBJCOMMENT) {		//This line is a comment, ignore it
			continue;
		}
		else if (currentLine == OBJMESH /*|| currentLine == OBJUSEMTL*/ || currentLine == OBJOBJECT) {	//This line is a submesh!
			currentMesh = new OBJSubMesh();
			inputSubMeshes.push_back(currentMesh);
		}
		else if (currentLine == OBJVERT) {	//This line is a vertex
			Vector3 vertex;
			f >> vertex.x; f >> vertex.y; f >> vertex.z;
			inputVertices.push_back(vertex);
		}
		else if (currentLine == OBJNORM) {	//This line is a Normal!
			Vector3 normal;
			f >> normal.x; f >> normal.y; f >> normal.z;
			inputNormals.push_back(normal);
		}
		else if (currentLine == OBJTEX) {	//This line is a texture coordinate!
			Vector2 texCoord;
			f >> texCoord.x; f >> texCoord.y;
			/*
			TODO! Some OBJ files might have 3D tex coords...
			*/
			inputTexCoords.push_back(texCoord);
		}
		else if (currentLine == OBJFACE) {	//This is an object face!
			if (!currentMesh) {
				inputSubMeshes.push_back(new OBJSubMesh());
				currentMesh = inputSubMeshes[inputSubMeshes.size() - 1];
			}

			std::string			faceData;		//Keep the entire line in this!
			std::vector<int>	faceIndices;	//Keep the extracted indices in here!
		
			getline(f, faceData);		//Use a string helper function to read in the entire face line

			/*
			It's possible an OBJ might have normals defined, but not tex coords!
			Such files should then have a face which looks like this:

				f <vertex index>//<normal index>
				instead of <vertex index>/<tex coord>/<normal index>

				you can be some OBJ files will have "/ /" instead of "//" though... :(
			*/
			bool	skipTex = false;
			if(faceData.find("//") != std::string::npos) {	
				skipTex = true;
			}

			/*
			Count the number of slashes, but also convert the slashes to spaces
			so that string streaming of ints doesn't fail on the slash

				"f  0/0/0" becomes "f 0 0 0" etc
			*/
			for(size_t i = 0; i < faceData.length(); ++i) {
				if(faceData[i] == '/') {
					faceData[i] = ' ';
				}
			}

			/*
			Now string stream the indices from the string into a temporary
			vector.
			*/
			int tempIndex;
			std::stringstream	ss(faceData);
			while(ss >> tempIndex) {
				faceIndices.push_back(tempIndex);
			}

			//If the face indices vector is a multiple of 3, we're looking at triangles
			//with some combination of vertices, normals, texCoords
			if (faceIndices.size()%3 == 0) {		//This face is a triangle (probably)!
				if (faceIndices.size() == 3) {	//This face has only vertex information;
					currentMesh->vertIndices.push_back(faceIndices.at(0));
					currentMesh->vertIndices.push_back(faceIndices.at(1));
					currentMesh->vertIndices.push_back(faceIndices.at(2));
				}
				else if (faceIndices.size() == 9) {	//This face has vertex, normal and tex information!
					for (int i = 0; i < 9; i += 3) {
						currentMesh->vertIndices.push_back(faceIndices.at(i));
						currentMesh->texIndices.push_back(faceIndices.at(i+1));
						currentMesh->normIndices.push_back(faceIndices.at(i+2));
					}
				}
				else if (faceIndices.size() == 6) {	//This face has vertex, and one other index...
					for (int i = 0; i < 6; i += 2) {
						currentMesh->vertIndices.push_back(faceIndices.at(i));
						if (!skipTex) {		// a double slash means it's skipping tex info...
							currentMesh->texIndices.push_back(faceIndices.at(i+1));
						}
						else {
							currentMesh->normIndices.push_back(faceIndices.at(i+1));
						}
					}
				}
			}
			else if (faceIndices.size()%4 == 0) {	// This face is a quad
				if (faceIndices.size() == 4) {	// This face has only vertex information
					// first triangle
					currentMesh->vertIndices.push_back(faceIndices.at(0));
					currentMesh->vertIndices.push_back(faceIndices.at(1));
					currentMesh->vertIndices.push_back(faceIndices.at(2));

					// second triangle
					currentMesh->vertIndices.push_back(faceIndices.at(0));
					currentMesh->vertIndices.push_back(faceIndices.at(2));
					currentMesh->vertIndices.push_back(faceIndices.at(3));
				}
				else if (faceIndices.size() == 12) {	// This face has vertex, normal and tex information
					int v[4], vt[4], vn[4];
					unsigned int index = 0;
					
					for (int i = 0; i < 12; i +=3) {
						v[index] = faceIndices.at(i);
						vt[index] = faceIndices.at(i+1);
						vn[index] = faceIndices.at(i+2);
						++index;
					}

					// first tirangle
					currentMesh->vertIndices.push_back(v[0]);
					currentMesh->texIndices.push_back(vt[0]);
					currentMesh->normIndices.push_back(vn[0]);

					currentMesh->vertIndices.push_back(v[1]);
					currentMesh->texIndices.push_back(vt[1]);
					currentMesh->normIndices.push_back(vn[1]);

					currentMesh->vertIndices.push_back(v[2]);
					currentMesh->texIndices.push_back(vt[2]);
					currentMesh->normIndices.push_back(vn[2]);

					// second triangle
					currentMesh->vertIndices.push_back(v[0]);
					currentMesh->texIndices.push_back(vt[0]);
					currentMesh->normIndices.push_back(vn[0]);

					currentMesh->vertIndices.push_back(v[2]);
					currentMesh->texIndices.push_back(vt[2]);
					currentMesh->normIndices.push_back(vn[2]);

					currentMesh->vertIndices.push_back(v[3]);
					currentMesh->texIndices.push_back(vt[3]);
					currentMesh->normIndices.push_back(vn[3]);
				}
				else if (faceIndices.size() == 8) {		// This face has vertex, and one other index...
					int v[4], u[4];
					unsigned int index = 0;

					for (int i = 0; i < 8; i+=2) {
						v[index] = faceIndices.at(i);
						u[index] = faceIndices.at(i+1);
						++index;
					}

					// first triangle
					currentMesh->vertIndices.push_back(v[0]);
					if (!skipTex) {		// a double slash means it's skipping tex info...
						currentMesh->texIndices.push_back(u[0]);
					}
					else {
						currentMesh->normIndices.push_back(u[0]);
					}

					currentMesh->vertIndices.push_back(v[1]);
					if (!skipTex) {		// a double slash means it's skipping tex info...
						currentMesh->texIndices.push_back(u[1]);
					}
					else {
						currentMesh->normIndices.push_back(u[1]);
					}

					currentMesh->vertIndices.push_back(v[2]);
					if (!skipTex) {		// a double slash means it's skipping tex info...
						currentMesh->texIndices.push_back(u[2]);
					}
					else {
						currentMesh->normIndices.push_back(u[2]);
					}

					// second triangle
					currentMesh->vertIndices.push_back(v[0]);
					if (!skipTex) {		// a double slash means it's skipping tex info...
						currentMesh->texIndices.push_back(u[0]);
					}
					else {
						currentMesh->normIndices.push_back(u[0]);
					}

					currentMesh->vertIndices.push_back(v[2]);
					if (!skipTex) {		// a double slash means it's skipping tex info...
						currentMesh->texIndices.push_back(u[2]);
					}
					else {
						currentMesh->normIndices.push_back(u[2]);
					}

					currentMesh->vertIndices.push_back(v[3]);
					if (!skipTex) {		// a double slash means it's skipping tex info...
						currentMesh->texIndices.push_back(u[3]);
					}
					else {
						currentMesh->normIndices.push_back(u[3]);
					}
				}
			}
			else {
				//Uh oh! Face isn't a triangle nor a quad. Have fun adding stuff to this ;)
				std::cout << "OBJMesh::LoadOBJMesh Face isn't a triangle nor a quad." << currentLine << std::endl;
			}
		}
		else {
			//std::cout << "OBJMesh::LoadOBJMesh Unknown file data:" << currentLine << std::endl;
		}
	}

	f.close();

	//We now have all our mesh data loaded in...Now to convert it into OpenGL vertex buffers!
	for(unsigned int i = 0; i < inputSubMeshes.size(); ++i) {
		OBJSubMesh*sm = inputSubMeshes[i];

		/*
		We're going to be lazy and turn the indices into an absolute list
		of vertex attributes (it makes handling the submesh list easier)
		*/
		if(!sm->vertIndices.empty()) {
			OBJMesh*m		= new OBJMesh();

			m->numVertices	= sm->vertIndices.size();

			m->vertices		= new Vector3[m->numVertices];
			for(unsigned int j = 0; j < sm->vertIndices.size(); ++j) {
				m->vertices[j] = inputVertices[sm->vertIndices[j]-1];
			}

			if(!sm->texIndices.empty())	{
				m->textureCoords	= new Vector2[m->numVertices];
				for(unsigned int j = 0; j < sm->texIndices.size(); ++j) {
					m->textureCoords[j] = inputTexCoords[sm->texIndices[j]-1];
				}
			}

#ifdef OBJ_USE_NORMALS
			if(sm->normIndices.empty()) {
				m->GenerateNormals();
			}
			else{
				m->normals		= new Vector3[m->numVertices];

				for(unsigned int j = 0; j < sm->normIndices.size(); ++j) {
					m->normals[j] = inputNormals[sm->normIndices[j]-1];
				}
			}
#endif
#ifdef OBJ_USE_TANGENTS_BUMPMAPS
			m->GenerateTangents();
#endif

			m->BufferData();
			AddChild(m);
		}
		delete inputSubMeshes[i];
	}
	return true;
}

/*
Draws the current OBJMesh. The handy thing about overloaded virtual functions
is that they can still run the code they have 'overridden', by calling the 
parent class function as you would a static function. So all of the new stuff
you've been building up in the Mesh class as the tutorials go on, will 
automatically be used by this overloaded function. Once 'this' has been drawn,
all of the children of 'this' will be drawn
*/
void OBJMesh::Draw() {
	Mesh::Draw();
	for(unsigned int i = 0; i < children.size(); ++i) {
		if (texture) {
			children.at(i)->SetTexture(texture);
		}
		if (bumpTexture) {
			children.at(i)->SetBumpMap(bumpTexture);
		}
		children.at(i)->Draw();
	}
};