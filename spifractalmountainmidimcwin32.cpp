// spifractalmountainmidimcwin32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "spifractalmountainmidimcwin32.h"
#include <mmsystem.h> //for timeSetEvent()
/*
#include <iostream>
#include <fstream>
*/
#include <string>
#include <vector>
using namespace std;
/*
#include "oifiilib.h" //note: oifiilib.lib/.dll is an MFC extension and resource DLL
*/

#include <list>
#include "portmidi.h"
#include <map>
#include <stdio.h>
#include "resource.h"

#include  <GL/gl.h>
#include <GL/glu.h>
#include "glfont.h"
#include "spiwavsetlib.h"
#include <assert.h>

#include <math.h>
#include <limits.h>           /* ULONG_MAX is defined here */
#include <float.h>            /* FLT_MAX is atleast defined here */

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
//TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
//TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szTitle[1024]={"spifractalmountainmidimcwin32title"};					// The title bar text
TCHAR szWindowClass[1024]={"spifractalmountainmidimcwin32class"};			// the main window class name

//new parameters
string global_begin="begin.ahk";
string global_end="end.ahk";


int global_titlebardisplay=1; //0 for off, 1 for on
int global_acceleratoractive=0; //0 for off, 1 for on
int global_menubardisplay=0; //0 for off, 1 for on

DWORD global_startstamp_ms;
float global_duration_sec=3600;
float global_sleeptimeperfractalmountain_sec=30;
int global_x=100;
int global_y=100;
int global_xwidth=300;
int global_yheight=300;
//BYTE global_alpha=220;
BYTE global_alpha=255;

int global_imageid=0;
HWND global_hwnd=NULL;
MMRESULT global_timer=1; //was 0
int global_imageheight=-1; //will be computed within WM_SIZE handler
int global_imagewidth=-1; //will be computed within WM_SIZE handler 
/*
vector<string> global_txtfilenames;
COW2Doc* global_pOW2Doc=NULL;
COW2View* global_pOW2View=NULL;
*/

const int global_pmeventlistsize = 64;
list<PmEvent*> global_pmeventlist[global_pmeventlistsize]; 
list<PmEvent*>::iterator it_pmeventlist;
bool global_pmevenlistbusyfrommiditimer[global_pmeventlistsize];
bool global_pmevenlistbusyfromglobaltimer[global_pmeventlistsize];
bool global_miditimerskip=false;
UINT global_miditimer=2;
UINT global_miditimer_programchange=3;
int global_prevstep=0; //was -1
int global_midistep_ms=250;
//int global_midistep_ms=125;
int global_midiprogramchangeperiod_ms=1000*3*60;
FILE* pFILE = NULL;

map<string,int> global_midioutputdevicemap;
string global_midioutputdevicename="Out To MIDI Yoke:  1";
//string global_midioutputdevicename="E-DSP MIDI Port [FFC0]";
//string global_midioutputdevicename="E-DSP MIDI Port 2 [FFC0]";
int global_minoutputmidichannel=0;
int global_maxoutputmidichannel=15;
int global_midicontrolnumber=9; //9 is undefined, so it is available for us to use
//bool global_bsendmidi=true;
bool global_bsendmidi=false;
bool global_bsendmidi_usingremap=true;
PmStream* global_pPmStream = NULL; // midi output
string global_noteremapstring="NONE";
//int global_noteremapid;

bool global_bdrawlabel=true;
GLFont global_GLFont;
/*
float x = 0.1, y = 0.1;		// starting point
float a = -0.966918;		// coefficients for "The King's Dream"
float b = 2.879879;			// coefficients a and b will be modified at random between a range of -3.0 to + 3.0 
float c = 0.765145;
float d = 0.744728;
int	global_count=0;
int global_axiscoef=1;
*/
HDC global_hDC; //returned by opengl InitGL()
int global_bsamenotevelocityandduration=1; //with 0 note, velocity and duration will be different for each midi channel
/*
int	initialIterations = 100,	// initial number of iterations
					// to allow the attractor to settle
	iterations = 10000;		// number of times to iterate through
	//iterations = 100000;		// number of times to iterate through
	//iterations = 1000000;		// number of times to iterate through
					// the functions and draw a point

class glfloatpair
{
public:
	GLfloat x;
	GLfloat y;

	glfloatpair(GLfloat xx, GLfloat yy)
	{
		x = xx;
		y = yy;
	}
	glfloatpair(double xx, double yy)
	{
		x = xx;
		y = yy;
	}
};

std::vector<glfloatpair*> global_pairvector;
std::vector<glfloatpair*>::iterator it;
*/

#ifdef _WIN32
#define drand48() (((float) rand())/((float) RAND_MAX))
#define srand48(x) (srand((x)))
#endif

typedef enum { NOTALLOWED, MOUNTAIN, TREE, ISLAND, BIGMTN, STEM, LEAF, 
               MOUNTAIN_MAT, WATER_MAT, LEAF_MAT, TREE_MAT, STEMANDLEAVES,
               AXES } DisplayLists;

#define MAXLEVEL 8

int Rebuild = 1,        // Rebuild display list in next display? 
    //Fract   = TREE,     // What fractal are we building 
    //Fract   = MOUNTAIN,     // What fractal are we building 
    //Fract   = AXES,     // What fractal are we building 
    Fract   = ISLAND,     // What fractal are we building 
    //Level   = 4;        // levels of recursion for fractals      
    Level   = 8;        // levels of recursion for fractals      

//int DrawAxes = 0;       
int DrawAxes = 1;       



// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void OnTimerMidi();
void OnTimerMidiProgramChange();
void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC);
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC);
void SetupAnimation(HDC hDC, int Width, int Height);
void CleanupAnimation();



/***************************************************************/
/************************* VECTOR JUNK *************************/
/***************************************************************/

  /* print vertex to stderr */
void printvert(float v[3])
{
  fprintf(stderr, "(%f, %f, %f)\n", v[0], v[1], v[2]);
}

  /* normalizes v */
void normalize(GLfloat v[3])
{
  GLfloat d = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

  if (d == 0)
    fprintf(stderr, "Zero length vector in normalize\n");
  else
    v[0] /= d; v[1] /= d; v[2] /= d;
}

  /* calculates a normalized crossproduct to v1, v2 */
void ncrossprod(float v1[3], float v2[3], float cp[3])
{
  cp[0] = v1[1]*v2[2] - v1[2]*v2[1];
  cp[1] = v1[2]*v2[0] - v1[0]*v2[2];
  cp[2] = v1[0]*v2[1] - v1[1]*v2[0];
  normalize(cp);
}

  /* calculates normal to the triangle designated by v1, v2, v3 */
void triagnormal(float v1[3], float v2[3], float v3[3], float norm[3])
{
  float vec1[3], vec2[3];

  vec1[0] = v3[0] - v1[0];  vec2[0] = v2[0] - v1[0];
  vec1[1] = v3[1] - v1[1];  vec2[1] = v2[1] - v1[1];
  vec1[2] = v3[2] - v1[2];  vec2[2] = v2[2] - v1[2];

  ncrossprod(vec2, vec1, norm);
}

float xzlength(float v1[3], float v2[3])
{
  return sqrt((v1[0] - v2[0])*(v1[0] - v2[0]) +
              (v1[2] - v2[2])*(v1[2] - v2[2]));
}

float xzslope(float v1[3], float v2[3])
{
  return ((v1[0] != v2[0]) ? ((v1[2] - v2[2]) / (v1[0] - v2[0]))
	                   : FLT_MAX);
}


/***************************************************************/
/************************ MOUNTAIN STUFF ***********************/
/***************************************************************/

GLfloat DispFactor[MAXLEVEL];  /* Array of what to multiply random number
				  by for a given level to get midpoint
				  displacement  */
GLfloat DispBias[MAXLEVEL];  /* Array of what to add to random number
				before multiplying it by DispFactor */

#define NUMRANDS 191
float RandTable[NUMRANDS];  /* hash table of random numbers so we can
			       raise the same midpoints by the same amount */ 

         /* The following are for permitting an edge of a moutain to be   */
         /* pegged so it won't be displaced up or down.  This makes it    */
         /* easier to setup scenes and makes a single moutain look better */

GLfloat Verts[3][3],    /* Vertices of outside edges of mountain */
        Slopes[3];      /* Slopes between these outside edges */
int     Pegged[3];      /* Is this edge pegged or not */           

 /*
  * Comes up with a new table of random numbers [0,1)
  */
void InitRandTable(unsigned int seed)
{
  int i;

  srand48((long) seed);
  for (i = 0; i < NUMRANDS; i++)
    RandTable[i] = drand48() - 0.5;
}

  /* calculate midpoint and displace it if required */
void Midpoint(GLfloat mid[3], GLfloat v1[3], GLfloat v2[3],
	      int edge, int level)
{
  unsigned hash;

  mid[0] = (v1[0] + v2[0]) / 2;
  mid[1] = (v1[1] + v2[1]) / 2;
  mid[2] = (v1[2] + v2[2]) / 2;
  if (!Pegged[edge] || (fabs(xzslope(Verts[edge], mid) 
                        - Slopes[edge]) > 0.00001)) {
    srand48((int)((v1[0]+v2[0])*23344));
    hash = drand48() * 7334334;
    srand48((int)((v2[2]+v1[2])*43433));
    hash = (unsigned)(drand48() * 634344 + hash) % NUMRANDS;
    mid[1] += ((RandTable[hash] + DispBias[level]) * DispFactor[level]);
  }
}

  /*
   * Recursive moutain drawing routine -- from lecture with addition of 
   * allowing an edge to be pegged.  This function requires the above
   * globals to be set, as well as the Level global for fractal level 
   */
void FMR(GLfloat v1[3], GLfloat v2[3], GLfloat v3[3], int level)
{
  if (level == Level) {
    GLfloat norm[3];

    triagnormal(v1, v2, v3, norm);
    glNormal3fv(norm);
    glVertex3fv(v1);
    glVertex3fv(v2);
    glVertex3fv(v3);

  } else {
    GLfloat m1[3], m2[3], m3[3];

    Midpoint(m1, v1, v2, 0, level);
    Midpoint(m2, v2, v3, 1, level);
    Midpoint(m3, v3, v1, 2, level);

    FMR(v1, m1, m3, level + 1);
    FMR(m1, v2, m2, level + 1);
    FMR(m3, m2, v3, level + 1);
    FMR(m1, m2, m3, level + 1);
  }
}

 /*
  * sets up lookup tables and calls recursive mountain function
  */
void FractalMountain(GLfloat v1[3], GLfloat v2[3], GLfloat v3[3],
                     int pegged[3])
{
  GLfloat lengths[MAXLEVEL];
  GLfloat fraction[8] = { 0.3, 0.3, 0.4, 0.2, 0.3, 0.2, 0.4, 0.4  };
  GLfloat bias[8]     = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1  };
  int i;
  float avglen = (xzlength(v1, v2) + 
                  xzlength(v2, v3) +
		  xzlength(v3, v1) / 3);

  for (i = 0; i < 3; i++) {
    Verts[0][i] = v1[i];      /* set mountain vertex globals */
    Verts[1][i] = v2[i];
    Verts[2][i] = v3[i];
    Pegged[i] = pegged[i];
  }

  Slopes[0] = xzslope(Verts[0], Verts[1]);   /* set edge slope globals */
  Slopes[1] = xzslope(Verts[1], Verts[2]);
  Slopes[2] = xzslope(Verts[2], Verts[0]);

  lengths[0] = avglen;          
  for (i = 1; i < Level; i++) {   
    lengths[i] = lengths[i-1]/2;     /* compute edge length for each level */
  }

  for (i = 0; i < Level; i++) {     /* DispFactor and DispBias arrays */      
    DispFactor[i] = (lengths[i] * ((i <= 7) ? fraction[i] : fraction[7]));
    DispBias[i]   = ((i <= 7) ? bias[i] : bias[7]);
  } 

  glBegin(GL_TRIANGLES);
    FMR(v1, v2, v3, 0);    /* issues no GL but vertex calls */
  glEnd();
}

 /*
  * draw a mountain and build the display list
  */
void CreateMountain(void)
{
  GLfloat v1[3] = { 0, 0, -1 }, v2[3] = { -1, 0, 1 }, v3[3] = { 1, 0, 1 };
  int pegged[3] = { 1, 1, 1 };

  glNewList(MOUNTAIN, GL_COMPILE);
  glPushAttrib(GL_LIGHTING_BIT);
    glCallList(MOUNTAIN_MAT);
    FractalMountain(v1, v2, v3, pegged);
  glPopAttrib();
  glEndList();
}

  /*
   * new random numbers to make a different moutain
   */
void NewMountain(void)
{
  InitRandTable(time(NULL));
}

/***************************************************************/
/***************************** TREE ****************************/
/***************************************************************/

long TreeSeed;   /* for srand48 - remember so we can build "same tree"
                     at a different level */

 /*
  * recursive tree drawing thing, fleshed out from class notes pseudocode 
  */
void FractalTree(int level)
{
  long savedseed;  /* need to save seeds while building tree too */

  if (level == Level) {
      glPushMatrix();
        glRotatef(drand48()*180, 0, 1, 0);
        glCallList(STEMANDLEAVES);
      glPopMatrix();
  } else {
    glCallList(STEM);
    glPushMatrix();
    glRotatef(drand48()*180, 0, 1, 0);
    glTranslatef(0, 1, 0);
    glScalef(0.7, 0.7, 0.7);

      savedseed = (long) drand48()*ULONG_MAX;    /* recurse on a 3-way branching */
      glPushMatrix();    
        glRotatef(110 + drand48()*40, 0, 1, 0);
        glRotatef(30 + drand48()*20, 0, 0, 1);
        FractalTree(level + 1);
      glPopMatrix();

      srand48(savedseed);
      savedseed = (long) drand48()*ULONG_MAX;
      glPushMatrix();
        glRotatef(-130 + drand48()*40, 0, 1, 0);
        glRotatef(30 + drand48()*20, 0, 0, 1);
        FractalTree(level + 1);
      glPopMatrix();

      srand48(savedseed);
      glPushMatrix();
        glRotatef(-20 + drand48()*40, 0, 1, 0);
        glRotatef(30 + drand48()*20, 0, 0, 1);
        FractalTree(level + 1);
      glPopMatrix();

    glPopMatrix();
  }
}

  /*
   * Create display lists for a leaf, a set of leaves, and a stem
   */
void CreateTreeLists(void)
{
  GLUquadricObj *cylquad = gluNewQuadric();
  int i;

  glNewList(STEM, GL_COMPILE);
  glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(cylquad, 0.1, 0.08, 1, 10, 2 );
  glPopMatrix();
  glEndList();

  glNewList(LEAF, GL_COMPILE);  /* I think this was jeff allen's leaf idea */
    glBegin(GL_TRIANGLES);
      glNormal3f(-0.1, 0, 0.25);  /* not normalized */
      glVertex3f(0, 0, 0);
      glVertex3f(0.25, 0.25, 0.1);
      glVertex3f(0, 0.5, 0);

      glNormal3f(0.1, 0, 0.25);
      glVertex3f(0, 0, 0);
      glVertex3f(0, 0.5, 0);
      glVertex3f(-0.25, 0.25, 0.1);
    glEnd();
  glEndList();

  glNewList(STEMANDLEAVES, GL_COMPILE);
  glPushMatrix();
  glPushAttrib(GL_LIGHTING_BIT);
    glCallList(STEM);
    glCallList(LEAF_MAT);
    for(i = 0; i < 3; i++) {
      glTranslatef(0, 0.333, 0);
      glRotatef(90, 0, 1, 0);
      glPushMatrix();
        glRotatef(0, 0, 1, 0);
        glRotatef(50, 1, 0, 0);
        glCallList(LEAF);
      glPopMatrix();
      glPushMatrix();
        glRotatef(180, 0, 1, 0);
        glRotatef(60, 1, 0, 0);
        glCallList(LEAF);
      glPopMatrix();
    }
  glPopAttrib();
  glPopMatrix();
  glEndList();
}

 /*
  * draw and build display list for tree
  */
void CreateTree(void)
{
  srand48(TreeSeed);

  glNewList(TREE, GL_COMPILE);
    glPushMatrix();
    glPushAttrib(GL_LIGHTING_BIT);
    glCallList(TREE_MAT);
    glTranslatef(0, -1, 0);
    FractalTree(0);
    glPopAttrib();
    glPopMatrix();
  glEndList();  
}

 /*
  * new seed for a new tree (groan)
  */
void NewTree(void)
{
  TreeSeed = time(NULL);
}

/***************************************************************/
/*********************** FRACTAL PLANET ************************/
/***************************************************************/

void CreateIsland(void)
{
  CreateMountain();
  glNewList(ISLAND, GL_COMPILE);
  glPushAttrib(GL_LIGHTING_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
    glCallList(WATER_MAT);

    glBegin(GL_QUADS);
      glNormal3f(0, 1, 0);
      glVertex3f(100, 0.01, 100);
      glVertex3f(100, 0.01, -100);
      glVertex3f(-100, 0.01, -100);
      glVertex3f(-100, 0.01, 100);
    glEnd();

    glPushMatrix();
    glTranslatef(0, -0.1, 0);
    glCallList(MOUNTAIN);
    glPopMatrix();

    glPushMatrix();
    glRotatef(135, 0, 1, 0);
    glTranslatef(0.2, -0.15, -0.4);
    glCallList(MOUNTAIN);
    glPopMatrix();

    glPushMatrix();
    glRotatef(-60, 0, 1, 0);
    glTranslatef(0.7, -0.07, 0.5);
    glCallList(MOUNTAIN);
    glPopMatrix();

    glPushMatrix();
    glRotatef(-175, 0, 1, 0);
    glTranslatef(-0.7, -0.05, -0.5);
    glCallList(MOUNTAIN);
    glPopMatrix();

    glPushMatrix();
    glRotatef(165, 0, 1, 0);
    glTranslatef(-0.9, -0.12, 0.0);
    glCallList(MOUNTAIN);
    glPopMatrix();

  glPopMatrix();
  glPopAttrib();
  glEndList();  
}


void NewFractals(void)
{
  NewMountain();
  NewTree();
}

void Create(int fract)
{
  switch(fract) {
    case MOUNTAIN:
      CreateMountain();
      break;
    case TREE:
      CreateTree();
      break;
    case ISLAND:
      CreateIsland();
      break;
  }
}


/***************************************************************/
/**************************** OPENGL ***************************/
/***************************************************************/


void SetupMaterials(void)
{
  GLfloat mtn_ambuse[] =   { 0.426, 0.256, 0.108, 1.0 };
  GLfloat mtn_specular[] = { 0.394, 0.272, 0.167, 1.0 };
  GLfloat mtn_shininess[] = { 10 };

  GLfloat water_ambuse[] =   { 0.0, 0.1, 0.5, 1.0 };
  GLfloat water_specular[] = { 0.0, 0.1, 0.5, 1.0 };
  GLfloat water_shininess[] = { 10 };

  GLfloat tree_ambuse[] =   { 0.4, 0.25, 0.1, 1.0 };
  GLfloat tree_specular[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat tree_shininess[] = { 0 };

  GLfloat leaf_ambuse[] =   { 0.0, 0.8, 0.0, 1.0 };
  GLfloat leaf_specular[] = { 0.0, 0.8, 0.0, 1.0 };
  GLfloat leaf_shininess[] = { 10 };

  glNewList(MOUNTAIN_MAT, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mtn_ambuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mtn_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mtn_shininess);
  glEndList();

  glNewList(WATER_MAT, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, water_ambuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, water_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, water_shininess);
  glEndList();

  glNewList(TREE_MAT, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tree_ambuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, tree_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, tree_shininess);
  glEndList();

  glNewList(LEAF_MAT, GL_COMPILE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, leaf_ambuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, leaf_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, leaf_shininess);
  glEndList();
}

void myGLInit(void)
{
  GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat light_position[] = { 0.0, 0.3, 0.3, 0.0 };

  GLfloat lmodel_ambient[] = { 0.4, 0.4, 0.4, 1.0 };

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_NORMALIZE);
#if 0
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
#endif

  glShadeModel(GL_SMOOTH);
#if 0
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

  SetupMaterials();
  CreateTreeLists();

  glFlush();
} 

/***************************************************************/
/************************ GLUT STUFF ***************************/
/***************************************************************/

void reshape(GLsizei w, GLsizei h)
{
  glViewport(0,0,w,h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (GLdouble)w/h, 0.01, 100);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glFlush();
}

void display(HDC hdc)
{ 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glFlush();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPushMatrix();  /* clear of last viewing xform, leaving perspective */

  //agvViewTransform();
	//spi, begin
	//glOrtho(-300, 300, -240, 240, 25, 75); //original
	//glOrtho(-300, 300, -8, 8, 25, 75); //spi, last
	//glOrtho(-2.0f, 2.0f, -2.0f, 2.0f, ((GLfloat)-1), (GLfloat)1); //spi
	//glOrtho(-300, 300, -240, 240, -1.0f, 1.0f); //TREE
	glOrtho(-100, 100, -80, 80, -1.0f, 1.0f); //MOUNTAIN
	//glOrtho(-100, 100, -80, 80, -10.0f, 10.0f); //AXES ?
	//spi, end


  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

	/*
	gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); //original
			//camera xyz, the xyz to look at, and the up vector (+y is up)
	*/
  if (Rebuild) {
    Create(Fract);
    Rebuild = 0;
  }

  glCallList(Fract);

  if (DrawAxes)
    glCallList(AXES);

  //glutSwapBuffers();
  SwapBuffers(hdc);
  glFlush();
}



int RandomInteger(int lowest, int highest) 
{
	int random_integer;
	int range=(highest-lowest)+1; 
	//random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));
	random_integer = lowest+rand()%range;
	return random_integer;
}

float RandomFloat(float a, float b) 
{
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}


void CALLBACK StartGlobalProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	//WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight, global_staticalignment);
	//HDC* pHDC=(HDC*)dwUser;

	DWORD nowstamp_ms = GetTickCount();
	while( (global_duration_sec<0.0f) || ((nowstamp_ms-global_startstamp_ms)/1000.0f)<global_duration_sec )
	{
		InvalidateRect(global_hwnd, NULL, false);

		//create midi events, various notes with various durations
		if(global_bsendmidi)
		{
			//global_miditimerskip=true;
			global_miditimerskip=false;
			for(int k=0; k<global_pmeventlistsize; k++)
			{
				if(k%4!=0) continue;
				//for testing
				//if(pFILE) fprintf(pFILE, "k=%d\n",k);
				//constant note 
				//int=note=60;
				//climbing note
				//int note=60+k;
				//random note
				//int note = RandomInteger(0, 127);

				/*
				//strange attractor note
				// compute a new point using the strange attractor equations
				float xnew = sin(y*b) + c*sin(x*b);
				float ynew = sin(x*a) + d*sin(y*a);
				// save the new point
				x = xnew;
				y = ynew;
				*/
 				//int note = (x/2.0f)*64+64;
				int note = 64;
				if(note>=128) note=127;
				if(note<=0) note=0;
				int velocity=100;
				//int velocity = (y/2.0f)*64+64;
				if(velocity>=128) velocity=127;
				if(velocity<=0) velocity=0;

				note = GetNoteRemap(global_noteremapstring.c_str(), note);
				//duration
				int duration = RandomInteger(1,8);
				for(int outputmidichannel=global_minoutputmidichannel; outputmidichannel<=global_maxoutputmidichannel; outputmidichannel++)
				{
					if(global_bsamenotevelocityandduration==0)
					{
						//get a new note, velocity and duration
						/*
						//strange attractor note
						// compute a new point using the strange attractor equations
						xnew = sin(y*b) + c*sin(x*b);
						ynew = sin(x*a) + d*sin(y*a);
						// save the new point
						x = xnew;
						y = ynew;
						*/
 						//note = (x/2.0f)*64+64;
						note = 64;
						if(note>=128) note=127;
						if(note<=0) note=0;
						velocity=100;
						//velocity = (y/2.0f)*64+64;
						if(velocity>=128) velocity=127;
						if(velocity<=0) velocity=0;

						note = GetNoteRemap(global_noteremapstring.c_str(), note);
						//duration
						duration = RandomInteger(1,8);

					}
					//note on
					PmEvent* pPmEvent = new PmEvent;
					pPmEvent->timestamp = 0;
					pPmEvent->message = Pm_Message(0x90+outputmidichannel, note, velocity);
					while(global_pmevenlistbusyfrommiditimer[k]==true) Sleep(10);
					global_pmevenlistbusyfromglobaltimer[k]=true;
					global_pmeventlist[k].push_back(pPmEvent);
					global_pmevenlistbusyfromglobaltimer[k]=false;
					//duration
					//int random_integer = 4;
					//int duration = RandomInteger(1,8);
					int kk=k+duration;
					if(kk>(global_pmeventlistsize-1)) kk=kk-global_pmeventlistsize;
					//create note off midi message
					pPmEvent = new PmEvent;
					pPmEvent->timestamp = 0;
					pPmEvent->message = Pm_Message(0x90+outputmidichannel, note, 0);
					while(global_pmevenlistbusyfrommiditimer[kk]==true) Sleep(10);
					global_pmevenlistbusyfromglobaltimer[kk]=true;
					global_pmeventlist[kk].push_back(pPmEvent);
					global_pmevenlistbusyfromglobaltimer[kk]=false;
				}
			}
			global_miditimerskip=false;
		}
		Sleep((int)(global_sleeptimeperfractalmountain_sec*1000));

		nowstamp_ms = GetTickCount();
	}
	PostMessage(global_hwnd, WM_DESTROY, 0, 0);
}




PCHAR*
    CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
    {
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
            i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( a = CmdLine[i] ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
    }


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	global_startstamp_ms = GetTickCount();

	//LPWSTR *szArgList;
	LPSTR *szArgList;
	int nArgs;

	//szArgList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	if(nArgs>1)
	{
		global_midioutputdevicename = szArgList[1];
	}
	if(nArgs>2)
	{
		global_minoutputmidichannel = atoi(szArgList[2]);
	}
	if(nArgs>3)
	{
		global_maxoutputmidichannel = atoi(szArgList[3]);
	}
	if(nArgs>4)
	{
		global_duration_sec = atof(szArgList[4]);
	}
	if(nArgs>5)
	{
		global_sleeptimeperfractalmountain_sec = atof(szArgList[5]);
	}
	if(nArgs>6)
	{
		global_midistep_ms = atoi(szArgList[6]);
	}
	if(nArgs>7)
	{
		global_midiprogramchangeperiod_ms = atoi(szArgList[7]);
	}
	if(nArgs>8)
	{
		global_noteremapstring = szArgList[8];
	}
	if(nArgs>9)
	{
		global_x = atoi(szArgList[9]);
	}
	if(nArgs>10)
	{
		global_y = atoi(szArgList[10]);
	}
	if(nArgs>11)
	{
		global_xwidth = atoi(szArgList[11]);
	}
	if(nArgs>12)
	{
		global_yheight = atoi(szArgList[12]);
	}
	if(nArgs>13)
	{
		global_alpha = atoi(szArgList[13]);
	}
	if(nArgs>14)
	{
		global_titlebardisplay = atoi(szArgList[14]);
	}
	if(nArgs>15)
	{
		global_menubardisplay = atoi(szArgList[15]);
	}
	if(nArgs>16)
	{
		global_acceleratoractive = atoi(szArgList[16]);
	}
	if(nArgs>17)
	{
		global_bsamenotevelocityandduration = atoi(szArgList[17]);
	}
	//new parameters
	if(nArgs>18)
	{
		strcpy(szWindowClass, szArgList[18]); 
	}
	if(nArgs>19)
	{
		strcpy(szTitle, szArgList[19]); 
	}
	if(nArgs>20)
	{
		global_begin = szArgList[20]; 
	}
	if(nArgs>21)
	{
		global_end = szArgList[21]; 
	}
	LocalFree(szArgList);

	ShellExecuteA(NULL, "open", global_begin.c_str(), "", NULL, nCmdShow);

	pFILE = fopen("debug.txt", "w");
	//parameter validation
	if(global_minoutputmidichannel<0 || global_minoutputmidichannel>15)
	{
		if(pFILE) fprintf(pFILE, "invalid minoutputmidichannel, midi channel must range from 0 to 15 inclusively.\n");
		return FALSE;
	}
	if(global_maxoutputmidichannel<0 || global_maxoutputmidichannel>15)
	{
		if(pFILE) fprintf(pFILE, "invalid maxoutputmidichannel, midi channel must range from 0 to 15 inclusively.\n");
		return FALSE;
	}
	if(global_minoutputmidichannel > global_maxoutputmidichannel)
	{
		if(pFILE) fprintf(pFILE, "invalid minoutputmidichannel, minoutputmidichannel has to be equal or smaller than maxoutputmidichannel.\n");
		return FALSE;
	}

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_SPIFRACTALMOUNTAINMIDIMCWIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	if(global_acceleratoractive)
	{
		hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPIFRACTALMOUNTAINMIDIMCWIN32));
	}
	else
	{
		hAccelTable = NULL;
	}


	//////////////////////////
	//initialize random number
	//////////////////////////
	srand((unsigned)time(0));


	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPIFRACTALMOUNTAINMIDIMCWIN32));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);

	if(global_menubardisplay)
	{
		wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SPIFRACTALMOUNTAINMIDIMCWIN32);
	}
	else
	{
		wcex.lpszMenuName = NULL; //no menu
	}
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

	//global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
	//global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"Segoe Script");

	if(global_titlebardisplay)
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, //original with WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	else
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_VISIBLE, //no WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	if (!hWnd)
	{
		return FALSE;
	}
	global_hwnd = hWnd;

	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//global_timer=timeSetEvent(100,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
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
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	//static HDC hDC; //returned by opengl InitGL()
	static HGLRC hRC; //returned by opengl InitGL()

	switch (message)
	{
	case WM_CREATE:
		{
			/*
			//0) execute cmd line to get all folder's .bmp filenames
			string quote = "\"";
			string pathfilter;
			string path=global_imagefolder;
			pathfilter = path + "\\*.bmp";
			//pathfilter = path + "\\*.jpg";
			string systemcommand;
			//systemcommand = "DIR " + quote + pathfilter + quote + "/B /O:N > wsic_filenames.txt"; //wsip tag standing for wav set (library) instrumentset (class) populate (function)
			systemcommand = "DIR " + quote + pathfilter + quote + "/B /S /O:N > spivoronoi_filenames.txt"; // /S for adding path into "wsic_filenames.txt"
			system(systemcommand.c_str());

			//2) load in all "spiss_filenames.txt" file
			//vector<string> global_txtfilenames;
			ifstream ifs("spivoronoi_filenames.txt");
			string temp;
			while(getline(ifs,temp))
			{
				//txtfilenames.push_back(path + "\\" + temp);
				global_txtfilenames.push_back(temp);
			}
			*/
			// setup OpenGL, then animation
			//InitGL( hWnd, hDC, hRC );  
			InitGL( hWnd, global_hDC, hRC );  
			//SetupAnimation(hDC, global_xwidth, global_yheight);
			SetupAnimation(global_hDC, global_xwidth, global_yheight);

			//3) initialize portmidi
			if(global_bsendmidi)
			{
				/////////////////////////
				//portmidi initialization
				/////////////////////////
				PmError err;
				Pm_Initialize();
				// list device information 
				if(pFILE) fprintf(pFILE, "MIDI output devices:\n");
				for (int i = 0; i < Pm_CountDevices(); i++) 
				{
					const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
					if (info->output) 
					{
						if(pFILE) fprintf(pFILE, "%d: %s, %s\n", i, info->interf, info->name);
						string devicename = info->name;
						global_midioutputdevicemap.insert(pair<string,int>(devicename,i));
					}
				}
				int midioutputdeviceid = 13;
				map<string,int>::iterator it;
				it = global_midioutputdevicemap.find(global_midioutputdevicename);
				if(it!=global_midioutputdevicemap.end())
				{
					midioutputdeviceid = (*it).second;
					if(pFILE) fprintf(pFILE, "%s maps to %d\n", global_midioutputdevicename.c_str(), midioutputdeviceid);
				}
				if(pFILE) fprintf(pFILE, "device %d selected\n", midioutputdeviceid);
				//err = Pm_OpenInput(&midi_in, inp, NULL, 512, NULL, NULL);
				err = Pm_OpenOutput(&global_pPmStream, midioutputdeviceid, NULL, 512, NULL, NULL, 0); //0 latency
				if (err) 
				{
					if(pFILE) fprintf(pFILE, Pm_GetErrorText(err));
					//Pt_Stop();
					//Terminate();
					//mmexit(1);
					global_bsendmidi = false;
				}
				//2.5)
				for(int k=0; k<global_pmeventlistsize; k++)
				{
					global_pmevenlistbusyfrommiditimer[k]=false;
					global_pmevenlistbusyfromglobaltimer[k]=false;
				}
				/*
				//3) set midi timers
				SetTimer( hWnd, global_miditimer, global_midistep_ms, NULL );
				SetTimer( hWnd, global_miditimer_programchange, global_midiprogramchangeperiod_ms, NULL );
				*/
			}

			//start timers
			//global_timer=timeSetEvent(100,25,(LPTIMECALLBACK)&StartGlobalProcess,(DWORD_PTR)(&hDC),TIME_ONESHOT);
			global_timer=timeSetEvent(1,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
			if(global_bsendmidi)
			{
				//3) set midi timers
				SetTimer( hWnd, global_miditimer, global_midistep_ms, NULL );
				SetTimer( hWnd, global_miditimer_programchange, global_midiprogramchangeperiod_ms, NULL );
			}
		}
		break;
	case WM_SIZE:
		{
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);

			global_imagewidth = rcClient.right - 0;
			global_imageheight = rcClient.bottom - 0; 

			//window resizing stuff
			//glViewport(0, 0, (GLsizei) global_imagewidth, (GLsizei) global_imageheight);
			reshape( (GLsizei) global_imagewidth, (GLsizei) global_imageheight);

		}
		break;
    case WM_ERASEBKGND:
		{
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		display(hdc);
		/*
		char buffer[100];
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPushMatrix();
        glPushMatrix();
		

		global_pairvector.clear();

		// draw some points
		glBegin(GL_POINTS);
 
			// go through the equations many times, drawing a point for each iteration
			//for (int i = 0; i &lt; iterations; i++) 
			for (int i = 0; i<iterations; i++) 
			{
 
				// compute a new point using the strange attractor equations
				float xnew = sin(y*b) + c*sin(x*b);
				float ynew = sin(x*a) + d*sin(y*a);
 
				// save the new point
				x = xnew;
				y = ynew;
 
				// draw the new point
				glVertex3f(x, y, 0.0f);
			}
 
		glEnd();

		//add last pair to vector
		global_pairvector.push_back(new glfloatpair(x,y));

		if(global_bdrawlabel)
		{
			glEnable(GL_TEXTURE_2D);
			//glColor4f(1.0f, 0.0f, 0.0f, 1.0f); //red
			//glColor4f(0.0f, 0.0f, 1.0f, 1.0f); //blue
			glScalef(0.02, 0.02, 0.02);
			for (int ii = 0; ii<1; ii++) 
			//for (int ii = 0; ii<iterations; ii++) 
			//for (int ii = 0; ii<10; ii++) 
			{
 
				// compute a new point using the strange attractor equations
				//float xnew = sin(y*b) + c*sin(x*b);
				//float ynew = sin(x*a) + d*sin(y*a);
 
				// save the new point
				//x = xnew;
				//y = ynew;
 				//spi, begin
				GLfloat xlabel = (global_pairvector[ii])->x;
				GLfloat ylabel = (global_pairvector[ii])->y; 

				//sprintf_s(buffer, 100-1, "%d", ii); 
				sprintf_s(buffer, 100-1, "+", ii); 
				global_GLFont.TextOut(buffer, xlabel/0.02,ylabel/0.02,0);
				//glColor4f(1.0f, 1.0f, 1.0f, 1.0f); //white
				//glScalef(1.0, 1.0, 1.0);
				//spi, end
				//glFlush();
			}
			glDisable(GL_TEXTURE_2D);
			//glColor4f(1.0f, 1.0f, 1.0f, 1.0f); //white
			glScalef(1.0, 1.0, 1.0);
		}

        glPopMatrix();

        glFlush();
        SwapBuffers(hdc);
		//SwapBuffers(global_hDC);
        glPopMatrix();
		

		switch(global_axiscoef)
		{
		case 1:
			a = a+0.1;
			if(a>3.0) 
			{
				a=-3.0;
				global_count++;
				if(global_count==2) 
				{
					global_count=0;
					a = -0.966918;		// coefficients for "The King's Dream"
					b = 2.879879;		
					c = 0.765145;
					d = 0.744728;
					global_axiscoef=RandomInteger(1,4);
				}
			}
			break;
		case 2:
			b = b+0.1;
			if(b>3.0) 
			{
				b=-3.0;
				global_count++;
				if(global_count==2) 
				{
					global_count=0;
					a = -0.966918;		// coefficients for "The King's Dream"
					b = 2.879879;		
					c = 0.765145;
					d = 0.744728;
					global_axiscoef=RandomInteger(1,4);
				}
			}
			break;
		case 3:
			c = c+0.1;
			if(c>1.0) 
			{
				c=-0.5;
				global_count++;
				if(global_count==2) 
				{
					global_count=0;
					a = -0.966918;		// coefficients for "The King's Dream"
					b = 2.879879;		
					c = 0.765145;
					d = 0.744728;
					global_axiscoef=RandomInteger(1,4);
				}
			}
			break;
		case 4:
			d = d+0.1;
			if(d>1.0) 
			{
				d=-0.5;
				global_count++;
				if(global_count==2) 
				{
					global_count=0;
					a = -0.966918;		// coefficients for "The King's Dream"
					b = 2.879879;		
					c = 0.765145;
					d = 0.744728;
					global_axiscoef=RandomInteger(1,4);
				}
			}
			break;
		}
		//reset position for net blit
		x = 0.1;
		y = 0.1;
		*/

		EndPaint(hWnd, &ps);
		break;
	  case WM_TIMER:
		  if(wParam==global_miditimer)
		  {
			  OnTimerMidi();
		  }
		  else if(wParam==global_miditimer_programchange)
		  {
			  OnTimerMidiProgramChange();
		  }
		return 0;                           

	case WM_DESTROY:
		if (global_timer) timeKillEvent(global_timer);
		/*
		if(global_pOW2Doc) delete global_pOW2Doc;
		if(global_pOW2View) delete global_pOW2View;
		*/
		for(int k=0; k<global_pmeventlistsize; k++)
		{
			for (it_pmeventlist = global_pmeventlist[k].begin(); it_pmeventlist != global_pmeventlist[k].end(); it_pmeventlist++)
			{
				if(*it_pmeventlist) delete *it_pmeventlist;
			}
			global_pmeventlist[k].clear();
		}
        CleanupAnimation();
		//CloseGL( hWnd, hDC, hRC );
		CloseGL( hWnd, global_hDC, hRC );
		if(pFILE) fclose(pFILE);
		/*
		//erase vector
		for(it=global_pairvector.begin(); it!=global_pairvector.end(); it++)
		{
			if(*it) delete *it;
		}
		global_pairvector.clear();
		*/

		if(global_bsendmidi)
		{
			KillTimer(hWnd, global_miditimer);
			KillTimer(hWnd, global_miditimer_programchange);
			if(global_pPmStream) 
			{
				//send all note off
				for(int k=0; k<128; k++)
				{
					for(int outputmidichannel=global_minoutputmidichannel; outputmidichannel<=global_maxoutputmidichannel; outputmidichannel++)
					{
						PmEvent myPmEvent;
						myPmEvent.timestamp = 0;
						myPmEvent.message = Pm_Message(0x90+outputmidichannel, k, 0);
						//send midi event
						Pm_Write(global_pPmStream, &myPmEvent, 1);
					}
				}
				Pm_Close(global_pPmStream);
			}
			//Pt_Stop();
			Pm_Terminate();
		}
		ShellExecuteA(NULL, "open", global_end.c_str(), "", NULL, 0);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void OnTimerMidi()
{
	if(global_bsendmidi)
	{
		//global_prevstep++;
		if(global_prevstep<0) global_prevstep=0;
		if(global_prevstep>(global_pmeventlistsize-1)) global_prevstep=0; //restart to begining

		//if(global_miditimerskip==true || global_pmevenlistbusyfromglobaltimer[global_prevstep]==false)
		if(global_miditimerskip==false && global_pmevenlistbusyfromglobaltimer[global_prevstep]==false)
		{
			//if(pFILE) fprintf(pFILE, "global_prevstep=%d, ",global_prevstep);
			global_pmevenlistbusyfrommiditimer[global_prevstep]=true;
			for (it_pmeventlist = global_pmeventlist[global_prevstep].begin(); it_pmeventlist != global_pmeventlist[global_prevstep].end(); it_pmeventlist++)
			{
				if((*it_pmeventlist)!=NULL)
				{
					//if(pFILE) fprintf(pFILE, "midievent sent, ");
					*it_pmeventlist;
					//send midi event
					Pm_Write(global_pPmStream, (*it_pmeventlist), 1);
					//delete pmevent
					delete *it_pmeventlist;
					*it_pmeventlist = NULL;
				}
			}
			//all midi event sent and deleted, empty list
			//global_pmeventlist[global_prevstep].empty();
			global_pmeventlist[global_prevstep].clear();
			global_pmevenlistbusyfrommiditimer[global_prevstep]=false;
			//if(pFILE) fprintf(pFILE, "\n");
		}
		global_prevstep++;
	}
}

void OnTimerMidiProgramChange()
{
	if(global_bsendmidi)
	{
		PmEvent myPmEvent;
		for(int outputmidichannel=global_minoutputmidichannel; outputmidichannel<=global_maxoutputmidichannel; outputmidichannel++)
		{
			int random_integer;
			int lowest=0;
			int highest=128-1;
			//int highest=7-1;
			int range=(highest-lowest)+1;
			//random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));
			random_integer = lowest+rand()%range;
			//fprintf(pFILE, "random_integer=%d\n", random_integer);	
		
			//PmEvent myPmEvent;
			myPmEvent.timestamp = 0;
			int midiprogramnumber = random_integer;
			if(midiprogramnumber>=128) midiprogramnumber=127;
			if(midiprogramnumber<=0) midiprogramnumber=0;
			int notused = 0;
			myPmEvent.message = Pm_Message(0xC0+outputmidichannel, midiprogramnumber, 0x00);
			//myPmEvent.message = Pm_Message(192+global_outputmidichannel, midiprogramnumber, 0);
			//send midi event
			Pm_Write(global_pPmStream, &myPmEvent, 1);
		}		
	}
}


// Initialize OpenGL
static void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC)
{
  
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory( &pfd, sizeof pfd );
  pfd.nSize = sizeof pfd;
  pfd.nVersion = 1;
  //pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL; //blaine's
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  
  hDC = GetDC( hWnd );
  
  int i = ChoosePixelFormat( hDC, &pfd );  
  SetPixelFormat( hDC, i, &pfd );

  hRC = wglCreateContext( hDC );
  wglMakeCurrent( hDC, hRC );

  //fractalmountain
	myGLInit();
}

// Shut down OpenGL
static void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( hRC );

  ReleaseDC( hWnd, hDC );
}


void SetupAnimation(HDC hDC, int Width, int Height)
{
		//glFont font
		GLuint textureName;
		glGenTextures(1, &textureName);
		global_GLFont.Create("arial-10.glf", textureName);


		reshape((GLsizei) Width, (GLsizei) Height);
		display(hDC);
		/*
		//window resizing stuff
        glViewport(0, 0, (GLsizei) Width, (GLsizei) Height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
		//spi, begin
        //glOrtho(-300, 300, -240, 240, 25, 75); //original
        //glOrtho(-300, 300, -8, 8, 25, 75); //spi, last
		glOrtho(-2.0f, 2.0f, -2.0f, 2.0f, ((GLfloat)-1), (GLfloat)1); //spi
        //glOrtho(-150, 150, -120, 120, 25, 75); //spi
		//spi, end
        
		
		glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();


        //background
        glClearColor(0.0, 0.0, 0.0, 0.0); //0.0s is black

		// set the foreground (pen) color
		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		// enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// enable point smoothing
		glEnable(GL_POINT_SMOOTH);
		glPointSize(1.0f);		
		

		// compute some initial iterations to settle into the orbit of the attractor
		//for (int i = 0; i &lt; initialIterations; i++) 
		for (int i = 0; i<initialIterations; i++) 
		{
 
			// compute a new point using the strange attractor equations
			float xnew = sin(y*b) + c*sin(x*b);
			float ynew = sin(x*a) + d*sin(y*a);
 
			// save the new point
			x = xnew;
			y = ynew;
		}
		//spi, begin
		//draw strange attractor once
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();
		glBegin(GL_POINTS);
			// go through the equations many times, drawing a point for each iteration
			for (int i = 0; i<iterations; i++) 
			//for (int i = 0; i<10; i++) 
			{
				// compute a new point using the strange attractor equations
				float xnew = sin(y*b) + c*sin(x*b);
				float ynew = sin(x*a) + d*sin(y*a);
 
				// save the new point
				x = xnew;
				y = ynew;
 
				// draw the new point
				glVertex3f(x, y, 0.0f);
			}
		glEnd();
		glFlush();
        SwapBuffers(hDC);
        glPopMatrix();

		glColor4f(1.0f, 0.0f, 0.0f, 1.0f); //red
		//spi, end
		*/
}


void CleanupAnimation()
{
        //didn't create any objects, so no need to clean them up
}


