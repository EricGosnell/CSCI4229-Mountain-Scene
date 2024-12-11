# CSCI4229 Final Project: Mountain Scene
### Eric Gosnell and Ragan Lee
## Key Bindings:
*     ESC             Quit Program
*     Arrows          Change View Angle
*     1-6             Cycle through spotlight locations 
*     w/s             Move Forward/Backward
*     a/d             Move Left/Right
*     q/e             Move Up/Down


## Compilation
The scene can be compiled and run using cmake. Unzip the files, then run `make` and then `./final`.
It may take a moment to load in all the information from file before the scene is displayed.

## Description
The CSCIx299 Library is provided by Vlakkies. Additionally, some functions such as sphere and reshape have been provided by Vlakkies. All credit is given appropriately in the source code.
### DEM Model:
The scene is generated using a 2-meter digital elevation model of Cirque of the Towers in the southern Wind River Range, WY. I downloaded this data from USGS, cropped it in ArcGIS Pro, converted it using GDAL, and parsed the data in convert.c.

The DEM is drawn using a VBO and EBO, and is updated every time the season changes. This allows for the large scene to be quickly rendered at a very high resolution.

The terrain colors smoothly transition between darker green forest, lighter tundra, dirt, stone, and finally snow at high elevation. Steeper terrain does not contain grass and is shaded darker which gives more depth in addition to the lighting. Snow is dependent on slope angle, slope aspect, and elevation. Lakes occur where the terrain is perfectly flat.
Each season has its own set of colors for the terrain. These RGB values were generated in part using ChatGPT, though all processing of them is my own work.

Unfortunately, the DEM input data has a slight grid pattern visible in it which I was not able to remove since I am unsure where it was introduced. This pattern is only noticeable at high resolutions (1-4 meters).

### Scene Decorations: 
There are two types of trees: pine and aspen. The aspen trees are fractals and can be generated to have many layers of branches, but for processing reasons we have minimized the levels of branches. In the winter, the aspens lose their leaves and the pine trees become covered with snow.

Frustum culling is used to render only the trees that can be seen. Thia ia done to improve performance.

In addition to the trees, there are also rabbits, deer, bears, and an owl perched atop one of the trees. The bears go into hibernation during the winter. The animals have realistic proportions, and the bears have two different poses: standing and on all fours.

The skybox was digitally painted and put on an idle function timer to allow the clouds to rotate. 
