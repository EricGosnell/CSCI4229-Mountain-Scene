#include "CSCIx229.h"

/* Global Variables */
vtx vertices[65536]; // Vertex list
unsigned int indices[393216]; // 3 indices per triangle

/*
 * Define the color of each triangle based on slope angle, slope aspect, and elevation. Linear interpolation
 * is used to transition between different zones, such as between tundra and exposed stone.
 *
 * TODO: explain all color transitions and values and update comments
 */

static void getColor(const float slope_angle, const float slope_aspect, const int index) {
    float t; // Color interpolation value
    // Calculate average elevation of the triangle starting at index
    const float elevation = (vertices[indices[index]].y + vertices[indices[index+1]].y + vertices[indices[index+2]].y) / 3.0f;

    // Colors
    const GLubyte forest[] = {33, 90, 33}; // Dark forest green
    const GLubyte tundra[] = {136, 141, 118}; // Tundra greenish grey
    const GLubyte dirtStone[] = {154, 128, 102}; // Exposed dirt and stone
    const GLfloat snow[] = {205, 205, 205}; // Snow

    if (elevation <= 3000.0f) {
        // Below 3000, use dark forest green
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = forest[j];
            }
        }
    } else if (elevation >= 3700.0f) {
        // Above 4000, use dirt/stone tri->rgba
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = dirtStone[j];
            }
        }
    } else if (elevation < 3300.0f) {
        // Smooth transition between dark forest green and tundra green/grey (3000 to 3500)
        t = (elevation - 3000.0f) / 300.0f;
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = (1-t)*forest[j] + t*tundra[j];
            }
        }
    } else {
        // Smooth transition between tundra green/grey and dirt/stone (3500 to 4000)
        t = (elevation - 3300.0f) / 400.0f;
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = (1-t)*tundra[j] + t*dirtStone[j];
            }
        }
    }

    // Darken steeper slopes
    if (slope_angle > 30) {
        const float darkeningFactor = 1.0f - ((slope_angle - 30) / 60.0f);
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] *= darkeningFactor;
            }
        }
    }

    // Snow
    if (slope_angle < 55 && (slope_aspect > 270 || slope_aspect < 90 || elevation > 3600)) {
        if (elevation > 3350) {
            // Smooth transition between dirt/stone and snow (3700 to 3800)
            t = (elevation - 3350.0f) / 100.0f;
            for (int i = index; i < index+3; i++) {
                for (int j = 0; j < 3; j++) {
                    vertices[indices[i]].rgb[j] = (1-t)*dirtStone[j] + t*snow[j];
                }
            }
        }
    }

    // Lakes
    if (slope_angle == 0) {
        const GLubyte lake[] = {0, 102, 154};
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = lake[j];
            }
        }
    }
}


/*
 * Read in elevation data from the .dem files, and populate the vertices array.
 */
static void readFile(const char *fileName, int start, int end) {
    FILE *file = fopen(fileName, "r");
    if (!file) Fatal("Cannot open file %s\n", fileName);
    float fy;
    for (int i = start; i < end; i++) {
        if (fscanf(file, "%f", &fy) != 1) Fatal("Error reading file %s\n", fileName);
        if ((i/DEM_W)%DEM_R == 0 && (i%DEM_W) % DEM_R == 0) {
            int index = ((i/DEM_W)/DEM_R) * (DEM_W/DEM_R) + (i%DEM_W)/DEM_R;
            if (index < (DEM_W/DEM_R)*(DEM_W/DEM_R)) {
                vertices[index].x = i/DEM_W;
                vertices[index].y = fy;
                vertices[index].z = i%DEM_W;
            }
        }
    }
    fclose(file);
}

/*
 *  Read 1 meter DEM from the 2 corresponding files and calculate the color and normals.
 */
void ReadDEM(const char *file1, const char *file2) {
    /* Read in data from files */
    readFile(file1,0,DEM_W*DEM_W/2);
    readFile(file2,DEM_W*DEM_W/2,DEM_W*DEM_W);

    /* Populate indices array */
    int n = 0;
    for (int i = 0; i < DEM_W/DEM_R - 1; i++) {
        for (int j = 0; j < DEM_W/DEM_R - 1; j++) {
            const unsigned int cell = i * DEM_W/DEM_R + j;
            // Bottom Left Triangle
            indices[n++] = cell;
            indices[n++] = cell+DEM_W/DEM_R;
            indices[n++] = cell+DEM_W/DEM_R + 1;
            // Top Right Triangle
            indices[n++] = cell;
            indices[n++] = cell+DEM_W/DEM_R + 1;
            indices[n++] = cell+1;
        }
    }

    /* Calculate color and normals for each triangle */
    for (int i = 0; i < sizeof(indices) / sizeof(indices[0]); i += 3) {
        // Calculate normal vector
        float N[3];
        N[0] = (vertices[indices[i]].y - vertices[indices[i+1]].y) * (vertices[indices[i+2]].z - vertices[indices[i]].z)
            - (vertices[indices[i+2]].y - vertices[indices[i]].y) * (vertices[indices[i]].z - vertices[indices[i+1]].z);
        N[1] = (vertices[indices[i]].z - vertices[indices[i+1]].z) * (vertices[indices[i+2]].x - vertices[indices[i]].x)
            - (vertices[indices[i+2]].z - vertices[indices[i]].z) * (vertices[indices[i]].x - vertices[indices[i+1]].x);
        N[2] = (vertices[indices[i]].x - vertices[indices[i+1]].x) * (vertices[indices[i+2]].y - vertices[indices[i]].y)
            - (vertices[indices[i+2]].x - vertices[indices[i]].x) * (vertices[indices[i]].y - vertices[indices[i+1]].y);
        // Add normal vector to each vertex of the triangle
        for (int j = i; j < i+3; j++) {
            for (int k = 0; k < 3; k++) {
                vertices[indices[j]].normal[k] = N[k];
            }
        }
        // Normalize the normal vector vertical component
        const float length = sqrtf(N[0] * N[0] + N[1] * N[1] + N[2] * N[2]);
        N[1] /= length;
        // Calculate slope angle
        const float slope_angle = acosf(N[1]) * (180 / M_PI);
        // Calculate slope aspect
        float slope_aspect = atan2f(-N[2], -N[0]) * (180 / M_PI);
        if (slope_aspect < 0) slope_aspect += 360;
        // Set color based on slope angle, slope aspect, and elevation
        getColor(slope_angle, slope_aspect, i);
    }
}

/*
 *  Draw 1 meter DEM Wireframe
 */
/*
void DrawDEM(double dx, double dy, double dz, double scale) {
    // Save transformation
    glPushMatrix();
    // Translate and Scale
    glScaled(scale, scale, scale);
    // glTranslated(dx,dy,dz);
    // Enable Face Culling
    glEnable(GL_CULL_FACE);

    // Calculate dot product between triangle and camera
    double F[3];
    double T[3];
    for (int i = 0; i < 3; i++) {
        F[i] = (C[i] - E[i]);
    }
    const double f_len = sqrt(F[0] * F[0] + F[1] * F[1] + F[2] * F[2]);
    F[0] /= f_len;
    F[1] /= f_len;
    F[2] /= f_len;

    // Set Color Properties
    const float black[] = {0, 0, 0, 1};
    const float white[] = {1, 1, 1, 1};
    glMaterialfv(GL_FRONT,GL_EMISSION, black);
    glMaterialfv(GL_FRONT,GL_SPECULAR, white);
    glMaterialf(GL_FRONT,GL_SHININESS, 1);
    // Draw DEM Triangles
    int nt = sizeof(triangles) / sizeof(DEM_triangle);
    for (int i = 0; i < nt; i++) {
        // Only draw triangle if the dot product between it and the forward vector > 0
        T[0] = triangles[i].A.x - E[0];
        T[1] = triangles[i].A.y - E[1];
        T[2] = triangles[i].A.z - E[2];
        if (F[0] * T[0] + F[1] * T[1] + F[2] * T[2] < 0) continue;
        // Face culling based on dot product of the forward vector and the normal vector
        // float nl = sqrt(triangles[i].normal[0]*triangles[i].normal[0]+triangles[i].normal[1]*triangles[i].normal[1]+triangles[i].normal[2]*triangles[i].normal[2]);
        if (F[0] * triangles[i].normal[0] + F[1] * triangles[i].normal[1] + F[2] * triangles[i].normal[2] > 90) continue
                ;
        // Set Normal and Color
        glNormal3f(triangles[i].normal[0], triangles[i].normal[1], triangles[i].normal[2]);
        glColor4f(triangles[i].rgba[0], triangles[i].rgba[1], triangles[i].rgba[2], triangles[i].rgba[3]);
        // glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,triangles[i].rgba); // TODO: FPS drops substantially
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
*/
