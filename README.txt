David Tu
david.tu2@csu.fullerton.edu
CPSC 486-02
Assignment: teapot-trackball
 
A toy program which renders a teapot and two light sources. 
This program uses quaternions to make rotations on a trackball.
The trackball implementation uses Shoemaker's method.

When using this program, use the left mouse button to use quaternion rotations. If you want to see Euler rotations, use the right mouse button instead. You should immediately notice that quaternion rotations are smoother.

When running this program, the first thing that it will do is take the screen coordinates of your mouse click and convert them into device coordinates that are in the range: [-1, 1]. This will be the previous coordinates. If the mouse button is held, another coordinate will be recorded. They would be the current coordinates.

The conversion between screen coordinates to device coordinates are done via Shoemaker's method (In fact when it is done, it's done by a method of the same name). When using Shoemaker's method, it does the following to get the xy device coordinates:

	xDeviceCoord = ((2 * xScreenCoord) - width) / width
	yDeviceCoord = -((2 * yScreenCoord) - height)/ height

Note that the device y-coordinate is negated, that is because in screen coordinates the orgin is in the top left corner of the screen. From that origin, the y value increases as it goes down.

Getting z's device coordinate is a bit tricky since the user can choose a screen coordinate that is outside the sphere. If that is the case, we simply set z = 0, otherwise the z coordinate is on the sphere and can be computed as follows:

	zDeviceCoord = sqrt(1 - (xDeviceCoord^2 + yDeviceCoord^2))

When the user selects a point that is outside the sphere, it also affects the xy coordinates. To remedy this, we simply normalize the coordinates (this will also keep z = 0). Since the radius of our sphere is 1 normalizing it will have the same effect as re-calculating the xy coordinates as follows:

	xDeviceCoord = (r / (sqrt(xDeviceCoord^2 + yDeviceCoord^2))) * x
	yDeviceCoord = (r / (sqrt(xDeviceCoord^2 + yDeviceCoord^2))) * y
	zDeviceCoord = (r / (sqrt(xDeviceCoord^2 + yDeviceCoord^2))) * 0

In this implementation, the normalizing method is utilized.

Once the previous and current coordinates are calculated, the calculation of rotation is straightforward. To get the angle, the dot product was taken between the coordinates, which we treat as vectors (because we assume that the origin is (0, 0, 0)). To get the axis, we simply take the cross product between the two vectors.

Once the angle and axis was determined, we used quaternions to represent a rotation. Once the quaternion was created we converted it into a 4x4 rotation matrix and used it to add to the rotation of the previous iteration. That way, when the teapot gets rendered the program will accurately rotate it based on the last rotation the teapot went before.
