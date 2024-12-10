#include "CSCIx229.h"

/* Global Variables */
vtx vertices[1048576]; // Vertex list
unsigned int indices[6291456]; // 3 indices per triangle
int dem_season = 0;


/*
 * Define the color of each triangle based on slope angle, slope aspect, and elevation. Linear interpolation
 * is used to transition between different zones, such as between tundra and exposed stone.
 *
 * TODO: explain all color transitions and values and update comments
 */

static void getColor(const int index) {
    float t; // Color interpolation value

    // Colors
    GLfloat forest[3];
    GLfloat tundra[3];
    GLfloat dirt[3];
    GLfloat snow[3];
    GLfloat lake[3];

    // Set colors based on season
    /*
     * Note: I used ChatGPT to help generate some of the RGB values listed below. I started with a baseline color, however
     * since I find color quite difficult to work with, I used prompts such as "Make the autumn forest less orange." or
     * "List the colors in between autumn and winter."
     *
     * All processing of the RGB values is my own work.
     */
    switch (season) {
        // Spring
        case 1:
            forest[0] = 0.2f; forest[1] = 0.6f; forest[2] = 0.2f;
            tundra[0] = 0.6f; tundra[1] = 0.7f; tundra[2] = 0.55f;
            dirt[0] = 0.5f; dirt[1] = 0.4f; dirt[2] = 0.3f;
            snow[0] = 0.9f; snow[1] = 0.9f; snow[2] = 0.9f;
            lake[0] = 0.1f; lake[1] = 0.6f; lake[2] = 0.7f;
            break;
        // Summer
        case 2:
            forest[0] = 0.13f; forest[1] = 0.35f; forest[2] = 0.13f;
            tundra[0] = 0.53f; tundra[1] = 0.55f; tundra[2] = 0.46f;
            dirt[0] = 0.6f; dirt[1] = 0.5f; dirt[2] = 0.4f;
            snow[0] = 0.8f; snow[1] = 0.8f; snow[2] = 0.8f;
            lake[0] = 0; lake[1] = 0.4f; lake[2] = 0.6f;
            break;
        // Autumn
        case 3:
            forest[0] = 0.2f; forest[1] = 0.4f; forest[2] = 0.1f;
            tundra[0] = 0.7f; tundra[1] = 0.6f; tundra[2] = 0.4f;
            dirt[0] = 0.7f; dirt[1] = 0.5f; dirt[2] = 0.35f;
            snow[0] = 0.9f; snow[1] = 0.85f; snow[2] = 0.8f;
            lake[0] = 0.05f; lake[1] = 0.35f; lake[2] = 0.55f;
            break;
        // Winter
        case 4:
            forest[0] = 0.1f; forest[1] = 0.3f; forest[2] = 0.1f;
            tundra[0] = 0.9f; tundra[1] = 0.9f; tundra[2] = 0.9f;
            dirt[0] = 0.85f; dirt[1] = 0.85f; dirt[2] = 0.85f;
            snow[0] = 0.85f; snow[1] = 0.85f; snow[2] = 0.85f;
            lake[0] = 0.7f; lake[1] = 0.9f; lake[2] = 1;
            break;
        default:
            forest[0] = 0; forest[1] = 0; forest[2] = 0;
            tundra[0] = 0; tundra[1] = 0; tundra[2] = 0;
            dirt[0] = 0; dirt[1] = 0; dirt[2] = 0;
            snow[0] = 0; snow[1] = 0; snow[2] = 0;
            lake[0] = 0; lake[1] = 0; lake[2] = 0;
            break;
    }

    if (vertices[indices[index]].avg_elevation <= 3000.0f) {
        // Below 3000, use dark forest green
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = forest[j];
            }
        }
    } else if (vertices[indices[index]].avg_elevation >= 3700.0f) {
        // Above 3700, use dirt
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = dirt[j];
            }
        }
    } else if (vertices[indices[index]].avg_elevation < 3300.0f) {
        // Smooth transition between forest and tundra (3000 to 3300)
        t = (vertices[indices[index]].avg_elevation - 3000.0f) / 300.0f;
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = (1-t)*forest[j] + t*tundra[j];
            }
        }
    } else {
        // Smooth transition between tundra and dirt (3300 to 3700)
        t = (vertices[indices[index]].avg_elevation - 3300.0f) / 400.0f;
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = (1-t)*tundra[j] + t*dirt[j];
            }
        }
    }

    // Darken steeper slopes
    if (vertices[indices[index]].slope_angle > 30) {
        const float darkeningFactor = 1.0f - ((vertices[indices[index]].slope_angle - 30) / 60.0f);
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] *= darkeningFactor;
            }
        }
    }

    // Snow
    if (vertices[indices[index]].slope_angle < 55 &&
        (vertices[indices[index]].slope_aspect > 270 || vertices[indices[index]].slope_aspect < 90
        || vertices[indices[index]].avg_elevation > 3600)) {

        if (vertices[indices[index]].avg_elevation > 3350) {
            // Smooth transition between dirt/stone and snow (3700 to 3800)
            t = (vertices[indices[index]].avg_elevation - 3350.0f) / 100.0f;
            for (int i = index; i < index+3; i++) {
                for (int j = 0; j < 3; j++) {
                    vertices[indices[i]].rgb[j] = (1-t)*dirt[j] + t*snow[j];
                }
            }
        }
    }

    // Lakes
    if (vertices[indices[index]].slope_angle == 0) {
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
        for (int j = i; j < i+3; j++) {
            vertices[indices[j]].slope_angle = slope_angle;
        }
        // Calculate slope aspect
        float slope_aspect = atan2f(-N[2], -N[0]) * (180 / M_PI);
        if (slope_aspect < 0) slope_aspect += 360;
        for (int j = i; j < i+3; j++) {
            vertices[indices[j]].slope_aspect = slope_aspect;
        }
        // Calculate average elevation
        float elevation = ((vertices[indices[i]].y + vertices[indices[i+1]].y + vertices[indices[i+2]].y) / 3.0f) + 3100;
        for (int j = i; j < i+3; j++) {
            vertices[indices[j]].avg_elevation = elevation;
        }
        // Set color based on slope angle, slope aspect, and elevation
        getColor(i);
    }

    // Initialize GPU Buffers
    glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1,&ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
void DrawDEM(const double scale) {
    // Update DEM if season has changed
    if (season != dem_season) {
        dem_season = season;
        for (int i = 0; i < sizeof(indices) / sizeof(unsigned int); i += 3) {
            getColor(i);
        }
        glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vertices),vertices);
    }
    // Save transformation
    glPushMatrix();
    // Translate and scale
    // TODO: may need to be reversed now that the DEM is centered at the origin
    glScaled(scale,scale*1.2,scale);

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
    polygon_count += 8388608;

    // Undo transformations
    glPopMatrix();
}
