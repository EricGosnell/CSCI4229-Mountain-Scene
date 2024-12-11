#include "CSCIx229.h"

/* Global Variables */
vtx vertices[4194304]; // Vertex list
unsigned int indices[25165824]; // 3 indices per triangle
int dem_season = 0; // Current season state


/*
 * Define the color of each triangle based on slope angle, slope aspect, and elevation. Linear interpolation
 * is used to transition between different zones, such as between tundra and dirt.
 */
static void getColor(const int index) {
    float t; // Color interpolation value

    // Colors
    GLfloat forest[3];
    GLfloat tundra[3];
    GLfloat dirt[3];
    GLfloat snow[3];
    GLfloat lake[3];
    const GLfloat stone[3] = {0.45f, 0.42f, 0.4f};
    int snow_offset = 0; // Elevation offset of snow-line

    /*
     * Note: I used ChatGPT to help generate some of the RGB values listed below. I started with a baseline color, however
     * since I find color quite difficult to work with, I used prompts such as "Make the autumn forest less orange."
     *
     * All processing of the RGB values is my own work.
     */
    // Set colors based on season
    switch (season) {
        // Spring
        case 1:
            forest[0] = 0.18f; forest[1] = 0.42f; forest[2] = 0.18f;
            tundra[0] = 0.40f; tundra[1] = 0.55f; tundra[2] = 0.35f;
            dirt[0] = 0.45f; dirt[1] = 0.42f; dirt[2] = 0.32f;
            snow[0] = 0.9f; snow[1] = 0.9f; snow[2] = 0.9f;
            lake[0] = 0; lake[1] = 0.4f; lake[2] = 0.6f;
            snow_offset = -50;
            break;
        // Summer
        case 2:
            forest[0] = 0.25f; forest[1] = 0.50f; forest[2] = 0.20f;
            tundra[0] = 0.47f; tundra[1] = 0.58f; tundra[2] = 0.38f;
            dirt[0] = 0.52f; dirt[1] = 0.45f; dirt[2] = 0.35f;
            snow[0] = 0.8f; snow[1] = 0.8f; snow[2] = 0.8f;
            lake[0] = 0; lake[1] = 0.4f; lake[2] = 0.6f;
            snow_offset = 50;
            break;
        // Autumn
        case 3:
            forest[0] = 0.30f; forest[1] = 0.35f; forest[2] = 0.18f;
            tundra[0] = 0.45f; tundra[1] = 0.40f; tundra[2] = 0.25f;
            dirt[0] = 0.50f; dirt[1] = 0.40f; dirt[2] = 0.30f;
            snow[0] = 0.9f; snow[1] = 0.85f; snow[2] = 0.8f;
            lake[0] = 0.05f; lake[1] = 0.35f; lake[2] = 0.55f;
            snow_offset = 250;
            break;
        // Winter
        case 4:
            forest[0] = 0.5f; forest[1] = 0.55f; forest[2] = 0.5f;
            tundra[0] = 0.9f; tundra[1] = 0.9f; tundra[2] = 0.9f;
            dirt[0] = 0.85f; dirt[1] = 0.85f; dirt[2] = 0.85f;
            snow[0] = 0.85f; snow[1] = 0.85f; snow[2] = 0.85f;
            lake[0] = 0.8f; lake[1] = 0.95f; lake[2] = 1.0f;
            snow_offset = -100;
            break;
        default:
            forest[0] = 0; forest[1] = 0; forest[2] = 0;
            tundra[0] = 0; tundra[1] = 0; tundra[2] = 0;
            dirt[0] = 0; dirt[1] = 0; dirt[2] = 0;
            snow[0] = 0; snow[1] = 0; snow[2] = 0;
            lake[0] = 0; lake[1] = 0; lake[2] = 0;
            snow_offset = 0;
            break;
    }

    const float elevation = vertices[indices[index]].avg_elevation;
    const float slope_angle = vertices[indices[index]].slope_angle;
    const float slope_aspect = vertices[indices[index]].slope_aspect;
    if (elevation <= 3150.0f) {
        // Below 3150, use forest
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = forest[j];
            }
        }
    } else if (elevation < 3250.0f) {
        // Smooth transition between forest and tundra (3150 to 3250)
        t = (elevation - 3150.0f) / 100.0f;
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = (1-t)*forest[j] + t*tundra[j];
            }
        }
    } else if (elevation < 3400.0f) {
        // From 3250 to 3400, use tundra
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = tundra[j];
            }
        }
    } else if (elevation < 3550.0f){
        // Smooth transition between tundra and dirt (3400 to 3550)
        t = (elevation - 3400.0f) / 150.0f;
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = (1-t)*tundra[j] + t*dirt[j];
            }
        }
    } else {
        // Above 3550, use dirt
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] = dirt[j];
            }
        }
    }

    // Steeper slopes cannot support grass
    if (slope_angle > 50) {
        if (elevation < 3400) {
            // Below 3400, use dirt
            for (int i = index; i < index+3; i++) {
                for (int j = 0; j < 3; j++) {
                    vertices[indices[i]].rgb[j] = dirt[j];
                }
            }
        } else {
            t = (slope_angle - 50) / 20.0f;
            t = (t > 1.0f) ? 1.0f : t;
            // Above 3400, use more stone the steeper the slope is
            for (int i = index; i < index+3; i++) {
                for (int j = 0; j < 3; j++) {
                    vertices[indices[i]].rgb[j] = (1-t)*vertices[indices[i]].rgb[j] + t*stone[j];
                }
            }
        }
    }

    // Darken steeper slopes
    if (slope_angle > 30) {
        const float darkeningFactor = 1.0f - ((slope_angle - 30) / (season==4?120.0f:100.0f));
        for (int i = index; i < index+3; i++) {
            for (int j = 0; j < 3; j++) {
                vertices[indices[i]].rgb[j] *= darkeningFactor;
            }
        }
    }

    // Snow
    if (slope_angle < 60) { // No snow on very steep slopes
        if (slope_aspect > 270 || slope_aspect < 90) { // North Facing
            if (elevation > 3400+snow_offset) {
                // Smooth transition to snow (3400 to 3450)
                t = (elevation - (float)(3400+snow_offset)) / 50.0f;
                for (int i = index; i < index+3; i++) {
                    for (int j = 0; j < 3; j++) {
                        vertices[indices[i]].rgb[j] = (1-t)*vertices[indices[i]].rgb[j] + t*snow[j];
                    }
                }
            } else if (elevation > 3450+snow_offset) {
                for (int i = index; i < index+3; i++) {
                    for (int j = 0; j < 3; j++) {
                        vertices[indices[i]].rgb[j] = snow[j];
                    }
                }
            }
        } else if (elevation > 3700+snow_offset) { // South Facing
            // Smooth transition to snow (3700 to 3750)
            t = (elevation - (float)(3700+snow_offset)) / 50.0f;
            for (int i = index; i < index+3; i++) {
                for (int j = 0; j < 3; j++) {
                    vertices[indices[i]].rgb[j] = (1-t)*vertices[indices[i]].rgb[j] + t*snow[j];
                }
            }
        } else if (elevation > 3750+snow_offset) {
            // Above 3750, use snow
            for (int i = index; i < index+3; i++) {
                for (int j = 0; j < 3; j++) {
                    vertices[indices[i]].rgb[j] = snow[j];
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
 *  Read 1 meter DEM from the corresponding file and calculate the color and normals.
 */
void ReadDEM(const char *fileName) {
    /* Read in elevation data from the .dem file, and populate the vertices array.*/
    FILE *file = fopen(fileName, "r");
    if (!file) Fatal("Cannot open file %s\n", fileName);
    float fy;
    for (int i = 0; i < DEM_W*DEM_W; i++) {
        if (fscanf(file, "%f", &fy) != 1) Fatal("Error reading file %s\n", fileName);
        if ((i/DEM_W)%DEM_R == 0 && (i%DEM_W) % DEM_R == 0) {
            int index = ((i/DEM_W)/DEM_R) * (DEM_W/DEM_R) + (i%DEM_W)/DEM_R;
            if (index < (DEM_W/DEM_R)*(DEM_W/DEM_R)) {
                vertices[index].x = 2*(i/DEM_W - DEM_W/2);
                vertices[index].y = fy - 3100;
                vertices[index].z = 2*(i%DEM_W - DEM_W/2);
            }
        }
    }
    fclose(file);

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
 *  Draw 1 meter DEM
 */
void DrawDEM() {
    // Update VBO if season has changed
    if (season != dem_season) {
        dem_season = season;
        for (int i = 0; i < sizeof(indices) / sizeof(unsigned int); i += 3) {
            getColor(i);
        }
        glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vertices),vertices);
    }
    // Save transformation
    glPushMatrix();
    // Vertically scale by 1.25
    glScaled(1,1.25,1);

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
    polygon_count += DEM_W*DEM_W*2;

    // Undo transformations
    glPopMatrix();
}
