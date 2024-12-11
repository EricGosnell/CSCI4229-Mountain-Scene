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
The scene is generating using a digital elevation model of Cirque of the Towers in the southern Wind River Range, WY. The terrain colors smoothly transition between darker green forest, lighter tundra, dirt, stone, and finally snow at high elevation. Steeper terrain is shaded darker which gives more depth in addition to the lighting. Snow is dependent on slope angle, slope aspect, and elevation.

I have implemented my own frustum culling to remove any unseen triangles that are either behind the camera or on a slope facing away.

There are two types of trees: pine, and aspen. In the winter, the aspens lose their leaves and the pine trees become covered with snow. In addition to the trees, there are also rabbits, deer, and an owl perched atop one of the trees.

## To-Do
The biggest part of the scene left to implement is rendering using a VBO and EBO. I have spent quite a while working on implementing this, which requires refactoring most of the existing code, however it is not yet functional. I do not expect it to take much longer to get working, and will lead to significant improvement in FPS and load time.
If I have enough time afterwards I will implement the system to load closer terrain at a higher resolution than further terrain which will allow for more detail and better performance.

Once the scene is rendered using a VBO I will increase both the scale and the resolution of the mountains so that it is more consistent with the scale of trees and animals.

The coloring system for the terrain also needs some adjustment along with adding support for different seasons such as having the snowline move down lower into the valley in the winter and spring seasons. The sun will also be adjusted to orbit around the scene properly instead of circling overhead.

The trees are currently placed in the scene in a simple manner which shows them in detail, however we will generate much more realistic and denser forests with more trees using a simple algorithm.

We will also implement a skybox surrounding the scene so that it is not always night, and if there is time then this will include features for day and night cycles.


