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
typedef struct {float x,y,z;} vtx;
int polygon_count = 0; // Total number of polygons
int frame_count = 0; // Frame count
int t0 = 0; // Initial time for measuring FPS
float fps = -1; // Starting value for FPS

/* Digital Elevation Model */
typedef struct {
    float** data; // Elevation Data
    float pos[2]; // Top left corner UTM coordinate
} DEM;
DEM gore_range = {.pos[0] = 0, // TODO: actual pos
				  .pos[1] = 0}; // 1 meter DEM starting at UTM 13 x383807 y4404735
typedef struct {
    vtx A, B, C;
    float rgba[4];
    float normal[3];
} DEM_triangle; // Triangle used in drawing the terrain
DEM_triangle triangles[763848]; // 2*(6178/10)^2  TODO: dynamic allocation based on chunks

float ymag = 1; // DEM vertical magnification

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
 *  Draw a sphere
 *     at (x,y,z)
 *     radius (r)
 *  Credit: Vlakkies
 */
static void sphere(double x,double y,double z,double r, int inc) {
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
         polygon_count++;
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}

// Linearly interpolates between two colors
void interpolateColor(const GLfloat color1[3], const GLfloat color2[3], float t, GLfloat result[3]) {
    for (int i = 0; i < 3; i++) {
        result[i] = (1.0f - t) * color1[i] + t * color2[i];
    }
}

static void getColor(float slope_angle, float orientation, DEM_triangle* tri) {
    float elevation = (tri->A.y + tri->B.y + tri->C.y)/3; // Average elevation of triangle
    float t; // Color interpolation value

    // Colors
	GLfloat forest[] = {0.13, 0.35, 0.13}; // Dark forest green
	GLfloat tundra[] = {0.53, 0.55, 0.46}; // Tundra greenish grey
	GLfloat dirtStone[] = {0.6, 0.5, 0.4}; // Exposed dirt and stone
    GLfloat snow[] = {0.8,0.8,0.8}; // Snow

    if (elevation <= 3300.0f) {
        // Below 3000, use dark forest green
        for (int i = 0; i < 3; i++) tri->rgba[i] = forest[i];
    } else if (elevation >= 4000.0f) {
        // Above 4000, use dirt/stone tri->rgba
        for (int i = 0; i < 3; i++) tri->rgba[i] = dirtStone[i];
    } else if (elevation < 3600.0f) {
        // Smooth transition between dark forest green and tundra green/grey (3000 to 3500)
        t = (elevation - 3300.0f) / 300.0f;
        interpolateColor(forest, tundra, t, tri->rgba);
    } else {
        // Smooth transition between tundra green/grey and dirt/stone (3500 to 4000)
        t = (elevation - 3600.0f) / 400.0f;
        interpolateColor(tundra, dirtStone, t, tri->rgba);
    }

	// Darken steeper slopes
    if (slope_angle > 30) {
    	float darkeningFactor = 1.0f - ((slope_angle-30) / 60.0f);
    	for (int i = 0; i < 3; i++) {
        	tri->rgba[i] *= darkeningFactor;
    	}
    }
    // Snow
    if (slope_angle < 55 && (orientation > 270 || orientation < 90 || elevation > 3900)) {
        if (elevation > 3650) {
			// Smooth transition between dirt/stone and snow (3700 to 3800)
        	t = (elevation - 3650.0f) / 100.0f;
        	interpolateColor(tundra, snow, t, tri->rgba);
        }
    }

    // Lakes
    if (slope_angle == 0) {
        tri->rgba[0] = 0;
        tri->rgba[1] = 0.4;
        tri->rgba[2] = 0.6;
    }

    // Set alpha value to 1
    tri->rgba[3] = 1;
}

/*
 *  Read 1 meter DEM from the 4 corresonding files
 */
void ReadDEM(char* file1, char* file2, char* file3, char* file4, DEM* dem) {
   // Initialize data array
   dem->data = malloc(6178 * sizeof(float*));
   for (int i = 0; i < 6178; i++) {
       dem->data[i] = malloc(6178 * sizeof(float));
   }

   // Read in data from files
   FILE* f1 = fopen(file1,"r");
   if (!f1) Fatal("Cannot open file %s\n",file1);
   FILE* f2 = fopen(file2,"r");
   if (!f2) Fatal("Cannot open file %s\n",file2);
   FILE* f3 = fopen(file3,"r");
   if (!f3) Fatal("Cannot open file %s\n",file3);
   FILE* f4 = fopen(file4,"r");
   if (!f4) Fatal("Cannot open file %s\n",file4);

   // Write elevation data for each point to the data array
   for (int i=0; i<6178; i++) {
       for (int j=0; j<6178; j++) {
           if (i<1544) {if (fscanf(f1,"%f",&dem->data[j][i])!=1) Fatal("Error reading %s\n", file1);}
           else if (i<3089) {if (fscanf(f2,"%f",&dem->data[j][i])!=1) Fatal("Error reading %s\n", file2);}
           else if (i<4632) {if (fscanf(f3,"%f",&dem->data[j][i])!=1) Fatal("Error reading %s\n", file3);}
           else {if (fscanf(f4,"%f",&dem->data[j][i])!=1) Fatal("Error reading %s\n", file4);}
	   }
   }

   fclose(f1);
   fclose(f2);
   fclose(f3);
   fclose(f4);

   /* Populate triangles array */ //TODO: different sized triangles based on distance to camera
   const int inc = 10; // Factor by which to decrease resolution
   int t = 6178/inc; // Width of triangles //TODO: dem->width

   // Add elevation values from data
   for (int i=0; i<t; i++) {
       for (int j=0; j<t; j++) {
           int n = (i*t+j)*2;
           float x = dem->pos[0] + j*inc;
           float z = dem->pos[1] - i*inc;
           // Bottom left triangle
           triangles[n].A.x = x;
           triangles[n].A.y = dem->data[i*inc][j*inc];
           triangles[n].A.z = z;

           triangles[n].B.x = x;
           triangles[n].B.y = dem->data[(i+1)*inc][j*inc];
           triangles[n].B.z = z-inc;

           triangles[n].C.x = x+inc;
           triangles[n].C.y = dem->data[(i+1)*inc][(j+1)*inc];
           triangles[n].C.z = z-inc;

           // Top right triangle
           triangles[n+1].A.x = x;
           triangles[n+1].A.y = dem->data[i*inc][j*inc];
           triangles[n+1].A.z = z;

           triangles[n+1].B.x = x+inc;
           triangles[n+1].B.y = dem->data[(i+1)*inc][(j+1)*inc];
           triangles[n+1].B.z = z-inc;

           triangles[n+1].C.x = x+inc;
           triangles[n+1].C.y = dem->data[i*inc][(j+1)*inc];
           triangles[n+1].C.z = z;

           for (int k=0; k<2; k++) {
               // Calculate normal vector
               float Nx = (triangles[n+k].A.y-triangles[n+k].B.y)*(triangles[n+k].C.z-triangles[n+k].A.z) -
               (triangles[n+k].C.y-triangles[n+k].A.y)*(triangles[n+k].A.z-triangles[n+k].B.z);
               float Ny = (triangles[n+k].A.z-triangles[n+k].B.z)*(triangles[n+k].C.x-triangles[n+k].A.x) -
               (triangles[n+k].C.z-triangles[n+k].A.z)*(triangles[n+k].A.x-triangles[n+k].B.x);
               float Nz = (triangles[n+k].A.x-triangles[n+k].B.x)*(triangles[n+k].C.y-triangles[n+k].A.y) -
               (triangles[n+k].C.x-triangles[n+k].A.x)*(triangles[n+k].A.y-triangles[n+k].B.y);
               // Add normal vector to triangle
               triangles[n+k].normal[0] = Nx;
               triangles[n+k].normal[1] = Ny;
               triangles[n+k].normal[2] = Nz;
               // Normalize the normal vector vertical component
               float length = sqrt(Nx*Nx+Ny*Ny+Nz*Nz);
               Ny /= length;
               // Calculate slope angle
               float slope_angle = acos(Ny) * (180/M_PI);
               // Calculate orientation
               float orientation = atan2(-Nz,-Nx) * (180/M_PI);
			   if (orientation < 0) orientation += 360;
               // Set color based on slope angle, orientation, and elevation
               getColor(slope_angle,orientation,&triangles[n+k]);
           }
       }
   }
}

/*
 * Free dynamically allocated memory for the DEM
 */
void freeDEM(DEM* dem) {
    for (int i = 0; i < 6178; i++) {
        free(dem->data[i]);
    }
    free(dem->data);
}

/*
 *  Draw 1 meter DEM Wireframe
 */
void drawDEM(double dx, double dy, double dz, double scale) {
    // Save transformation
    glPushMatrix();
    // Translate and Scale
    // TODO: I really don't know why these have to be backwards
    glScaled(scale,scale,scale);
    glRotated(180,0,1,0);
    glTranslated(dx,dy,dz);


    // Enable Face Culling
    glEnable(GL_CULL_FACE);
    // Set Color Properties
    float black[]  = {0.0,0.0,0.0,1.0};
    float white[]  = {1.0,1.0,1.0,1.0};
    glMaterialfv(GL_FRONT,GL_EMISSION,black);
    glMaterialfv(GL_FRONT,GL_SPECULAR,white);
    glMaterialf(GL_FRONT,GL_SHININESS,shiny);
    // Draw DEM Triangles
    int nt = sizeof(triangles)/sizeof(DEM_triangle);
    for (int i=0; i<nt; i++) {
        glNormal3f(triangles[i].normal[0],triangles[i].normal[1],triangles[i].normal[2]);
        glColor4f(triangles[i].rgba[0],triangles[i].rgba[1],triangles[i].rgba[2],triangles[i].rgba[3]);
//        glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,triangles[i].rgba);
        glBegin(GL_TRIANGLES);
            glVertex3f(triangles[i].C.x, triangles[i].C.y, triangles[i].C.z);
            glVertex3f(triangles[i].B.x, triangles[i].B.y, triangles[i].B.z);
            glVertex3f(triangles[i].A.x, triangles[i].A.y, triangles[i].A.z);
        glEnd();
        polygon_count++;
    }
    // Disable Face Culling
    glDisable(GL_CULL_FACE);

    // Undo transformations
    glPopMatrix();
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
    float Position[]  = {distance*Cos(l_th),l_ph,distance*Sin(l_th),1.0};
    //  Draw light position as ball (still no lighting here)
    glColor3f(1,1,1);
    sphere(Position[0],Position[1],Position[2], 0.01*dim, 30);
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
    const int center = 6178/2; // TODO: gore_range.width
    drawDEM(-gore_range.pos[0]-center,-3500*ymag,gore_range.pos[1]+center,3);


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
            freeDEM(&gore_range);
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
    // Load DEM
    ReadDEM("GoreRange1.dem","GoreRange2.dem","GoreRange3.dem","GoreRange4.dem",&gore_range);

    //  Pass control to GLUT so it can interact with the user
    ErrCheck("init");
    glutMainLoop();
    freeDEM(&gore_range);
    return 0;
}