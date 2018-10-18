//////////////////
// Robert Lach 




#include <stdio.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <math.h>

	


#define M_PI       3.14159265358979323846


float sealevel;
float polysize;
long old_time;
float zAngle;
float minAirspeed = 0.0001f;

typedef struct 
{
	double		x;
	double		y;
	double		z;
} vector3;

typedef struct 
{
	float		x;
	float		y;
	float		z;
} vector3f;


typedef struct 
{
	vector3		location;
	vector3		fwd;
	float		fwd_a;
	float		angle;
	float		pitch;
	float		roll;
	float		speed;
	float		clip_near;
	float		clip_far;
	float		fov;
	float		half_fov;
	float		ax1, ax2, by1, by2, d1, d2;

		
} Camera;

Camera plane;

int seed(float x, float y) {
    static int a = 15635564565, b = 121151901;
	int xi = *(int *)&x;
	int yi = *(int *)&y;
    return ((xi * a) % b) - ((yi * b) % a);
}

void mountain(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float s)
{
	float x01,y01,z01,x12,y12,z12,x20,y20,z20;

	if (s < polysize) {
		x01 = x1 - x0;
		y01 = y1 - y0;
		z01 = z1 - z0;

		x12 = x2 - x1;
		y12 = y2 - y1;
		z12 = z2 - z1;

		x20 = x0 - x2;
		y20 = y0 - y2;
		z20 = z0 - z2;

		float nx = y01*(-z20) - (-y20)*z01;
		float ny = z01*(-x20) - (-z20)*x01;
		float nz = x01*(-y20) - (-x20)*y01;

		float den = sqrt(nx*nx + ny*ny + nz*nz);

		if (den > 0.0) {
			nx /= den;
			ny /= den;
			nz /= den;
		}

		glNormal3f(nx,ny,nz);
		glBegin(GL_TRIANGLES);
			glVertex3f(x0,y0,z0);
			glVertex3f(x1,y1,z1);
			glVertex3f(x2,y2,z2);
		glEnd();

		return;
	}

	x01 = 0.5*(x0 + x1);
	y01 = 0.5*(y0 + y1);
	z01 = 0.5*(z0 + z1);

	x12 = 0.5*(x1 + x2);
	y12 = 0.5*(y1 + y2);
	z12 = 0.5*(z1 + z2);

	x20 = 0.5*(x2 + x0);
	y20 = 0.5*(y2 + y0);
	z20 = 0.5*(z2 + z0);

	s *= 0.5;

	srand(seed(x01,y01));
	z01 += 0.3*s*(2.0*((float)rand()/(float)RAND_MAX) - 1.0);
	srand(seed(x12,y12));
	z12 += 0.3*s*(2.0*((float)rand()/(float)RAND_MAX) - 1.0);
	srand(seed(x20,y20));
	z20 += 0.3*s*(2.0*((float)rand()/(float)RAND_MAX) - 1.0);

	mountain(x0,y0,z0,x01,y01,z01,x20,y20,z20,s);
	mountain(x1,y1,z1,x12,y12,z12,x01,y01,z01,s);
	mountain(x2,y2,z2,x20,y20,z20,x12,y12,z12,s);
	mountain(x01,y01,z01,x12,y12,z12,x20,y20,z20,s);
}


//our flight simulation function
void fly (void)
{
	if (fabs(plane.speed) > 0)
	{
		plane.location.x += 0.5 * plane.speed * (cos(plane.fwd_a) * cos(plane.pitch) );
		plane.location.y += 0.5 * plane.speed * (sin(plane.fwd_a) * cos(plane.pitch) );
		plane.location.z += 0.5 * plane.speed * sin(plane.pitch);

		plane.angle -= sin (plane.roll / 180.0 * M_PI) * 0.004;
		plane.fwd_a -= sin (plane.roll / 180.0 * M_PI) * 0.004;
	}



	plane.speed += (0.00001 * -tan(plane.pitch));

	if(plane.speed >= 0.002)
		plane.speed = 0.002;
	if(plane.speed <= minAirspeed)
		plane.speed = minAirspeed;
	if(plane.speed < 0.0001)
		plane.speed = 0.0001;


	if (plane.pitch > M_PI)
	{
		plane.angle += M_PI;
		plane.fwd_a += M_PI;
	}
	if (plane.pitch < -M_PI)
	{
		plane.angle += M_PI;
		plane.fwd_a += M_PI;
	}

	glutPostRedisplay();

	return;
}

void init(void) 
{
	zAngle = 0;
	GLfloat white[] = {1.0,1.0,1.0,1.0};
	GLfloat lpos[] = {0.0,1.0,0.0,0.0};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	glClearColor (0.5, 0.5, 1.0, 0.0);
	glShadeModel (GL_FLAT); 
	glEnable(GL_DEPTH_TEST);

	sealevel = 0.0;
	polysize = 0.01;

	// Plane (camera) init
	plane.location.x = 0.0;
	plane.location.y = 1.0;
	plane.location.z = 0.3;
	plane.roll = 0.0f;
	plane.angle = 0.0f;
	plane.fwd_a = 0.0f;
	plane.pitch = 0.0f;
	plane.speed = 0.0001f;
	plane.clip_near = 0.0002f;
	plane.clip_far = 2500.0f;
	plane.fov = 40.0f;
	plane.half_fov = 0.2+(plane.fov / (2.0 * (180.0 / M_PI)));

	glEnable (GL_FOG);
	float	fogColor [] = { 1.0, 1.0, 1.0, 1.0 };
	glFogfv (GL_FOG_COLOR, fogColor);
	glFogf (GL_FOG_START, 20.0);
	glFogf (GL_FOG_END, 500.0);
	glFogf (GL_FOG_DENSITY, 0.5);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(plane.fov, 1.3333, plane.clip_near, plane.clip_far);
	glPushMatrix();

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

void display(void)
{

	vector3 lookAt;
	static GLfloat angle = 0.0;

	GLfloat tanamb[] = {0.2,0.15,0.1,1.0};
	GLfloat tandiff[] = {0.4,0.3,0.2,1.0};

	GLfloat seaamb[] = {0.0,0.0,0.2,1.0};
	GLfloat seadiff[] = {0.0,0.0,0.8,1.0};
	GLfloat seaspec[] = {0.5,0.5,1.0,1.0};


	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, tanamb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tandiff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tandiff);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0);

	glScalef(4.0f,4.0f,4.0f);
	mountain(0.0,0.0,0.0, 1.0,0.0,0.0, 0.0,1.0,0.0, 1.0);
	mountain(1.0,1.0,0.0, 0.0,1.0,0.0, 1.0,0.0,0.0, 1.0);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, seaamb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, seadiff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, seaspec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0);

	glNormal3f(0.0,0.0,1.0);
	glBegin(GL_QUADS);
		glVertex3f(0.0,0.0,sealevel);
		glVertex3f(1.0,0.0,sealevel);
		glVertex3f(1.0,1.0,sealevel);
		glVertex3f(0.0,1.0,sealevel);
	glEnd();


		plane.fwd.x = cos(plane.angle + zAngle) * cos(plane.pitch);
	plane.fwd.y = sin(plane.angle + zAngle) * cos(plane.pitch);
	plane.fwd.z = sin(plane.pitch);

	lookAt.x = plane.location.x + plane.fwd.x;
	lookAt.y = plane.location.y + plane.fwd.y;
	lookAt.z = plane.location.z + plane.fwd.z;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
	glRotated (plane.roll, 0.0, 0.0, 1.0);
	gluLookAt (plane.location.x, plane.location.y, plane.location.z, lookAt.x, lookAt.y, lookAt.z, 0.0, 0.0, 1.0);


	char string[64];
	sprintf_s(string, "( [,] )Minimum Airspeed: %f", minAirspeed*1000+1);
	printtext(500,520,string);
	sprintf_s(string, "Airspeed: %f knots",plane.speed*1000+1);
    printtext(500,500,string);
	sprintf_s(string, "( w,s ) Attitude: %f  ", plane.pitch);
	printtext(500,480,string);
	sprintf_s(string, "( a,d ) Bank: %f", plane.roll);
	printtext(500,460,string);
	sprintf_s(string, "Altitude: %f feet", (plane.location.z-sealevel)*10000);
	printtext(500,440,string);
	glutSwapBuffers();
	glFlush ();

	angle += 1.0;

	glutPostRedisplay();
}


void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective(90.0,1.0,0.01,10.0);
	glMatrixMode (GL_MODELVIEW);
}

void Timer( int i)
{
	fly();
	// Need to call this method again after the desired amount of time has passed:
	glutTimerFunc( 10, Timer, 1 );

}

void special(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_UP:
			plane.angle += 0.015 * sin(M_PI * plane.roll / 180.0);
			plane.fwd_a += 0.015 * sin(M_PI * plane.roll / 180.0);
			plane.pitch -= 0.015 * cos(M_PI * plane.roll / 180.0);
			break;
	case GLUT_KEY_DOWN:
			plane.angle -= 0.015 * sin(M_PI * plane.roll / 180.0);
			plane.fwd_a -= 0.015 * sin(M_PI * plane.roll / 180.0);
			plane.pitch += 0.015 * cos(M_PI * plane.roll / 180.0);
			break;
	case GLUT_KEY_RIGHT:
			plane.roll += 1.0;
			break;
	case GLUT_KEY_LEFT:
			plane.roll -= 1.0;
			break;
	}
}



void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
		case 'D':
			zAngle -= 0.55;
			break;
		case 'A':
			zAngle += 0.55;
			break;
		case 'S':
			zAngle = 0;
			break;
		case ']':
			if(plane.speed<0.0022)
				minAirspeed += 0.0001;
 			break;
		case '[':
			if(minAirspeed>0.0002)
				minAirspeed -= 0.0001;
			break;
		case 'q':
			plane.angle += 0.015 * cos(M_PI * plane.roll / 180.0);
			plane.fwd_a += 0.015 * cos(M_PI * plane.roll / 180.0);
			plane.pitch += 0.015 * sin(M_PI * plane.roll / 180.0);
			break;
		case 'e':
			plane.angle -= 0.015 * cos(M_PI * plane.roll / 180.0);
			plane.fwd_a -= 0.015 * cos(M_PI * plane.roll / 180.0);
			plane.pitch -= 0.015 * sin(M_PI * plane.roll / 180.0);
			break;
		case 'a':
			plane.roll -= 1.0;
			break;
		case 'd':
			plane.roll += 1.0;
			break;
		case 's':
			plane.angle -= 0.015 * sin(M_PI * plane.roll / 180.0);
			plane.fwd_a -= 0.015 * sin(M_PI * plane.roll / 180.0);
			plane.pitch += 0.015 * cos(M_PI * plane.roll / 180.0);
			break;
		case 'w':
			plane.angle += 0.015 * sin(M_PI * plane.roll / 180.0);
			plane.fwd_a += 0.015 * sin(M_PI * plane.roll / 180.0);
			plane.pitch -= 0.015 * cos(M_PI * plane.roll / 180.0);
			break;

		case '-':
			sealevel -= 0.01;
			break;
		case '+':
		case '=':
			sealevel += 0.01;
			break;
		case 'f':
			polysize *= 0.5;
			break;
		case 'g':
			polysize *= 2.0;
			break;
		case 27:
			exit(0);
			break;
   }
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize (800, 600); 
   glutInitWindowPosition (100, 100);
   glutCreateWindow (argv[0]);
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(special);
   Timer(1);
   glutMainLoop();
   return 0;
}
