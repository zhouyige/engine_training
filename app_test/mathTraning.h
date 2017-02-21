#pragma once
#include <math.h>

class Vector3
{
public:
	Vector3():x(0.0f),y(0.0f),z(0.0f) { }
	Vector3(float px, float py, float pz) { x = px; y = py; z = pz; }
	
	Vector3 operator+(const Vector3 &vec3) const
	{
		return Vector3(x + vec3.x, y + vec3.y, z + vec3.z);
	}
	
	Vector3 operator-(const Vector3 &vec3) const
	{
		return Vector3(x - vec3.x, y - vec3.y, z - vec3.z);
	}

	Vector3 operator*(const Vector3 &vec3) const
	{
		return Vector3(x * vec3.x, y * vec3.y, z * vec3.z);
	}

	Vector3 operator/(const Vector3 &vec3) const
	{
		return Vector3(x / vec3.x, y / vec3.y, z / vec3.z);
	}

	float length()
	{
		return sqrtf(x * x + y * y + z * z);
	}

	Vector3 normalize()
	{
		float inlen = 1 / length();
		return Vector3(inlen * x, inlen * y, inlen * z);
	}

	Vector3 lerp(const Vector3 &vec3, float n) const
	{
		return Vector3(x + n*(vec3.x - x), y + n*(vec3.y - y), z + n*(vec3.z - z));
	}

	Vector3 cross(const Vector3 &vec3) const
	{
		return Vector3(y * vec3.z - z * vec3.y, z * vec3.x - x*vec3.z, x * vec3.y - y * vec3.x);
	}

	Vector3 dot(const Vector3 &vec3) const
	{
		return Vector3(x * vec3.x, y * vec3.y, z * vec3.z);
	}

	Vector3 toRotate() const
	{
		Vector3 vec3;
		if (y != 0) vec3.x = atan2f(y, x*x + z*z);
		if (x != 0 && z != 0) vec3.y = atan2f(-x, -z);
	}

	~Vector3() { }

private:
	float x, y, z;
};

class Quaternion
{
public:
	Quaternion():x(0.0f),y(0.0f),z(0.0f),w(0.0f) {}
	Quaternion(float px, float py, float pz, float pw):x(px),y(py),z(pz),w(pw) {}
	Quaternion(const float aulerX, const float aulerY, const float aulerZ)
	{
		Quaternion roll(sinf(0.5f*aulerX), 0, 0, cosf(aulerX * 0.5f));
		Quaternion pitch(0, sinf(0.5f*aulerY), 0, cosf(aulerY * 0.5f));
		Quaternion yaw(0, 0, sinf(0.5f*aulerZ), cosf(aulerZ * 0.5f));

		*this = roll * yaw * pitch;
	}
	
	Quaternion operator*(const Quaternion &q) const
	{
		return Quaternion( x*q.w + q.x*w + y*q.z - z*q.y
						 , y*q.w + w*q.y + z*q.x - x*q.z
						 , z*q.w + w*q.z + x*q.y - y*q.x
						 , w*q.w - (x*q.x + y*q.y + z*q.z));
	}

	~Quaternion(){}

	Quaternion slerp(const Quaternion &q, const float &n) const
	{
		Quaternion q1(q);


		float cosTheta = q.x*x + q.y*y + q.z*z + q.w*w;

		if (cosTheta < 0)
		{
			cosTheta = -cosTheta;
			q1.x = -q.x;
			q1.y = -q.y;
			q1.z = -q.z;
			q1.w = -q.w;
		}

		float s1 = 1 - n;
		float s2 = n;

		if (1-cosTheta > 0.01f)
		{
			float theta = acosf(cosTheta);
			float sinTheta = sinf(theta);
			s1 = sinf(s1*theta) / sinTheta;
			s2 = sinf(s2*theta) / sinTheta;
		}

		return Quaternion(x*s1+s2*q.x, y*s1+s2*q.y, z*s1+s2*q.z, w*s1+s2*q.w);
	}


private:
	float x, y, z, w;
};

class Matrix4
{
public:
	Matrix4();
	~Matrix4();

private:

};
