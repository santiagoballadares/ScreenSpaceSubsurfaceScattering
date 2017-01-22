#pragma once

#include "Window.h"
#include "Matrix4.h"
#include "Vector3.h"

#define MIN_SPEED 0.05f
#define MAX_SPEED 20.0f

#define HEIGHT_FROM_GROUND 10.0f

class Camera
{
public:
	Camera(void)
	{
		yaw = 0.0f;
		pitch = 0.0f;
		speed = 1.0f;
	}

	Camera(float pitch, float yaw, Vector3 position, float speed = 1.0f)
	{
		this->pitch		= pitch;
		this->yaw		= yaw;
		this->position	= position;
		this->speed		= speed;
	}

	~Camera(void) { };

	Vector3 GetPosition() const		{ return position; }
	void SetPosition(Vector3 pos)	{ position = pos; }

	float GetYaw() const			{ return yaw; }
	void SetYaw(float y)			{ yaw = y; }

	float GetPitch() const			{ return pitch; }
	void SetPitch(float p)			{ pitch = p; }

	float GetSpeed() const			{ return speed; }
	void SetSpeed(float s)			{ speed = s; }

	void UpdateCamera(float msec = 10.0f, Vector3 groundPos = Vector3(0.0f, 0.0f, 0.0f));

	Matrix4 BuildViewMatrix();

	void toggleViewMode();
	void toggleFreeView();

protected:
	Vector3 position;		// Set to 0 ,0 ,0 by Vector3 constructor
	float yaw;
	float pitch;
	float speed;
};