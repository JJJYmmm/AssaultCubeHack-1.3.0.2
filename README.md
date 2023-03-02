# AssaultCubeHack-1.3.0.2

## Self-aiming and perspective tools for Assault Cube ver1.3.0.2
Perspective is realized by drawing transparent windows with GDI, and double buffer is added to prevent flicker; Self-aiming is to move the collimator by updating the two angles of yaw/pitch. The key to the realization of the two functions is the conversion from the perspective matrix to the screen coordinates (of course, it is also difficult to find the matrix). Perspective needs to use the coordinates of the head and legs of the figure to determine the drawing point of the box; Self-aiming requires the enemy's screen coordinates to determine the closest one to the collimator (the distance used for the previous selection algorithm is the closest, and the effect is not very friendly to the eyes). Finally, the hotkey implementation is relatively simple, but the menu implementation effect is not good, so it is not added. The overall optimization is still to be improved.

> See the pdf for more details
