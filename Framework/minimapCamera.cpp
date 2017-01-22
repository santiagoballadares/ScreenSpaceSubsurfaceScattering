#include "minimapCamera.h"

void minimapCamera::updateCamera(float msec)
{
	// clamp msec value
	msec = min(msec, MIN_SPEED);

	// get values
	speed += (Window::GetKeyboard()->KeyDown(KEYBOARD_PLUS));
	speed -= (Window::GetKeyboard()->KeyDown(KEYBOARD_MINUS));
	
	// clamp values
	speed = min(speed, MAX_SPEED);
	speed = max(speed, MIN_SPEED);

}

Matrix4 minimapCamera::buildViewMatrix()
{
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		   Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		   Matrix4::Translation(-position);
};