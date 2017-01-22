#pragma once

#include <iostream>
#include "common.h"
#include "Vector3.h"
#include "Vector4.h"

class Vector3;

class Matrix3 {
public:
	Matrix3(void);
	Matrix3(float elements[9]);
	~Matrix3(void);

	float	values[9];

	//Set all matrix values to zero
	void	ToZero();
	
	//Sets matrix to identity matrix (1.0 down the diagonal)
	void	ToIdentity();

	//Multiplies 'this' matrix by matrix 'a'. Performs the multiplication in 'OpenGL' order (ie, backwards)
	inline Matrix3 operator*(const Matrix3 &a) const {
		Matrix3 out;
		
		for (unsigned int r = 0; r < 3; ++r) {
			for (unsigned int c = 0; c < 3; ++c) {
				out.values[c + (r*3)] = 0.0f;
				for (unsigned int i = 0; i < 3; ++i) {
					out.values[c + (r*3)] += this->values[c+(i*3)] * a.values[(r*3)+i];
				}
			}
		}
		return out;
	}

	inline Vector3 operator*(const Vector3 &v) const {
		return Vector3(
			v.x*values[0] + v.y*values[3] + v.z*values[6],
			v.x*values[1] + v.y*values[4] + v.z*values[7],
			v.x*values[2] + v.y*values[5] + v.z*values[8]
		);
	};

	// String output for the matrix.
	inline friend std::ostream& operator<<(std::ostream& o, const Matrix3& m) {
		o << "Mat3(";
		o << "\t"	<< m.values[0] << "," << m.values[1] << "," << m.values[2] << std::endl;
		o << "\t\t" << m.values[3] << "," << m.values[4] << "," << m.values[5] << std::endl;
		o << "\t\t" << m.values[6] << "," << m.values[7] << "," << m.values[8] << " )" <<std::endl;
		return o;
	}

	// Transpose the matrix
	Matrix3 Transpose() const;
};

