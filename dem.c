#include "CSCIx229.h"

/* Global Variables */
vtx vertices[4194304]; // Vertex list
unsigned int indices[25165824]; // 3 indices per triangle


/*
 * Define the color of each triangle based on slope angle, slope aspect, and elevation. Linear interpolation
 * is used to transition between different zones, such as between tundra and exposed stone.
 *
 * TODO: explain all color transitions and values and update comments
 */

static void getColor(const float slope_angle, const float slope_aspect, const int index) {
    float t; // Color interpolation value
    // Calculate average elevation of the triangle starting at index
    const float elevation = ((vertices[indices[index]].y + vertices[indices[index+1]].y + vertices[indices[index+2]].y) / 3.0f) + 3100;

    // Colors
    const GLfloat forest[] = {0.13f, 0.35f, 0.13f}; // Dark forest green
    const GLfloat tundra[] = {0.53f, 0.55f, 0.46f}; // Tundra greenish grey
    const GLfloat dirtStone[] = {0.6f, 0.5f, 0.4f}; // Exposed dirt and stone
    const GLfloat snow[] = {0.8f, 0.8f, 0.8f}; // Snow

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
        const GLfloat lake[] = {0,0.4,0.6};
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
                vertices[index].x = i/DEM_W - DEM_W/2;
                vertices[index].y = fy - 3100;
                vertices[index].z = i%DEM_W - DEM_W/2;
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
            const unsigned int cell = i * (DEM_W/DEM_R) + j;
            // Top Right Triangle
            indices[n++] = cell;
            indices[n++] = cell+(DEM_W/DEM_R);
            indices[n++] = cell+(DEM_W/DEM_R) + 1;

            // Bottom Left Triangle
            indices[n++] = cell;
            indices[n++] = cell+(DEM_W/DEM_R) + 1;
            indices[n++] = cell+1;
        }
    }

    /* Calculate color and normals for each triangle */
    for (int i = 0; i < sizeof(indices) / sizeof(unsigned int); i += 3) {
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
        // Normalize the normal vector
        const float length = sqrtf(N[0] * N[0] + N[1] * N[1] + N[2] * N[2]);
        N[0] /= length;
        N[1] /= length;
        N[2] /= length;
        // Calculate slope angle
        const float slope_angle = acosf(N[1]) * (180 / M_PI);
        // Calculate slope aspect
        float slope_aspect = atan2f(-N[2], -N[0]) * (180 / M_PI);
        if (slope_aspect < 0) slope_aspect += 360;
        // Set color based on slope angle, slope aspect, and elevation
        getColor(slope_angle, slope_aspect, i);
    }

    // Initialize GPU Buffers
    glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1,&ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    // Enable client states and set vbo pointers
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3,GL_FLOAT,sizeof(vtx),(void*)0);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3,GL_FLOAT,sizeof(vtx),(void*)(3*sizeof(GLfloat)));
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT,sizeof(vtx),(void*)(3*sizeof(GLfloat)+3*sizeof(GLfloat)));
}

/*
 *  Draw 1 meter DEM Wireframe
 */
void DrawDEM(double dx, double dy, double dz, double scale) {
    // Save transformation
    glPushMatrix();
    // Translate and scale
    // TODO: may need to be reversed now that the DEM is centered at the origin
    glScaled(scale,scale*1.2,scale);
    glTranslated(dx,dy,dz);

    // Set Color Properties
    const float black[] = {0, 0, 0, 1};
    const float white[] = {1, 1, 1, 1};
    glMaterialfv(GL_FRONT,GL_EMISSION, black);
    glMaterialfv(GL_FRONT,GL_SPECULAR, white);

    // Bind the vbo buffers
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);

    // Draw Elements
    glDrawElements(GL_TRIANGLES,sizeof(indices)/sizeof(unsigned int),GL_UNSIGNED_INT,0);

    // Undo transformations
    glPopMatrix();
}
