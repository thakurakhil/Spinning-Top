/*
" ------Instructions to play------------------------
1. arrows-> move STtu
2. z,x-> direction of STtu
3. d,a -> rotate camera
4. ws-> change view
5. space bar execute	"
----------------------------------------------------
*/
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "imageloader.h"
#include "vec3f.h"
//#define print printf("test\n");
using namespace std;

#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
 int space;

void renderBitmapString(
		float x,
		float y,
		float z,
		void *font,
		char *string) {

	char *c;
	glRasterPos3f(x, y,z);
	for (c=string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}


class Vector3 {
	public:
		float x, y, z;
		
		Vector3 () {
		}
		Vector3 (float _x, float _y, float _z) {
			x = _x;
			y = _y;
			z = _z;
		}
		~Vector3 () {
		}
};


void drawVertex (int i, Vector3 *vertices)
{
	//glColor3f  (  colors[i].x,   colors[i].y,   colors[i].z);
	glVertex3f (vertices[i].x, vertices[i].y, vertices[i].z);
}


class ST
{
	public:
		float STx, STz, STvx, STvz, STh1, STy;
		float STr1, STr2, STh2, STn1, STn2;
		float speed;
		int score;
		ST()
		{
			STn1 = 0.1;
			STn2 = 0.3;
			STx = 0.0f;
			STz = 127.0f;
			STvx = 0.0f;
			STvz = 0.0f;
			STh1 = 2.5f;
			STr1 = 1.2f;
			STr2 = 1.0f;
			STh2 = 2.0f;
			speed = 0.0; 
			score = 0;
		}
		void upSTs(float s)
		{
			this->speed = s;
		}
		void upSTx(float x)
		{
			this->STx = x;
		}
		void upSTz(float z)
		{
			this->STz = z;
		}
		void upSTvx(float vx)
		{
			this->STvx = vx;
		}
		void upSTvz(float vz)
		{
			this->STvz = vz;
		}
		void upSTy(float y)
		{
			this->STy = y;
		}
};


class Target
{
public:
	float tarx, tarz, tarr1, tarr2, tarr3;
	Target()
	{
		tarr1 = 2.5f;
		tarr3 = 3.5f;
		tarr2 = 3.0f;
	} 
};
//Represents a terrain, by storing a set of heights and normals at 2D locations
class Terrain {
	private:
		int w; //Width
		int l; //Length
		float** hs; //Heights
		Vec3f** normals;
		bool computedNormals; //Whether normals is up-to-date
	public:
		Terrain(int w2, int l2) {
			w = w2;
			l = l2;
			
			hs = new float*[l];
			for(int i = 0; i < l; i++) {
				hs[i] = new float[w];
			}
			
			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}
			
			computedNormals = false;
		}
		
		~Terrain() {
			for(int i = 0; i < l; i++) {
				delete[] hs[i];
			}
			delete[] hs;
			
			for(int i = 0; i < l; i++) {
				delete[] normals[i];
			}
			delete[] normals;
		}
		
		int width() {
			return w;
		}
		
		int length() {
			return l;
		}
		
		//Sets the height at (x, z) to y
		void setHeight(int x, int z, float y) {
			hs[z][x] = y;
			computedNormals = false;
		}
		
		//Returns the height at (x, z)
		float getHeight(int x, int z) {
			return hs[z][x];
		}
		
		//Computes the normals, if they haven't been computed yet
		void computeNormals() {
			if (computedNormals) {
				return;
			}
			
			//Compute the rough version of the normals
			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}
			
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);
					
					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
					}
					
					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}
					
					normals2[z][x] = sum;
				}
			}
			
			//Smooth out the normals
			const float FALLOUT_RATIO = 0.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];
					
					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}
					
					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}
					normals[z][x] = sum;
				}
			}
			
			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;
			
			computedNormals = true;
		}
		
		//Returns the normal at (x, z)
		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};

//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for(int y = 0; y < image->height; y++) {
		for(int x = 0; x < image->width; x++) {
			unsigned char color = (unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}
	
	delete image;
	t->computeNormals();
	return t;
}


float ang = 0.0f;
float ang2 = -30.0f;
float ang3 =  0.0f;
float ang4 = 0.0f;
float trans = 0.0f;
Terrain* ter;

void cleanup() {
	delete ter;
}



ST top;
Target destination;

Vec3f g(0.0, -1.0, 0.0);
float dott;
Vec3f gt;

int sp = 1;

float xtrans = 0.0;
float ytrans = 0.0;
float ztrans = -7.8;
float horangle = -ang;
float verangle = -ang2;
int karma = 1;
void handleKeypress(unsigned char key, int x, int y) {
	switch (key) {
		case 27: //Escape key
			cleanup();
			exit(0);
		case 'a':
			//printf("sdas\n");
			//printf("a = %f\n", ang);
		karma = 1;
			ang2 = -30.0;
			ang += 1.0f;
			if (ang > 360)
				ang -= 360;
			xtrans = 0.0;
			ytrans = 0.0;
			ztrans = -7.8;
			horangle = -ang;
			verangle = -ang2;
			break;
		case 'd':
		karma = 2;
			ang2 = -30.0;
			ang-=1.0f;
			if(ang < 0)
				ang +=360;
			xtrans = 0.0;
			ytrans = 0.0;
			ztrans = -7.8;
			horangle = -ang;
			verangle = -ang2;
			break;
		case 'w':
			//printf("sdas\n");
			//printf("a = %f\n", ang);
		karma = 3;
			ang2 = -90.0f;
			xtrans = 0.0;
			ytrans = 0.0;
			ztrans = -7.8;
			horangle = -ang;
			verangle = -ang2;
			break;
		
		case 's':
			//printf("sdas\n");
			//printf("a = %f\n", ang);
		karma = 4;
			ang2 = -60.0f;
			xtrans = 0.0;
			ytrans = 0.0;
			ztrans = -7.8;
			horangle = -ang;
			verangle = -ang2;
			break;

		case 'e':
			//printf("sdas\n");
			//printf("a = %f\n", ang);
			karma = 5;
			ang2 = -30.0f;
			xtrans = -1;
			//cout << top.STy << endl;
			ytrans = 0.0;
			ztrans = -7.8;
			horangle = atan(destination.tarx/destination.tarz)*30;
			verangle = -ang2;
			break;

		case 'f':
			//printf("sdas\n");
			//printf("a = %f\n", ang);
			karma = 6;
			ang2 = -30.0f;
			xtrans = 0;
			//cout << top.STx << endl;
			ytrans = 0.0;
			ztrans = -7.8;
			horangle = atan(destination.tarx/destination.tarz)*30;
			verangle = -ang2;
			break;

		case 'g':
			//printf("sdas\n");
			//printf("a = %f\n", ang);
			karma = 7;
			ang2 = -30.0f;
			xtrans = 0;
			//cout << top.STx << endl;
			ytrans = 0.0;
			ztrans = -7.8;
			horangle = atan(destination.tarx/destination.tarz)*30;
			verangle = -ang2;
			break;
		case 'x':
			if(ang3!=-90)
				ang3-=5;
			break;
		case 'z':
			if(ang3!=90)
				ang3 += 5;
			break;
		case ' ':
			top.speed = sp*0.1;
			top.STvz = top.speed*cos(DEG2RAD(-ang3));
			top.STvx = top.speed*sin(DEG2RAD(-ang3));
			space = 1;
			break;
		case 'r':
			destination.tarx = rand()%128;
			destination.tarz = rand()%128;
			if(destination.tarz >= 64)
				destination.tarz -= 50;
			break;
		default:
			break;
	}
}

void myfuncnow(){

if(karma == 5){
	horangle = atan(destination.tarx/destination.tarz)*30;
}

if(karma == 6){
	horangle = atan(destination.tarx/destination.tarz)*30;
	ztrans = -top.STz/133 - 7.8;
	xtrans = 1.45 - top.STx/30 ;

}

if(karma == 7){
	horangle = atan(destination.tarx/destination.tarz)*30;	
	ztrans = -top.STz/20 + 1.0;
	xtrans = - top.STx/30 + 1.45;
	ytrans = 	-0.5;

}

	glTranslatef(xtrans, ytrans, ztrans);
	//glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(verangle, 1.0f, 0.0f, 0.0f);
	glRotatef(horangle, 0.0f, 1.0f, 0.0f);
	
}
void controlspeed(int key, int x, int y)
{
	if (key == GLUT_KEY_UP)
    {
    	if(sp!=10)
    	sp++;
    }
    else if(key == GLUT_KEY_DOWN)
    {
    	if(sp!=0)
    	sp--;
    }
    else if(key == GLUT_KEY_LEFT)
    {
    	if(top.STx!=0.0)
    		top.STx-=1.0;
    }
    else if(key == GLUT_KEY_RIGHT)
    {
    	if(top.STx!=127.0)
    		top.STx+=1.0;
    }
}




void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}

int check_collision()
{
	if((pow((top.STx - destination.tarx), 2) + pow((top.STz - destination.tarz), 2)) <= 10)
		return 1;
	return 0;
}

int check_boundary()
{
	if(top.STx >= 127 ||  top.STz <= 0 || top.STx <=0)
		return 1;
	return 0;
}

void drawtarget()
{
	glPushMatrix();
		glTranslatef(destination.tarx, ter->getHeight(destination.tarx, destination.tarz)+destination.tarr1+1, destination.tarz);
  		glColor3f(1.0, 0.0, 0.0);
  		glutSolidTorus(1, destination.tarr2, 10, 10);
  		glColor3f(1.0, 1.0, 1.0);
  		glutSolidTorus(1, destination.tarr1, 10, 10);
  		int val;
for(int i = 0;i< 8; i++){
	val = val + val/3;
}
  		glColor3f(1.0, 0.0, 0.0);
  		glutSolidSphere(destination.tarr1-1, 32, 32);
  	glPopMatrix();
}

void drawDirection()
{
	glPushMatrix();
	glColor3f(205.0/256.0, 201.0/256.0, 235.0/256.0);
		GLUquadricObj *quadratic3;
  		quadratic3 = gluNewQuadric();
  		glTranslatef(top.STx , ter->getHeight(top.STx,top.STz)+top.STh2 - 1, top.STz + 3.25);
		//glTranslatef(0.0, 0.0, 20.0);
  		glRotatef(ang3, 0.0, 1.0, 0.0);
  		gluCylinder(quadratic3, 0.2, 0.2, 15.0, 32, 32);
  		glPushMatrix();
  			glRotatef(45.0, 0.0, 1.0, 0.0);
  			gluCylinder(quadratic3, 0.2, 0.2, 5.0, 32, 32);
  			int val;
for(int i = 0;i< 8; i++){
	val = val + val/3;
}
  		glPopMatrix();
  		glPushMatrix();
  			glRotatef(-45.0, 0.0, 1.0, 0.0);
  			gluCylinder(quadratic3, 0.2, 0.2, 5.0, 32, 32);
  		glPopMatrix();
	glPopMatrix();
}

//power
void drawcube(float z, int i)
{
	glPushMatrix();
		glTranslatef(0, 20.0, z );
		glColor3f((i*28.4)/256.0, ((9-i)*28.4)/256.0, 0.0);
		//glutSolidSphere(2.0, 32, 32);
		GLUquadricObj *quadratic3;
  		quadratic3 = gluNewQuadric();
  		int val;
for(int i = 0;i< 8; i++){
	val = val + val/3;
}
  		gluCylinder(quadratic3, 1.0, 1.0, 8.0, 32, 32);
  		//glTranslatef(0.0, 0.0, 0.0);
  		//gluCylinder(quadratic3, 4.0, 4.0, 4.0, 32, 32);
		//glutSolidCube(8.0);
	glPopMatrix();
}

void drawtop(Vec3f nor)
{
	Vector3 verticesT [8];

	
	verticesT [0] = Vector3 (-0.25, -1.1,   0.25);
	verticesT [1] = Vector3 ( 0.25, -1.1,   0.25);
	verticesT [2] = Vector3 ( 0.25, -1.1,    -0.25);
	verticesT [3] = Vector3 (-0.25, -1.1,   -0.25);
	verticesT [4] = Vector3 (-0.25, 1.1, 0.25);
	verticesT [5] = Vector3 ( 0.25, 1.1, 0.25);
	verticesT [6] = Vector3 ( 0.25, 1.1,-0.25);
	verticesT [7] = Vector3 (-0.25, 1.1, -0.25);
	
	
	

	glPushMatrix();
		//lower tip
		
			glTranslatef(top.STx, ter->getHeight(top.STx,top.STz)+top.STh2, top.STz);
			glRotatef(90.0, nor[0], nor[1], nor[2]);
			glRotatef(90.0f, 1.0, 0.0, 0.0);
			glRotatef(ang4, 0.0, 0.0, -1.0);
			GLUquadricObj *quadratic;
  			quadratic = gluNewQuadric();
  			glColor3f(0.6,0.3,0);	
	gluCylinder(quadratic, top.STn2, top.STn1, top.STh2, 32, 32);
		int val;
for(int i = 0;i< 8; i++){
	val = val + val/3;
}

		//middle tip
		
			glTranslatef(0.0, 0.0, -top.STh1);
			glColor3f(0.8,0.4,0);
			GLUquadricObj *quadratic2;
			
for(int i = 0;i< 8; i++){
	val = val + val/3;
}
  			quadratic2 = gluNewQuadric();
  			gluCylinder(quadratic2, top.STr1+0.5, top.STn2, top.STh1+0.5, 32, 32);

		//upper handle
			glColor3f(0.4,0,0);

	glBegin (GL_QUADS);

	drawVertex (3, verticesT );
	drawVertex (2, verticesT );
	drawVertex (1, verticesT );
	drawVertex (0, verticesT );

	drawVertex (4, verticesT );
	drawVertex (5, verticesT );
	drawVertex (6, verticesT );
	drawVertex (7, verticesT );

	drawVertex (0, verticesT );
	drawVertex (1, verticesT );
	drawVertex (5, verticesT );
	drawVertex (4, verticesT );

	drawVertex (1, verticesT );
	drawVertex (2, verticesT );
	drawVertex (6, verticesT );
	drawVertex (5, verticesT );

	drawVertex (2, verticesT );
	drawVertex (3, verticesT );
	drawVertex (7, verticesT );
	drawVertex (6, verticesT );

	drawVertex (3, verticesT );
	drawVertex (0, verticesT );
	drawVertex (4, verticesT );
	drawVertex (7, verticesT );

	glEnd();
	glPopMatrix();
	
}




void drawScene() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

/*	glTranslatef(0.0f, 0.0f, -7.8f);
	//glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(-ang2, 1.0f, 0.0f, 0.0f);
	glRotatef(-atan(destination.tarx/destination.tarz)*30, 0.0f, 1.0f, 0.0f);
*/	
	myfuncnow();

	/*
	glTranslatef(0, -1.0, -7.8);
	//glTranslatef(0.0f, 0.0f, -7.8f);
	//glRotatef(-ang2, 1.0f, 0.0f, 0.0f);
	glRotatef(-atan(destination.tarx/destination.tarz)*30, 0.0f, 1.0f, 0.0f);
*/
	GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
	
	GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	
	float scale = 6.0f / max(ter->width() - 1, ter->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef(-(float)(ter->width() - 1) / 2,
				 0.0f,
				 -(float)(ter->length() - 1) / 2);

	glColor3f(0, 0.19, 0);
	for(int z = 0; z < ter->length() - 1; z++)
	{
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < ter->width(); x++) {
			Vec3f normal = ter->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, ter->getHeight(x, z), z);
			normal = ter->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, ter->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}
	Vec3f nor = ter->getNormal(top.STx, top.STz);
	//Vec3f g(0.0, 1.0, 0.0);
	dott = nor.dot(g);
	//printf("%d\n", top.score);
	gt = g + nor*dott;
	
	drawtop(nor);
	if(space == 0)
	drawDirection();
	
	for (int i = 0; i < sp; ++i)
	{
		drawcube(127.0 - i*5.0, i);
	}
	
	drawtarget();
char scbuf[100];
	
sprintf(scbuf,"%s","Score :");
			renderBitmapString(85,0.0,135.0f,GLUT_BITMAP_TIMES_ROMAN_24,scbuf);
			sprintf(scbuf,"%d",top.score);
			renderBitmapString(93,0.0,135.0f,GLUT_BITMAP_TIMES_ROMAN_24,scbuf);

	glutSwapBuffers();
}

void update(int value) {
	
	glutPostRedisplay();
	ang4+=10;
	//top.score++;
	if(check_collision())
	{
		top.score++;
		top.STx = 64.0f;
		top.STz = 127.0f;
		top.STvx = 0.0f;
		top.STvz = 0.0f;
		destination.tarx = rand()%128;
		destination.tarz = rand()%128;
		if(destination.tarz >= 64)
			destination.tarz -= 50;
		space = 0;
	}
	//printf("%f %f\n", top.STx, top.STz);
	if(check_boundary())
	{
		top.STx = 64.0f;
		top.STz = 127.0f;
		top.STvx = 0.0f;
		top.STvz = 0.0f;
		destination.tarx = rand()%128;
		destination.tarz = rand()%128;
		if(destination.tarz >= 64)
			destination.tarz -= 50;
		space = 0;
	}
	int val;
for(int i = 0;i< 8; i++){
	val = val + val/3;
}
	top.STx+=top.STvx;
	top.STz-=top.STvz;
	glutTimerFunc(25, update, 0);
}

int main(int argc, char** argv) {


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	int windowWidth = glutGet(GLUT_SCREEN_WIDTH);
    int windowHeight = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	space = 0;
	glutCreateWindow("Spinning Top");
	initRendering();
	destination.tarx = rand()%128;
	destination.tarz = rand()%128;
	if(destination.tarz >= 64)
		destination.tarz -= 50;
	ter = loadTerrain("Sample.bmp", 20);
	//cout << ter->length();
	//cout << ter->width();
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(controlspeed);
	glutReshapeFunc(handleResize);

	glutTimerFunc(25, update, 0);
	
	glutMainLoop();
	return 0;
}









