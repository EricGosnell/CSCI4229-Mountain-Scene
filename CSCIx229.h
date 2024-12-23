#ifndef CSCIx229
#define CSCIx229

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>

// GLEW _MUST_ be included first
#ifdef USEGLEW
#include <GL/glew.h>
#endif
//  Get all GL prototypes
#define GL_GLEXT_PROTOTYPES
//  Select SDL, SDL2, GLFW or GLUT
#if defined(SDL2)
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#elif defined(SDL)
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#elif defined(GLFW)
#include <GLFW/glfw3.h>
#elif defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
//  Make sure GLU and GL are included
#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
// Tell Xcode IDE to not gripe about OpenGL deprecation
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif
//  Default resolution
//  For Retina displays compile with -DRES=2
#ifndef RES
#define RES 1
#endif

//  cos and sin in degrees
#define Cos(th) cos(3.14159265/180*(th))
#define Sin(th) sin(3.14159265/180*(th))

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
void Print(const char* format , ...) __attribute__ ((format(printf,1,2)));
void Fatal(const char* format , ...) __attribute__ ((format(printf,1,2))) __attribute__ ((noreturn));
#else
void Print(const char* format , ...);
void Fatal(const char* format , ...);
#endif
unsigned int LoadTexBMP(const char* file);
void Project(double fov,double asp,double dim);
void ErrCheck(const char* where);
int  LoadOBJ(const char* file);
void ReadDEM(const char *fileName);
void DrawDEM();
void sphere(double x,double y,double z,double rx, double ry, double rz, double thx, double thy, double thz, float r, float g, float b);
void halfSphere(double x,double y,double z,double rx, double ry, double rz, double thx, double thy, double thz, float r, float g, float b);
void cylinder(double x,double y,double z,double dx, double dy, double dz, double thx, double thy, double thz, int inc, float r, float g, float b);
void triangle(double x, double y, double z, double dx, double dy, double dz, double thx, double thy, double thz);
void owl(double x, double y, double z, double dx, double dy, double dz, double th);
void rabbit(double x, double y, double z, double dx, double dy, double dz, double th);
void deer(double x, double y, double z, double dx, double dy, double dz, double th);
void aspenTree(double x, double y, double z, double dx, double dy, double dz);
void PineTree(double x, double y, double z, double dx, double dy, double dz);
void blackBear(double x, double y, double z, double dx, double dy, double dz, double th);
void standingBlackBear(double x, double y, double z, double dx, double dy, double dz, double th);
void cone(double x, double y, double z, double dx, double dy, double dz, double thX, double thY, double thZ);
void Sky(double D, double th);
void forest();
void releaseTheAnimals();
int frustumCulling(double x, double y, double z);

#define DEM_W 2048 // Width of the DEM
#define DEM_R 1 // Factor by which to reduce resolution

typedef struct {
    GLfloat x, y, z;
    GLfloat rgb[3];
    GLfloat normal[3];
    float slope_angle;
    float slope_aspect;
    float avg_elevation;
} vtx; // Triangle used in drawing the terrain

extern vtx vertices[4194304];
extern unsigned int indices[25165824];
extern int polygon_count;
extern double E[3];
extern double C[3];
extern GLuint vbo, ebo;
extern int season;
extern int sky[2];


#ifdef __cplusplus
}
#endif

#endif
