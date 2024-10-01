## Goal
* The goal of this project is to understand how various ways to simulate a dynamic environment
** The project is intended for educational purposes, hence has a lot of experimental unoptimized code

## Build instructions:
* Navigate to DynamicEnvironment, open DynamicEnvironment solution, and Build solution
* Alternatively navigate to run folder in DynamicEnvironment, and select "DynamicEnvironment_Release_x64.exe" to run the build

Three Game Modes:
- Raymarched Terrain
- Reflection, foliage, and terrain Playground
- Test-bed for anything raymarching

## Controls
** Keyboard:
* "Esc" to quit
* "Space" to speed up time
* "P" to pause
* "T" to slow down time
* "WASD" to move
* "QE" to elevate
* "F1" toggle debug mode, in raymarched mode enables debug camera, and pressing L now locks the debug camera, you can move the default camera around to see the raymarched world being generated entirely on the shader
* "L" in debug mode locks the debug camera movement
* "1" Raymarched Mode
* "2" Reflection, foliage and terrain playground
* "3" Test-bed for anything raymarching 

## Known Issues
* Raymarched terrain mode has hardcoded perspective mapping for figuring out ray direction, this has been solved and used in the test-bed mode
* Raymarched terrain has no constraint on the number of steps when below the world (tanks performance)
* The reflection plane has inverted reflections in Reflection, foliage, and terrain Playground mode
* Only 700,000+ instances of grass blades can be rendered using the unoptimized way, will move to compute pipeline for populating buffers for vertexes and wind influence
* Test-bed raymarched plane has a negative falloff after a certain distance from camera
* Raymarched terrain mode has a specular sun that can be seen underground, and the sky color doesn't reflect the time of day

