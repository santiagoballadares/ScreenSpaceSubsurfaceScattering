#pragma once
#include "Vector3.h"

class MyPlane {
public:
	MyPlane(Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3) {
		this->v0 = v0;
		this->v1 = v1;
		this->v2 = v2;
		this->v3 = v3;

		Vector3 sideA = v0-v1;
		Vector3 sideB = v0-v2;
		
		normal = Vector3::Cross(sideA, sideB);
		normal.Normalise();
	}

	~MyPlane() {}

	Vector3 v0;
	Vector3 v1;
	Vector3 v2;
	Vector3 v3;

	Vector3 normal;
};