/*
* CSCI 4229 F24 Final Project: Mountain Scene
* Eric Gosnell and Ragan Lee
*
* Key Bindings:
*     ESC             Quit Program
*     Arrows          Change View Angle
*     1-6             Cycle through spotlight locations
*
*     First Person Movement:
*     w/s             Move Forward/Backward
*     a/d             Move Left/Right
*     q/e             Move Up/Down
*/

#include "CSCIx229.h"

/* Global Variables */
double dt = 0; // Time step
int polygon_count = 0; // Total number of polygons
int frame_count = 0; // Frame count
int t0 = 0; // Initial time for measuring FPS
float fps = -1; // Starting value for FPS
GLuint vbo, ebo; // Vertex Buffer Object and Element Buffer Object

/* First Person Camera Settings */
int th = -78; // Azimuth of view angle
int fov = 60; // Field of view
double asp = 1; // Aspect ratio of screen
double dim = 1000; // Size of world
double E[3]; // Eye position for first person (Position you're at)
double C[3] = {0,85,0}; // Camera position for first person (Position you're looking at)

/* Lighting Values */
int distance;    		// Light distance
int ambient     = 35;   // Ambient intensity (%)
int diffuse     = 35;   // Diffuse intensity (%)
int specular    = 15;   // Specular intensity (%)
int emission    = 100;  // Emission intensity (%)
float shiny     = 1;    // Shininess (value)
int l_th        = 90;   // Light azimuth
float l_ph;    			// Elevation of light
int season = 0;         // 1: Spring, 2: Summer, 3: Autumn, 4: Winter
int sky[2];             // Sky textures


int frustumCulling(double x, double y, double z) {
    double F[3]; // Forward Vector F = C - E
    for (int i=0; i<3; i++) {
        F[i] = (C[i]-E[i]);
    }
    // Normalize the forward vector
    const float f_len = sqrt(F[0]*F[0]+F[1]*F[1]+F[2]*F[2]);
    for (int i=0; i<3; i++) {
        F[i] /= f_len;
    }
    // If it is close to the camera, include it regardless
    double distance = sqrt((E[0]-x)*(E[0]-x)+(E[2]-z)*(E[2]-z));
    if (distance < 200) return 1;
    // Do not include point if it is outside the view window
    if (F[0]*(x - E[0]) + F[1]*(y - E[1]) + F[2]*(z - E[2]) < 180) return 0;
    return 1;
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
    // glShadeModel(GL_FLAT);
    //  Translate intensity to color vectors
    float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
    float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
    float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
    //  Light position
    float Position[]  = {distance*Cos(l_th),l_ph,distance*Sin(l_th),1.0};
    //  Draw light position as ball (still no lighting here)
     //  White ball with yellow specular
    float yellow[]   = {1.0,1.0,0.0,1.0};
    float Emission[] = {0.0,0.0,0.01*emission,1.0};
    glMaterialf(GL_FRONT,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
    glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
    sphere(Position[0],Position[1],Position[2], 10, 10,10,0,0,0,1,1,1);
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

    /* Draw skybox */
    glDisable(GL_LIGHTING);
    Sky(5*dim,dt);
    glEnable(GL_LIGHTING);

    /* Draw Digital Elevation Models */
    DrawDEM(1);

    /* Draw forest */
    forest();

    /* Draw animals */
    releaseTheAnimals();

    // Calculate FPS
    frame_count++;
    int t = glutGet(GLUT_ELAPSED_TIME);
    if (t - t0 >= 1000) {
        float seconds = (t-t0)/1000.0;
        fps = frame_count/seconds;
        t0 = t;
        frame_count = 0;
    }

    // Display polygon count and framerate
    glColor3f(1,1,1);
    glWindowPos2i(5,5);
    Print("Polygon Count: %d, FPS: %2.2f",polygon_count,fps);

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
    const double df = 0.005; // First person movement factor
    switch (ch) {
        // ESC - Quit Program
        case 27:
            // Clean up VBO
            glDeleteBuffers(1,&vbo);
            glDeleteBuffers(1,&ebo);
            exit(0);
        /* Spotlight Locations */
        // Starting View Angle
        case '1':
            th = -78;
            E[0] = -218; E[1] = 35; E[2] = 550;
            C[1] = 85;
            break;
        // Bear and Aspens
        case '2':
            th = -144;
            E[0] = -88; E[1] = 20; E[2] = -4;
            C[1] = 135;
            break;
        // Alpine lake with bear
        // Fun Fact - Bears have been observed to climb up ridges for no reason other than to enjoy the view :)
        case '3':
            th = -69;
            E[0] = -110; E[1] = 440; E[2] = -924;
            C[1] = 385;
            break;
        // Deer herd with pretty peaks
        case '4':
            th = -12;
            E[0] = 532; E[1] = 95; E[2] = -129;
            C[1] = 230;
            break;
        // Back bowl
        case '5':
            th = -81;
            E[0] = -981; E[1] = 570; E[2] = 169;
            C[1] = 295;
            break;
        // Owl
        case '6':
            th = 45;
            E[0] = 136; E[1] = 110; E[2] = 249;
            C[1] = 235;
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
    dt = fmod(t, 360);
    l_th = fmod(t*30,360);
    //set season
    double seasonTime = fmod(t,60);
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
    E[0] = -218;
    E[1] = 35;
    E[2] = 550;
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
    ReadDEM("cirque.dem");
    //sky box textures
    sky[0] = LoadTexBMP("sky1.bmp");
    sky[1] = LoadTexBMP("sky2.bmp");
    //  Pass control to GLUT so it can interact with the user
    ErrCheck("init");
    glutMainLoop();

    // Clean up VBO
    glDeleteBuffers(1,&vbo);
    glDeleteBuffers(1,&ebo);
    return 0;
}
