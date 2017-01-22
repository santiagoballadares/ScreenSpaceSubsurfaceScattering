#pragma once
#include "Vector3.h"

class CollisionData {
public:
	Vector3		m_point;
	Vector3		m_normal;
	float		m_penetration;
};