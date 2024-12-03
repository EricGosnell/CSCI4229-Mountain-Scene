/*
 * Eric Gosnell
 *
 * Digital Elevation Models are available for download from the USGS 3DEP Progam. https://apps.nationalmap.gov/downloader/
 * Multiple DEM's can be combined and cropped using ArcGIS Pro.
 * These DEM's were processed into readable text file using GDAL. https://gdal.org/en/latest/index.html.
 * The command <gdal_translate -of XYZ input.tif output.dem>. This produces a file with grid_size^2 rows each containing
 * latitude and longitude in decimal degrees, and elevation in meters.
 *
 * This file processes this format to produce grid_size^2 rows elevations in meters.
 * This is then passed to the readDEM function, and values such as top left coordinates, resolution, and grid size are
 * manually added to the corresponding DEM struct.
 * 
 * Usage: ./convert <input_file.dem> <output_file1.dem> <output_file2.dem> <grid_size>
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <input_file> <output_file1> <output_file2> <grid_size>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("Error opening input file");
        return 1;
    }

    FILE *output1 = fopen(argv[2], "w");
    if (output1 == NULL) {
        perror("Error opening output file");
        fclose(input);
        return 1;
    }

    FILE *output2 = fopen(argv[3], "w");
    if (output2 == NULL) {
        perror("Error opening output file");
        fclose(input);
        return 1;
    }

    int grid_size = atoi(argv[4]);
    if (grid_size <= 0) {
        fprintf(stderr, "Grid size must be a positive integer!\n");
        return 1;
    }

    float x, y, z;
    float** data = malloc(grid_size * sizeof(float*));
    for (int i = 0; i < grid_size; i++) {
        data[i] = malloc(grid_size * sizeof(float));
    }

    // Read each line of the input file
    printf("Reading input data...\n");
    int row = 0, col = 0;
    while (fscanf(input, "%f %f %f", &x, &y, &z) == 3) {
        data[row][col] = z;
        col++;
        if (col == grid_size) {
            col = 0;
            row++;
            printf("\r %.2f%%", (float)row/grid_size*100);
        }
        if (row == grid_size) break;
    }

    fclose(input);

    // Write the values in the required format
    int count = 0;
    printf("\nWriting output data...\n");
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (count < grid_size*grid_size/2) fprintf(output1, "%.2f\n", data[i][j]);
            else fprintf(output2, "%.2f\n", data[i][j]);
            count++;
        }
        printf("\r %.2f%%", (float)(i+1)/grid_size*100);
    }

    fclose(output1);
    fclose(output2);

    // Free dynamically allocated memory
    for (int i = 0; i < grid_size; i++) {
        free(data[i]);
    }
    free(data);

    printf("\nOutput written to %s and %s\n", argv[2], argv[3]);
    return 0;
}
