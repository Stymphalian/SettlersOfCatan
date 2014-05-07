#pragma once

class Vector3d{
public:
	Vector3d();
	Vector3d(float x, float y, float z = 0.0f);
	virtual ~Vector3d();
	float x();
	float y();
	float z();
	void x(float value);
	void y(float value);
	void z(float value);
	void init(float new_x, float new_y, float new_z = 0.0f);

	// convenience methods	
	float get_magnitude();
	float projection(Vector3d& axis,bool is_unit_vector = true);
	Vector3d projection_vector(Vector3d& axis);
	Vector3d get_unit_vector();
	Vector3d get_clockwise_2d_normal();
	Vector3d get_counterclockwise_2d_normal();
	void apply_2d_rotation(float angle);

	
	// overloading operators
	void operator= (const Vector3d& rhs);
	float operator*(const Vector3d& rhs);
	Vector3d operator*(const float scalar);
	Vector3d operator*(const int scalar);
	void operator*= (const float scalar);
	void operator*= (const int scalar);
	Vector3d operator+(const Vector3d& rhs);
	void operator+=(const Vector3d& rhs);
	Vector3d operator-(const Vector3d& rhs);
	void operator-= (const Vector3d& rhs);
	bool operator== (const Vector3d& rhs);
protected:
	const int num_values = 3;
	float values[3];
};

