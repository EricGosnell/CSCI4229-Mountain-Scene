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
int axes = 0; // Toggle axes
int polygon_count = 0; // Total number of polygons
int frame_count = 0; // Frame count
int t0 = 0; // Initial time for measuring FPS
float fps = -1; // Starting value for FPS

/* First Person Camera Settings */
int th = -135; // Azimuth of view angle
int fov = 60; // Field of view
double asp = 1; // Aspect ratio of screen
double dim = 1000; // Size of world
double E[3]; // Eye position for first person (Position you're at)
double C[3] = {2067,3100,-2120}; // Camera position for first person (Position you're looking at)

/* Lighting Values */
int distance;    		// Light distance
int ambient     = 35;   // Ambient intensity (%)
int diffuse     = 35;   // Diffuse intensity (%)
int specular    = 15;   // Specular intensity (%)
int emission    = 100;  // Emission intensity (%)
float shiny     = 1;    // Shininess (value)
int l_th        = 90;   // Light azimuth
float l_ph;    			// Elevation of light
float season = 0;

static void summer(){
    PineTree(2113,3140,-2453,.5*dim,.5*dim,.5*dim);
    PineTree(2332,3140,-2385,.5*dim,.5*dim,.5*dim);
    PineTree(1846,3110,-2382,.5*dim,.5*dim,.5*dim);
    PineTree(1853,3080,-2262,.5*dim,.5*dim,.5*dim);
    PineTree(1767,3110,-2168,.5*dim,.5*dim,.5*dim);
    PineTree(2545,3110,-2156,.5*dim,.5*dim,.5*dim);
    aspenTree(2607,3140,-1655,.2*dim,.2*dim,.2*dim);
    aspenTree(2701,3110,-1774,.2*dim,.2*dim,.2*dim);
    aspenTree(2786,3110,-1784,.2*dim,.2*dim,.2*dim);
    deer(2373,3150,-2154,.03*dim,.03*dim,.03*dim,0);
    deer(2273,3150,-2154,.03*dim,.03*dim,.03*dim,15);
    rabbit(2485,3140,-1664,.02*dim,.02*dim,.02*dim,180);
    owl(2570,3320,-2160,.02*dim, .02*dim,.02*dim,0);
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display() {
    // Reset polygon count
    polygon_count = 0;
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
//    glShadeModel(GL_FLAT);
    //  Translate intensity to color vectors
    float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
    float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
    float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
    //  Light position
    float Position[]  = {distance*Cos(l_th)+DEM_W/2,l_ph+3000,distance*Sin(l_th)-DEM_W/2,1.0};
    //  Draw light position as ball (still no lighting here)
     //  White ball with yellow specular
    float yellow[]   = {1.0,1.0,0.0,1.0};
    float Emission[] = {0.0,0.0,0.01*emission,1.0};
    glMaterialf(GL_FRONT,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
    glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
    sphere(Position[0],Position[1],Position[2], 0.1, 0.1,0.1,0,0,0,1,1,1);
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
    // DrawDEM(0,-3500,0,1);
    // glColor3f(1,1,1);
    // glWindowPos2i(5,100);
    // Print("Cx: %.2f, Cy: %.2f, Cz: %.2f",C[0],C[1],C[2]);
    // glWindowPos2i(5,80);
    // Print("Ex: %.2f, Ey: %.2f, Ez: %.2f",E[0],E[1],E[2]);

    summer();
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

    // Calculate FPS
    frame_count++;
    int t = glutGet(GLUT_ELAPSED_TIME);
    if (t - t0 >= 1000) {
        float seconds = (t-t0)/1000.0;
        fps = frame_count/seconds;
        t0 = t;
        frame_count = 0;
    }

    // Display parameters
    glWindowPos2i(5,45);
    Print("Polygon Count: %d, FPS: %2.2f",polygon_count,fps);
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
        // Reset View Angle TODO: reimplement
        // case '0':
        //     th = -135;
        //     // Eye position
        //     E[0] = 1.5; E[1] = 1.5; E[2] = 1.5;
        //     // Camera position for first person
        //     C[0] = 0; C[1] = -1.5; C[2] = 0;
        //     break;
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
    int seasonTime = fmod(t,60);
    if(seasonTime >= 0 && seasonTime < 15){
        season = 1;
    }
    else if(seasonTime >= 15 && seasonTime < 30){
        season = 2;
    }
    else if(seasonTime >= 30 && seasonTime < 45){
        season = 3;
    }
    if(seasonTime >= 45 && seasonTime <= 60){
        season = 4;
    }

    // Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[]) {
    // Set camera position
    E[0] = 2934;
    E[1] = 3230;
    E[2] = -1620;
    // Set light source position
    l_ph = 1.5*dim;
    distance = 1.5*dim;
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
    // Load DEM
    ReadDEM("cirque1.dem","cirque2.dem");

    //  Pass control to GLUT so it can interact with the user
    ErrCheck("init");
    glutMainLoop();
    return 0;
}
