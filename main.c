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
int th = -135; // Azimuth of view angle
double dt = 0; // Time step
int axes = 1; // Toggle axes
int polygon_count;

/* First Person Camera Settings */
int fov = 60; // Field of view
double asp = 1; // Aspect ratio of screen
double dim = 1; // Size of world
double E[3] = {1.5,1.5,1.5}; // Eye position for first person
double C[3] = {0,-1.5,0}; // Camera position for first person

/* Lighting Values */
int distance    = 4;    // Light distance
int ambient     = 35;   // Ambient intensity (%)
int diffuse     = 35;   // Diffuse intensity (%)
int specular    = 15;   // Specular intensity (%)
int emission    = 100;    // Emission intensity (%)
float shiny     = 1;    // Shininess (value)
int l_th        = 90;   // Light azimuth
float l_ph      = 2;    // Elevation of light
float season = 0;


const float black[] = {0,0,0,1};
const float white[] = {1,1,1,1};
const float pineTreeColors[][3] = {{0.1, 0.2, 0.03},{0.2, 0.4, 0.05},{0.3, 0.5, 0.1},{0.4, 0.6, 0.2},{0.5, 0.7, 0.3},{0.6, 0.8, 0.4}};
const float aspentrunk[3] = {0.9f, 0.9f, 0.9f};



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


static void CylinderVertex(double th,double inputy) {
   double x = Cos(th);
   double y = inputy;  
   double z = Sin(th);
   
   glNormal3d(x,0,z);
   glVertex3d(x,y,z);
}


/*
 *  Draw a sphere
 *     at (x,y,z)
 *     radius (r)
 *  Credit: Vlakkies
 */
static void sphere(double x,double y,double z,double rx, double ry, double rz, double thx, double thy, double thz, float r, float g, float b) {
   const int inc = 15;
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glRotated(thx,1,0,0);
   glRotated(thy,0,1,0);
   glRotated(thz,0,0,1);
   glScaled(rx,ry,rz);
   glColor3f(r,g,b);
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
static void halfSphere(double x,double y,double z,double rx, double ry, double rz, double thx, double thy, double thz, float r, float g, float b) {
   const int inc = 15;
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glRotated(thx,1,0,0);
   glRotated(thy,0,1,0);
   glRotated(thz,0,0,1);
   glScaled(rx,ry,rz);
   glColor3f(r,g,b);
   //  Bands of latitude
   for (int ph=-90;ph<0;ph+=inc) {
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

/*
 *  Draw a sphere
 *     at (x,y,z)
 *     radius (r)
 *  Credit: Vlakkies
 */
    static void cylinder(double x,double y,double z,double dx, double dy, double dz, double thx, double thy, double thz, int inc, float r, float g, float b) {
    //  Save transformation
    glPushMatrix();
    glTranslated(x,y,z);
    glRotated(thx,1,0,0);
    glRotated(thy,0,1,0);
    glRotated(thz,0,0,1);
    glScaled(dx,dy,dz);
    glColor3f(r,g,b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,.5,0);
    for(int th = 0; th <= 360; th+=2*inc){
        CylinderVertex(th,.5);
        polygon_count++;
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    for (int th=0;th<=360;th+=2*inc) {
        CylinderVertex(th,0);
        CylinderVertex(th,.5);
        polygon_count++;
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0);
    for(int th = 0; th <= 360; th+=2*inc){
        CylinderVertex(th,0);
        polygon_count++;
    }
    glEnd();
    //  Undo transofrmations
    glPopMatrix();
}
static void owl(double x, double y, double z, double dx, double dy, double dz, double th){
    glPushMatrix();

    glMaterialfv(GL_FRONT,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT,GL_EMISSION,black);

    //adjust placement, rotation, scale
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);

    sphere(0,0,0,.20,.4,.2,0,0,-30,1,1,1);
    
    sphere(.23,.20,-.07,.02,.03,.03,30,0,0,0,0,0);
    sphere(.23,.20,.07,.02,.03,.03,-30,0,0,0,0,0);
    sphere(.24,.18,0,.03,.04,.015,0,0,0,0,0,0); //beak - change color!

    sphere(-.08,-.08,.17,.15,.3,.05,0,0,-30,1,1,1);
    sphere(-.08,-.08,-.17,.15,.3,.05,0,0,-30,1,1,1);

    sphere(-.25,-.25,0,.05,.2,.05,0,0,-30,1,1,1);
    sphere(-.25,-.25,-.05,.05,.2,.05,0,0,-30,1,1,1);
    sphere(-.25,-.25,.05,.05,.2,.05,0,0,-30,1,1,1);
    sphere(-.25,-.25,-.1,.05,.2,.05,15,0,-30,1,1,1);
    sphere(-.25,-.25,.1,.05,.2,.05,-15,0,-30,1,1,1);

    cylinder(0,-.4,.07, .025,.5,.025,0,0,0,15,1,1,1);
    cylinder(0,-.4,-.07, .025,.5,.025,0,0,0,15,1,1,1);

    glPopMatrix();

}
static void rabbit(double x, double y, double z, double dx, double dy, double dz, double th){
    glPushMatrix();

    glMaterialfv(GL_FRONT,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT,GL_EMISSION,black);

    //adjust placement, rotation, scale
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);

    sphere(-.2,1,.5,.15,.6,.15,0,0,0,1,1,1); //ears
    sphere(.2,1,.5,.15,.6,.15,0,0,0,1,1,1); //ears
    sphere(0,.65,.65,.4,.4,.4,0,0,0,1,1,1); //head
    sphere(0,.6,.9,.25,.2,.25,0,0,0,1,1,1); //nose
    sphere(0,.6,1.15,.05,.02,.02,0,0,0,0.961, 0.757, 0.757); //nose button
    sphere(.25,.75,.8,.12,.07,.07,0,0,0,0,0,0); //eye right side
    sphere(-.25,.75,.8,.12,.07,.07,0,0,0,0,0,0); //eye left side
    sphere(0,0,.5,.5,.45,.45,0,0,0,1,1,1); //body front
    sphere(-.25,-.2,.7,.2,.4,.2,0,0,0,1,1,1); //leg left side
    sphere(.25,-.2,.7,.2,.4,.2,0,0,0,1,1,1); //leg right side
    sphere(0,-.05,0,.6,.6,.6,0,0,0,1,1,1); //back half of body
    sphere(0,.1,-.6,.3,.3,.3,0,0,0,1,1,1); //tail
    sphere(.3,-.1,0,.4,.5,.4,0,0,0,1,1,1); //back leg right
    sphere(-.32,-.1,0,.4,.5,.4,0,0,0,1,1,1); //back leg left

    glPopMatrix();
}
static void deerBody(double x, double y, double z){
    sphere(0,0,z-.5,.3,.5,.6,0,0,0,0.545, 0.353, 0.169); //back body sphere
    sphere(0,0,z,.3,.5,1.1,0,0,0,0.545, 0.353, 0.169); //mid body sphere
    sphere(0,0,z+.7,.35,.5,.55,0,0,0,0.545, 0.353, 0.169); //front body sphere
}
static void deerLeg(double x, double y, double z){
    if(z > 0){
        sphere(x,y,z,.15,.7,.15,5,0,0,0.545, 0.353, 0.169); //thigh
        cylinder(x,y-1.35,z-.05,.08,1.7,.08,0,0,0,15,0.545, 0.353, 0.169); 
    }
    else{
        sphere(x,y,z,.2,.45,.4,-15,0,0,0.545, 0.353, 0.169); //thigh
        sphere(x,y-.5,z-.15,.1,.5,.2,50,0,0,0.545, 0.353, 0.169); 
        cylinder(x,y-1.6,z-.35,.08,1.8,.08,-5,0,0,15,0.545, 0.353, 0.169); 
    }
}
static void deerHead(double x, double y, double z){
    sphere(0,.4,1.1,.25,.5,.25,15,0,0,0.545, 0.353, 0.169); //neck
    sphere(0,.85,1.2,.25,.27,.27,0,0,0,0.545, 0.353, 0.169); //head
    halfSphere(0,.85,1.3,.15,.15,.4,15,180,0,0.545, 0.353, 0.169); //nose
    sphere(0,.8,1.6,.1,.05,.1,0,0,0,0,0,0); //tip of nose
    sphere(.3,1.1,1.15,.1,.23,.1,0,-10,-45,0.545, 0.353, 0.169); // right ear
    sphere(-.3,1.1,1.15,.1,.23,.1,0,10,45,0.545, 0.353, 0.169); // left ear
    sphere(.15,.9,1.38,.04,.04,.04,0,30,0,0 , 0, 0); // right eye
    sphere(-.15,.9,1.38,.04,.04,.04,0,30,0,0, 0, 0); // left eye
}
static void deer(double x, double y, double z, double dx, double dy, double dz, double th){
    glPushMatrix();

    glMaterialfv(GL_FRONT,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT,GL_EMISSION,black);

    //adjust placement, rotation, scale
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);

    deerBody(0,0,0);

    deerLeg(.2,-.1,-.6);
    deerLeg(-.2,-.1,-.6);
    deerLeg(.17,-.3,.8);
    deerLeg(-.17,-.3,.8);

    deerHead(0,0,0);
    cylinder(-.07,1,1.3,.04,.8,.04,0,0,20,15,0.545, 0.416, 0.278);//antler base right side
    cylinder(-.19,1.35,1.3,.04,.5,.04,0,0,-5,15,0.545, 0.416, 0.278);
    cylinder(-.19,1.35,1.3,.04,1,.04,0,0,45,15,0.545, 0.416, 0.278);
    cylinder(-.45,1.6,1.3,.04,.5,.04,0,0,-5,15,0.545, 0.416, 0.278);
    cylinder(.07,1,1.3,.04,.8,.04,0,0,-20,15,0.545, 0.416, 0.278);//antler base right side
    cylinder(.19,1.35,1.3,.04,.5,.04,0,0,5,15,0.545, 0.416, 0.278);
    cylinder(.19,1.35,1.3,.04,1,.04,0,0,-45,15,0.545, 0.416, 0.278);
    cylinder(.45,1.6,1.3,.04,.5,.04,0,0,5,15,0.545, 0.416, 0.278);

    glPopMatrix();
}
static void triangle(double x, double y, double z, double dx, double dy, double dz, double thx, double thy, double thz){
    glTranslated(x,y,z);
    glRotated(thx,1,0,0);
    glRotated(thy,0,1,0);
    glRotated(thz,0,0,1);
    glScaled(dx,dy,dz);
    glNormal3d(1,0,0);
    glBegin(GL_TRIANGLES);
    glVertex3d(0,0,1);
    glVertex3d(0,0,-1);
    glVertex3d(0,1,0);
    glEnd();
}
static void makeBranch(int depth, int th){
    if(depth < 3){
        glPushMatrix();
        glTranslated(0,.33,0);
        glRotated(30+(depth*5),0,0,1);
        glRotated(th,0,1,0);
        glScaled(.8,.7,.8);

        for(int th = 0; th < 360; th+=120){
            glPushMatrix();
            glRotated(th,0,1,0);
            makeBranch(depth+1,th);
            glPopMatrix();
        }
        cylinder(0,0,0,.025,1,.025,0,0,0,30,aspentrunk[0],aspentrunk[1],aspentrunk[2]);
        if(depth >= 1){
            for(int th = 0; th < 360; th+=90){
            glPushMatrix();
            glRotated(th,0,1,0);
            glColor3f(pineTreeColors[3][0],pineTreeColors[3][1],pineTreeColors[3][2]);
            triangle(0,.33,0,.3,.3,.3,0,0,0);
            glPopMatrix();
        }
        }
        glPopMatrix();
    }

}
static void aspenTree(double x, double y, double z, double dx, double dy, double dz){
    glPushMatrix();

    glMaterialfv(GL_FRONT,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT,GL_EMISSION,black);

    //adjust placement, rotation, scale
    glTranslated(x,y,z);
    glScaled(dx,dy,dz);

    cylinder(0,0,0,.025,1.5,.025,0,0,0,15,aspentrunk[0],aspentrunk[1],aspentrunk[2]);
    for(int th = 0; th < 360; th+=90){
        glPushMatrix();
        glRotated(th,0,1,0);
        makeBranch(0,th);
        glPopMatrix();
    }

    glPopMatrix();
}
static void PineTree(double x, double y, double z, double dx, double dy, double dz){
    glPushMatrix();

    glMaterialfv(GL_FRONT,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT,GL_EMISSION,black);

    //adjust placement, rotation, scale
    glTranslated(x,y,z);
    glScaled(dx,dy,dz);
    cylinder(0,0,0,.015,.75,.015,0,0,0,30,0.55, 0.27, 0.07);
    double needleLength = .3;
    double height = .2;
    for(int i = 1; i < 7; i+=1){
        height += (.055 - (i*.005));
        needleLength-=.035;
        glColor3f(pineTreeColors[i-1][0],pineTreeColors[i-1][1],pineTreeColors[i-1][2]);
        for(int th = 0; th<360; th+=5){
            glPushMatrix();
            glTranslated(0,height,0);
            glRotated(th,0,1,0);
                glPushMatrix();
                glRotated(130,0,0,1);
                glScaled(.01,needleLength,.01);
                glBegin(GL_TRIANGLES);
                glVertex3d(0,0,1);
                glVertex3d(0,0,-1);
                glVertex3d(0,1,0);
                glEnd();
                glPopMatrix();
            glPopMatrix();
        }    
    }
    glPopMatrix();


    
}
/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display() {
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

    aspenTree(0,0,0,.5,.5,.5);
    PineTree(.5,0,.5,1,1,1);

    /* Draw axes */
    glDisable(GL_LIGHTING);
    glColor3f(1,1,1);
    if (axes) {
        const double len=1;
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
    glWindowPos2i(5,25);
    Print("Polygon Count: %d",polygon_count);

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
        // Toggle axes
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
            E[1] += df;
            C[1] += df;
            break;
        // Move down
        case 'e':
        case 'E':
            E[1] -= df;
            C[1] -= df;
            break;

        /* Lighting Controls */
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
            l_ph += 0.1;
            break;
        // Increase light elevation
        case '[':
            l_ph -= 0.1;
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
    l_th = fmod(t*90,360);
    season = t;

    // Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[]) {
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
    //  TODO: Load DEMs
    //  Pass control to GLUT so it can interact with the user
    ErrCheck("init");
    glutMainLoop();
    return 0;
}