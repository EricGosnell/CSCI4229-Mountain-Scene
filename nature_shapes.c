#include "CSCIx229.h"

const float black[] = {0,0,0,1};
const float white[] = {1,1,1,1};
const float pineTreeColors[][3] = {{0.1, 0.2, 0.03},{0.2, 0.4, 0.05},{0.3, 0.5, 0.1},{0.4, 0.6, 0.2},{0.5, 0.7, 0.3},{0.6, 0.8, 0.4}};
const float aspentrunk[3] = {0.9f, 0.9f, 0.9f};

void owl(double x, double y, double z, double dx, double dy, double dz, double th) {
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

void rabbit(double x, double y, double z, double dx, double dy, double dz, double th){
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

void deer(double x, double y, double z, double dx, double dy, double dz, double th){
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
static void bearHead(double x, double y, double z){
    sphere(x, y, z,.7,.65,.65,0,0,0,0.294, 0.235, 0.192); //head
    sphere(x +.4, y-.2, z,.6,.3,.3,0,0,-15, 0.627, 0.466, 0.314); //muzzle
    halfSphere(x + .9, y -.2 , z,.1, .07,.1,0,-90,0,0,0,0); //nose 
    halfSphere(x + .63, y + .05,  z- .18,.05,.05,.05,0,-90,0,0,0,0); //eye
    halfSphere(x + .63, y + .05,  z+ .18,.05,.05,.05,0,-90,0,0,0,0);//eye

    halfSphere(x + .3, y + .5,  z+ .4,.15,.25,.2,0,90,0,0.294, 0.235, 0.192); //ear
    halfSphere(x + .3, y + .5,  z- .4,.15,.25,.2,0,90,0,0.294, 0.235, 0.192);//ear
}
void blackBear(double x, double y, double z, double dx, double dy, double dz, double th){
    glPushMatrix();

    glMaterialfv(GL_FRONT,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT,GL_EMISSION,black);

    //adjust placement, rotation, scale
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);

    sphere(0,0,0,1.5,1,1,0,0,0,0.294, 0.235, 0.192);
    sphere(-.8,.2,0,.9,.9,.9,0,0,0,0.294, 0.235, 0.192);
    sphere(.8,.3,0,1,.9,1,0,0,0,0.294, 0.235, 0.192);

    sphere(-1.2,-.5,.7,.5,1.3,.4,0,0,-5,0.294, 0.235, 0.192);
    sphere(-1.2,-.5,-.7,.5,1.3,.4,0,0,-5,0.294, 0.235, 0.192);
    sphere(1,-.7,-.6,.4,1.1,.4,0,0,2,0.294, 0.235, 0.192);
    sphere(1,-.7,.6,.4,1.1,.4,0,0,2,0.294, 0.235, 0.192);

    sphere(1.25,-1.6,.6,.3,.2,.3,0,0,2,0.294, 0.235, 0.192);
    sphere(1.25,-1.6,-.6,.3,.2,.3,0,0,2,0.294, 0.235, 0.192);
    sphere(-1.2,-1.6,.7,.4,.2,.3,0,0,2,0.294, 0.235, 0.192);
    sphere(-1.2,-1.6,-.7,.4,.2,.3,0,0,2,0.294, 0.235, 0.192);

    bearHead(1.8,.5,0);

    glPopMatrix();
}
void standingBlackBear(double x, double y, double z, double dx, double dy, double dz, double th){
    glPushMatrix();

    glMaterialfv(GL_FRONT,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT,GL_EMISSION,black);

    //adjust placement, rotation, scale
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);

    bearHead(.1,1.9,0);

    sphere(.1,.4,.7,.5,1,.4,-15,0,15,0.294, 0.235, 0.192);
    sphere(.1,.4,-.7,.5,1,.4,15,0,15,0.294, 0.235, 0.192);

    //sphere(0,1.4,0,.4,.5,.5,0,0,0,0.294, 0.235, 0.192);
    sphere(0,.2,0,.7,1.5,1,0,0,0,0.294, 0.235, 0.192);

    sphere(0,-.7,.6,.5,1.3,.4,0,0,-5,0.294, 0.235, 0.192);
    sphere(0,-.7,-.6,.5,1.3,.4,0,0,-5,0.294, 0.235, 0.192);
    sphere(0,-1.7,.5,.3,.2,.3,0,0,2,0.294, 0.235, 0.192);
    sphere(0,-1.7,-.5,.3,.2,.3,0,0,2,0.294, 0.235, 0.192);

    glPopMatrix();
}

static void makeBranch(int depth, int th){
    if(depth < 3){
        glPushMatrix();
        glTranslated(0,.33,0);
        glRotated(30-(depth*5),0,0,1);
        glRotated(th,0,1,0);
        glScaled(.5,.7,.5);

        for(int angle = 0; angle < 360; angle +=120){
            glPushMatrix();
            glRotated(angle,0,1,0);
            makeBranch(depth+1,angle);
            glPopMatrix();
        }
        cylinder(0,0,0,.025,1,.025,0,0,0,30,aspentrunk[0],aspentrunk[1],aspentrunk[2]);
        if(depth >= 1 && season != 4){
            for(int th = 0; th < 360; th+=90){
            if(season == 1 || season == 2){
            glColor3f(pineTreeColors[3][0],pineTreeColors[3][1],pineTreeColors[3][2]);
            }
            else{
            glColor3f(0.9f, 0.7f, 0.0f);
            }
            glPushMatrix();
            glTranslated(0,.33*(depth),0);
            glRotated(th,0,1,0);
            glScaled(.3,.3,.3);
            triangle(0,0,0,1,1,1,0,0,45);
            triangle(0,0,0,1,1,1,0,0,-45);
            triangle(0,0,0,1,1,1,0,90,45);
            triangle(0,0,0,1,1,1,0,-90,45);

            triangle(0,-.5,0,.7,1,.7,0,0+45,45+45);
            triangle(0,-.5,0,.7,1,.7,0,0+45,-45+45);
            triangle(0,-.5,0,.7,.7,.7,0,90+45,45+45);
            triangle(0,-.5,0,.7,1,.7,0,-90+45,45+45);


            glPopMatrix();
        }
        }
        glPopMatrix();
        glPushMatrix();
        glTranslated(0,.5,0);
        glRotated(15-(depth*5),0,0,1);
        glRotated(th,0,1,0);
        glScaled(.5,.7,.5);

        for(int angle = 0;angle < 360; angle+=120){
            glPushMatrix();
            glRotated(angle,0,1,0);
            makeBranch(depth+1,th);
            glPopMatrix();
        }
        cylinder(0,0,0,.025,1,.025,0,0,0,30,aspentrunk[0],aspentrunk[1],aspentrunk[2]);
        glPopMatrix();
    }

}

void aspenTree(double x, double y, double z, double dx, double dy, double dz){
    glPushMatrix();

    glMaterialfv(GL_FRONT,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT,GL_EMISSION,black);

    //adjust placement, rotation, scale
    glTranslated(x,y,z);
    glScaled(dx,dy,dz);

    cylinder(0,-.2,0,.015,1.5,.015,0,0,0,15,aspentrunk[0],aspentrunk[1],aspentrunk[2]);
    for(int th = 0; th < 360; th+=90){
        glPushMatrix();
        glRotated(th,0,1,0);
        makeBranch(0,th);
        glPopMatrix();
    }

    glPopMatrix();
}

void PineTree(double x, double y, double z, double dx, double dy, double dz){
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
