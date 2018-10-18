// Robert Lach
// Simple OBJ loader + Model Renderer
//
// Utilizes some nVidia SDK helper functions, glut, and gl helper.



#if defined(WIN32)
#  include <windows.h>
#  pragma warning (disable : 4786)
#endif

#define GLH_EXT_SINGLE_FILE

//GLH Extensions
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>


//Some basic helper functions I found so I wouldn't have to repeat busy work.
#include <utils/data_path.h>
#include <utils/read_text_file.h>


#include <iostream>
#include <fstream> 
#include <vector> 
#include <sstream> 

using namespace glh;


//These will be the functions we'll be using
void display();
void idle();
void resize(int w, int h);
void key(unsigned char k, int x, int y);
void special(int k, int x, int y);
void init_opengl();
void menu(int i) { key((unsigned char) i, 0, 0); }


//Shader Stuff
GLhandleARB vertexLighting = 0;
GLhandleARB fragmentLighting = 0;
GLhandleARB curProgram = 0;

float scale = 1.0f;


//Lighting colors
float rSpec = 1.0f;
float gSpec = 1.0f;
float bSpec = 1.0f;
float rDiff = 0.0f;
float gDiff = 0.0f;
float bDiff = 1.0f;
float rDiv = 0.01f;
float gDiv = 0.02f;
float bDiv = 0.03f;
bool CycleRGB = false;

int curModel = 0;		//Current Model Number
string filePaths[4];	//Filepaths to models


// Model Data----------------------------------------
typedef struct
{
	float x,y,z;
} ObjVertex;


typedef struct
{
	float x,y,z;
} ObjNormal;


typedef struct
{
	float u,v;
} ObjTexCoord;


typedef struct
{
	unsigned int m_aVertexIndices[3],		//	Vertex Indices
		m_aNormalIndices[3],		//	Normal Indices
		m_aTexCoordIndicies[3];	//	Texture Coords
} ObjFace;

typedef struct _ObjMesh
{
	ObjVertex		*m_aVertexArray;		//	Mesh's vertex array
	ObjNormal		*m_aNormalArray;		//	Mesh's normal array
	ObjTexCoord		*m_aTexCoordArray;		//	Mesh's Texture Coord array
	ObjFace			*m_aFaces;				//	Mesh's face array

	unsigned int	 m_iNumberOfVertices,	//	Counters...
		m_iNumberOfNormals,	
		m_iNumberOfTexCoords,	
		m_iNumberOfFaces;		


} ObjMesh;


ObjMesh*	MakeOBJ(void);
ObjMesh		 *pMesh;	//Our Currently Loaded Mesh

bool modelLoaded;
//------------------------------------END MODEL DATA


// Model Loader-------------------------------------
ObjMesh *MakeOBJ( void )
{

	ObjMesh *pMesh = NULL;


	pMesh = (ObjMesh*) malloc (sizeof(ObjMesh));

	pMesh->m_aFaces				= NULL;
	pMesh->m_aNormalArray		= NULL;
	pMesh->m_aTexCoordArray		= NULL;
	pMesh->m_aVertexArray		= NULL;
	pMesh->m_iNumberOfFaces		= 0;
	pMesh->m_iNumberOfNormals	= 0;
	pMesh->m_iNumberOfTexCoords = 0;
	pMesh->m_iNumberOfVertices	= 0;

	return pMesh;
}

int LoadOBJ(const char *filename)
{
	unsigned int vc=0,nc=0,tc=0,fc=0;
	char buffer[256];
	FILE *fp = NULL;


	fp = fopen(filename,"r");


	if(!fp)					//File Not Found
		return 0;


	// Create our mesh in memory
	pMesh = MakeOBJ();


	while(!feof(fp))
	{
		fgets(buffer,256,fp);

		if( strncmp("vn ",buffer,3) == 0 )	//Searching for Vertex Normals
		{
			++pMesh->m_iNumberOfNormals;
		}
		else


			if( strncmp("vt ",buffer,3) == 0 )	//Searching for Texture Coords
			{
				++pMesh->m_iNumberOfTexCoords;
			}
			else

				if( strncmp("v ",buffer,2) == 0 )	//Searching for Vertices
				{
					++pMesh->m_iNumberOfVertices;
				}
				else

					if( strncmp("f ",buffer,2) == 0 )	//Searching for Faces
					{
						++pMesh->m_iNumberOfFaces;
					}
	}

	fclose(fp);

	// MEMORY ALLOCATION
	pMesh->m_aVertexArray	= (ObjVertex*  )malloc( pMesh->m_iNumberOfVertices	* sizeof(ObjVertex)	  );
	pMesh->m_aNormalArray	= (ObjNormal*  )malloc( pMesh->m_iNumberOfNormals	* sizeof(ObjNormal)	  );
	pMesh->m_aTexCoordArray = (ObjTexCoord*)malloc( pMesh->m_iNumberOfTexCoords	* sizeof(ObjTexCoord) );
	pMesh->m_aFaces			= (ObjFace*    )malloc( pMesh->m_iNumberOfFaces		* sizeof(ObjFace)	  );

	fp = fopen(filename,"r");

	while(!feof(fp))
	{
		fgets(buffer,256,fp);

		if( strncmp("vn ",buffer,3) == 0 )	//Searching for Vertex Normals
		{
			sscanf((buffer+2),"%f%f%f",
				&pMesh->m_aNormalArray[ nc ].x,
				&pMesh->m_aNormalArray[ nc ].y,
				&pMesh->m_aNormalArray[ nc ].z);
			++nc;
		}
		else


			if( strncmp("vt ",buffer,3) == 0 )	//Searching for Texture Coords
			{
				sscanf((buffer+2),"%f%f",
					&pMesh->m_aTexCoordArray[ tc ].u,
					&pMesh->m_aTexCoordArray[ tc ].v);
				++tc;
			}
			else

				if( strncmp("v ",buffer,2) == 0 )	//Searching for Vertices
				{
					sscanf((buffer+1),"%f%f%f",
						&pMesh->m_aVertexArray[ vc ].x,
						&pMesh->m_aVertexArray[ vc ].y,
						&pMesh->m_aVertexArray[ vc ].z);
					++vc;
				}
				else

					if( strncmp("f ",buffer,2) == 0 )	//Searching for Faces
					{
						unsigned int i;

						ObjFace *pf = &pMesh->m_aFaces[fc];

						sscanf(buffer+2, "%d/%d/%d %d/%d/%d %d/%d/%d",
							&pf->m_aVertexIndices   [0],			// Each face has 9 Vertices
							&pf->m_aTexCoordIndicies[0],			// Here we are getting those 9 vertices
							&pf->m_aNormalIndices   [0],			// And storing them in the proper arrays
							&pf->m_aVertexIndices   [1],
							&pf->m_aTexCoordIndicies[1],
							&pf->m_aNormalIndices   [1],
							&pf->m_aVertexIndices   [2],
							&pf->m_aTexCoordIndicies[2],
							&pf->m_aNormalIndices   [2] );


						// This is for messing with OBJ's funky numbering system (1 vs 0)
						for(i=0;i<3;i++)
						{
							--pf->m_aTexCoordIndicies[i];
							--pf->m_aNormalIndices   [i];
							--pf->m_aVertexIndices   [i];
						}

						++fc;
					}
	}

	fclose(fp);

	modelLoaded = true;	// Now we Can Render it
	return 0;
}
//----------------------------------END MODEL LOADER

glut_simple_mouse_interactor object;	// God I love this beast
glut_callbacks cb;
bool b[256];


// I needed to add these or else program bluescreens sometimes.
// Basically releasing our programmable gpu stuff
void cleanExit(int exitval)
{
	if (vertexLighting) 
	{
		glDeleteObjectARB(vertexLighting);
	}
	if (fragmentLighting) 
	{
		glDeleteObjectARB(fragmentLighting);
	}
	exit(0);
}


void addShader(GLhandleARB programObject, const GLcharARB *shaderSource, GLenum shaderType, bool linkProgram)
{
	assert(programObject != 0);
	assert(shaderSource != 0);
	assert(shaderType != 0);

	GLhandleARB object = glCreateShaderObjectARB(shaderType);
	assert(object != 0);

	GLint length = (GLint)strlen(shaderSource);
	glShaderSourceARB(object, 1, &shaderSource, &length);

	// compile vertex shader object
	glCompileShaderARB(object);

	// check if shader compiled
	GLint compiled = 0;
	glGetObjectParameterivARB(object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

	if (!compiled)
	{
		cleanExit(-1);
	}

	// attach vertex shader to program object
	glAttachObjectARB(programObject, object);

	// delete vertex object, no longer needed
	glDeleteObjectARB(object);

	if (linkProgram)
	{
		glLinkProgramARB(programObject);

		GLint linked = false;
		glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &linked);
		if (!linked)
		{
			cout << "Shaders failed to link, exiting..." << endl;
			cleanExit(-1);
		}
	}

	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
		cout << "OpenGL error: " << gluErrorString(err) << endl;
}

void init_opengl()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.05, 0.05, 0.05, 1.0);	// Black is so emo... now dark grey is chic

	if(!glh_init_extensions("GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader"))
	{
		cout << "Necessary extensions unsupported: " << glh_get_unsupported_extensions() << endl;
		exit(0);
	}

	// define search path for simple_lighting.glsl file
	data_path media;
	media.path.push_back(".");


	string filename = media.get_file("shaders/vertex_lighting.glsl");
	if (filename == "")
	{
		printf("Unable to locate vertex_lighting.glsl, exiting...\n");
		cleanExit(-1);
	}

	// load and create vertex lighting program
	vertexLighting = glCreateProgramObjectARB();

	GLcharARB *shaderData = read_text_file(filename.c_str());
	addShader(vertexLighting, shaderData, GL_VERTEX_SHADER_ARB, true);

	filename = media.get_file("shaders/passthrough_vp.glsl");
	if (filename == "")
	{
		printf("Unable to locate passthrough_vp.glsl, exiting...\n");
		cleanExit(-1);
	}

	// load and create fragment lighting program
	fragmentLighting = glCreateProgramObjectARB();

	shaderData = read_text_file(filename.c_str());
	addShader(fragmentLighting, shaderData, GL_VERTEX_SHADER_ARB, false);

	filename = media.get_file("shaders/fragment_lighting.glsl");
	if (filename == "")
	{
		printf("Unable to locate fragment_lighting.glsl, exiting...\n");
		cleanExit(-1);
	}

	shaderData = read_text_file(filename.c_str());
	addShader(fragmentLighting, shaderData, GL_FRAGMENT_SHADER_ARB, true);


	// default to using vertex lighting program
	curProgram = vertexLighting;

	// import external geometry
	filename = media.get_file("models/Raygun_02.OBJ");
	if (filename == "")
	{
		printf("Unable to locate model, exiting...\n");
		cleanExit(-1);
	}

	filePaths[0]=filename;

	filename = media.get_file("models/Raygun_01.OBJ");
	if (filename == "")
	{
		printf("Unable to locate model, exiting...\n");
		cleanExit(-1);
	}

	filePaths[1]=filename;


	filename = media.get_file("models/UFO-01.OBJ");
	if (filename == "")
	{
		printf("Unable to locate model, exiting...\n");
		cleanExit(-1);
	}

	filePaths[2]=filename;


	filename = media.get_file("models/HateAlien-POSE.OBJ");
	if (filename == "")
	{
		printf("Unable to locate model, exiting...\n");
		cleanExit(-1);
	}

	filePaths[3]=filename;

	cout<<"Loading Model...";
	LoadOBJ(filePaths[curModel].c_str());
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	object.apply_transform();
	glRotatef(-15, 1.0, 0.0, 0.0);
	glRotatef( 25, 0.0, 1.0, 0.0);
	glScalef(scale,scale,scale);	// We have a universal scalar, and we can change it.
	glTranslatef(0.0f,-1.0f,0.0f);	// Lower the model a bit...

	glUseProgramObjectARB(curProgram);	// Shader stuff

	if(CycleRGB)	// Just playing around, changes the diffuse lighting values sent to the shader
	{
		if(rDiff<0.05f)
		{
			rDiv = 0.001f;
		}

		if(rDiff>0.95f)
		{
			rDiv = -0.001f;
		}

		if(gDiff<0.05f)
		{
			gDiv = 0.002f;
		}

		if(gDiff>0.95f)
		{
			gDiv = -0.002f;
		}

		if(bDiff<0.05f)
		{
			bDiv = 0.003f;
		}

		if(bDiff>0.95f)
		{
			bDiv = -0.003f;
		}

		rDiff += rDiv;
		gDiff += gDiv;
		bDiff += bDiv;

		rSpec = gSpec = bSpec +=bDiv/2;
	}
	else
	{
		rDiff = 0.0f;
		gDiff = 0.0f;
		bDiff = 1.0f;
		rSpec = gSpec = bSpec = 1.0f;
	}

	glUniform3fARB(glGetUniformLocationARB(curProgram, "lightVec"), 0.0, 0.0, 1.0);					// Sets Location of Light
	glUniform3fARB(glGetUniformLocationARB(curProgram, "diffuseMaterial"), rDiff, gDiff, bDiff);	// Sets Diffuse Values
	glUniform3fARB(glGetUniformLocationARB(curProgram, "specularMaterial"), rSpec, gSpec, bSpec);	// Sets Specular Values

	glPushMatrix();

	if(modelLoaded)
	{
		glBegin(GL_TRIANGLES); // DRAW THEM FACES
		for(int i=0;i<pMesh->m_iNumberOfFaces;i++)
		{
			ObjFace *pf = &pMesh->m_aFaces[i];

			for(int j=0;j<3;j++)
			{
				glTexCoord2f( pMesh->m_aTexCoordArray[ pf->m_aTexCoordIndicies[j] ].u,
					pMesh->m_aTexCoordArray[ pf->m_aTexCoordIndicies[j] ].v);
				glNormal3f( pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].x,
					pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].y,
					pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].z);
				glVertex3f( pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].x,
					pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].y,
					pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].z);
			}
		}
		glEnd();
	}
	glPopMatrix();
	glUseProgramObjectARB(0);
	glutSwapBuffers();
}


//Keyboard Capture
void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];
	if(k==27 || k=='q') cleanExit(0);

	if (k == 'w' || k == 'W')
	{
		if (b[k])
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(0.5f);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	if (k == 'c')
	{
		CycleRGB = !CycleRGB;
	}

	if (k == '1')
	{
		curProgram = vertexLighting;
	}

	if (k == '2')
	{
		curProgram = fragmentLighting;
	}

	if (k == 'n')
	{
		scale/=1.5f;
	}

	if (k == 'm')
	{
		scale*=1.5f;
	}

	if (k == ']')	//Loads a new model
	{

		curModel++;

		if(curModel>3)
			curModel=0;

		modelLoaded = false;
		LoadOBJ(filePaths[curModel].c_str());

	}

	if (k == '[')	//Loads a new model
	{
		curModel--;

		if(curModel<0)
			curModel=3;
		modelLoaded = false;
		LoadOBJ(filePaths[curModel].c_str());

	}

	glutPostRedisplay();
}


void special(int k, int x, int y)
{
	glutPostRedisplay();
}

void idle()		//Animates the model when you're not doing anything
{
	if (b[' '])
		object.trackball.increment_rotation();

	glutPostRedisplay();
}

void resize(int w, int h)	// called if window is resized so the dimensions won't get funky
{
	if (h == 0) h = 1;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CS 418 MP1 : Robert Lach");

	init_opengl();
	glutCreateMenu(menu);
	glutAddMenuEntry("Load Next Model []]", ']');
	glutAddMenuEntry("Load Previous Model [[]", '[');
	glutAddMenuEntry("Vertex lighting [1]", '1');
	glutAddMenuEntry("Fragment lighting [2]", '2');
	glutAddMenuEntry("Toggle RGB Cycler [c]", 'c');
	glutAddMenuEntry("Toggle WireFrame [w]", 'w');
	glutAddMenuEntry("Zoom In [m]", 'm');
	glutAddMenuEntry("Zoom Out [n]", 'n');
	glutAddMenuEntry("Toggle Animation [space]", 32);
	glutAddMenuEntry("Quit [q]", 'q');
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glut_helpers_initialize();

	object.configure_buttons(1);
	b['t'] = true;
	object.dolly.dolly[2] = -5;

	cb.keyboard_function = key;
	cb.special_function = special;
	cb.display_function = display;
	cb.reshape_function = resize;

	glut_add_interactor(&cb);
	glut_add_interactor(&object);
	glutIdleFunc(idle);

	b[' '] = true;

	glutMainLoop();

	return 0;
}
