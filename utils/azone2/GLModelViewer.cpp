
// Quick and dirty EQ model viewer for Windows, using OpenGL
//
// Put together by Derision from EQEmu forums using basic Windows OpenGL framework from NeHe tutorials @ gamedev.net
// and S3D and EQG file loaders from OpenEQ (modified a bit to fix some bugs and support newer EQG formats).



#ifdef WIN32
#define Polygon Polygon_win32
#include <windows.h>
#undef Polygon
#endif
#include <gl\gl.h>
#include <gl\glu.h>
#include <stdio.h>
#include <cstdlib>
#include <math.h>
#include "types.h"
#include "wld.hpp"
#include "archive.hpp"
#include "pfs.hpp"
#include "file_loader.hpp"
#include "zon.hpp"
#include "zonv4.hpp"
#include "ter.hpp"


typedef struct _vertex{
	float x;
	float y;
	float z;

}VERTEX;

void DrawEQModel(FileLoader *fileloader, int modnum);
FileLoader *mfileloader;
bool ProcessZoneFile(const char *shortname);
enum EQFileType { S3D, EQG, UNKNOWN };

HDC		hDC=NULL;
HGLRC		hRC=NULL;
HWND		hWnd=NULL;
HINSTANCE	hInstance;

bool	keys[256];
char	ch;
bool	active=true;
GLuint	base;

int modelnum = 1;			// The Number of the model we are currently displaying.
float angle = 0;			// used to rotate the model. Updated by a timer

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

GLvoid BuildFont(GLvoid)
{
	HFONT	font;
	HFONT	oldfont;

	base = glGenLists(96);

	font = CreateFont(-24, 0, 0, 0, FW_BOLD, false, false, false, ANSI_CHARSET,
			  OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		    	  FF_DONTCARE|DEFAULT_PITCH, "Courier New");

	oldfont = (HFONT)SelectObject(hDC, font);
	wglUseFontBitmaps(hDC, 32, 96, base);
	SelectObject(hDC, oldfont);
	DeleteObject(font);
}

GLvoid KillFont(GLvoid)
{
	glDeleteLists(base, 96);
}

GLvoid glPrint(const char *fmt, ...)
{
	char		text[256];
	va_list		ap;

	if (fmt == NULL) return;

	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	glPushAttrib(GL_LIST_BIT);
	glListBase(base - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if (height==0)height=1;

	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f,(GLfloat)width/(GLfloat)height,0.1f,12000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int InitGL(GLvoid)
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	BuildFont();
	return true;
}

int DrawGLScene(char *ZoneFileName)
{
	char textBuffer[100];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if(mfileloader->model_data.models[modelnum])
		DrawEQModel(mfileloader, modelnum);

	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-1.2f);
	glColor3f(100.0f,0.0f,0.0f);
	glRasterPos2f(-1.15f,0.56f);

	if(mfileloader->model_data.models[modelnum])
		if(mfileloader->model_data.models[modelnum]->name)
			sprintf(textBuffer,"  %s: Model Number %4d. Name %s", ZoneFileName, modelnum, mfileloader->model_data.models[modelnum]->name);
		else
			sprintf(textBuffer,"  %s: Model Number %4d. Not Viewable (load failed).", ZoneFileName, modelnum);
	else
		sprintf(textBuffer,"  %s: Model Number %4d. Not Viewable (probably zone mesh).", ZoneFileName, modelnum);

	glPrint(textBuffer);
	sprintf(textBuffer,"  Use the + and - keys to cycle through models.");
	glRasterPos2f(-1.15f,0.50f);
	glPrint(textBuffer);


	return true;
}

GLvoid KillGLWindow(GLvoid) {
	if (hRC) {
		if (!wglMakeCurrent(NULL,NULL)) {
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC)) {
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;
	}

	if (hDC && !ReleaseDC(hWnd,hDC)) {
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;
	}

	if (hWnd && !DestroyWindow(hWnd)) {
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;
	}

	if (!UnregisterClass("OpenGL",hInstance)) {
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;
	}
	KillFont();
}


BOOL CreateGLWindow(char* title, int width, int height, int bits) {
	GLuint		PixelFormat;
	WNDCLASS	wc;
	DWORD		dwExStyle;
	DWORD		dwStyle;
	RECT		WindowRect;
	WindowRect.left=(long)0;
	WindowRect.right=(long)width;
	WindowRect.top=(long)0;
	WindowRect.bottom=(long)height;

	hInstance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC) WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "OpenGL";

	if (!RegisterClass(&wc)) {
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dwStyle=WS_OVERLAPPEDWINDOW;

	AdjustWindowRectEx(&WindowRect, dwStyle, false, dwExStyle);

	if (!(hWnd=CreateWindowEx(dwExStyle, "OpenGL", title, dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
				  0, 0,	WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top,
				  NULL,	NULL, hInstance, NULL))) {
		KillGLWindow();
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	static	PIXELFORMATDESCRIPTOR pfd= {
		sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, bits, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16,	0, 0, PFD_MAIN_PLANE,
		0, 0, 0, 0
	};

	if (!(hDC=GetDC(hWnd))) {
		KillGLWindow();
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	{
		KillGLWindow();
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd)) {
		KillGLWindow();
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	if (!(hRC=wglCreateContext(hDC))) {
		KillGLWindow();
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	if(!wglMakeCurrent(hDC,hRC)) {
		KillGLWindow();
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	ShowWindow(hWnd,SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	ReSizeGLScene(width, height);

	if (!InitGL()) {
		KillGLWindow();
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	SetTimer(hWnd, 1, 50, (TIMERPROC) NULL);

	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_ACTIVATE:
			if (!HIWORD(wParam)) active=true;
			else
				active=false;

			return 0;
		case WM_SYSCOMMAND:
			switch (wParam) {
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
			}
			break;
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		case WM_KEYDOWN:
			keys[wParam] = true;
			return 0;
		case WM_CHAR:
			ch = wParam;
			return 0;
		case WM_KEYUP:
			keys[wParam] = false;
			return 0;
		case WM_SIZE:
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));
			return 0;
		case WM_TIMER:
			angle = angle + 1;
			if(angle>359) angle = 0;
			return 0;

	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void TranslateVertex(VERTEX &v, float XOffset, float YOffset, float ZOffset) {

	v.x = v.x + XOffset;
	v.y = v.y + YOffset;
	v.z = v.z + ZOffset;
}

void ScaleVertex(VERTEX &v, float XScale, float YScale, float ZScale) {

	v.x = v.x * XScale;
	v.y = v.y * YScale;
	v.z = v.z * ZScale;
}


void DrawEQModel(FileLoader *fileloader, int modnum) {

	Polygon *poly;
	Vertex *verts[3];
	VERTEX v1, v2, v3;

	float maxDimension = 0, minx = 999999, miny = 999999, minz = 999999, maxx = -999999, maxy=-999999, maxz=-999999;
	Model *model = fileloader->model_data.models[modnum];

	for(int i = 0; i < model->poly_count; ++i) {
		poly = model->polys[i];

		verts[0] = model->verts[poly->v1];
		verts[1] = model->verts[poly->v2];
		verts[2] = model->verts[poly->v3];

		for(int j=0; j<3; j++) {
			if(verts[j]->x > maxDimension) maxDimension = verts[j]->x;
			if(verts[j]->y > maxDimension) maxDimension = verts[j]->y;
			if(verts[j]->z > maxDimension) maxDimension = verts[j]->z;

			if(verts[j]->x < minx) minx = verts[j]->x;
			if(verts[j]->y < miny) miny = verts[j]->y;
			if(verts[j]->z < minz) minz = verts[j]->z;

			if(verts[j]->x > maxx) maxx = verts[j]->x;
			if(verts[j]->y > maxy) maxy = verts[j]->y;
			if(verts[j]->z > maxz) maxz = verts[j]->z;
		}
	}

	maxDimension = 0;
	if(maxx-minx>maxDimension) maxDimension = maxx-minx;
	if(maxy-miny>maxDimension) maxDimension = maxy-miny;
	if(maxz-minz>maxDimension) maxDimension = maxz-minz;

	// Hack for very small models (e.g. spoons)
	if(maxDimension>1)
		glTranslatef(-1.5f,0.0f,-(maxDimension*2));
	else
		glTranslatef(-1.5f,0.0f,-10);

	// angle is updated by a timer every 50ms.
	glRotatef(angle, 1, 0, 0);
	glRotatef(angle, 0, 1, 0);
	glRotatef(angle, 0, 0, 1);

	glBegin(GL_TRIANGLES);

	for(int i = 0; i < model->poly_count; ++i) {
		poly = model->polys[i];

		verts[0] = model->verts[poly->v1];
		verts[1] = model->verts[poly->v2];
		verts[2] = model->verts[poly->v3];

		v1.x = verts[0]->x; v1.y = verts[0]->y; v1.z = verts[0]->z;
		v2.x = verts[1]->x; v2.y = verts[1]->y; v2.z = verts[1]->z;
		v3.x = verts[2]->x; v3.y = verts[2]->y; v3.z = verts[2]->z;

		// The aim of this is to centre the model in the window

		TranslateVertex(v1, -(minx + (maxx-minx)/2), -(miny + (maxy-miny)/2), -(minz + (maxz-minz)/2));
		TranslateVertex(v2, -(minx + (maxx-minx)/2), -(miny + (maxy-miny)/2), -(minz + (maxz-minz)/2));
		TranslateVertex(v3, -(minx + (maxx-minx)/2), -(miny + (maxy-miny)/2), -(minz + (maxz-minz)/2));

		// For very small models, magnify them
		if(maxDimension<=1) {
			ScaleVertex(v1, 10, 10, 10);
			ScaleVertex(v2, 10, 10, 10);
			ScaleVertex(v3, 10, 10, 10);
		}

		// Assign a kind of random colour to each polygon

		//glColor3b((i%50)+50,(i*5)%200,(i*10)%200);
		float col = (float)(100 + ((i*10) % 150)) /250 * 1.0f;
		glColor3f(col, col, col);

		glVertex3f(v1.x, v1.z, v1.y);
		glVertex3f(v2.x, v2.z, v2.y);
		glVertex3f(v3.x, v3.z, v3.y);

	}
	glEnd();
}

bool ProcessZoneFile(const char *shortname) {

	char bufs[96];
  	Archive *archive;

  	Zone_Model *zm;
	FILE *fff;
	EQFileType FileType = UNKNOWN;
	GLuint *textures;

	sprintf(bufs, "%s.s3d", shortname);

	archive = new PFSLoader();
	fff = fopen(bufs, "rb");
	if(fff != NULL)
		FileType = S3D;
	else {
		sprintf(bufs, "%s.eqg", shortname);
		fff = fopen(bufs, "rb");
		if(fff != NULL)
			FileType = EQG;
	}

	if(FileType == UNKNOWN) {
		MessageBox(NULL,"Unable to locate specified zone either as EQG or S3D","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return(false);
	}

  	if(archive->Open(fff) == 0) {
		MessageBox(NULL,"Unable to open container file","ERROR",MB_OK|MB_ICONEXCLAMATION);

		return(false);
	}

	switch(FileType) {
		case S3D:
  			mfileloader = new WLDLoader();
  			if(mfileloader->Open(NULL, (char *) shortname, archive) == 0) {

				MessageBox(NULL,"Error reading WLD from container file","ERROR",MB_OK|MB_ICONEXCLAMATION);
	  			return(false);
  			}
			break;
		case EQG:
			mfileloader = new ZonLoader();
			if(mfileloader->Open(NULL, (char *) shortname, archive) == 0) {
				delete mfileloader;
				mfileloader = new Zonv4Loader();
                if(mfileloader->Open(NULL, (char *) shortname, archive) == 0) {
                    MessageBox(NULL,"Error reading ZON/TER from container file","ERROR",MB_OK|MB_ICONEXCLAMATION);
					return(false);
	        	}
			}
			break;
		case UNKNOWN:
			break;
	}

	if(mfileloader->model_data.model_count==0) {
		MessageBox(NULL,"No models found. For S3D, probably could not locate _obj.s3d file","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}


	zm = mfileloader->model_data.zone_model;

	return(true);
}

int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	BOOL done=false;
	FILE *fp;

	for(int i=0;i<256;i++)
		keys[i] = false;


	char* buf = (char *) new char[strlen(lpCmdLine) + 1] ;
	char *pTmp = buf;

	if(!strlen(lpCmdLine)) {
		MessageBox(NULL,"Specify the shortname of the zone (without the S3D or EQG extension) on the command line.",
			   "ERROR",MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	strcpy(buf,lpCmdLine);

	if(!ProcessZoneFile(buf)) return 0;

	if (!CreateGLWindow("EQ Model Viewer",1280,768,16))
		return 0;

	while(!done) {
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if (msg.message==WM_QUIT)
				done=true;
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			if ((active && !DrawGLScene(buf)) || keys[VK_ESCAPE])
				done=true;
			else
				SwapBuffers(hDC);

			// Use + and - to cycle through the models

			if (toupper(ch)=='+') {
				if(modelnum+1 < mfileloader->model_data.model_count)
					modelnum++;
				else
					modelnum=0;
				angle = 0;
			}
			if (toupper(ch)=='-') {
				if(modelnum>0)
					modelnum--;
				else
					modelnum =  mfileloader->model_data.model_count - 1;

				angle = 0;
			}
			ch = 0;
		}
	}

	KillGLWindow();
	return (msg.wParam);
}
