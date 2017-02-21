#ifndef _LEAN3DVECTOR_H_
#define  _LEAN3DVECTOR_H_

#include <cmath>
#include <iostream>
#include <assert.h>
#if defined(_MSC_VER)
//windows 平台SSE
#include <xmmintrin.h>
#else 
#error can't support other platform except windows.
#endif

namespace Lean3D 
{

	namespace Math 
	{
		class Vector2
		{
		public: float x, y;

		public:
			inline Vector2() :x(0.0f), y(0.0f){}

			inline Vector2(const float fX, const float fY)
			{
				x = fX;
				y = fY;
			}

			inline Vector2(const int iX, const int iY)
			{
				x = (float)iX;
				y = (float)iY;
			}

			inline explicit Vector2(const float fArr[2])
			{
				x = fArr[0];
				y = fArr[1];
			}

			inline explicit Vector2(const int fArr[2])
			{
				x = (float)fArr[0];
				y = (float)fArr[1];
			}

			inline void swap(Vector2 &other)
			{
				std::swap(x, other.x);
				std::swap(y, other.y);
			}

			inline float operator [] (const size_t i) const
			{
				assert(i < 2);
				return *(&x + i);
			}

			inline float& operator [] (const size_t i)
			{
				assert(i < 2);
				return *(&x + i);
			}

			inline Vector2& operator = (const Vector2 &vec2)
			{
				x = vec2.x;
				y = vec2.y;
				return *this;
			}

			inline bool operator == (const Vector2 &vec2) const
			{
				return (x == vec2.x && y == vec2.y);
			}

			inline bool operator != (const Vector2 &vec2) const
			{
				return (x != vec2.x || y != vec2.y);
			}

			inline Vector2 operator + (const Vector2 &vec2) const
			{
				return Vector2(x + vec2.x, y + vec2.y);
			}

			inline Vector2 operator - (const Vector2 &vec2) const
			{
				return Vector2(x - vec2.x, y - vec2.y);
			}

			inline Vector2 operator * (const Vector2 &vec2) const
			{
				return Vector2(x * vec2.x, y * vec2.y);
			}

			inline Vector2 operator / (const Vector2 &vec2) const
			{
				return Vector2(x / vec2.x, y / vec2.y);
			}

			//前加正号
			inline Vector2 operator + () const
			{
				return *this;
			}
			//取负号
			inline Vector2 operator - ()
			{
				return Vector2(-x, -y);
			}

			inline Vector2 operator * (const float scale) const
			{
				return Vector2(x * scale, y * scale);
			}

			inline Vector2 operator / (const float scale) const
			{
				return Vector2(x / scale, y / scale);
			}

			inline Vector2& operator += (const Vector2 &vec2)
			{
				x += vec2.x;
				y += vec2.y;
				return *this;
			}

			inline Vector2& operator -= (const Vector2 &vec2)
			{
				x -= vec2.x;
				y -= vec2.y;
				return *this;
			}

			inline Vector2& operator *= (const Vector2 &vec2)
			{
				x *= vec2.x;
				y *= vec2.y;
				return *this;
			}

			inline Vector2& operator /= (const Vector2 &vec2)
			{
				x /= vec2.x;
				y /= vec2.y;
				return *this;
			}

			inline Vector2& operator *= (const float scale)
			{
				x *= scale;
				y *= scale;
				return *this;
			}

			inline Vector2& operator /= (const float scale)
			{
				x /= scale;
				y /= scale;
				return *this;
			}

			inline bool operator < (const Vector2 &vec2) const
			{
				if (x < vec2.x && y < vec2.y)
					return true;
				return false;
				
			}

			inline bool operator > (const Vector2 &vec2) const
			{
				if (x > vec2.x && y > vec2.y)
					return true;
				return false;
			}

			inline float length() const
			{
				return sqrt(x*x + y*y);
			}

			inline float squareLength() const
			{
				return x*x + y*y;
			}

			inline float distance(const Vector2 &vec2) const
			{
				return (*this - vec2).length();
			}

			inline float squareDistance(const Vector2 &vec2) const
			{
				return (*this - vec2).squareLength();
			}

			inline void normalize()
			{
				float len = (*this).length();

				if (len > 0.0f)
				{
					float invlen = 1.0f / len;
					x *= invlen;
					y *= invlen;
				}
			}

			inline Vector2 normalizeProduct() const
			{
				float invLen = 1.0f / (*this).length();
				return Vector2(x*invLen, y*invLen);
			}

			inline float dot(const Vector2 &vec2) const
			{
				return x*vec2.x + y*vec2.y;
			}

			/*inline Vector2 cross(const Vector2 &vec2) const
			{
				return Vector2(x*vec2.y, y*vec2.x);
			}*/
			//取较小值
			inline void makeFloor(const Vector2 &cmp)
			{
				if (cmp.x < x) x = cmp.x;
				if (cmp.y < y) y = cmp.y;
			}

			inline Vector2 midPoint(const Vector2 &vec2) const
			{
				return Vector2((x + vec2.x)*0.5f, (y + vec2.y)*0.5f);
			}
			//取较大值
			inline void makeCeil(const Vector2 &cmp)
			{
				if (cmp.x > x) x = cmp.x;
				if (cmp.y > y) y = cmp.y;
			}
			
			//inline Vector2 reflect(const Vector2 &normal) const
			//{
			//	return Vector2(*this - (2 * this->dot(normal) * normal));
			//}

			/*Vector2 toRotation() const
			{
			Vector2 vec2;

			}*/

			/*inline Vector2 lerp(const Vector2 &vec2, float f) const
			{
				return Vector2();
			}*/
		};

		class Vector3
		{
		public:
			float x, y, z;

			Vector3() : x(0.0f), y(0.0f), z(0.0f){}

			Vector3(const float x, const float y, const float z) : x(x), y(y), z(z)
			{}

			Vector3(const Vector3 & vec3) : x(vec3.x), y(vec3.y), z(vec3.z)
			{}

			~Vector3()
			{
			}
		};

	}

}

#endif // _LEAN3DVECTOR_H_
