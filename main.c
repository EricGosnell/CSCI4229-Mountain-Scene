/*
* CSCI 4229 F24 Final Project: Mountain Scene
* Eric Gosnell and Ragan Lee
*
* Key Bindings:
*     ESC             Quit Program
*     Arrows          Change View Angle
*     0               Reset View Angle
*     PgUp/PgDown     Zoom In/Out
*
*     First Person Movement:
*     w/s             Move Forward/Backward
*     a/d             Move Left/Right
*     q/e             Move Up/Down
*/

#include "CSCIx229.h"

/* Global Variables */
double dt = 0; // Time step
int axes = 1; // Toggle axes
typedef struct {float x,y,z;} vtx;

/* Digital Elevation Model */
typedef struct {
    float data[3612][3612]; // Elevation Data
    float pos[2]; // Top left corner latitude and longitude
    const float resolution; // 1 arc-second = 24 meters
} DEM_1_arc;
DEM_1_arc n39w107 = {.resolution=24,
					 .pos[0] = -43344, // TODO: should not be centered at (0,0) meters
					 .pos[1] = 43344}; // 1 arc-second DEM starting at 39N, 107W
typedef struct {
    float data[6178][6178]; // Elevation Data
    float pos[2]; // Top left corner UTM coordinate
    const float resolution; // 1 meter
} DEM_1m;
DEM_1m gore_range = {.resolution=1,
					 .pos[0] = 383807,
					 .pos[1] = 4404735}; // 1 meter DEM starting at UTM 13 x383807 y4404735
float ymag = 1.5; // DEM vertical magnification

/* First Person Camera Settings */
int th = -135; // Azimuth of view angle
int fov = 60; // Field of view
double asp = 1; // Aspect ratio of screen
double dim = 1000; // Size of world
double E[3]; // Eye position for first person
double C[3] = {0,0,0}; // Camera position for first person

/* Lighting Values */
int distance;    		// Light distance
int ambient     = 35;   // Ambient intensity (%)
int diffuse     = 35;   // Diffuse intensity (%)
int specular    = 15;   // Specular intensity (%)
int emission    = 100;  // Emission intensity (%)
float shiny     = 1;    // Shininess (value)
int l_th        = 90;   // Light azimuth
float l_ph;    			// Elevation of light

/*
 *  Draw vertex in polar coordinates with normal
 *  Credit: Vlakkies
 */
static void Vertex(double th,double ph) {
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

/*
 * Draw a triangle
 */
static void triangle(vtx A, vtx B, vtx C) {
    //  Normal vector
    float Nx = (A.y-B.y)*(C.z-A.z) - (C.y-A.y)*(A.z-B.z);
    float Ny = (A.z-B.z)*(C.x-A.x) - (C.z-A.z)*(A.x-B.x);
    float Nz = (A.x-B.x)*(C.y-A.y) - (C.x-A.x)*(A.y-B.y);
    //  Draw triangle
    glNormal3f(Nx,Ny,Nz);
    glBegin(GL_TRIANGLES);
        glVertex3f(A.x,A.y,A.z);
        glVertex3f(B.x,B.y,B.z);
        glVertex3f(C.x,C.y,C.z);
    glEnd();
}

/*
 *  Draw a sphere
 *     at (x,y,z)
 *     radius (r)
 *  Credit: Vlakkies
 */
static void sphere(double x,double y,double z,double r) {
   const int inc = 15;
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball with yellow specular
   float yellow[]   = {1.0,1.0,0.0,1.0};
   float Emission[] = {0.0,0.0,0.01*emission,1.0};
   glColor3f(1,1,1);
   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (int ph=-90;ph<90;ph+=inc) {
      glBegin(GL_QUAD_STRIP);
      for (int th=0;th<=360;th+=2*inc) {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}

/*
 *  Draw 1 arc-second DEM Wireframe
 */
void drawDEM_1_arc(DEM_1_arc* dem, double scale) {
    const float dimension = sizeof(dem->data[0])/sizeof(float);
    const int y_norm = 3000; // Rough average elevation in meters
    const int inc = 1; // Factor to reduce resolution by
    // Save transformation
    glPushMatrix();
    // Scale
    glScaled(scale,scale,scale);

	//  Draw DEM Triangles
	for (int i=7*dimension/16;i<9*dimension/16-1;i+=inc) {
		for (int j=7*dimension/16;j<9*dimension/16-1;j+=inc) {
            glColor3f(1,0,1);
			float x = dem->pos[0] + dem->resolution*i; // Latitude
			float z = dem->pos[1] - dem->resolution*j; // Longitude
//            if (dem->data[i][j] > 3200) glColor3f(1,1,1);
            vtx A = {x,ymag*dem->data[i][j]-y_norm,-z};
            vtx B = {x+dem->resolution*inc,ymag*dem->data[i+inc][j]-y_norm,-z};
            vtx C = {x,ymag*dem->data[i][j+inc]-y_norm,-(z-dem->resolution*inc)};
            vtx D = {x+dem->resolution*inc,ymag*dem->data[i+inc][j+inc]-y_norm,-(z-dem->resolution*inc)};
            triangle(A,B,C);
            triangle(C,B,D);
		}
    }

    // Undo transformations
    glPopMatrix();
}

/*
 *  Read 1 arc-second DEM from file
 */
void ReadDEM_1_arc(char* file, DEM_1_arc* dem) {
   const float dimension = sizeof(dem->data[0])/sizeof(float);
   int i,j;
   FILE* f = fopen(file,"r");
   if (!f) Fatal("Cannot open file %s\n",file);
   for (j=0; j<dimension; j++) {
      for (i=0; i<dimension; i++) {
         if (fscanf(f,"%f",&dem->data[i][j])!=1) Fatal("Error reading %s\n", file);
      }
    }
   fclose(f);
}

/*
 *  Draw 1 meter DEM Wireframe
 */
void drawDEM_1m(DEM_1m* dem, double dx, double dy, double dz, double scale) {
    const float dimension = sizeof(dem->data[0])/sizeof(float);
    const int inc = 10; // Factor to reduce resolution by
    // Save transformation
    glPushMatrix();
    // Translate and Scale
    // TODO: I really don't know why these have to be backwards
    glScaled(scale,scale,scale);
    glTranslated(dx,dy,dz);

    // Draw DEM Triangles
	for (int i=0;i<dimension-inc;i+=inc) {
		for (int j=0;j<dimension-inc;j+=inc) {
            if (dem->data[i][j] == 0) continue;
            glColor3f(1,0,1);
			float x = dem->pos[0] + dem->resolution*i; // UTM x-coordinate
			float z = dem->pos[1] - dem->resolution*j; // UTM z-coordinate
//            if (dem->data[i][j] > 3200) glColor3f(1,1,1);
            vtx A = {x,ymag*dem->data[i][j],-z};
            vtx B = {x+dem->resolution*inc,ymag*dem->data[i+inc][j],-z};
            vtx C = {x,ymag*dem->data[i][j+inc],-(z-dem->resolution*inc)};
            vtx D = {x+dem->resolution*inc,ymag*dem->data[i+inc][j+inc],-(z-dem->resolution*inc)};
            triangle(A,B,C);
            triangle(C,B,D);
		}
    }

    // Undo transformations
    glPopMatrix();
}

/*
 *  Read 1 meter DEM from the 4 corresonding files
 */
void ReadDEM_1m(char* file1, char* file2, char* file3, char* file4, DEM_1m* dem) {
   FILE* f1 = fopen(file1,"r");
   if (!f1) Fatal("Cannot open file %s\n",file1);
   FILE* f2 = fopen(file2,"r");
   if (!f2) Fatal("Cannot open file %s\n",file2);
   FILE* f3 = fopen(file3,"r");
   if (!f3) Fatal("Cannot open file %s\n",file3);
   FILE* f4 = fopen(file4,"r");
   if (!f4) Fatal("Cannot open file %s\n",file4);

   for (int i=0; i<6178; i++) {
       for (int j=0; j<6178; j++) {
           if (i<1544) {if (fscanf(f1,"%f",&dem->data[i][j])!=1) Fatal("Error reading %s\n", file1);}
           else if (i<3089) {if (fscanf(f2,"%f",&dem->data[i][j])!=1) Fatal("Error reading %s\n", file2);}
           else if (i<4632) {if (fscanf(f3,"%f",&dem->data[i][j])!=1) Fatal("Error reading %s\n", file3);}
           else {if (fscanf(f4,"%f",&dem->data[i][j])!=1) Fatal("Error reading %s\n", file4);}
	   }
   }

   fclose(f1);
   fclose(f2);
   fclose(f3);
   fclose(f4);
}


/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display() {
    //  Erase the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //  Enable Z-buffering in OpenGL
    glEnable(GL_DEPTH_TEST);
    //  Undo previous transformations
    glLoadIdentity();

    // Set view angle
    C[0] = E[0] + dim*cos(th*3.1415926/180); // Fx = Cx - Ex = cos(th)
    C[2] = E[2] + dim*sin(th*3.1415926/180); // Fz = Cz - Ez = sin(th)
    gluLookAt(E[0],E[1],E[2], C[0],C[1],C[2], 0,1,0);

    /* Set lighting values and create light source */
    glShadeModel(GL_SMOOTH);
    //  Translate intensity to color vectors
    float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
    float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
    float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
    //  Light position
    float Position[]  = {distance*Cos(l_th),l_ph,distance*Sin(l_th),1.0};
    //  Draw light position as ball (still no lighting here)
    glColor3f(1,1,1);
    sphere(Position[0],Position[1],Position[2], 0.01*dim);
    //  OpenGL should normalize normal vectors
    glEnable(GL_NORMALIZE);
    //  Enable lighting
    glEnable(GL_LIGHTING);
    //  Location of viewer for specular calculations
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1); // Local = True
    //  glColor sets ambient and diffuse color materials
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    //  Enable light 0
    glEnable(GL_LIGHT0);
    //  Set ambient, diffuse, specular components and position of light 0
    glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
    glLightfv(GL_LIGHT0,GL_POSITION,Position);

    /* Draw Digital Elevation Models */
//    drawDEM_1_arc(&n39w107,10);
    const int center = sizeof(gore_range.data[0])/sizeof(float)*gore_range.resolution/2;
    drawDEM_1m(&gore_range,-gore_range.pos[0]-center,-3300*ymag,gore_range.pos[1]-center,1);

    /* Draw axes */
    glDisable(GL_LIGHTING);

    //  Draw axes
    glColor3f(1,1,1);
    if (axes) {
        const double len=dim/2;
        glBegin(GL_LINES);
        glVertex3d(0,0,0);
        glVertex3d(len,0,0);
        glVertex3d(0,0,0);
        glVertex3d(0,len,0);
        glVertex3d(0,0,0);
        glVertex3d(0,0,len);
        glEnd();

        //  Label axes
        glRasterPos3d(len,0,0);
        Print("X");
        glRasterPos3d(0,len,0);
        Print("Y");
        glRasterPos3d(0,0,len);
        Print("Z");
    }

    // Display parameters
    glWindowPos2i(5,25);
    Print("Ambient: %d%%, Diffuse: %d%%, Specular: %d%%",ambient, diffuse, specular);
    glWindowPos2i(5,5);
    Print("X: %.1f, Y: %.1f, Z: %.1f, Dim: %.0f",E[0],E[1],E[2],dim);

    //  Render the scene and make it visible
    ErrCheck("display");
    glFlush();
    glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y) {
    //  Right arrow key - increase angle by 5 degrees
    if (key == GLUT_KEY_RIGHT)
        th += 3;
    //  Left arrow key - decrease angle by 5 degrees
    else if (key == GLUT_KEY_LEFT)
        th -= 3;
    //  Up arrow key - increase elevation by 5% of diem
    else if (key == GLUT_KEY_UP)
        C[1] += 0.05*dim;
    //  Down arrow key - decrease elevation by 5% of dim
    else if (key == GLUT_KEY_DOWN)
        C[1] -= 0.05*dim;
    //  PageUp key - increase dim
    else if (key == GLUT_KEY_PAGE_DOWN)
        dim += 1;
    //  PageDown key - decrease dim
    else if (key == GLUT_KEY_PAGE_UP && dim>1)
        dim -= 1;
    //  Keep angles to +/-360 degrees
    th %= 360;
    //  Update projection
    Project(fov,asp,dim);
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y) {
    const double df = 0.03; // First person movement factor
    switch (ch) {
        // ESC - Quit Program
        case 27:
            exit(0);
        // Reset View Angle
        case '0':
            th = -135;
            // Eye position
            E[0] = 1.5; E[1] = 1.5; E[2] = 1.5;
            // Camera position for first person
            C[0] = 0; C[1] = -1.5; C[2] = 0;
            break;
        // Toggle axes TODO: Remove in final product
        case 'x':
        case 'X':
            axes = !axes;
            break;

        /* First Person Movement */
        // Move forward
        case 'w':
        case 'W':
            // F = C - E
            E[0] += df*(C[0]-E[0]); // Ex += df*Fx
            E[2] += df*(C[2]-E[2]); // Ez += df*Fz
            break;
        // Move left
        case 'a':
        case 'A':
            // S = F x U = (-Fz, 0, Fx) where U = (0, 1, 0)
            E[0] -= (-1)*df*(C[2]-E[2]); // Ex -= df*Sx
            E[2] -= df*(C[0]-E[0]); // Ez -= df*Sz
            break;
        // Move backwards
        case 's':
        case 'S':
            // F = C - E
            E[0] -= df*(C[0]-E[0]); // Ex -= df*Fx
            E[2] -= df*(C[2]-E[2]); // Ez -= df*Fz
            break;
        // Move right
        case 'd':
        case 'D':
            // S = F x U = (-Fz, 0, Fx)
            E[0] += (-1)*df*(C[2]-E[2]); // Ex += df*Sx
            E[2] += df*(C[0]-E[0]); // Ez += df*Sz
            break;
        // Move up
        case 'q':
        case 'Q':
            E[1] += df*dim;
            C[1] += df*dim;
            break;
        // Move down
        case 'e':
        case 'E':
            E[1] -= df*dim;
            C[1] -= df*dim;
            break;

        /* Lighting Controls */
        // TODO: Remove in final product
        // Increase ambient light
        case '1':
            if (ambient < 100) ambient += 5;
            break;
        // Decrease ambient light
        case '!':
            if (ambient > 0) ambient -= 5;
            break;
        // Increase diffuse light
        case '2':
            if (diffuse < 100) diffuse += 5;
            break;
        // Decrease diffuse light
        case '@':
            if (diffuse > 0) diffuse -= 5;
            break;
        // Increase specular light
        case '3':
            if (specular < 100) specular += 5;
            break;
        // Decrease specular light
        case '#':
            if (specular > 0) specular -= 5;
            break;
        // Increase light elevation
        case ']':
            l_ph += 0.01*dim;
            break;
        // Increase light elevation
        case '[':
            l_ph -= 0.01*dim;
            break;
    }
    //  Reproject
    Project(fov,asp,dim);
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height) {
    //  Ratio of the width to the height of the window
    asp = (height>0) ? (double)width/height : 1;
    //  Set the viewport to the entire window
    glViewport(0,0, RES*width,RES*height);
    //  Set projection
    Project(60,asp,dim);
}

static void idle(void) {
    double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    dt = fmod(t*200, 360);
    l_th = fmod(t*30,360);

    // Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[]) {
    // Set camera position
    E[0] = 0.66*dim;
    E[1] = 0.85*dim;
    E[2] = 0.66*dim;
    // Set light source position
    l_ph = 1.5*dim;
    distance = 2*dim;
    //  Initialize GLUT
    glutInit(&argc,argv);
    //  Request double buffered, true color window with Z buffering at 600x600
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(800,800);
    glutCreateWindow("Eric Gosnell and Ragan Lee");
#ifdef USEGLEW
    //  Initialize GLEW
    if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
    //  Set callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);
    // TODO: Mouse control
//    glutMouseFunc(mouse);
//    glutMotionFunc(motion);
    //  TODO: Load textures
    // Load DEMs
//    ReadDEM_1_arc("n39w107_1a_c.dem",&n39w107);
    ReadDEM_1m("GoreRange1.dem","GoreRange2.dem","GoreRange3.dem","GoreRange4.dem",&gore_range);

    //  Pass control to GLUT so it can interact with the user
    ErrCheck("init");
    glutMainLoop();
    return 0;
}