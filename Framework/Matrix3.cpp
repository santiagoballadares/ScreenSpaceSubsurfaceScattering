#include "Matrix3.h"

Matrix3::Matrix3(void)	{
	ToIdentity();
}

Matrix3::Matrix3( float elements[9] )	{
	memcpy(this->values,elements,9*sizeof(float));
}

Matrix3::~Matrix3(void)	{
	ToIdentity();
}

void Matrix3::ToIdentity() {
	ToZero();
	values[0] = 1.0f;
	values[4] = 1.0f;
	values[8] = 1.0f;
}

void Matrix3::ToZero()	{
	for(int i = 0; i < 9; i++)	{
		values[i] = 0.0f;
	}
}

Matrix3 Matrix3::Transpose() const{
	Matrix3 mat;

	mat.values[0] = values[0];
	mat.values[4] = values[4];
	mat.values[8] = values[8];
	
	mat.values[1] = values[3];
	mat.values[3] = values[1];

	mat.values[2] = values[6];
	mat.values[6] = values[2];
	
	mat.values[5] = values[7];
	mat.values[7] = values[5];
	
	return mat;
}