/*


MP3 - Bezeir Surfaces + Environment Mapping
By Robert Lach

November 12th, 2008

*/



#if defined(WIN32)
#include <windows.h>
#elif defined(UNIX)
#include <GL/glx.h>
#endif

#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>
#include <glh/glh_obs.h>

#include <utils/read_text_file.h>
#include <utils/data_path.h>


#include <utils/nv_dds.h>

using namespace glh;
using namespace nv_dds;

typedef struct point_3d {			// Structure for a 3d point
	double x, y, z;
} POINT_3D;

typedef struct bpatch {				// Structure for a bezier patches

	POINT_3D	anchors[4][4];		
	GLuint		dlBPatch;				
} BEZIER_PATCH;

// Adds 2 points. Don't just use '+' ;)
POINT_3D pointAdd(POINT_3D p, POINT_3D q) {
	p.x += q.x;		p.y += q.y;		p.z += q.z;
	return p;
}

// Multiplies a point and a constant. Don't just use '*'
POINT_3D pointTimes(double c, POINT_3D p) {
	p.x *= c;	p.y *= c;	p.z *= c;
	return p;
}

// Function for quick point creation
POINT_3D makePoint(double a, double b, double c) {
	POINT_3D p;
	p.x = a;	p.y = b;	p.z = c;
	return p;
}


// Calculates 3rd degree polynomial based on array of 4 points
// and a single variable (u) which is generally between 0 and 1
POINT_3D Bernstein(float u, POINT_3D *p) {
	POINT_3D	a, b, c, d, r;

	a = pointTimes(pow(u,3), p[0]);
	b = pointTimes(3*pow(u,2)*(1-u), p[1]);
	c = pointTimes(3*u*pow((1-u),2), p[2]);
	d = pointTimes(pow((1-u),3), p[3]);

	r = pointAdd(pointAdd(a, b), pointAdd(c, d));

	return r;
}

//Simple crossproduct calculator, need it for normals
void CrossProduct(POINT_3D *v, POINT_3D *v1, POINT_3D *result)
{
	result->x = v->y * v1->z - v1->y * v->z;
	result->y = v->z * v1->x - v1->z * v->x;
	result->z = v->x * v1->y - v1->x * v->y;
	float length = (float)sqrt(result->x*result->x+result->y*result->y+result->z*result->z);
	result->x /= length;
	result->y /= length;
	result->z /= length;
}

// Generates a display list based on the data in the patch
// and the number of divisions
GLuint genBezier(BEZIER_PATCH patch, int divs) {

	GLuint	drawlist = glGenLists(1);		// make the display list

	if (patch.dlBPatch != NULL)			
		glDeleteLists(patch.dlBPatch, 1);

	glNewList(drawlist, GL_COMPILE);				

	GLdouble anchorPoints[4][4][3] = 
	{
		{
			{patch.anchors[0][0].x,patch.anchors[0][0].y,patch.anchors[0][0].z},
			{patch.anchors[0][1].x,patch.anchors[0][1].y,patch.anchors[0][1].z},
			{patch.anchors[0][2].x,patch.anchors[0][2].y,patch.anchors[0][2].z},
			{patch.anchors[0][3].x,patch.anchors[0][3].y,patch.anchors[0][3].z}},
			{
				{patch.anchors[1][0].x,patch.anchors[1][0].y,patch.anchors[1][0].z},
				{patch.anchors[1][1].x,patch.anchors[1][1].y,patch.anchors[1][1].z},
				{patch.anchors[1][2].x,patch.anchors[1][2].y,patch.anchors[1][2].z},
				{patch.anchors[1][3].x,patch.anchors[1][3].y,patch.anchors[1][3].z}},
				{
					{patch.anchors[2][0].x,patch.anchors[2][0].y,patch.anchors[2][0].z},
					{patch.anchors[2][1].x,patch.anchors[2][1].y,patch.anchors[2][1].z},
					{patch.anchors[2][2].x,patch.anchors[2][2].y,patch.anchors[2][2].z},
					{patch.anchors[2][3].x,patch.anchors[2][3].y,patch.anchors[2][3].z}},
					{
						{patch.anchors[3][0].x,patch.anchors[3][0].y,patch.anchors[3][0].z},
						{patch.anchors[3][1].x,patch.anchors[3][1].y,patch.anchors[3][1].z},
						{patch.anchors[3][2].x,patch.anchors[3][2].y,patch.anchors[3][2].z},
						{patch.anchors[3][3].x,patch.anchors[3][3].y,patch.anchors[3][3].z}}
	};
	glMap2d(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &anchorPoints[0][0][0]);

	glMapGrid2d(divs, 0.0, 1.0, divs, 0.0, 1.0);
	glEvalMesh2(GL_FILL, 0, divs, 0, divs);
	glEndList();								// END the list
	return drawlist;							// Return the display list
}


//Fields:

int				divs = 7;			// Number of intrapolations (conrols poly resolution)
int				numPoints = 0;
int				numPatches = 0;
BEZIER_PATCH	*m_patchArray;
point_3d		*m_pointArray;
float			uniScale = 0.25;
int				currentEnviroMap = 0;


// Generates acutal Bezier Patches
void generatePatches()
{
	for(int i =0; i<numPatches; i++)
	{
		m_patchArray[i].dlBPatch = genBezier(m_patchArray[i], divs);
	}
}


// Loads a .bez file.
int LoadBezier(const char *filename)
{
	FILE *fp = NULL;


	fp = fopen(filename,"r");


	if(!fp)					//File Not Found
		return 0;


	numPatches = 0;
	numPoints = 0;

	while(!feof(fp))
	{
		int index;

		char buffer [100];
		fgets(buffer, 100, fp);
		sscanf(buffer, "%d", &index);
		if(index>0)
		{
			++numPoints;

		}
		else if(index<0)
		{
			++numPatches;
		}
	}

	fclose(fp);

	m_pointArray	= (point_3d*)malloc(numPoints * sizeof(point_3d));
	m_patchArray	= (BEZIER_PATCH*)malloc(numPatches * sizeof(BEZIER_PATCH));

	fp = fopen(filename,"r");

	char buffer [100];
	for(int points=1; points<=numPoints; points++)
	{

		fgets(buffer, 100, fp);
		int index=0;
		float x,y,z;
		sscanf(buffer, "%d %f %f %f",&index, &x, &y, &z);	//read point
		m_pointArray[points-1].x=(double)x;
		m_pointArray[points-1].y=(double)y;
		m_pointArray[points-1].z=(double)z;
	}
	for(int patches=0; patches<numPatches; patches++)
	{
		fgets(buffer, 100, fp);
		int a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p;
		sscanf(buffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",&a,&b,&c,&d,&e,&f,&g,&h,&i,&j,&k,&l,&m,&n,&o,&p); //read surface
		a = a*-1;
		m_patchArray[patches].anchors[0][0] = m_pointArray[a-1];
		m_patchArray[patches].anchors[0][1] = m_pointArray[b-1];
		m_patchArray[patches].anchors[0][2] = m_pointArray[c-1];
		m_patchArray[patches].anchors[0][3] = m_pointArray[d-1];
		m_patchArray[patches].anchors[1][0] = m_pointArray[e-1];
		m_patchArray[patches].anchors[1][1] = m_pointArray[f-1];
		m_patchArray[patches].anchors[1][2] = m_pointArray[g-1];
		m_patchArray[patches].anchors[1][3] = m_pointArray[h-1];
		m_patchArray[patches].anchors[2][0] = m_pointArray[i-1];
		m_patchArray[patches].anchors[2][1] = m_pointArray[j-1];
		m_patchArray[patches].anchors[2][2] = m_pointArray[k-1];
		m_patchArray[patches].anchors[2][3] = m_pointArray[l-1];
		m_patchArray[patches].anchors[3][0] = m_pointArray[m-1];
		m_patchArray[patches].anchors[3][1] = m_pointArray[n-1];
		m_patchArray[patches].anchors[3][2] = m_pointArray[o-1];
		m_patchArray[patches].anchors[3][3] = m_pointArray[p-1];
	}
	fclose(fp);

	generatePatches();
	return 0;

}

void initBezier(void) {	
	data_path media;
	media.path.push_back(".");

	string filename = media.get_file("models/teapot.bez");
	if (filename == "")
	{
		cout << "Unable to load teapot.bez, exiting..." << endl;
		exit(0);
	}
	LoadBezier(filename.c_str());

}



glut_callbacks cb;
glut_simple_mouse_interactor camera, object;
glut_perspective_reshaper reshaper(60, 0.1, 10.0);

GLhandleARB programObject;

GLint eyePosParam, displaceParam, viewInverseParam;
GLint etaParam, fresnelParam;
GLint environmentMap;

// environment map
tex_object_cube_map cubemap;

float eta = 1.1, eta_delta = -0.02; // ratio of indices of refraction

GLuint dispersion_combiners;

bool b[256];
float anim = 0.0f;


void init_opengl();
void keyboard(unsigned char k, int x, int y);
void display();
void menu(int entry);
void idle();

int main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
	glutInitWindowSize( 1024, 768 );
	glutCreateWindow( "Robert Lach - MP3 - Bezier Surface + Environment Mapping (Dispersion)" );

	init_opengl();

	glut_helpers_initialize();

	cb.keyboard_function = keyboard;
	cb.idle_function = idle;

	camera.configure_buttons(1);
	camera.set_camera_mode(false);
	camera.dolly.dolly[2] = -1.5;

	object.configure_buttons(1);
	object.dolly.dolly[2] = 0.0;

	object.disable();
	camera.enable();

	glut_add_interactor(&cb);
	glut_add_interactor(&camera);
	glut_add_interactor(&object);
	glut_add_interactor(&reshaper);

	glut_idle(1);

	cout<<"Load Different Environment Map [a]\n";
	cout<< "Increase refraction index [+]\n";
	cout<< "Decrease refraction index [-]\n";
	cout<< "Increase dispersion []]\n";
	cout<< "Decrease dispersion [[]\n";
	cout<< "Increase Divisions [l]\n";
	cout<< "Decrease Divisions [k]\n";
	cout<< "Zoom In[h]\n";
	cout<< "Zoom Out[j]\n";
	cout<<"Move object [o]\n";
	cout<< "Move camera [c]\n";
	cout<<"Toggle wireframe [w]\n";
	cout<< "Toggle deformation [d]\n"; 
	cout<< "Toggle camera spin [space]\n";
	cout<< "Quit [esc]\n";



	glutCreateMenu( menu );
	glutAddMenuEntry( "Load Different Environment Map [a]", 'a');
	glutAddMenuEntry( "Increase refraction index [+]", '+');
	glutAddMenuEntry( "Decrease refraction index [-]", '-');
	glutAddMenuEntry( "Increase dispersion []]", ']');
	glutAddMenuEntry( "Decrease dispersion [[]", '[');
	glutAddMenuEntry( "Increase Divisions [l]", 'l');
	glutAddMenuEntry( "Decrease Divisions [k]", 'k');
	glutAddMenuEntry( "Zoom In[h]", 'h');
	glutAddMenuEntry( "Zoom Out[j]", 'j');
	glutAddMenuEntry( "Move object [o]", 'o');
	glutAddMenuEntry( "Move camera [c]", 'c');
	glutAddMenuEntry( "Toggle wireframe [w]", 'w' );
	glutAddMenuEntry( "Toggle deformation [d]", 'd' );
	glutAddMenuEntry( "Toggle camera spin [space]", ' ' );
	glutAddMenuEntry( "Quit [esc]", 27 );
	glutAttachMenu( GLUT_RIGHT_BUTTON) ;

	b[' '] = true;

	glutDisplayFunc( display );
	glutMainLoop();

	return 0;
}

void cleanExit(int exitval)
{
	if (programObject) 
		glDeleteObjectARB(programObject);

	if(exitval == 0) { exit(0); }
	else { exit(exitval); }
}


void addShader(GLhandleARB programObject, const GLcharARB *shaderSource, GLenum shaderType)
{
	assert(programObject != 0);
	assert(shaderSource != 0);
	assert(shaderType != 0);

	GLhandleARB object = glCreateShaderObjectARB(shaderType);
	GLint length = (GLint)strlen(shaderSource);
	glShaderSourceARB(object, 1, &shaderSource, &length);

	// compile vertex shader object
	glCompileShaderARB(object);

	// check if shader compiled
	GLint compiled = 0;
	glGetObjectParameterivARB(object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

	// attach vertex shader to program object
	glAttachObjectARB(programObject, object);

	// delete vertex object, no longer needed
	glDeleteObjectARB(object);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
		cout << "OpenGL error: " << gluErrorString(err) << endl;
}


//Loads cubmap textures. Currently between 2 nvidia ones.
void loadCubeMapTex(int map)
{
	data_path media;
	media.path.push_back(".");

	string filename;
	switch(map)
	{


	case 0:
		filename = media.get_file("textures/nvlobby_new_cube_mipmap.dds");
		break;

	case 1:
		filename = media.get_file("textures/nvlobby_cube_mipmap.dds");
		break;

	}

	if (filename.empty())
	{
		cout << "Unable to locate nvlobby_new_cube_mipmap.dds, exiting..." << endl;
		exit(0);
	}

	CDDSImage image;
	if (image.load(filename, false))
		image.upload_textureCubemap();
}

void init_opengl()
{
	if(!glh_init_extensions("GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader"))
	{
		cout << "Necessary extensions unsupported: " <<  glh_get_unsupported_extensions() << endl;
		exit(0);
	}

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_MAP2_VERTEX_3);

	data_path media;
	media.path.push_back(".");

	string filename = media.get_file("shaders/dispersion_vertex.glsl");
	if (filename == "")
	{
		cout << "Unable to load dispersion_vertex.glsl, exiting..." << endl;
		cleanExit(-1);
	}

	programObject = glCreateProgramObjectARB();

	GLcharARB *shaderData = read_text_file(filename.c_str());
	addShader(programObject, shaderData, GL_VERTEX_SHADER_ARB);

	delete [] shaderData;

	filename = media.get_file("shaders/dispersion_fragment.glsl");
	if (filename == "")
	{
		cout << "Unable to load dispersion_fragment.glsl, exiting..." << endl;
		cleanExit(-1);
	}

	shaderData = read_text_file(filename.c_str());
	addShader(programObject, shaderData, GL_FRAGMENT_SHADER_ARB);

	delete [] shaderData;

	glLinkProgramARB(programObject);

	GLint linked = false;
	glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &linked);

	glValidateProgramARB(programObject);

	GLint validated = false;
	glGetObjectParameterivARB(programObject, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);

	eyePosParam = glGetUniformLocationARB(programObject, "eyePos");
	displaceParam = glGetUniformLocationARB(programObject, "displace");
	viewInverseParam = glGetUniformLocationARB(programObject, "viewInverse");
	etaParam = glGetUniformLocationARB(programObject, "etaValues");
	fresnelParam = glGetUniformLocationARB(programObject, "fresnelValues");
	environmentMap = glGetUniformLocationARB(programObject, "environmentMap");

	if ((eyePosParam < 0) || (displaceParam < 0) || (etaParam < 0) || 
		(fresnelParam < 0) || (environmentMap < 0) || (viewInverseParam < 0))
	{
		cout << "Unable to locate uniform parameters, exiting..." << endl;
		cleanExit(-1);
	}

	// Load the cube map
	cubemap.bind();
	cubemap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	cubemap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	cubemap.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	cubemap.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	cubemap.parameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



	loadCubeMapTex(0);

	// use program object in order to set some uniform parameters
	glUseProgramObjectARB(programObject);

	glUniform3fARB(fresnelParam, 2.0, 2.0, 0.1);
	glUniform4fARB(displaceParam, 5.0, 0.0, 0.02, 0.0);

	// set environmentMap sampler to fetch from texunit 0
	glUniform1iARB(environmentMap, 0);

	// back to fixed function
	glUseProgramObjectARB(0);

	initBezier();	// Initialize the Bezier surface



	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
		cout << "OpenGL error: " << gluErrorString(err) << endl;
}



// draw cubemap background
void drawSkyBox(void)
{
	glDisable(GL_DEPTH_TEST);

	cubemap.bind();
	cubemap.enable();

	// initialize object linear texgen
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	GLfloat s_plane[] = { 1.0, 0.0, 0.0, 0.0 };
	GLfloat t_plane[] = { 0.0, 1.0, 0.0, 0.0 };
	GLfloat r_plane[] = { 0.0, 0.0, 1.0, 0.0 };
	glTexGenfv(GL_S, GL_OBJECT_PLANE, s_plane);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, t_plane);
	glTexGenfv(GL_R, GL_OBJECT_PLANE, r_plane);
	glPopMatrix();
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	camera.trackball.apply_inverse_transform();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glScalef(10.0, 10.0, 10.0);
	glutSolidCube(1.0);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);

	cubemap.disable();

	glEnable(GL_DEPTH_TEST);
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// draw background
	drawSkyBox();

	// concatenate view matrix with existing projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	camera.apply_transform();
	glMatrixMode(GL_MODELVIEW);

	object.apply_transform();

	glUseProgramObjectARB(programObject);

	// set uniform parameters

	glUniformMatrix4fvARB(viewInverseParam, 1, GL_FALSE, camera.get_inverse_transform().m);
	glUniform4fARB(eyePosParam, 0.0, 0.0, 0.0, 1.0);
	glUniform3fARB(etaParam, eta, eta + eta_delta, eta + (eta_delta*2.0));

	if (b['d'])
		glUniform4fARB(displaceParam, 20.0, anim, 0.02, 0.0);
	else
		glUniform4fARB(displaceParam, 0.0, 0.0, 0.0, 0.0);

	// bind environment cubemap
	cubemap.bind();

	// draw object
	//glutSolidTorus(0.25, 0.5, 64, 64);
	//glutSolidTeapot(0.5f);
	glTranslatef(0, -0.25, 0);
	glScalef(uniScale, uniScale, uniScale);
		for(int i =0; i<numPatches; i++)
		{
			glCallList(m_patchArray[i].dlBPatch);
		}
	cubemap.disable();
	glUseProgramObjectARB(0);

	// restore projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glutSwapBuffers();
}

void keyboard( unsigned char k, int x, int y )
{
	switch(k) {
	case 'h':
		if(uniScale>0.1)
		{
			uniScale-=0.01;
		}
		generatePatches();
		break;
	case 'j':
		uniScale+=0.01;

		break;
	case 'k':
		if(divs>1)
		{
			divs-=1;
		}
		init_opengl();
		break;
	case 'l':
		divs+=1;
		init_opengl();
		break;
	case 'a':
		if(currentEnviroMap == 1)
		{
			currentEnviroMap =0;
		}
		else
		{
			currentEnviroMap++;
		}
		loadCubeMapTex(currentEnviroMap);
		break;

	}


	b[k] = ! b[k];

	switch(k) {
	case 27:
	case 'q':
		cleanExit(0);
		break;

	case 'w':
		if (b['w'])
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;

	case 'o':
		object.enable();
		camera.disable();
		break;

	case 'c':
		camera.enable();
		object.disable();
		break;

	case '=':
	case '+':
		eta += 0.01;
		break;
	case '-':
		eta -= 0.01;
		break;

	case ']':
		eta_delta += 0.001;
		break;
	case '[':
		eta_delta -= 0.001;
		break;

	case 'r':
		eta = 1.0;
		eta_delta = 0.0;
		break;
	}
}

void idle()
{
	if (b[' ']) {
		rotationf increment;
		increment.set_value(vec3f(0.0, 1.0, 0), 0.01);
		camera.trackball.r *= increment;
	}

	if (b['d']) {
		anim += 0.1f;
	}

	glutPostRedisplay();
}

void menu( int entry )
{
	keyboard((unsigned char)entry, 0, 0);
}

