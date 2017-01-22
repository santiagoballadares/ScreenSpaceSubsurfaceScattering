/*
Class:Vector4
Implements:
Author:Rich Davison
Description:VERY simple Vector4 class. Students are encouraged to modify this as necessary!

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*/
#pragma once

class Vector4	{
public:
	Vector4(void) {
		x = y = z = w = 1.0f;
	}
	Vector4(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	~Vector4(void){}

	float x;
	float y;
	float z;
	float w;

	inline Vector4  operator+(const Vector4  &a) const{
		return Vector4(x + a.x, y + a.y, z + a.z, w + a.w);
	}
	
	inline Vector4  operator-(const Vector4  &a) const{
		return Vector4(x - a.x, y - a.y, z - a.z, w - a.w);
	}

	inline Vector4  operator-() const{
		return Vector4(-x, -y, -z, -w);
	}

	inline void operator+=(const Vector4  &a){
		x += a.x;
		y += a.y;
		z += a.z;
		w += a.w;
	}

	inline void operator-=(const Vector4  &a){
		x -= a.x;
		y -= a.y;
		z -= a.z;
		w -= a.w;
	}

	inline Vector4  operator*(const float a) const{
		return Vector4(x * a, y * a, z * a, w * a);
	}

	inline Vector4  operator*(const Vector4  &a) const{
		return Vector4(x * a.x, y * a.y, z * a.z, w * a.w);
	}

	inline Vector4  operator/(const Vector4  &a) const{
		return Vector4(x / a.x, y / a.y, z / a.z, w / a.w);
	};

	inline Vector4  operator/(const float v) const{
		return Vector4(x / v, y / v, z / v, w / v);
	};
};

