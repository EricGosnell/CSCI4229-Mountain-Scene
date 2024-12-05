#include "CSCIx229.h"

/* Global Variables */
float **data; // Elevation Data
DEM_triangle triangles[524288]; // 2*(4096/8)^2

/*
 * Linearly interpolates between two colors
 */
void interpolateColor(const GLfloat color1[3], const GLfloat color2[3], const float t, GLfloat result[3]) {
    for (int i = 0; i < 3; i++) {
        result[i] = (1.0f - t) * color1[i] + t * color2[i];
    }
}

/*
 * Define the color of each triangle based on slope angle, slope aspect, and elevation. Linear interpolation
 * is used to transition between different zones, such as between tundra and exposed stone.
 *
 * TODO: explain all color transitions and values
 */
static void getColor(const float slope_angle, const float orientation, DEM_triangle *tri) {
    float elevation = (tri->A.y + tri->B.y + tri->C.y) / 3; // Average elevation of triangle
    float t; // Color interpolation value

    // Colors
    const GLfloat forest[] = {0.13f, 0.35f, 0.13f}; // Dark forest green
    const GLfloat tundra[] = {0.53f, 0.55f, 0.46f}; // Tundra greenish grey
    const GLfloat dirtStone[] = {0.6f, 0.5f, 0.4f}; // Exposed dirt and stone
    const GLfloat snow[] = {0.8f, 0.8f, 0.8f}; // Snow

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
        const float darkeningFactor = 1.0f - ((slope_angle - 30) / 60.0f);
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
        tri->rgba[1] = 0.4f;
        tri->rgba[2] = 0.6f;
    }

    // Set alpha value to 1
    tri->rgba[3] = 1;
}

/*
 *  Read 1 meter DEM from the 4 corresponding files
 */
void ReadDEM(char *file1, char *file2) {
    // Initialize data array
    data = malloc(DEM_W * sizeof(float *));
    for (int i = 0; i < DEM_W; i++) {
        data[i] = malloc(DEM_W * sizeof(float));
    }

    // Read in data from files
    FILE *f1 = fopen(file1, "r");
    if (!f1) Fatal("Cannot open file %s\n", file1);
    FILE *f2 = fopen(file2, "r");
    if (!f2) Fatal("Cannot open file %s\n", file2);

    // Write elevation data for each point to the data array
    for (int i = 0; i < DEM_W; i++) {
        for (int j = 0; j < DEM_W; j++) {
            // Read first half of data from file 1
            if (i < DEM_W / 2) {
                if (fscanf(f1, "%f", &data[i][j]) != 1) Fatal("Error reading %s\n", file1);
            }
            // Read second half of data from file 2
            else {
                if (fscanf(f2, "%f", &data[i][j]) != 1) Fatal("Error reading %s\n", file2);
            }
        }
    }

    fclose(f1);
    fclose(f2);

    /* Populate triangles array */ //TODO: different sized triangles based on distance to camera
    const int inc = 8; // Factor by which to decrease resolution
    int t = DEM_W / inc - 1; // Width of triangles

    // Add elevation values from data
    for (int i = 0; i < t; i++) {
        for (int j = 0; j < t; j++) {
            const int n = (i * t + j) * 2;
            const float x = j * inc;
            const float z = -i * inc;
            // Bottom left triangle
            triangles[n].A.x = x;
            triangles[n].A.y = data[i * inc][j * inc];
            triangles[n].A.z = z;

            triangles[n].B.x = x;
            triangles[n].B.y = data[(i + 1) * inc][j * inc];
            triangles[n].B.z = z - inc;

            triangles[n].C.x = x + inc;
            triangles[n].C.y = data[(i + 1) * inc][(j + 1) * inc];
            triangles[n].C.z = z - inc;

            // Top right triangle
            triangles[n + 1].A.x = x;
            triangles[n + 1].A.y = data[i * inc][j * inc];
            triangles[n + 1].A.z = z;

            triangles[n + 1].B.x = x + inc;
            triangles[n + 1].B.y = data[(i + 1) * inc][(j + 1) * inc];
            triangles[n + 1].B.z = z - inc;

            triangles[n + 1].C.x = x + inc;
            triangles[n + 1].C.y = data[i * inc][(j + 1) * inc];
            triangles[n + 1].C.z = z;

            for (int k = 0; k < 2; k++) {
                // Calculate normal vector
                const float Nx = (triangles[n + k].A.y - triangles[n + k].B.y) * (triangles[n + k].C.z - triangles[n + k].A.z)
                                 -
                                 (triangles[n + k].C.y - triangles[n + k].A.y) * (
                                     triangles[n + k].A.z - triangles[n + k].B.z);
                float Ny = (triangles[n + k].A.z - triangles[n + k].B.z) * (triangles[n + k].C.x - triangles[n + k].A.x)
                           -
                           (triangles[n + k].C.z - triangles[n + k].A.z) * (
                               triangles[n + k].A.x - triangles[n + k].B.x);
                const float Nz = (triangles[n + k].A.x - triangles[n + k].B.x) * (triangles[n + k].C.y - triangles[n + k].A.y)
                                 -
                                 (triangles[n + k].C.x - triangles[n + k].A.x) * (
                                     triangles[n + k].A.y - triangles[n + k].B.y);
                // Add normal vector to triangle
                triangles[n + k].normal[0] = Nx;
                triangles[n + k].normal[1] = Ny;
                triangles[n + k].normal[2] = Nz;
                // Normalize the normal vector vertical component
                const float length = sqrt(Nx * Nx + Ny * Ny + Nz * Nz);
                Ny /= length;
                // Calculate slope angle
                const float slope_angle = acos(Ny) * (180 / M_PI);
                // Calculate orientation
                float orientation = atan2(-Nz, -Nx) * (180 / M_PI);
                if (orientation < 0) orientation += 360;
                // Set color based on slope angle, orientation, and elevation
                getColor(slope_angle, orientation, &triangles[n + k]);
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
