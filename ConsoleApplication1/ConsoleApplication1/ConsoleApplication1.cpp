#include "stdafx.h"
#include<gl/glut.h> 
#include<windows.h> 
#include<math.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <vector>
using namespace std;

GLfloat light_position1[] = { 0, 28, -20, 1.0 };
GLfloat model_ambient[] = { 0.05f, 0.05f, 0.05f, 1.0f };

GLfloat mat_specular[] = { 0.8, 1.0, 1.0, 1.0 };
GLfloat mat_shininess[] = { 5.0 };
GLfloat mat_ambient[] = { 0.1, 0.1, 0.1, 1 };

GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light[] = { 1.0, 1.0, 1.0, 1 };
GLfloat light_position0[] = { 0, 28, 20, 1.0 };

GLUquadricObj *m_quadratic = gluNewQuadric();				// Create A Pointer To The Quadric Object (Return 0 If No Memory)
GLfloat bx = 0;
GLfloat bz = 0;
GLfloat bx_old = 0;
GLfloat bz_old = 0;
vector<GLfloat> vbx, vbz;

GLint WinWidth;
GLint WinHeight;

//eye point
typedef struct EyePoint
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
}EyePoint;

EyePoint myEye;
EyePoint vPoint;
GLfloat vAngle = 0;


#define BMP_Header_Length 54 
void grab(void) 
{
	FILE* pDummyFile; FILE* pWritingFile;
	GLubyte* pPixelData;
	GLubyte BMP_Header[BMP_Header_Length];
	GLint i, j;
	GLint PixelDataLength;

	i = WinWidth * 3;
	while (i % 4 != 0)
		++i;
	PixelDataLength = i * WinHeight;
	pPixelData = (GLubyte*)malloc(PixelDataLength);
	if (pPixelData == 0)
		exit(0);
	pDummyFile = fopen("dummy.bmp", "rb");
	if (pDummyFile == 0)
		exit(0);
	pWritingFile = fopen("grab.bmp", "wb");
	if (pWritingFile == 0)
		exit(0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glReadPixels(0, 0, WinWidth, WinHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, pPixelData);

	fread(BMP_Header, sizeof(BMP_Header), 1, pDummyFile);
	fwrite(BMP_Header, sizeof(BMP_Header), 1, pWritingFile);
	fseek(pWritingFile, 0x0012, SEEK_SET);
	i = WinWidth;
	j = WinHeight;
	fwrite(&i, sizeof(i), 1, pWritingFile);
	fwrite(&j, sizeof(j), 1, pWritingFile);
	fseek(pWritingFile, 0, SEEK_END);
	fwrite(pPixelData, PixelDataLength, 1, pWritingFile);
	fclose(pDummyFile); fclose(pWritingFile); free(pPixelData);
}

int power_of_two(int n)
{
	if (n <= 0)
		return 0;
	return (n & (n - 1)) == 0;
}


GLuint load_texture(const char* file_name)
{
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLint last_texture_ID = 0;
	GLuint texture_ID = 0;


	FILE* pFile = fopen(file_name, "rb");
	if (pFile == 0)
		return 0;

	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	{
		GLint line_bytes = width * 3;
		while (line_bytes % 4 != 0)
			++line_bytes;
		total_bytes = line_bytes * height;
	}

	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		return 0;
	} 

	if (fread(pixels, total_bytes, 1, pFile) <= 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	} 

	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if (!power_of_two(width) || !power_of_two(height) || width > max || height > max)
		{
			const GLint new_width = 256;
			const GLint new_height = 256;
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;

			new_line_bytes = new_width * 3;
			while (new_line_bytes % 4 != 0)
				++new_line_bytes;
			new_total_bytes = new_line_bytes * new_height;

			new_pixels = (GLubyte*)malloc(new_total_bytes);
			if (new_pixels == 0){
				free(pixels);
				fclose(pFile);
				return 0;
			}

			gluScaleImage(GL_RGB, width, height, GL_UNSIGNED_BYTE, pixels, new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);
			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}

	glGenTextures(1, &texture_ID);
	if (texture_ID == 0) 
	{
		free(pixels);
		fclose(pFile);
		return 0;
	} 

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture_ID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture_ID);


	free(pixels);
	return texture_ID;
}

//set the names of the texture objects  
GLuint texblackboard, texwindow, texceiling,
texdoor, texbackwall, texgaodi, textdesk;

void drawscence()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texceiling);
	glColor3f(0.3, 0.3, 0.3);

	glBegin(GL_QUADS);
	glNormal3f(0.0f, -1.0f, 0.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-50.0f, 30.0f, 30.0f);

	glTexCoord2f(0.0f, 3.0f);
	glVertex3f(-50.0f, 30.0f, -30.0f);

	glTexCoord2f(6.0f, 3.0f);
	glVertex3f(50.0f, 30.0f, -30.0f);

	glTexCoord2f(6.0f, 0.0f);
	glVertex3f(50.0f, 30.0f, 30.0f);

	glEnd();
	glDisable(GL_TEXTURE_2D);

	//draw the floor
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);

	glVertex3f(-50.0f, 0.0f, 30.0f);
	glVertex3f(-50.0f, 0.0f, -30.0f);
	glVertex3f(50.0f, 0.0f, -30.0f);
	glVertex3f(50.0f, 0.0f, 30.0f);
	glEnd();

	//the wall and the windows in left
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_QUADS);
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-50.0f, 0.0f, 30.0f);
	glVertex3f(-50.0f, 30.0f, 30.0f);
	glVertex3f(-50.0f, 30.0f, -30.0f);
	glVertex3f(-50.0f, 0.0f, -30.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, texwindow);

	for (int n = 0; n <= 1; n++)
	{
		glBegin(GL_QUADS);
		glNormal3f(1.0, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-49.9, 10, -8 + n * 18);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-49.9, 20, -8 + n * 18);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-49.9, 20, -18 + n * 18);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-49.9, 10, -18 + n * 18);
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);

	//the wall and the window in right 
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_QUADS);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(50.0f, 0.0f, 30.0f);
	glVertex3f(50.0f, 30.0f, 30.0f);
	glVertex3f(50.0f, 30.0f, -30.0f);
	glVertex3f(50.0f, 0.0f, -30.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, texwindow);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(49.5, 10, 10);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(49.5, 20, 10);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(49.5, 20, 0);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(49.5, 10, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//back wall 
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texbackwall);
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-50.0f, 0.0f, 30.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-50.0f, 30.0f, 30.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(50.0f, 30.0f, 30.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(50.0f, 0.0f, 30.0f);
	glEnd();

	//front wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texbackwall);
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-50.0f, 0.0f, -30.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-50.0f, 30.0f, -30.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(50.0f, 30.0f, -30.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(50.0f, 0.0f, -30.0f);
	glEnd();

	//blackboard
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texblackboard);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-20.0, 8.0f, -29.9f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-20.0, 18.0f, -29.9f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(20.0, 18.0f, -29.9f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(20.0, 8.0f, -29.9f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//gao di 
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texgaodi);

	//top 
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-30.0f, 1.5f, -22.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-30.0f, 1.5f, -30.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(30.0f, 1.5f, -30.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(30.0f, 1.5f, -22.0f);
	glEnd();

	//down
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-30.0f, 0, -22.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-30.0f, 1.5f, -22.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-30.0f, 1.5f, -30.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-30.0f, 0, -30.0f);
	glEnd();

	//front 
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-30.0f, 0, -22.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-30.0f, 1.5f, -22.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(30.0f, 1.5f, -22.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(30.0f, 0, -22.0f);
	glEnd();

	//back 
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(30.0f, 0, -22.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(30.0f, 1.5f, -22.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(30.0f, 1.5f, -30.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(30.0f, 0, -30.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//door 
	glColor3f(0.521f, 0.121f, 0.0547f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texdoor);
	glBegin(GL_QUADS);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(49.9f, 0.0f, -25.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(49.9f, 14.0f, -25.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(49.9f, 14.0f, -19.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(49.9f, 0.0f, -19.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

}

void reshape(int we, int he)
{
	WinWidth = we;
	WinHeight = he;

	glViewport(0, 0, (GLsizei)we, (GLsizei)he);
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(90.0f, (GLfloat)we / (GLfloat)he, 0.01f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(myEye.x, myEye.y, myEye.z, vPoint.x + 30 * sin(vAngle), vPoint.y, -30 * cos(vAngle), 0.0f, 1.0f, 0.0f);
}

void drawAnchor()
{
	glPointSize(12);   // 绘制前设置下点的大小和颜色
	glColor3f(1, 0, 0);
	glBegin(GL_POINTS);
	glVertex3f(0, 1, 0);
	glVertex3f(-22.5, 1.6, 4.5);
	glVertex3f(-2.5, 1.6, -27.8);
	glVertex3f(-15.6, 1.6, -26.0);
	glEnd();
}



void drawMoving()
{
	glLineWidth(5);
	glColor3f(0, 1, 0);
	glBegin(GL_LINE_STRIP);
	glVertex3f(0, 1, 0);
	glVertex3f(1, 1, 0);
	
	for (int i = 0; i != vbx.size(); i++)
	{
		glVertex3f(vbx.at(i), 1, vbz.at(i));
	}
	
	glEnd();
}

void drawball()
{
	gluQuadricNormals(m_quadratic, GLU_SMOOTH);		 // Create Smooth Normals 
	gluQuadricTexture(m_quadratic, GL_TRUE);		// Create Texture Coords 

	glColor3f(1.1, 0.01, 0.02);
	glPushMatrix();
	glTranslatef(bx, 0.5, bz);
	glScalef(0.5, 0.5, 0.5);
	gluCylinder(m_quadratic, 0.7, 0.7, 0.5, 26, 13);
	gluDisk(m_quadratic, 0.0, 1.5, 26, 13);
	glPopMatrix();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawball();

	drawAnchor();

	drawMoving();
	drawscence();

	glFlush();
}

//the action of the keyboard
GLvoid OnKeyboard(unsigned char key, int x, int y)
{
	switch (key){
		//left
	case 97:
		myEye.x -= 0.5;
		vPoint.x -= 0.5;
		if (myEye.x <= -40)
			myEye.x = -40;
		break;
		//right
	case 100:
		myEye.x += 0.5;
		vPoint.x += 0.5;
		if (myEye.x >= 40)
			myEye.x = 40;
		break;
		//down
	case 119:
		myEye.z -= 0.5;
		if (myEye.z <= -30)
			myEye.z = -30;
		break;
		//up
	case 115:
		myEye.z += 0.5;
		if (myEye.z >= 30)
			myEye.z = 30;
		break;
	case 'j':
		bx -= 1;
		if (bx <= -39)
		{
			bx = -39;
		}
		break;
	case 'l':
		bx += 1;
		if (bx >= 39)
		{
			bx = 39;
		}
		break;
	case 'k':
		bx = 0;
		bz = 0;
		break;
	case 'i':
		bz -= 1;
		if (bz <= -29)
		{
			bz = -29;
		}
		break;
	case 'm':
		bz += 1;
		if (bz >= 29)
		{
			bz = 29;
		}
		break;
	case 27:
		exit(0);
	}

	if (bx_old != bx || bz_old != bz)
	{
		vbx.push_back(bx);
		vbz.push_back(bz);
		bx_old = bx;
		bz_old = bz;
	}
	printf("x: %f, y: %f\n", bx * 10, bz * 10);
	reshape(WinWidth, WinHeight);
	glutPostRedisplay();
}

GLvoid OnSpecial(int key, int x, int y)
{
	switch (key){

	case GLUT_KEY_LEFT:
		vAngle -= 0.05;
		break;
	case GLUT_KEY_RIGHT:
		vAngle += 0.05;
		break;

	case GLUT_KEY_UP:
		myEye.y += 0.05;
		if (myEye.y >= 30)
			myEye.y = 30;
		break;

	case GLUT_KEY_DOWN:
		myEye.y -= 0.5;
		if (myEye.y <= 0)
			myEye.y = 30;
		break;

	case GLUT_KEY_PAGE_DOWN:
		myEye.z += 0.5;
		if (myEye.z >= 30)
			myEye.z = 30;
		break;

	case GLUT_KEY_PAGE_UP:
		myEye.z -= 0.5;
		if (myEye.z <= -30)
			myEye.z = -30;
		break;
	}
	reshape(WinWidth, WinHeight);
	glutPostRedisplay();
}

GLvoid OnIdle()
{
	glutPostRedisplay();
}

//initial many parameters including the light and so on
void initial()
{
	glClearColor(0, 0, 0, 0);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);//set the mode of the current texture mapping 

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, mat_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, mat_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH); //set the mode of the color(flat or smooth)
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glEnable(GL_DEPTH_TEST);
}


int main(int argc, char* argv[])
{
	myEye.x = 0;
	myEye.y = 15;
	myEye.z = 25;

	vPoint.x = 0;
	vPoint.y = 15;
	vPoint.z = -30;
	vAngle = 0;

	glEnable(GL_DEPTH_TEST);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowPosition(400, 0);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Demo Lab Room");

	initial();

	glutDisplayFunc(&display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(OnKeyboard);
	glutSpecialFunc(OnSpecial);
	glutIdleFunc(OnIdle);

	/* set the texture */
	texblackboard = load_texture("blackboard.bmp");
	texwindow = load_texture("window.bmp");
	texgaodi = load_texture("gaodi.bmp");
	texceiling = load_texture("ceiling.bmp");
	texdoor = load_texture("door.bmp");
	texbackwall = load_texture("backwall.bmp");
	textdesk = load_texture("tdesk.bmp");

	glutMainLoop();

	return 0;
}
