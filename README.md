# CSCI4229-Mountain-Scene
## Key Bindings:
*     ESC             Quit Program
*     Arrows          Change View Angle
*     w/s             Move Forward/Backward
*     a/d             Move Left/Right
*     q/e             Move Up/Down
*     1/!             Increase/Decrease Ambient
*     2/@             Increase/Decrease Diffuse
*     3/#             Increase/Decrease Specular

## Compilation
The scene can be compiled and run using cmake. Unzip the files, then run `make` and then `./final`.
It may take a moment to load in all the information from file before the scene is displayed.

## Description
DEM model:
The scene is generated using a digital elevation model of Cirque of the Towers in the southern Wind River Range, WY. The terrain colors smoothly transition between darker green forest, lighter tundra, dirt, stone, and finally snow at high elevation. Steeper terrain is shaded darker which gives more depth in addition to the lighting. Snow is dependent on slope angle, slope aspect, and elevation.

I have implemented my own frustum culling to remove any unseen triangles that are either behind the camera or on a slope facing away.

Scene Decorations: 
There are two types of trees: pine and aspen. The aspen trees are fractals and can be generated to have many layers of branches, but for processing reasons we have minimized the levels of branches. In the winter, the aspens lose their leaves and the pine trees become covered with snow.

In addition to the trees, there are also rabbits, deer, bears, and an owl perched atop one of the trees. The bears go into hibernation during the winter. 

The skybox was digitally painted and put on an idle function timer to allow the clouds to rotate. 
