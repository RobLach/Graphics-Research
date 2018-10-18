// Robert Lach
// Cloth Simulator




#if defined(WIN32)
#  include <windows.h>
#  pragma warning (disable : 4786)
#endif

#define GLH_EXT_SINGLE_FILE

//GLH Extensions
#include <glh/glh_extensions.h>
#include <glh/glh_glut.h>

#include <sstream> 

using namespace glh;

//Simulation Data

double x[64][64], y[64][64], z[64][64];
double grav[64][64];
double nx[64][64], ny[64][64], nz[64][64];

int spheres = 3;
double ssx[5], ssy[5], ssz[5], ssr[5];
double sx[5], sy[5], sz[5], sr[5];

double dist[5][5];

//BOOL DrawMe = FALSE;

double r1=0, r2=0;

double a = 0;
double gravity = -0.0004;
int clothSize = 22;
bool cornerLocked = false;

double sphere_x[16][16];
double sphere_y[16][16];
double sphere_z[16][16];

void Sphere(double sx, double sy, double sz, double sr)
{
	int i, j;
	double x, y, z;

	for(j=0; j<16; j++) for(i=0; i<16; i++) {
		x = sr*sin(i*3.14159/16);
		y = sr*cos(i*3.14159/16);
		z = 0;

		//  Rotate about y axis

		double nx = z*sin(j*3.14159/8) + x*cos(j*3.14159/8);
		double ny = y;
		double nz = z*cos(j*3.14159/8) - x*sin(j*3.14159/8);

		sphere_x[j][i] = sx+nx;
		sphere_y[j][i] = sy+ny;
		sphere_z[j][i] = sz+nz;
	}

	double r, g, b;
	int ii, jj;

	for(j=0; j<16; j++) for(i=0; i<16; i++) {

		ii = (i+1)&15;
		jj = (j+1)&15;


		if(((i^j)&1) == 0) {
			r = 0.55; g = 0.55; b = 0.55;
		} else {
			r = 0.65; g = 0.65; b = 0.65;
		}



		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(r, g, b);
		glVertex3f(sphere_x[ j][ i], sphere_y[ j][ i], sphere_z[ j][ i]);
		glVertex3f(sphere_x[ j][ii], sphere_y[ j][ii], sphere_z[ j][ii]);
		glVertex3f(sphere_x[jj][ i], sphere_y[jj][ i], sphere_z[jj][ i]);
		glVertex3f(sphere_x[jj][ii], sphere_y[jj][ii], sphere_z[jj][ii]);
		glEnd();

	}

}

void initCloth()
{
	int i, j;
	for(j=0; j<clothSize; j++) for(i=0; i<clothSize; i++) {
		x[j][i] = (i-(clothSize/2))*(0.1);
		y[j][i] = 1.5;
		z[j][i] = (j-(clothSize/2))*(0.1);
		grav[j][i] = 0.0;
	}
}

void InitialiseShapes()
{
	int i, j;

	initCloth();

	for(j=0; j<5; j++) for(i=0; i<5; i++) {
		dist[j][i] = sqrt((double)(2-i)*(2-i) + (2-j)*(2-j));
	}

	for(int p = 0; p<spheres; p++)
	{
		srand(p);
		ssx[p] =  (double)((double)(rand() % 20 - 10)/10); 
		srand(p);
		ssy[p] = (double)((double)(rand() % 20 - 10)/10); 
		srand(p);
		ssz[p] =  (double)((double)(rand() % 20 - 10)/10); 
		srand(p);
		ssr[p] = 0.5 + (double)((double)(rand() % 10 +1)/50);
	}

}



void DrawSim()
{
	int i, j, ii, jj, k, kk;

	double r, g, b;
	double d, dd;

	//  Move spheres

	for(i=0; i<spheres; i++) {
		sx[i] = ssx[i];
		sy[i] = ssy[i];
		sz[i] = ssz[i];
		sr[i] = ssr[i];
	}

	a ++;

	sy[0] = ssy[0] - 1.16*cos(-a*3.14159*0.0007722-0.5947);
	sy[1] = ssy[1] - 1.67*cos(-a*3.14159*0.0006224+0.1344);
	sz[2] = ssz[2] - 1.92*sin(-a*3.14159*0.0004629+0.6143);
	sy[3] = ssy[3] + 1.55*cos(-a*3.14159*0.0003726-0.8941);
	sy[4] = ssy[4] - 1.71*cos(-a*3.14159*0.0001722-0.7228);

	//  Move cloth

	double mx, my, mz;
	double vx, vy, vz;

	for(k=0; k<15; k++) {

		for(j=0; j<clothSize; j++) {
			for(i=0; i<clothSize; i++) {

				mx = 0; my = grav[i][j] ; mz = 0;

				if(grav[i][j]>gravity)
				{
					grav[i][j] -= 0.0000001;
				}

				//Differential Equations + Calculations

				for(jj=j-1; jj<=j+1; jj++) if(jj>=0 && jj<clothSize) {
					for(ii=i-1; ii<=i+1; ii++) if(ii>=0 && ii<clothSize) if(ii != i || jj != j) {
						vx = x[jj][ii] - x[j][i];
						vy = y[jj][ii] - y[j][i];
						vz = z[jj][ii] - z[j][i];

						d = vx*vx+vy*vy+vz*vz;
						dd = 0.0295 * (sqrt(d)-0.1*dist[2+jj-j][2+ii-i])/(d);

						mx += vx*dd;
						my += vy*dd;
						mz += vz*dd;
					}
				}

				nx[j][i] = x[j][i] + mx; ny[j][i] = y[j][i] + my; nz[j][i] = z[j][i] + mz;

				//  Floor collision

				if(ny[j][i] < -2.999) ny[j][i] = -2.999;

				//  Sphere collision

				for(kk = 0; kk<spheres; kk++) {
					d = (nx[j][i]-sx[kk])*(nx[j][i]-sx[kk]) + (ny[j][i]-sy[kk])*(ny[j][i]-sy[kk]) + (nz[j][i]-sz[kk])*(nz[j][i]-sz[kk]);
					if(d < sr[kk]*sr[kk]) {
						d = sr[kk]/sqrt(d);
						nx[j][i] = (nx[j][i]-sx[kk])*d + sx[kk];
						ny[j][i] = (ny[j][i]-sy[kk])*d + sy[kk];
						nz[j][i] = (nz[j][i]-sz[kk])*d + sz[kk];
						grav[i][j] = 0.00001;
					}
				}

			}
		}

		for(j=0; j<clothSize; j++) for(i=0; i<clothSize; i++) {
			if(!(cornerLocked && (j==0) ))
			{
				x[j][i] = nx[j][i];

				y[j][i] = ny[j][i];

				z[j][i] = nz[j][i];
			}
		}

	}

	//  Draw spheres

	for(k = 0; k<spheres; k++) {
		Sphere(sx[k], sy[k], sz[k], sr[k]-0.01);
	}

	//  Draw floor

	for(j=0; j<8; j++) {
		for(i=0; i<8; i++) {

			if(((i^j)&1) == 0) {
				r = 0.1; g = 0.1; b = 0.9;
			} else {
				r = 0.1; g = 0.1; b = 0.1;
			}

			glBegin(GL_TRIANGLE_STRIP);
			glColor3f(r, g, b);
			glVertex3f(0.5*(i-4), -3, 0.5*(j-4));
			glVertex3f(0.5*(i-3), -3, 0.5*(j-4));
			glVertex3f(0.5*(i-4), -3, 0.5*(j-3));
			glVertex3f(0.5*(i-3), -3, 0.5*(j-3));
			glEnd();

		}
	}

	//  Draw cloth

	for(j=0; j<clothSize-1; j++) {
		for(i=0; i<clothSize-1; i++) {

			if(((i^j)&1) == 0) {
				r = 0.35; g = 0.35; b = 0.35;
			} else {
				r = 0.25; g = 0.25; b = 0.25;
			}

			glBegin(GL_TRIANGLE_STRIP);
			glColor3f(r, g, b); glVertex3f(x[j  ][i  ], y[j  ][i  ], z[j  ][i  ]);
			glColor3f(r, g, b); glVertex3f(x[j  ][i+1], y[j  ][i+1], z[j  ][i+1]);
			glColor3f(r, g, b); glVertex3f(x[j+1][i  ], y[j+1][i  ], z[j+1][i  ]);
			glColor3f(r, g, b); glVertex3f(x[j+1][i+1], y[j+1][i+1], z[j+1][i+1]);
			glEnd();

		}
	}

}

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
	glShadeModel(GL_SMOOTH);

	glClearColor(0.05, 0.05, 0.05, 1.0);	// Black is so emo... now dark grey is chic

	InitialiseShapes();
}

void printtext(int x, int y, char * string)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 800, 0, 600, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	glRasterPos2i(x,y);
	for (char *p = string; *p; p++)
	{
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
	}
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 0.1, 0.1);
	printtext(10,590, "Pretty Simple Cloth Simulation");
	printtext(10,580, "The \"cloth\" consists of a");
	printtext(10,570, "bunch of vertices in a grid");
	printtext(10,560, "with springs connecting each");
	printtext(10,550, "vertex. This program isn't very");
	printtext(10,540, "optimized so the simulation is");
	printtext(10,530, "set to run pretty slowly.");

	printtext(10,490, "Collision detection also has");
	printtext(10,480, "issues, as in, the vertices");
	printtext(10,470, "can pull apart to where a");
	printtext(10,460, "sphere can pass through, so");
	printtext(10,450, "its more of a net simulator");

	char string[64];
	sprintf_s(string, "Cloth Size: %d", clothSize);
	printtext(10,300, string);

	sprintf_s(string, "Gravity: %f", gravity*100);
	printtext(10,280, string);

	if(cornerLocked)
	{
		printtext(10,260, "--EDGE LOCKED--");
	}


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	object.apply_transform();


	glPushMatrix();

	glTranslatef(0.f, 1.f, 0.f);

	glScalef(scale,scale,scale);
	DrawSim();


	glPopMatrix();
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

	if(k == 'd')
	{		
		if(spheres <5)
		{
			init_opengl();
			spheres++;

			init_opengl();
		}
	}

	if(k == 'f')
	{		
		if(spheres >0)
		{
			init_opengl();
			spheres--;

			init_opengl();
		}
	}



	if( k == 'b')
	{
		if(gravity > -0.0015)
		{
			gravity -= 0.00001;
		}
	}

	if( k == 'v')
	{
		if(gravity < -0.00001)
		{
			gravity += 0.00001;
		}
	}

	if( k == 'h')
	{
		InitialiseShapes();
		init_opengl();
	}
	if (k == 'a')
	{
		cornerLocked = !cornerLocked;
	}

	if (k == 't')
	{
		if (clothSize < 51)
		{
			clothSize++;
			initCloth();
		}

	}

	if (k == 'y')
	{

		if (clothSize > 4)
		{
			clothSize--;
			initCloth();
		}
	}

	if (k == 'r')
	{
		initCloth();
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
		scale/=1.2f;
	}

	if (k == 'm')
	{
		scale*=1.2f;
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
	glutCreateWindow("CS 418 MP4 : Cloth Simulation : Robert Lach");

	init_opengl();
	glutCreateMenu(menu);
	glutAddMenuEntry("NOTE: MANY OF THESE OPTIONS REQUIRE RESETTING OF THE OBJECT OR SCENE", '*');
	glutAddMenuEntry("Lock Edge [a]", 'a');
	glutAddMenuEntry("Add Sphere [d]", 'd');
	glutAddMenuEntry("Remove Sphere [f]", 'f');
	glutAddMenuEntry("Reset Cloth [r]", 'r');
	glutAddMenuEntry("Increase Cloth Size [t]", 't');
	glutAddMenuEntry("Decrease Cloth Size [y]", 'y');
	glutAddMenuEntry("Respawn Spheres / Reset Scene [h]", 'h');
	glutAddMenuEntry("Increase Gravity [b]", 'b');
	glutAddMenuEntry("Decrease Gravity [v]", 'v');
	glutAddMenuEntry("Toggle WireFrame [w]", 'w');
	glutAddMenuEntry("Zoom In [m]", 'm');
	glutAddMenuEntry("Zoom Out [n]", 'n');
	glutAddMenuEntry("Toggle Animation [space]", 32);
	glutAddMenuEntry("Quit [q]", 'q');
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glut_helpers_initialize();

	object.configure_buttons(1);
	object.dolly.dolly[2] = -5;

	cb.keyboard_function = key;
	cb.special_function = special;
	cb.display_function = display;
	cb.reshape_function = resize;

	glut_add_interactor(&cb);
	glut_add_interactor(&object);
	glutIdleFunc(idle);

	b[' '] = false;

	glutMainLoop();

	return 0;
}
