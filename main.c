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
#define DEM_W 4096

/* Global Variables */
double dt = 0; // Time step
int axes = 0; // Toggle axes
typedef struct {float x,y,z;} vtx;
int polygon_count = 0; // Total number of polygons
int frame_count = 0; // Frame count
int t0 = 0; // Initial time for measuring FPS
float fps = -1; // Starting value for FPS

/* Digital Elevation Model */
float** data; // Elevation Data
typedef struct {
    vtx A, B, C;
    float rgba[4];
    float normal[3];
} DEM_triangle; // Triangle used in drawing the terrain
DEM_triangle triangles[524288]; // 2*(4096/8)^2

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
   //  Undo transformations
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
   //  Undo transformations
   glPopMatrix();
}

/*
 *  Draw a cylinder
 *     at (x,y,z)
 *     radius (r)
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
        if(depth >= 1 && season != 4){
            for(int th = 0; th < 360; th+=90){
            glPushMatrix();
            glRotated(th,0,1,0);
            if(season == 1 || season == 2){
            glColor3f(pineTreeColors[3][0],pineTreeColors[3][1],pineTreeColors[3][2]);
            }
            else{
            glColor3f(0.9f, 0.7f, 0.0f);
            }
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
        if(season == 4){
            glColor3f(1,1,1);
        }
        else{
            glColor3f(pineTreeColors[i-1][0],pineTreeColors[i-1][1],pineTreeColors[i-1][2]);
        }
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

    if (elevation <= 3000.0f) {
        // Below 3000, use dark forest green
        for (int i = 0; i < 3; i++) tri->rgba[i] = forest[i];
    } else if (elevation >= 3700.0f) {
        // Above 4000, use dirt/stone tri->rgba
        for (int i = 0; i < 3; i++) tri->rgba[i] = dirtStone[i];
    } else if (elevation < 3300.0f) {
        // Smooth transition between dark forest green and tundra green/grey (3000 to 3500)
        t = (elevation - 3000.0f) / 300.0f;
        interpolateColor(forest, tundra, t, tri->rgba);
    } else {
        // Smooth transition between tundra green/grey and dirt/stone (3500 to 4000)
        t = (elevation - 3300.0f) / 400.0f;
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
    if (slope_angle < 55 && (orientation > 270 || orientation < 90 || elevation > 3600)) {
        if (elevation > 3350) {
			// Smooth transition between dirt/stone and snow (3700 to 3800)
        	t = (elevation - 3350.0f) / 100.0f;
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
 *  Read 1 meter DEM from the 4 corresponding files
 */
void ReadDEM(char* file1, char* file2) {
   // Initialize data array
   data = malloc(DEM_W * sizeof(float*));
   for (int i = 0; i < DEM_W; i++) {
       data[i] = malloc(DEM_W * sizeof(float));
   }

   // Read in data from files
   FILE* f1 = fopen(file1,"r");
   if (!f1) Fatal("Cannot open file %s\n",file1);
   FILE* f2 = fopen(file2,"r");
   if (!f2) Fatal("Cannot open file %s\n",file2);

   // Write elevation data for each point to the data array
   for (int i=0; i<DEM_W; i++) {
       for (int j=0; j<DEM_W; j++) {
           // Read first half of data from file 1
           if (i < DEM_W/2) {
               if (fscanf(f1,"%f",&data[i][j]) != 1) Fatal("Error reading %s\n", file1);
           }
           // Read second half of data from file 2
           else {
               if (fscanf(f2,"%f",&data[i][j]) != 1) Fatal("Error reading %s\n", file2);
           }
	   }
   }

   fclose(f1);
   fclose(f2);

   /* Populate triangles array */ //TODO: different sized triangles based on distance to camera
   const int inc = 8; // Factor by which to decrease resolution
   int t = DEM_W/inc-1; // Width of triangles

   // Add elevation values from data
   for (int i=0; i<t; i++) {
       for (int j=0; j<t; j++) {
           int n = (i*t+j)*2;
           float x = j*inc;
           float z = -i*inc;
           // Bottom left triangle
           triangles[n].A.x = x;
           triangles[n].A.y = data[i*inc][j*inc];
           triangles[n].A.z = z;

           triangles[n].B.x = x;
           triangles[n].B.y = data[(i+1)*inc][j*inc];
           triangles[n].B.z = z-inc;

           triangles[n].C.x = x+inc;
           triangles[n].C.y = data[(i+1)*inc][(j+1)*inc];
           triangles[n].C.z = z-inc;

           // Top right triangle
           triangles[n+1].A.x = x;
           triangles[n+1].A.y = data[i*inc][j*inc];
           triangles[n+1].A.z = z;

           triangles[n+1].B.x = x+inc;
           triangles[n+1].B.y = data[(i+1)*inc][(j+1)*inc];
           triangles[n+1].B.z = z-inc;

           triangles[n+1].C.x = x+inc;
           triangles[n+1].C.y = data[i*inc][(j+1)*inc];
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
    // Free data array
    for (int i = 0; i < DEM_W; i++) {
        free(data[i]);
    }
    free(data);
}

/*
 *  Draw 1 meter DEM Wireframe
 */
void drawDEM(double dx, double dy, double dz, double scale) {
    // Save transformation
    glPushMatrix();
    // Translate and Scale
    glScaled(scale,scale,scale);
    // glTranslated(dx,dy,dz);
    // Enable Face Culling
    glEnable(GL_CULL_FACE);

    // Calculate dot product between triangle and camera
    float F[3];
    float T[3];
    for (int i=0; i<3; i++) {
        F[i] = (C[i]-E[i]);
    }
    float f_len = sqrt(F[0]*F[0]+F[1]*F[1]+F[2]*F[2]);
    F[0] /= f_len;
    F[1] /= f_len;
    F[2] /= f_len;

    // Set Color Properties
    float black[]  = {0.0,0.0,0.0,1.0};
    float white[]  = {1.0,1.0,1.0,1.0};
    glMaterialfv(GL_FRONT,GL_EMISSION,black);
    glMaterialfv(GL_FRONT,GL_SPECULAR,white);
    glMaterialf(GL_FRONT,GL_SHININESS,shiny);
    // Draw DEM Triangles
    int nt = sizeof(triangles)/sizeof(DEM_triangle);
    for (int i=0; i<nt; i++) {
        // Only draw triangle if the dot product between it and the forward vector > 0
        T[0] = triangles[i].A.x - E[0];
        T[1] = triangles[i].A.y - E[1];
        T[2] = triangles[i].A.z - E[2];
        if (F[0]*T[0] + F[1]*T[1] + F[2]*T[2] < 0) continue;
        // Face culling based on dot product of the forward vector and the normal vector
        // float nl = sqrt(triangles[i].normal[0]*triangles[i].normal[0]+triangles[i].normal[1]*triangles[i].normal[1]+triangles[i].normal[2]*triangles[i].normal[2]);
        if (F[0]*triangles[i].normal[0]+F[1]*triangles[i].normal[1]+F[2]*triangles[i].normal[2] > 90) continue;
        // Set Normal and Color
        glNormal3f(triangles[i].normal[0],triangles[i].normal[1],triangles[i].normal[2]);
        glColor4f(triangles[i].rgba[0],triangles[i].rgba[1],triangles[i].rgba[2],triangles[i].rgba[3]);
//        glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,triangles[i].rgba); // TODO: FPS drops substantially
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
    drawDEM(0,-3500,0,1);
    glColor3f(1,1,1);
    glWindowPos2i(5,100);
    Print("Cx: %.2f, Cy: %.2f, Cz: %.2f",C[0],C[1],C[2]);
    glWindowPos2i(5,80);
    Print("Ex: %.2f, Ey: %.2f, Ez: %.2f",E[0],E[1],E[2]);

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
