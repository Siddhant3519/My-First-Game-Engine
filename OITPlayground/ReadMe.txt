Four game modes
* OIT (subgroup modes: worst case, cpu sorted, depth peeling, virtual pixel maps, weighted blended, per pixel linked list, multi layer alpha blending)
* Depth test (subgroup modes: depth comparision <=, depth comparision >=, depth test disabled)
* Alpha blending (subgroup modes: over operator, under operator)
* UAV writes (subgroup modes: with ROV, without ROV)

## Controls
** Keyboard:
* "T" to toggle texture on or off
* "WASD" to move
* "QE" to elevate
* "L" locks the game mode
* "Right/Left" arrow keys switch game modes when the game mode is not locked
* "Right/Left" arrow keys switches game scenes when game mode is locked
* "Up/Down" arrow keys switch between the various submodes the current selected game mode has
* "V" turns off retained mode rendering(using vertex and index buffers to render the scene) and switches to immediate mode rendering where one gigantic draw call containing all the objcts in the scene is issued every frame
** The above key is mainly only usefull for UAV Writes game mode
* DevConsole command "PeelCount Count=x", where x the number can be used to increase/decrease the number of redering passes for Depth peeling and Virtual pixel maps (when in OIT game mode)
* "J" switches between different number of nodes (2, 4, 32) used for creating per pixel linked list, and multi layer alpha blending (when in OIT game mode)
* "N" splitscreen (1, 2, 4)
* "1" when in split screen applies the above mentioned controls to the first quadrant (top left)
* "2" when in split screen applies the above mentioned controls to the second quadrant (top Rght)
* "3" when in split screen applies the above mentioned controls to the third quadrant (bottom left)
* "4" when in split screen applies the above mentioned controls to the fourth quadrant (bottom Rght)
** Scene and game mode switching happens globally and applies to all the quadrants
* "Z" in depth test mode enables the depth tests
* "V" in UAV writes mode showcases the problem of writing to a UAV resource

