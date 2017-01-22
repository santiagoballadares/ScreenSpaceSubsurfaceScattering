#pragma once

#include "Window.h"
#include "Matrix4.h"
#include "Vector3.h"

#define MIN_SPEED 1.0f
#define MAX_SPEED 20.0f

class minimapCamera
{
public:
	minimapCamera(void)
	{
		yaw = 0.0f;
		pitch = 0.0f;
		speed = 1.0f;
	}

	minimapCamera(float pitch, float yaw, Vector3 position)
	{
		this->pitch = pitch;
		this->yaw = yaw;
		this->position = position;
	}

	~minimapCamera(void) { };

	Vector3 getPosition() const		{ return position; }
	void setPosition(Vector3 pos)	{ position = pos; }

	float getYaw() const			{ return yaw; }
	void setYaw(float y)			{ yaw = y; }

	float getPitch() const			{ return pitch; }
	void setPitch(float p)			{ pitch = p; }

	float getSpeed() const			{ return speed; }
	void setSpeed(float s)			{ speed = s; }

	void updateCamera(float msec = 10.0f);

	Matrix4 buildViewMatrix();

protected:
	Vector3 position;
	float yaw;
	float pitch;
	float speed;
};