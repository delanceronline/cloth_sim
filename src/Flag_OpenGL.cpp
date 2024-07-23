// TriOBBCD_OpenGL.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Flag_OpenGL.h"
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <gl\glaux.h>		// Header File For The Glaux Library
#include <gl\glut.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include "Vector3D.h"
#include "Matrix.h"
#include "Particle.h"
#include "Spring.h"
#include "Sphere.h"
#include "Plane.h"

#define DEG_RAD 0.017453292519943295769236907684886
#define RAD_DEG 57.295779513082320876798154814105
#define ETOL 1e-5f

#define NUM_COL_PARTICLE 21
#define NUM_ROW_PARTICLE 21
#define GAP_WIDTH_H 4
#define GAP_WIDTH_V 4

// Global Variables:

HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool	keys[256];			// Array Used For The Keyboard Routine
bool	active=TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen=TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default
GLuint	base;

Vector3D gCameraPos;
Vector3D gGravity = Vector3D(0.0f, -9.8f, 0.0f);
Vector3D gInitPos = Vector3D(-2.0f, 2.0f, 0.0f);
Vector3D gOffset = Vector3D(0.0f, 0.0f, 0.0f);

bool gIsDetach = false;
bool gIsEnableShear = true;
bool gIsEnableBend = false;
float gWindLevel = 10.0f;

Particle gParticle[NUM_COL_PARTICLE * NUM_ROW_PARTICLE];
Spring gVSpring[(NUM_ROW_PARTICLE - 1) * NUM_COL_PARTICLE];
Spring gHSpring[(NUM_COL_PARTICLE - 1) * NUM_ROW_PARTICLE];
Spring gShearSpringLR[(NUM_ROW_PARTICLE - 1) * (NUM_COL_PARTICLE - 1)];
Spring gShearSpringRL[(NUM_ROW_PARTICLE - 1) * (NUM_COL_PARTICLE - 1)];

Spring gBendSpringH[NUM_ROW_PARTICLE * (NUM_COL_PARTICLE - 2)];
Spring gBendSpringV[NUM_COL_PARTICLE * (NUM_ROW_PARTICLE - 2)];

//To hold instant speed and direction of particle at once, for enhancing performance in each simulation loop.
float gParticles_vMag[NUM_COL_PARTICLE * NUM_ROW_PARTICLE];
Vector3D gParticles_unitDir[NUM_COL_PARTICLE * NUM_ROW_PARTICLE];

Sphere gSphere;
Plane gPlane;

GLUquadricObj *gluObj, *gluObjSphere;
GLuint	texture[3];

float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;

// Forward declarations of functions included in this code module:
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

AUX_RGBImageRec *LoadBMP(char *Filename);
int LoadGLTextures();

Matrix *GetRM_3x3(float x, float y, float z);
void GetRM_3x3(float x, float y, float z, Matrix *pRMat);
void InitParticles(void);
void InitSprings(void);
void Simulate(float dt);
void SolveSpring(Spring *pSpring);
bool ParticleSphereCD(Particle *pAtom, Sphere *pSphere, Vector3D *pContact, Vector3D *pNormal);
bool ParticlePlaneCD(Particle *pAtom, Plane *pPlane, Vector3D *pContact, Vector3D *pNormal);

GLvoid BuildFont(GLvoid);
GLvoid KillFont(GLvoid);
GLvoid glPrint(const char *fmt, ...);

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,1.0f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	float lightpos0[4], ambientLight[4], diffuseLight[4], specular[4], specref[4];

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	lightpos0[0] = 0.0f;
	lightpos0[1] = 0.0f;
	lightpos0[2] = 20.0f;
	lightpos0[3] = 0.0f;

	ambientLight[0] = 1.0f;
	ambientLight[1] = 1.0f;
	ambientLight[2] = 1.0f;
	ambientLight[3] = 1.0f;

	diffuseLight[0] = 1.0f;
	diffuseLight[1] = 1.0f;
	diffuseLight[2] = 1.0f;
	diffuseLight[3] = 1.0f;

	specular[0] = 0.6f;
	specular[1] = 0.6f;
	specular[2] = 0.6f;
	specular[3] = 0.6f;

	specref[0] = 1.0f;
	specref[1] = 1.0f;
	specref[2] = 1.0f;
	specref[3] = 1.0f;

	glEnable(GL_LIGHTING);

	glLightfv(GL_LIGHT0, GL_POSITION, lightpos0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glEnable(GL_LIGHT0);

	if(!LoadGLTextures())								
		return FALSE;									

	BuildFont();

	return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	float color[4], TexGapX, TexGapY, U, V;
	Matrix RMat(3, 3);
	Vector3D v1, v2, v3, v4, n1, n2;
	int row, col, i = 0;

	//Simulation.
	Simulate(0.01f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix

	gluLookAt(gCameraPos.x, gCameraPos.y, gCameraPos.z, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	
	glDisable(GL_TEXTURE_2D);

	//Draw rod.
	color[0] = 0.3f; color[1] = 0.3f; color[2] = 0.3f; color[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

	glPushMatrix();
	glTranslatef(gInitPos.x, gInitPos.y, gInitPos.z);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	gluCylinder(gluObj, 0.1f, 0.1f, 4.0f, 20, 20);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);

	//Draw ball.
	color[0] = 0.6f; color[1] = 0.6f; color[2] = 0.6f; color[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

	glBindTexture(GL_TEXTURE_2D, texture[1]);

	glPushMatrix();
	glTranslatef(gSphere.r.x, gSphere.r.y, gSphere.r.z);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	gluSphere(gluObjSphere, gSphere.radius * 0.9f, 20, 20);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glPopMatrix();

	//Draw floor.
	color[0] = 0.6f; color[1] = 0.6f; color[2] = 0.6f; color[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

	glBindTexture(GL_TEXTURE_2D, texture[2]);

	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0, 0);
		glVertex3f(50.0f, -4.1f, 50.0f);
		glTexCoord2f(0.0f, 25.0f);
		glVertex3f(50.0f, -4.1f, -50.0f);
		glTexCoord2f(25.0f, 25.0f);
		glVertex3f(-50.0f, -4.1f, -50.0f);
		glTexCoord2f(25.0f, 0.0f);
		glVertex3f(-50.0f, -4.1f, 50.0f);
	glEnd();

	//Draw cloth.
	color[0] = 0.4f; color[1] = 0.4f; color[2] = 0.4f; color[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

	TexGapX = 1.0f / (float)(NUM_COL_PARTICLE - 1);
	TexGapY = 1.0f / (float)(NUM_ROW_PARTICLE - 1);
	U = 0.0f;
	V = 1.0f;
	
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glBegin(GL_TRIANGLES);

	for(row = 0; row < NUM_ROW_PARTICLE - 1; row++)
	{
		for(col = 0; col < NUM_COL_PARTICLE - 1; col++)
		{
			i = row * NUM_COL_PARTICLE + col;

			v1 = gParticle[i].r;
			v2 = gParticle[(row + 1) * NUM_COL_PARTICLE + col].r;
			v3 = gParticle[(row + 1) * NUM_COL_PARTICLE + col + 1].r;
			v4 = gParticle[i + 1].r;

			n1 = ((v3 - v2).CrossProduct(&(v1 - v2))).Normalize();
			n2 = ((v4 - v3).CrossProduct(&(v1 - v3))).Normalize();

			//Triangle 1.
			glNormal3f(n1.x, n1.y, n1.z);

			glTexCoord2f(U, V);
			glVertex3f(v1.x, v1.y, v1.z);

			glTexCoord2f(U, V - TexGapY);
			glVertex3f(v2.x, v2.y, v2.z);

			glTexCoord2f(U + TexGapX, V - TexGapY);
			glVertex3f(v3.x, v3.y, v3.z);

			//Triangle 2.
			glNormal3f(n2.x, n2.y, n2.z);

			glTexCoord2f(U, V);
			glVertex3f(v1.x, v1.y, v1.z);

			glTexCoord2f(U + TexGapX, V - TexGapY);
			glVertex3f(v3.x, v3.y, v3.z);

			glTexCoord2f(U + TexGapX, V);
			glVertex3f(v4.x, v4.y, v4.z);
			
			U += TexGapX;
		}

		U = 0.0f;
		V -= TexGapY;
	}

	glEnd();

	//Draw texts.
	glDisable(GL_TEXTURE_2D);

	color[0] = 0.9f; color[1] = 0.9f; color[2] = 0.9f; color[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

	glLoadIdentity();									// Reset The Current Modelview Matrix
	glTranslatef(0.0f,0.0f,-1.0f);						// Move One Unit Into The Screen

	glRasterPos2f(-0.54f, -0.24f);
	if(!gIsEnableShear)
		glPrint("Enable Shear Springs: F1");
	else
		glPrint("Disable Shear Springs: F1");

	glRasterPos2f(-0.54f, -0.26f);
	if(!gIsEnableBend)
		glPrint("Enable Bend Springs: F2");
	else
		glPrint("Disable Bend Springs: F2");

	glRasterPos2f(-0.54f, -0.28f);
	glPrint("Fullscreen: F3");

	glRasterPos2f(-0.54f, -0.3f);
	glPrint("Camera Forward: A");

	glRasterPos2f(-0.54f, -0.32f);
	glPrint("Camera Backward: Z");

	glRasterPos2f(-0.54f, -0.34f);
	glPrint("Wind speed: 0-9");

	glRasterPos2f(-0.54f, -0.36f);
	glPrint("Detach: D");

	glRasterPos2f(-0.54f, -0.38f);
	glPrint("Reset: R");

	glRasterPos2f(-0.54f, -0.4f);
	glPrint("Rod's Movements: Arrow, INS, DEL");

	gOffset = Vector3D(0.0f, 0.0f, 0.0f);

	return TRUE;										// Keep Going
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}

	KillFont();
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG		msg;									// Windows Message Structure
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	fullscreen=FALSE;

	// Create Our OpenGL Window
	if (!CreateGLWindow("MingFai's Cloth Simulation",640,480,16,fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	srand((unsigned)time(NULL));

	gCameraPos = Vector3D(0.0f, -1.0f, 8.0f);
	gSphere.r = Vector3D(0.0f, -2.0f, -2.0f);
	gSphere.radius = 1.0f;

	gPlane.p = Vector3D(0.0f, -4.0f, 0.0f);
	gPlane.n = Vector3D(0.0f, 1.0f, 0.0f);

	gluObj = gluNewQuadric();
	gluObjSphere = gluNewQuadric();

	InitParticles();
	InitSprings();

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done=TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
			}

			if(keys[VK_F1])
			{
				keys[VK_F1] = FALSE;
				gIsEnableShear = !gIsEnableShear;
			}

			if(keys[VK_F2])
			{
				keys[VK_F2] = FALSE;
				gIsEnableBend = !gIsEnableBend;
			}

			if (keys[VK_F3])						// Is F1 Being Pressed?
			{
				keys[VK_F3]=FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("MingFai's Cloth Simulation",640,480,16,fullscreen))
				{
					return 0;						// Quit If Window Was Not Created
				}
			}

			if(GetAsyncKeyState('A'))
				gCameraPos.z -= 0.1f;

			if(GetAsyncKeyState('Z'))
				gCameraPos.z += 0.1f;

			if(GetAsyncKeyState(VK_UP))
			{
				gOffset.z = -0.01f;
				gInitPos.z += gOffset.z;
			}

			if(GetAsyncKeyState(VK_DOWN))
			{
				gOffset.z = 0.01f;
				gInitPos.z += gOffset.z;
			}

			if(GetAsyncKeyState(VK_LEFT))
			{
				gOffset.x = -0.01f;
				gInitPos.x += gOffset.x;
			}

			if(GetAsyncKeyState(VK_RIGHT))
			{
				gOffset.x = 0.01f;
				gInitPos.x += gOffset.x;
			}

			if(GetAsyncKeyState(VK_INSERT))
			{
				if(gInitPos.y <= 10.0f)
				{
					gOffset.y = 0.01f;
					gInitPos.y += gOffset.y;
				}
			}

			if(GetAsyncKeyState(VK_DELETE))
			{
				if(gInitPos.y >= -3.8f)
				{
					gOffset.y = -0.01f;
					gInitPos.y += gOffset.y;
				}
			}

			if(GetAsyncKeyState('R'))
			{
				InitParticles();
				InitSprings();

				gIsDetach = false;
			}

			if(GetAsyncKeyState('D'))
				gIsDetach = true;

			if(GetAsyncKeyState('0'))
				gWindLevel = 0.0f;

			if(GetAsyncKeyState('1'))
				gWindLevel = 10.0f;

			if(GetAsyncKeyState('2'))
				gWindLevel = 20.0f;

			if(GetAsyncKeyState('3'))
				gWindLevel = 30.0f;

			if(GetAsyncKeyState('4'))
				gWindLevel = 40.0f;

			if(GetAsyncKeyState('5'))
				gWindLevel = 50.0f;

			if(GetAsyncKeyState('6'))
				gWindLevel = 60.0f;

			if(GetAsyncKeyState('7'))
				gWindLevel = 70.0f;

			if(GetAsyncKeyState('8'))
				gWindLevel = 80.0f;

			if(GetAsyncKeyState('9'))
				gWindLevel = 90.0f;
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window

	gluDeleteQuadric(gluObj);
	gluDeleteQuadric(gluObjSphere);

	return (msg.wParam);	
}



//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)									// Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				active=TRUE;						// Program Is Active
			}
			else
			{
				active=FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE

			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}

		case WM_LBUTTONDOWN:
		{
			return 0;
		}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,message,wParam,lParam);
}

Matrix *GetRM_3x3(float x, float y, float z)
{
	Matrix *pRMat = new Matrix(3, 3);

	Matrix RotX(3, 3), RotY(3, 3), RotZ(3, 3);

	RotX.SetToIdentity();
	RotX.m[4] = cosf(x);
	RotX.m[5] = -sinf(x);
	RotX.m[7] = -RotX.m[5];
	RotX.m[8] = RotX.m[4];

	RotY.SetToIdentity();
	RotY.m[0] = cosf(y);
	RotY.m[2] = sinf(y);
	RotY.m[6] = -RotY.m[2];
	RotY.m[8] = RotY.m[0];

	RotZ.SetToIdentity();
	RotZ.m[0] = cosf(z);
	RotZ.m[1] = -sinf(z);
	RotZ.m[3] = -RotZ.m[1];
	RotZ.m[4] = RotZ.m[0];

	RotX.Product(&RotY, &RotY);
	RotY.Product(&RotZ, pRMat);

	return pRMat;
}

void GetRM_3x3(float x, float y, float z, Matrix *pRMat)
{
	Matrix RotX(3, 3), RotY(3, 3), RotZ(3, 3);

	RotX.SetToIdentity();
	RotX.m[4] = cosf(x);
	RotX.m[5] = -sinf(x);
	RotX.m[7] = -RotX.m[5];
	RotX.m[8] = RotX.m[4];

	RotY.SetToIdentity();
	RotY.m[0] = cosf(y);
	RotY.m[2] = sinf(y);
	RotY.m[6] = -RotY.m[2];
	RotY.m[8] = RotY.m[0];

	RotZ.SetToIdentity();
	RotZ.m[0] = cosf(z);
	RotZ.m[1] = -sinf(z);
	RotZ.m[3] = -RotZ.m[1];
	RotZ.m[4] = RotZ.m[0];

	RotX.Product(&RotY, &RotY);
	RotY.Product(&RotZ, pRMat);
}

void InitParticles(void)
{
	int i, j, k;
	float GapX = (float)GAP_WIDTH_H / ((float)NUM_COL_PARTICLE - 1.0f);
	float GapY = (float)GAP_WIDTH_V / ((float)NUM_ROW_PARTICLE - 1.0f);

	k = 0;
	for(i = 0; i < NUM_ROW_PARTICLE; i++)
	{
		for(j = 0; j < NUM_COL_PARTICLE; j++)
		{
			gParticle[k].mass = 0.3f;
			
			if(gParticle[k].mass > -1e-5 && gParticle[k].mass < 1e-5)
				gParticle[k].invMass = 0.0f;
			else
				gParticle[k].invMass = 1.0f / gParticle[k].mass;

			gParticle[k].AirDragCoef = 0.08f;
			gParticle[k].r = Vector3D((float)j * GapX + gInitPos.x, gInitPos.y, (float)i * GapY + gInitPos.z);
			gParticle[k].v = Vector3D(0.0f, 0.0f, 0.0f);
			gParticle[k].a = Vector3D(0.0f, 0.0f, 0.0f);
			gParticle[k].IsFixed = false;

			k++;
		}
	}

	for(i = 0; i < NUM_COL_PARTICLE; i += 5)
		gParticle[i].IsFixed = true;
	gParticle[NUM_COL_PARTICLE - 1].IsFixed = true;
}

void InitSprings(void)
{
	int row, col;

	//Springs in column.
	for(row = 0; row < NUM_ROW_PARTICLE; row++)
	{
		for(col = 0; col < NUM_COL_PARTICLE - 1; col++)
		{
			gHSpring[row * (NUM_COL_PARTICLE - 1) + col].k = 500.0f;
			gHSpring[row * (NUM_COL_PARTICLE - 1) + col].DampCoef = 2.0f;
			gHSpring[row * (NUM_COL_PARTICLE - 1) + col].length = (gParticle[row * NUM_COL_PARTICLE + col].r - gParticle[row * NUM_COL_PARTICLE + col + 1].r).Mag();
			gHSpring[row * (NUM_COL_PARTICLE - 1) + col].nObj1 = row * NUM_COL_PARTICLE + col;
			gHSpring[row * (NUM_COL_PARTICLE - 1) + col].nObj2 = row * NUM_COL_PARTICLE + col + 1;
		}
	}

	//Springs in row.
	for(col = 0; col < NUM_COL_PARTICLE; col++)
	{
		for(row = 0; row < NUM_ROW_PARTICLE - 1; row++)
		{
			gVSpring[row * NUM_COL_PARTICLE + col].k = 500.0f;
			gVSpring[row * NUM_COL_PARTICLE + col].DampCoef = 2.0f;
			gVSpring[row * NUM_COL_PARTICLE + col].length = (gParticle[row * NUM_COL_PARTICLE + col].r - gParticle[(row + 1) * NUM_COL_PARTICLE + col].r).Mag();
			gVSpring[row * NUM_COL_PARTICLE + col].nObj1 = row * NUM_COL_PARTICLE + col;
			gVSpring[row * NUM_COL_PARTICLE + col].nObj2 = (row + 1) * NUM_COL_PARTICLE + col;
		}
	}

	//Shear springs from upper left to lower right.
	for(row = 0; row < NUM_COL_PARTICLE - 1; row++)
	{
		for(col = 0; col < NUM_ROW_PARTICLE - 1; col++)
		{
			gShearSpringLR[row * (NUM_COL_PARTICLE - 1) + col].k = 600.0f;
			gShearSpringLR[row * (NUM_COL_PARTICLE - 1) + col].DampCoef = 2.0f;
			gShearSpringLR[row * (NUM_COL_PARTICLE - 1) + col].length = (gParticle[row * NUM_COL_PARTICLE + col].r - gParticle[(row + 1) * NUM_COL_PARTICLE + col + 1].r).Mag();
			gShearSpringLR[row * (NUM_COL_PARTICLE - 1) + col].nObj1 = row * NUM_COL_PARTICLE + col;
			gShearSpringLR[row * (NUM_COL_PARTICLE - 1) + col].nObj2 = (row + 1) * NUM_COL_PARTICLE + col + 1;
		}
	}

	//Shear springs from upper right to lower left.
	for(row = 0; row < NUM_COL_PARTICLE - 1; row++)
	{
		for(col = 0; col < NUM_ROW_PARTICLE - 1; col++)
		{
			gShearSpringRL[row * (NUM_COL_PARTICLE - 1) + col].k = 600.0f;
			gShearSpringLR[row * (NUM_COL_PARTICLE - 1) + col].DampCoef = 2.0f;
			gShearSpringRL[row * (NUM_COL_PARTICLE - 1) + col].length = (gParticle[row * NUM_COL_PARTICLE + col + 1].r - gParticle[(row + 1) * NUM_COL_PARTICLE + col].r).Mag();
			gShearSpringRL[row * (NUM_COL_PARTICLE - 1) + col].nObj1 = row * NUM_COL_PARTICLE + col + 1;
			gShearSpringRL[row * (NUM_COL_PARTICLE - 1) + col].nObj2 = (row + 1) * NUM_COL_PARTICLE + col;
		}
	}

	for(row = 0; row < NUM_ROW_PARTICLE; row++)
	{
		for(col = 0; col < NUM_COL_PARTICLE - 2; col++)
		{
			gBendSpringH[row * (NUM_COL_PARTICLE - 2) + col].k = 300.0f;
			gBendSpringH[row * (NUM_COL_PARTICLE - 2) + col].DampCoef = 2.0f;
			gBendSpringH[row * (NUM_COL_PARTICLE - 2) + col].length = (gParticle[row * NUM_COL_PARTICLE + col].r - gParticle[row * NUM_COL_PARTICLE + col + 2].r).Mag();
			gBendSpringH[row * (NUM_COL_PARTICLE - 2) + col].nObj1 = row * NUM_COL_PARTICLE + col;
			gBendSpringH[row * (NUM_COL_PARTICLE - 2) + col].nObj2 = row * NUM_COL_PARTICLE + col + 2;
		}
	}

	for(col = 0; col < NUM_COL_PARTICLE; col++)
	{
		for(row = 0; row < NUM_ROW_PARTICLE - 2; row++)
		{
			gBendSpringV[col * (NUM_ROW_PARTICLE - 2) + row].k = 300.0f;
			gBendSpringV[col * (NUM_ROW_PARTICLE - 2) + row].DampCoef = 2.0f;
			gBendSpringV[col * (NUM_ROW_PARTICLE - 2) + row].length = (gParticle[row * NUM_COL_PARTICLE + col].r - gParticle[(row + 2) * NUM_COL_PARTICLE + col].r).Mag();
			gBendSpringV[col * (NUM_ROW_PARTICLE - 2) + row].nObj1 = row * NUM_COL_PARTICLE + col;
			gBendSpringV[col * (NUM_ROW_PARTICLE - 2) + row].nObj2 = (row + 2) * NUM_COL_PARTICLE + col;
		}
	}
}

void Simulate(float dt)
{
	int i, num;
	float wind;
	Vector3D AirDrag, Contact, Normal;
	
	//Reset force.
	num = NUM_COL_PARTICLE * NUM_ROW_PARTICLE;
	for(i = 0; i < num; i++)
		gParticle[i].a = Vector3D(0.0f, 0.0f, 0.0f);

	if(gIsDetach)
	{
		for(i = 0; i < num; i++)
		{
			if(gParticle[i].IsFixed)
				gParticle[i].IsFixed = false;
		}
	}

	//Precalculate each particle's direction and speed.
	for(i = 0; i < num; i++)
	{
		gParticles_unitDir[i] = gParticle[i].v.Normalize();
		gParticles_vMag[i] = gParticle[i].v.Mag();
	}

	//Update fixed particle's positions.
	for(i = 0; i < num; i++)
	{
		if(gParticle[i].IsFixed)
			gParticle[i].r = gParticle[i].r + gOffset;
	}

	//Apply hortizontal spring forces.
	num = (NUM_COL_PARTICLE - 1) * NUM_ROW_PARTICLE;
	for(i = 0; i < num; i++)
		SolveSpring(&gHSpring[i]);

	//Apply vertical spring forces.
	num = (NUM_ROW_PARTICLE - 1) * NUM_COL_PARTICLE;
	for(i = 0; i < num; i++)
		SolveSpring(&gVSpring[i]);

	if(gIsEnableShear)
	{
		//Apply shear spring forces.
		num = (NUM_ROW_PARTICLE - 1) * (NUM_COL_PARTICLE - 1);
		for(i = 0; i < num; i++)
			SolveSpring(&gShearSpringLR[i]);
		for(i = 0; i < num; i++)
			SolveSpring(&gShearSpringRL[i]);
	}

	if(gIsEnableBend)
	{
		//Apply bend spring forces.
		num = (NUM_ROW_PARTICLE - 2) * (NUM_COL_PARTICLE - 2);
		for(i = 0; i < num; i++)
			SolveSpring(&gBendSpringH[i]);
		for(i = 0; i < num; i++)
			SolveSpring(&gBendSpringV[i]);
	}

	num = NUM_COL_PARTICLE * NUM_ROW_PARTICLE;

	//Apply air dragging force.
	for(i = 0; i < num; i++)
	{
		if(!gParticle[i].IsFixed)
		{
			AirDrag = gParticles_unitDir[i] * powf(gParticles_vMag[i], 2.0f) * (-gParticle[i].AirDragCoef);
			gParticle[i].a = gParticle[i].a + AirDrag * gParticle[i].invMass;
		}
	}

	//Apply gravity.	
	for(i = 0; i < num; i++)
	{
		if(!gParticle[i].IsFixed)
			gParticle[i].a = gParticle[i].a + gGravity;
	}

	//Apply collision response if collision occurs.
	for(i = 0; i < num; i++)
	{
		if(ParticleSphereCD(&gParticle[i], &gSphere, &Contact, &Normal))
		{
			float j, coef = 0.1f;

			j = (-(1.0f + coef) * (gParticle[i].v * Normal));

			gParticle[i].r = Contact;
			gParticle[i].a = gParticle[i].a + Normal * (j / dt);
		}

		if(ParticlePlaneCD(&gParticle[i], &gPlane, &Contact, &Normal))
		{
			float j, coef = 0.05f;

			j = (-(1.0f + coef) * (gParticle[i].v * Normal));

			gParticle[i].r = Contact;
			gParticle[i].a = gParticle[i].a + Normal * (j / dt) - gParticle[i].v * (0.8f / dt);
		}
	}

	//Apply wind.
	for(i = 0; i < num; i++)
	{
		if(!gParticle[i].IsFixed)
		{
			wind = (0.6f + ((rand() / (float)RAND_MAX) - 0.5f) * 0.4f) * gWindLevel * 0.24f;
			gParticle[i].a = gParticle[i].a + Vector3D(0.0f, 0.0f, -wind);
		}
	}

	//Advance simulation step.
	for(i = 0; i < num; i++)
	{
		if(!gParticle[i].IsFixed)
		{
			gParticle[i].v = gParticle[i].v + (gParticle[i].a * dt);
			gParticle[i].r = gParticle[i].r + (gParticle[i].v * dt);
		}
	}
}

GLvoid BuildFont(GLvoid)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID
	HFONT	oldfont;									// Used For Good House Keeping

	base = glGenLists(96);								// Storage For 96 Characters

	font = CreateFont(	-12,							// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Courier New");					// Font Name

	oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
	wglUseFontBitmaps(hDC, 32, 96, base);				// Builds 96 Characters Starting At Character 32
	SelectObject(hDC, oldfont);							// Selects The Font We Want
	DeleteObject(font);									// Delete The Font
}

GLvoid KillFont(GLvoid)									// Delete The Font List
{
	glDeleteLists(base, 96);							// Delete All 96 Characters
}

GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}

AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *File=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		return NULL;									// If Not Return NULL
	}

	File=fopen(Filename,"r");							// Check To See If The File Exists

	if (File)											// Does The File Exist?
	{
		fclose(File);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}

	return NULL;										// If Load Failed Return NULL
}

int LoadGLTextures()									// Load Bitmaps And Convert To Textures
{
	int Status=FALSE;									// Status Indicator

	AUX_RGBImageRec *TextureImage[3];					// Create Storage Space For The Texture

	memset(TextureImage,0,sizeof(void *)*1);           	// Set The Pointer To NULL

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit

	//For cloth.
	if (TextureImage[0]=LoadBMP("cloth.bmp"))
	{
		Status=TRUE;									// Set The Status To TRUE

		glGenTextures(1, &texture[0]);					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}

	if (TextureImage[0])									// If Texture Exists
	{
		if (TextureImage[0]->data)							// If Texture Image Exists
		{
			free(TextureImage[0]->data);					// Free The Texture Image Memory
		}

		free(TextureImage[0]);								// Free The Image Structure
	}

	//For sphere.
	if (TextureImage[1]=LoadBMP("ball.bmp"))
	{
		Status=TRUE;									// Set The Status To TRUE

		glGenTextures(1, &texture[1]);					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[1]->sizeX, TextureImage[1]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[1]->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}

	if (TextureImage[1])									// If Texture Exists
	{
		if (TextureImage[1]->data)							// If Texture Image Exists
		{
			free(TextureImage[1]->data);					// Free The Texture Image Memory
		}

		free(TextureImage[1]);								// Free The Image Structure
	}

	//For floor.
	if (TextureImage[2]=LoadBMP("floor.bmp"))
	{
		Status=TRUE;									// Set The Status To TRUE

		glGenTextures(1, &texture[2]);					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[1]->sizeX, TextureImage[1]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[2]->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}

	if (TextureImage[2])									// If Texture Exists
	{
		if (TextureImage[2]->data)							// If Texture Image Exists
		{
			free(TextureImage[2]->data);					// Free The Texture Image Memory
		}

		free(TextureImage[2]);								// Free The Image Structure
	}

	return Status;										// Return The Status
}

void SolveSpring(Spring *pSpring)
{
	Vector3D dir, dir_unit, rel_v, ForceSpring;
	float d;

	dir = gParticle[pSpring->nObj1].r - gParticle[pSpring->nObj2].r;
	rel_v = gParticle[pSpring->nObj1].v - gParticle[pSpring->nObj2].v;
	d = dir.Mag();
	dir_unit = dir.Normalize();
	pSpring->extension = d;

	if(!gParticle[pSpring->nObj1].IsFixed)
	{
		ForceSpring = dir_unit * ((pSpring->extension - pSpring->length) * pSpring->k + (rel_v * dir_unit) * pSpring->DampCoef) * -1.0f;

		gParticle[pSpring->nObj1].a = gParticle[pSpring->nObj1].a + ForceSpring * gParticle[pSpring->nObj1].invMass;
	}

	if(!gParticle[pSpring->nObj2].IsFixed)
	{
		if(!gParticle[pSpring->nObj1].IsFixed)
			ForceSpring = ForceSpring * -1.0f;
		else
			ForceSpring = dir_unit * ((pSpring->extension - pSpring->length) * pSpring->k + (rel_v * dir_unit) * pSpring->DampCoef);

		gParticle[pSpring->nObj2].a = gParticle[pSpring->nObj2].a + ForceSpring * gParticle[pSpring->nObj2].invMass;
	}
}

bool ParticleSphereCD(Particle *pAtom, Sphere *pSphere, Vector3D *pContact, Vector3D *pNormal)
{
	Vector3D dir = pAtom->r - pSphere->r; 
	float dot, dist;

	dot = pAtom->v * dir;
	dist = dir.Mag();

	//If particle is not penetrating sphere or not going toward sphere, no responses have to be applied.
	if(dist > pSphere->radius || (dot >= 1e-5 && dot <= 1.0f))
		return false; 

	*pNormal = dir.Normalize();
	*pContact = pAtom->r + *pNormal * (pSphere->radius - dist);

	return true;
}

bool ParticlePlaneCD(Particle *pAtom, Plane *pPlane, Vector3D *pContact, Vector3D *pNormal)
{
	float d, dot;

	d = (pAtom->r - pPlane->p) * pPlane->n;
	dot = pAtom->v * pPlane->n;

	if(d > -1e-5 ||  (dot >= 1e-5 && dot <= 1.0f))
		return false;

	*pNormal = pPlane->n;
	*pContact = pAtom->r + (*pNormal) * (float)fabs(d);

	return true;
}