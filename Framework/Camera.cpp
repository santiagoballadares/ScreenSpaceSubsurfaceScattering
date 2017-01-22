#include "Camera.h"

void Camera::UpdateCamera(float msec, Vector3 groundPos)
{
	// clamp msec value
	msec = min(msec, MIN_SPEED);

	// get values
	pitch	-= (Window::GetMouse()->GetRelativePosition().y);
	yaw		-= (Window::GetMouse()->GetRelativePosition().x);

	speed	+= (Window::GetKeyboard()->KeyDown(KEYBOARD_PLUS));
	speed	-= (Window::GetKeyboard()->KeyDown(KEYBOARD_MINUS));
	
	// clamp values
	speed = min(speed, MAX_SPEED);
	speed = max(speed, MIN_SPEED);

	pitch = min(pitch,  90.0f);
	pitch = max(pitch, -90.0f);

	if (yaw < 0.0f) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	// calculate position
	if ( Window::GetKeyboard()->KeyDown(KEYBOARD_W) ) {
		position += Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) * Vector3(0.0f, 0.0f, -1.0f) * msec * speed;
	}
	if ( Window::GetKeyboard()->KeyDown(KEYBOARD_S) ) {
		position -= Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) * Vector3(0.0f, 0.0f, -1.0f) * msec * speed;
	}

	if ( Window::GetKeyboard()->KeyDown(KEYBOARD_A) ) {
		position += Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) * Vector3(-1.0f, 0.0f, 0.0f) * msec * speed;
	}
	if ( Window::GetKeyboard()->KeyDown(KEYBOARD_D) ) {
		position -= Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) * Vector3(-1.0f, 0.0f, 0.0f) * msec * speed;
	}

	if ( Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		position.y += msec * speed;
	}
	if ( Window::GetKeyboard()->KeyDown (KEYBOARD_SPACE)) {
		position.y -= msec * speed;
	}
/*
	if (groundView)
	{
		// interpolate and set y axis value
		float deltaY = fabs(groundPos.y) - fabs(position.y);
		position.y = groundPos.y + HEIGHT_FROM_GROUND - (deltaY / 2.0f);
	}
*/
}

Matrix4 Camera::BuildViewMatrix()
{
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		   Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		   Matrix4::Translation(-position);
};