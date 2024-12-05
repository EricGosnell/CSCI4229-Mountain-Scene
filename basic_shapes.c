#include "CSCIx229.h"

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
void sphere(double x,double y,double z,double rx, double ry, double rz, double thx, double thy, double thz, float r, float g, float b) {
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
void halfSphere(double x,double y,double z,double rx, double ry, double rz, double thx, double thy, double thz, float r, float g, float b) {
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
void cylinder(double x,double y,double z,double dx, double dy, double dz, double thx, double thy, double thz, int inc, float r, float g, float b) {
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

void triangle(double x, double y, double z, double dx, double dy, double dz, double thx, double thy, double thz) {
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
