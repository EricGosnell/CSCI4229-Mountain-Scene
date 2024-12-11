#include "CSCIx229.h"

/*
* Generates sky box. Function taken from Vlakkies' Ex 8, bmp is hand drawn by me
*/
void Sky(double D, double th) {
    //  Textured white box dimension (-D,+D)
    glPushMatrix();
    glScaled(D,D,D);
    glRotated(th,0,1,0);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1,1,1);

    //  Sides
    glBindTexture(GL_TEXTURE_2D,sky[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.00,0); glVertex3f(-1,-1,-1);
    glTexCoord2f(0.25,0); glVertex3f(+1,-1,-1);
    glTexCoord2f(0.25,1); glVertex3f(+1,+1,-1);
    glTexCoord2f(0.00,1); glVertex3f(-1,+1,-1);

    glTexCoord2f(0.25,0); glVertex3f(+1,-1,-1);
    glTexCoord2f(0.50,0); glVertex3f(+1,-1,+1);
    glTexCoord2f(0.50,1); glVertex3f(+1,+1,+1);
    glTexCoord2f(0.25,1); glVertex3f(+1,+1,-1);

    glTexCoord2f(0.50,0); glVertex3f(+1,-1,+1);
    glTexCoord2f(0.75,0); glVertex3f(-1,-1,+1);
    glTexCoord2f(0.75,1); glVertex3f(-1,+1,+1);
    glTexCoord2f(0.50,1); glVertex3f(+1,+1,+1);

    glTexCoord2f(0.75,0); glVertex3f(-1,-1,+1);
    glTexCoord2f(1.00,0); glVertex3f(-1,-1,-1);
    glTexCoord2f(1.00,1); glVertex3f(-1,+1,-1);
    glTexCoord2f(0.75,1); glVertex3f(-1,+1,+1);
    glEnd();

    //  Top and bottom
    glBindTexture(GL_TEXTURE_2D,sky[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0,0); glVertex3f(+1,+1,-1);
    glTexCoord2f(0.5,0); glVertex3f(+1,+1,+1);
    glTexCoord2f(0.5,1); glVertex3f(-1,+1,+1);
    glTexCoord2f(0.0,1); glVertex3f(-1,+1,-1);

    glTexCoord2f(1.0,1); glVertex3f(-1,-1,+1);
    glTexCoord2f(0.5,1); glVertex3f(+1,-1,+1);
    glTexCoord2f(0.5,0); glVertex3f(+1,-1,-1);
    glTexCoord2f(1.0,0); glVertex3f(-1,-1,-1);
    glEnd();

    //  Undo
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}
/*
* Places pine trees and aspens in scene
*/
void forest() {
    //arrays of tree coords
    float pineCoords[27][3] = {
        // Flat Area
        {-187,-30,688},
        {-160,-30,1000},
        {-40,-30,900},
        {20,-30,350},
        {1,-30,1100},
        {-163,-30, 1090},
        {-40,-30, 990},
        {-150,-40, 1150},
        {50, -40, 1000},
        {70,-40, 200},
        {-120,-40, 850},
        {-91,-40,584},
        {-118,-40,745},
        {60, -5, 638},
        {4, -5, 735},
        {-354, 20, 772},
        {-375, 20, 954},
        {-285, 20, 952},
        {-402, 20, 900},
        {-367, 20, 157},
        {-379, 20, 313},
        {-247, 10, 387},
        {-417, 20, 460},

        // Area next to lake
        {263,0,500},
        {200,0,300},
        {150,0,250},
        {180,0,430},
    };

    float aspenCoords[8][3] = {
        {-244,30,-250},
        {-321,30,-78.1f},
        {266,20,-154},
        {157,20,-264},
        {-4.5,30,-332},
        {137,30,-156},
        {183,50,602},
        {283,60,788},
    };

    //loops through coords and places trees
    for (int i=0; i<sizeof(pineCoords)/sizeof(pineCoords[0]); i++) {
        if (frustumCulling(pineCoords[i][0],pineCoords[i][1],pineCoords[i][2])) {
            PineTree(pineCoords[i][0],pineCoords[i][1],pineCoords[i][2],200,300,200);
        }
    }
    //loops through coords and places trees
    for (int i=0; i<sizeof(aspenCoords)/sizeof(aspenCoords[0]); i++) {
        if (frustumCulling(aspenCoords[i][0],aspenCoords[i][1],aspenCoords[i][2])) {
            aspenTree(aspenCoords[i][0],aspenCoords[i][1],aspenCoords[i][2],100,100,100);
        }
    }
}

/*
* Generates animals
*/
void releaseTheAnimals() {
    blackBear(-315,50,613,20,20,20,50);
    blackBear(-217,25,642,15,15,15,80);
    blackBear(-6.9,380,-1496,15,15,15,270);
    standingBlackBear(-361,60,-239,20,20,20,330);
    deer(178,30,67,18,18,18,270);
    deer(195,30,-22,18,18,18,300);
    deer(890,140,-110,18,18,18,260);
    deer(872,135,-233,18,18,18,300);
    deer(1025,150,-201,18,18,18,290);
}
