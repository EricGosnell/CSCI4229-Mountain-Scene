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

/* Digital Elevation Model */
typedef struct {
    float data[3612][3612]; // Elevation Data
    float pos[2]; // Top left corner latitude and longitude
    const float resolution; // 1 arc-second = 24 meters
} DEM_1_arc;
DEM_1_arc n39w107 = {.resolution=24,
					 .pos[0] = -43344, // TODO: should not be centered at (0,0) meters
					 .pos[1] = 43344}; // 1 arc-second DEM starting at 39N, 107W

float ymag = 2; // DEM vertical magnification

/* First Person Camera Settings */
int th = -135; // Azimuth of view angle
int fov = 60; // Field of view
double asp = 1; // Aspect ratio of screen
double dim = 50000; // Size of world
double E[3]; // Eye position for first person
double C[3] = {0,0,0}; // Camera position for first person

/*
 *  Draw DEM Wireframe
 */
void drawDEM_1_arc(DEM_1_arc* dem) {
    const float dimension = sizeof(dem->data[0])/sizeof(float);
    const int y_norm = 3000; // Rough average elevation
    const int inc = 5; // Factor to reduce resolution by
    // Save transformation
    glPushMatrix();

	//  Draw DEM Points
    glPointSize(1);
	for (int i=0;i<dimension/2-1;i+=inc) {
		for (int j=0;j<dimension/2-1;j+=inc) {
            glColor3f(1,0,1);
			float x = dem->pos[0] + dem->resolution*i; // Latitude
			float z = dem->pos[1] - dem->resolution*j; // Longitude
//            glBegin(GL_POINTS);
////            if (dem->data[i][j] > 4000) glColor3f(1,1,0);
//            glVertex3f(x,ymag*dem->data[i][j]-y_norm,-z);
//            glEnd();
            glBegin(GL_QUAD_STRIP);
            glVertex3f(x,ymag*dem->data[i][j]-y_norm,-z);
            glVertex3f(x+dem->resolution*inc,ymag*dem->data[i+inc][j]-y_norm,-z);
            glVertex3f(x,ymag*dem->data[i][j+inc]-y_norm,-(z-dem->resolution*inc));
            glVertex3f(x+dem->resolution*inc,ymag*dem->data[i+inc][j+inc]-y_norm,-(z-dem->resolution*inc));
			glEnd();
		}
    }

    // Undo transformations
    glPopMatrix();
}

/*
 *  Read DEM from file
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

    // Draw Digital Elevation Models
    drawDEM_1_arc(&n39w107);

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
    Project(60,asp,dim);
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
    }
    //  Reproject
    Project(60,asp,dim);
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
    ReadDEM_1_arc("n39w107_1a_c.dem",&n39w107);

    //  Pass control to GLUT so it can interact with the user
    ErrCheck("init");
    glutMainLoop();
    return 0;
}