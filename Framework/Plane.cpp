#include "Plane.h"

Plane::Plane(const Vector3 &normal, float distance, bool normalise)
{
	if (normalise)
	{
		float lenght = Vector3::Dot(normal, normal);

		this->normal = normal / lenght;
		this->distance = distance / lenght;
	}
	else
	{
		this->normal = normal;
		this->distance = distance;
	}
}

bool Plane::SphereInPlane(const Vector3 &position, float radius) const
{
	if (Vector3::Dot(position, normal) + distance <= -radius)
	{
		return false;
	}
	return true;
}