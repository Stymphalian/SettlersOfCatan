#include <cmath> // sqrt()
#include "Vector3d.h"

Vector3d::Vector3d(float x, float y, float z){
	values[0] = x;
	values[1] = y;
	values[2] = z;
}
Vector3d::Vector3d(){
	for(int i = 0; i < num_values; ++i){ values[i] = 0.0f; }
}
Vector3d::~Vector3d(){}
float Vector3d::x(){ return values[0]; }
float Vector3d::y(){ return values[1]; }
float Vector3d::z(){ return values[2]; }
void Vector3d::x(float value){ values[0] = value; }
void Vector3d::y(float value){ values[1] = value; }
void Vector3d::z(float value){ values[2] = value; }
void Vector3d::init(float new_x, float new_y, float new_z){
	values[0] = new_x;
	values[1] = new_y;
	values[2] = new_z;
}

void Vector3d::apply_2d_rotation(float angle){
	float cos_value = cos(angle);
	float sin_value = sin(angle);
	float new_x = cos_value*values[0] - sin_value*values[1];
	float new_y = sin_value*values[0] + cos_value*values[1];
	values[0] = new_x;
	values[1] = new_y;	
}

float Vector3d::get_magnitude(){
	float mag = 0;
	for(int i = 0; i < num_values; ++i){
		mag += values[i] * values[i];
	}
	return sqrt(mag);
}
Vector3d Vector3d::get_unit_vector(){
	Vector3d new_vector;
	float mag = get_magnitude();
	for(int i = 0; i < num_values; ++i){
		new_vector.values[i] = values[i] / mag;
	}
	return new_vector;
}
Vector3d Vector3d::get_clockwise_2d_normal(){
	Vector3d normal(values[1], -values[0]);
	return normal;
}
Vector3d Vector3d::get_counterclockwise_2d_normal(){
	Vector3d normal(-values[1],values[0]);
	return normal;
}

float Vector3d::projection(Vector3d& axis, bool is_unit_vector)
{	
	if(is_unit_vector == false){
		return (*this)*(axis.get_unit_vector());	
	} else {
		return (*this)*(axis);
	}	
}
Vector3d Vector3d::projection_vector(Vector3d& axis){		
	float scalar = ((*this)*axis);
	scalar /= axis*axis;
	return axis*scalar;
}

// overloading operators
void Vector3d::operator= (const Vector3d& rhs){
	for(int i = 0; i < num_values; ++i){
		this->values[i] = rhs.values[i];
	}
}
float Vector3d::operator*(const Vector3d& rhs){
	float rs = 0;
	for(int i = 0; i < num_values; ++i){
		rs += this->values[i] * rhs.values[i];
	}
	return rs;
}
Vector3d Vector3d::operator*(const float scalar){
	Vector3d new_Vector3d;
	for(int i = 0; i < num_values; ++i){
		new_Vector3d.values[i] = this->values[i] * scalar;
	}
	return new_Vector3d;
}
Vector3d Vector3d::operator*(const int scalar){
	Vector3d new_Vector3d;
	for(int i = 0; i < num_values; ++i){
		new_Vector3d.values[i] = this->values[i] * scalar;
	}
	return new_Vector3d;
}
void Vector3d::operator*= (const float scalar){
	for(int i = 0; i < num_values; ++i){
		this->values[i] *= scalar;
	}
}
void Vector3d::operator*= (const int scalar){
	for(int i = 0; i < num_values; ++i){
		this->values[i] *= scalar;
	}
}
Vector3d Vector3d::operator+(const Vector3d& rhs){
	Vector3d new_Vector3d;
	for(int i = 0; i < num_values; ++i){
		new_Vector3d.values[i] = this->values[i] + rhs.values[i];
	}
	return new_Vector3d;
}
void Vector3d::operator+=(const Vector3d& rhs){
	for(int i = 0; i < num_values; ++i){
		this->values[i] += rhs.values[i];
	}
}
Vector3d Vector3d::operator-(const Vector3d& rhs){
	Vector3d new_Vector3d;
	for(int i = 0; i < num_values; ++i){
		new_Vector3d.values[i] = this->values[i] - rhs.values[i];
	}
	return new_Vector3d;
}
void Vector3d::operator-= (const Vector3d& rhs){
	for(int i = 0; i < num_values; ++i){
		this->values[i] -= rhs.values[i];
	}
}
bool Vector3d::operator== (const Vector3d& rhs){	
	static float e = 0.0000001f;
	for(int i = 0; i < num_values; ++i){
		if(values[i] > rhs.values[i] + e  ||
			values[i] < rhs.values[i] - e)
		{return false;}
	}
	return true;
}
