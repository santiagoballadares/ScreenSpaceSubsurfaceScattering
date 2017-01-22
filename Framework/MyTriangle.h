#pragma once
#include "Vector3.h"

class MyTriangle {
public:
	Vector3 v1;
	Vector3 v2;
	Vector3 v3;

	MyTriangle(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3) {
		this->v1 = v1;
		this->v2 = v2;
		this->v3 = v3;
	}

	~MyTriangle() {}

	Vector3 GetNormal() {
		Vector3 sideA = v1-v2;
		Vector3 sideB = v1-v3;
		
		normal = Vector3::Cross(sideA, sideB);
		normal.Normalise();

		return normal;
	}

private:
	Vector3 normal;
};