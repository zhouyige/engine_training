/***************************************Lean3D**********************************************/
// 
// - 坐标系统是右手坐标系
// - 旋转为逆时针方向
// - 向量默认指向-Z坐标轴
//
/*************************************************************************************/


#ifndef _utMath_H_
#define _utMath_H_

#include <cmath>


namespace Lean3D {

namespace Math
{
	const unsigned int MaxUInt32 = 0xFFFFFFFF;
	const int MinInt32 = 0x80000000;
	const int MaxInt32 = 0x7FFFFFFF;
	const float MaxFloat = 3.402823466e+38F;
	const float MinPosFloat = 1.175494351e-38F;
	
	const float Pi = 3.141592654f;
	const float TwoPi = 6.283185307f;
	const float PiHalf = 1.570796327f;

	const float Epsilon = 0.000001f;     //精度
	const float ZeroEpsilon = 32.0f * MinPosFloat; 
#ifdef __GNUC__
	const float NaN = __builtin_nanf("");
#else
	const float NaN = *(float *)&MaxUInt32;
#endif

	enum NoInitHint
	{
		NO_INIT
	};
};


// -------------------------------------------------------------------------------------------------
// 常用操作
// -------------------------------------------------------------------------------------------------

static inline float degToRad( float f ) 
{
	return f * 0.017453293f;
}

static inline float radToDeg( float f ) 
{
	return f * 57.29577951f;
}

static inline float clamp( float f, float min, float max )
{
	if( f < min ) f = min;
	else if( f > max ) f = max;

	return f;
}

static inline float minf( float a, float b )
{
	return a < b ? a : b;
}

static inline float maxf( float a, float b )
{
	return a > b ? a : b;
}

static inline int maxi(int a, int b)
{
	return a > b ? a : b;
}

static inline float fsel( float test, float a, float b )
{
	return test >= 0 ? a : b;
}


// -------------------------------------------------------------------------------------------------
// 值类型转换
// -------------------------------------------------------------------------------------------------

static inline int ftoi_t( double val )
{	
	return (int)val;
}

static inline int ftoi_r( double val )
{
	// 比直接强制转换块的转换方式
	union
	{
		double dval;
		int ival[2];
	} u;

	u.dval = val + 6755399441055744.0;  // 常数值: 2^52 * 1.5;
	return u.ival[0];        
}


// -------------------------------------------------------------------------------------------------
// Vector
// -------------------------------------------------------------------------------------------------

class Vec3
{
public:
	float x, y, z;
	
	Vec3() : x( 0.0f ), y( 0.0f ), z( 0.0f )
	{ 
	}

	explicit Vec3( Math::NoInitHint )
	{
	}
	
	Vec3( const float x, const float y, const float z ) : x( x ), y( y ), z( z ) 
	{
	}

	Vec3( const Vec3 &v ) : x( v.x ), y( v.y ), z( v.z )
	{
	}

	float &operator[]( unsigned int index )
	{
		return *(&x + index);
	}
	
	bool operator==( const Vec3 &v ) const
	{
		return (x > v.x - Math::Epsilon && x < v.x + Math::Epsilon && 
		        y > v.y - Math::Epsilon && y < v.y + Math::Epsilon &&
		        z > v.z - Math::Epsilon && z < v.z + Math::Epsilon);
	}

	bool operator!=( const Vec3 &v ) const
	{
		return (x < v.x - Math::Epsilon || x > v.x + Math::Epsilon || 
		        y < v.y - Math::Epsilon || y > v.y + Math::Epsilon ||
		        z < v.z - Math::Epsilon || z > v.z + Math::Epsilon);
	}
	
	Vec3 operator-() const
	{
		return Vec3( -x, -y, -z );
	}

	Vec3 operator+( const Vec3 &v ) const
	{
		return Vec3( x + v.x, y + v.y, z + v.z );
	}

	Vec3 &operator+=( const Vec3 &v )
	{
		return *this = *this + v;
	}

	Vec3 operator-( const Vec3 &v ) const 
	{
		return Vec3( x - v.x, y - v.y, z - v.z );
	}

	Vec3 &operator-=( const Vec3 &v )
	{
		return *this = *this - v;
	}

	Vec3 operator*( const float f ) const
	{
		return Vec3( x * f, y * f, z * f );
	}

	Vec3 &operator*=( const float f )
	{
		return *this = *this * f;
	}

	Vec3 operator/( const float f ) const
	{
		return Vec3( x / f, y / f, z / f );
	}

	Vec3 &operator/=( const float f )
	{
		return *this = *this / f;
	}

	float dot( const Vec3 &v ) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Vec3 cross( const Vec3 &v ) const
	{
		return Vec3( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x );
	}

	float length() const 
	{
		return sqrtf( x * x + y * y + z * z );
	}

	Vec3 normalized() const
	{
		float invLen = 1.0f / length();
		return Vec3( x * invLen, y * invLen, z * invLen );
	}

	void normalize()
	{
		float invLen = 1.0f / length();
		x *= invLen;
		y *= invLen;
		z *= invLen;
	}
	//转换为与xyz坐标的夹角
	Vec3 toRotation() const
	{
		// 假定向量未旋转时 (0, 0, -1)
		Vec3 v;
		
		if( y != 0 ) v.x = atan2f( y, sqrtf( x*x + z*z ) );
		if( x != 0 || z != 0 ) v.y = atan2f( -x, -z );

		return v;
	}

	Vec3 lerp( const Vec3 &v, float f ) const
	{
		return Vec3( x + (v.x - x) * f, y + (v.y - y) * f, z + (v.z - z) * f ); 
	}
};


class Vec4
{
public:
	
	float x, y, z, w;

	Vec4() : x( 0 ), y( 0 ), z( 0 ), w( 0 )
	{
	}
	
	explicit Vec4( const float x, const float y, const float z, const float w ) :
		x( x ), y( y ), z( z ), w( w )
	{
	}

	explicit Vec4( Vec3 v ) : x( v.x ), y( v.y ), z( v.z ), w( 1.0f )
	{
	}

	Vec4 operator+( const Vec4 &v ) const
	{
		return Vec4( x + v.x, y + v.y, z + v.z, w + v.w );
	}

	Vec4 operator-() const
	{
		return Vec4( -x, -y, -z, -w );
	}
	
	Vec4 operator*( const float f ) const
	{
		return Vec4( x * f, y * f, z * f, w * f );
	}
};


// 四元数
class Quaternion
{
public:	
	
	float x, y, z, w;

	Quaternion() : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f ) 
	{ 
	}
	
	explicit Quaternion( const float x, const float y, const float z, const float w ) :
		x( x ), y( y ), z( z ), w( w )
	{
	}
	
	Quaternion( const float eulerX, const float eulerY, const float eulerZ )
	{
		Quaternion roll( sinf( eulerX * 0.5f ), 0, 0, cosf( eulerX * 0.5f ) );
		Quaternion pitch( 0, sinf( eulerY * 0.5f ), 0, cosf( eulerY * 0.5f ) );
		Quaternion yaw( 0, 0, sinf( eulerZ * 0.5f ), cosf( eulerZ * 0.5f ) );
	
		// 顺序: y * x * z
		*this = pitch * roll * yaw;
	}

	Quaternion operator*( const Quaternion &q ) const
	{
		//(x1 y1 z1 w1)(x2 y2 z2 w2) = (w1*x2+w2*x1+y1*z2-z1*y2
		//////////////////////////////, w1*y2+w2*y1+z1*x2-x1*z2
		//////////////////////////////, w1*z2+w2*z1+x1*y2-y1*x2
		//////////////////////////////, w1*w2-x1*x2-y1*y2-z1*z2) 
		return Quaternion(
			y * q.z - z * q.y + q.x * w + x * q.w,
			z * q.x - x * q.z + q.y * w + y * q.w,
			x * q.y - y * q.x + q.z * w + z * q.w,
			w * q.w - (x * q.x + y * q.y + z * q.z) );
	}

	Quaternion &operator*=( const Quaternion &q )
	{
		return *this = *this * q;
	}
	
	// 球面线性插值
	Quaternion slerp( const Quaternion &q, const float t ) const
	{
		
		// 插值结果以常数角速率变化
		Quaternion q1( q );

        // 夹角余弦值
        float cosTheta = x * q.x + y * q.y + z * q.z + w * q.w;
        //
        if( cosTheta < 0 )
		{
			cosTheta = -cosTheta; 
			q1.x = -q.x; q1.y = -q.y;
			q1.z = -q.z; q1.w = -q.w;
        }
        // 以线性插值初始插值方式
		float scale0 = 1 - t, scale1 = t;
		// 只有当两四元数向量夹角不是很小的时候插值
		if( (1 - cosTheta) > 0.001f )
		{
			float theta = acosf( cosTheta );
			float sinTheta = sinf( theta );
			scale0 = sinf( (1 - t) * theta ) / sinTheta;
			scale1 = sinf( t * theta ) / sinTheta;
		} 

		return Quaternion( x * scale0 + q1.x * scale1, y * scale0 + q1.y * scale1,
		                   z * scale0 + q1.z * scale1, w * scale0 + q1.w * scale1 );
	}

	Quaternion nlerp( const Quaternion &q, const float t ) const
	{
		//旋转线性插值 （较快）
		Quaternion qt;
		float cosTheta = x * q.x + y * q.y + z * q.z + w * q.w;
		
		if( cosTheta < 0 )
			qt = Quaternion( x + (-q.x - x) * t, y + (-q.y - y) * t,
							 z + (-q.z - z) * t, w + (-q.w - w) * t );
		else
			qt = Quaternion( x + (q.x - x) * t, y + (q.y - y) * t,
							 z + (q.z - z) * t, w + (q.w - w) * t );

		//归一
		float invLen = 1.0f / sqrtf( qt.x * qt.x + qt.y * qt.y + qt.z * qt.z + qt.w * qt.w );
		return Quaternion( qt.x * invLen, qt.y * invLen, qt.z * invLen, qt.w * invLen );
	}

	Quaternion inverted() const
	{
		float len = x * x + y * y + z * z + w * w;
		if( len > 0 )
        {
            float invLen = 1.0f / len;
            return Quaternion( -x * invLen, -y * invLen, -z * invLen, w * invLen );
		}
		else return Quaternion();
	}
};


// -------------------------------------------------------------------------------------------------
// 矩阵
// -------------------------------------------------------------------------------------------------

class Matrix4
{
public:
	
	union
	{
		float c[4][4];	// c[行][列]
		float x[16];
	};

	static Matrix4 TransMat( float x, float y, float z )
	{
		Matrix4 m;

		m.c[3][0] = x;
		m.c[3][1] = y;
		m.c[3][2] = z;

		return m;
	}

	static Matrix4 ScaleMat( float x, float y, float z )
	{
		Matrix4 m;
		
		m.c[0][0] = x;
		m.c[1][1] = y;
		m.c[2][2] = z;

		return m;
	}

	static Matrix4 RotMat( float x, float y, float z )
	{
		// 旋转顺序 YXZ
		return Matrix4( Quaternion( x, y, z ) );
	}

	static Matrix4 RotMat( Vec3 axis, float angle )
	{
		axis = axis * sinf( angle * 0.5f );
		return Matrix4( Quaternion( axis.x, axis.y, axis.z, cosf( angle * 0.5f ) ) );
	}

	static Matrix4 PerspectiveMat( float left, float right, float buttom, float top, float near, float far )
	{
		Matrix4 m;
 
		m.x[0] = 2 * near / (right - left);
		m.x[5] = 2 * near / (top - buttom);
		m.x[8] = (right + left) / (right - left);
		m.x[9] = (top + buttom) / (top - buttom);
		m.x[10] = -(far + near) / (far - near);
		m.x[11] = -1;
		m.x[14] = -2 * far * near / (far - near);
		m.x[15] = 0;

		return m;
	}

	static Matrix4 OrthoMat(float left, float right, float buttom, float top, float near, float far)
	{
		Matrix4 m;

		m.x[0] = 2 / (right - left);
		m.x[5] = 2 / (top - buttom);
		m.x[10] = -2 / (far - near);
		m.x[12] = -(right + left) / (right - left);
		m.x[13] = -(top + buttom) / (top - buttom);
		m.x[14] = -(far + near) / (far - near);

		return m;
	}

	static void fastMult43( Matrix4 &dst, const Matrix4 &m1, const Matrix4 &m2 )
	{
		float *dstx = dst.x;
		const float *m1x = m1.x;
		const float *m2x = m2.x;
		
		dstx[0] = m1x[0] * m2x[0] + m1x[4] * m2x[1] + m1x[8] * m2x[2];
		dstx[1] = m1x[1] * m2x[0] + m1x[5] * m2x[1] + m1x[9] * m2x[2];
		dstx[2] = m1x[2] * m2x[0] + m1x[6] * m2x[1] + m1x[10] * m2x[2];
		dstx[3] = 0.0f;

		dstx[4] = m1x[0] * m2x[4] + m1x[4] * m2x[5] + m1x[8] * m2x[6];
		dstx[5] = m1x[1] * m2x[4] + m1x[5] * m2x[5] + m1x[9] * m2x[6];
		dstx[6] = m1x[2] * m2x[4] + m1x[6] * m2x[5] + m1x[10] * m2x[6];
		dstx[7] = 0.0f;

		dstx[8] = m1x[0] * m2x[8] + m1x[4] * m2x[9] + m1x[8] * m2x[10];
		dstx[9] = m1x[1] * m2x[8] + m1x[5] * m2x[9] + m1x[9] * m2x[10];
		dstx[10] = m1x[2] * m2x[8] + m1x[6] * m2x[9] + m1x[10] * m2x[10];
		dstx[11] = 0.0f;

		dstx[12] = m1x[0] * m2x[12] + m1x[4] * m2x[13] + m1x[8] * m2x[14] + m1x[12] * m2x[15];
		dstx[13] = m1x[1] * m2x[12] + m1x[5] * m2x[13] + m1x[9] * m2x[14] + m1x[13] * m2x[15];
		dstx[14] = m1x[2] * m2x[12] + m1x[6] * m2x[13] + m1x[10] * m2x[14] + m1x[14] * m2x[15];
		dstx[15] = 1.0f;
	}

	Matrix4()
	{
		c[0][0] = 1; c[1][0] = 0; c[2][0] = 0; c[3][0] = 0;
		c[0][1] = 0; c[1][1] = 1; c[2][1] = 0; c[3][1] = 0;
		c[0][2] = 0; c[1][2] = 0; c[2][2] = 1; c[3][2] = 0;
		c[0][3] = 0; c[1][3] = 0; c[2][3] = 0; c[3][3] = 1;
	}

	explicit Matrix4( Math::NoInitHint )
	{
		// 不构造默认单位矩阵
	}

	Matrix4( const float *floatArray16 )
	{
		for( unsigned int i = 0; i < 4; ++i )
		{
			for( unsigned int j = 0; j < 4; ++j )
			{
				c[i][j] = floatArray16[i * 4 + j];
			}
		}
	}

	Matrix4( const Quaternion &q )
	{
		float x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
		float xx = q.x * x2,  xy = q.x * y2,  xz = q.x * z2;
		float yy = q.y * y2,  yz = q.y * z2,  zz = q.z * z2;
		float wx = q.w * x2,  wy = q.w * y2,  wz = q.w * z2;

		c[0][0] = 1 - (yy + zz);  c[1][0] = xy - wz;	   c[2][0] = xz + wy;		 c[3][0] = 0;
		c[0][1] = xy + wz;        c[1][1] = 1 - (xx + zz); c[2][1] = yz - wx;        c[3][1] = 0;
		c[0][2] = xz - wy;        c[1][2] = yz + wx;	   c[2][2] = 1 - (xx + yy);  c[3][2] = 0;
		c[0][3] = 0;              c[1][3] = 0;			   c[2][3] = 0;              c[3][3] = 1;
		
	}

	Matrix4 operator+( const Matrix4 &m ) const 
	{
		Matrix4 mf( Math::NO_INIT );
		
		mf.x[0] = x[0] + m.x[0];
		mf.x[1] = x[1] + m.x[1];
		mf.x[2] = x[2] + m.x[2];
		mf.x[3] = x[3] + m.x[3];
		mf.x[4] = x[4] + m.x[4];
		mf.x[5] = x[5] + m.x[5];
		mf.x[6] = x[6] + m.x[6];
		mf.x[7] = x[7] + m.x[7];
		mf.x[8] = x[8] + m.x[8];
		mf.x[9] = x[9] + m.x[9];
		mf.x[10] = x[10] + m.x[10];
		mf.x[11] = x[11] + m.x[11];
		mf.x[12] = x[12] + m.x[12];
		mf.x[13] = x[13] + m.x[13];
		mf.x[14] = x[14] + m.x[14];
		mf.x[15] = x[15] + m.x[15];

		return mf;
	}

	Matrix4 &operator+=( const Matrix4 &m )
	{
		return *this = *this + m;
	}

	Matrix4 operator*( const Matrix4 &m ) const 
	{
		Matrix4 mf( Math::NO_INIT );
		
		mf.x[0] = x[0] * m.x[0] + x[4] * m.x[1] + x[8] * m.x[2] + x[12] * m.x[3];
		mf.x[1] = x[1] * m.x[0] + x[5] * m.x[1] + x[9] * m.x[2] + x[13] * m.x[3];
		mf.x[2] = x[2] * m.x[0] + x[6] * m.x[1] + x[10] * m.x[2] + x[14] * m.x[3];
		mf.x[3] = x[3] * m.x[0] + x[7] * m.x[1] + x[11] * m.x[2] + x[15] * m.x[3];

		mf.x[4] = x[0] * m.x[4] + x[4] * m.x[5] + x[8] * m.x[6] + x[12] * m.x[7];
		mf.x[5] = x[1] * m.x[4] + x[5] * m.x[5] + x[9] * m.x[6] + x[13] * m.x[7];
		mf.x[6] = x[2] * m.x[4] + x[6] * m.x[5] + x[10] * m.x[6] + x[14] * m.x[7];
		mf.x[7] = x[3] * m.x[4] + x[7] * m.x[5] + x[11] * m.x[6] + x[15] * m.x[7];

		mf.x[8] = x[0] * m.x[8] + x[4] * m.x[9] + x[8] * m.x[10] + x[12] * m.x[11];
		mf.x[9] = x[1] * m.x[8] + x[5] * m.x[9] + x[9] * m.x[10] + x[13] * m.x[11];
		mf.x[10] = x[2] * m.x[8] + x[6] * m.x[9] + x[10] * m.x[10] + x[14] * m.x[11];
		mf.x[11] = x[3] * m.x[8] + x[7] * m.x[9] + x[11] * m.x[10] + x[15] * m.x[11];

		mf.x[12] = x[0] * m.x[12] + x[4] * m.x[13] + x[8] * m.x[14] + x[12] * m.x[15];
		mf.x[13] = x[1] * m.x[12] + x[5] * m.x[13] + x[9] * m.x[14] + x[13] * m.x[15];
		mf.x[14] = x[2] * m.x[12] + x[6] * m.x[13] + x[10] * m.x[14] + x[14] * m.x[15];
		mf.x[15] = x[3] * m.x[12] + x[7] * m.x[13] + x[11] * m.x[14] + x[15] * m.x[15];

		return mf;
	}

	Matrix4 operator*( const float f ) const
	{
		Matrix4 m( *this );
		
		m.x[0]  *= f; m.x[1]  *= f; m.x[2]  *= f; m.x[3]  *= f;
		m.x[4]  *= f; m.x[5]  *= f; m.x[6]  *= f; m.x[7]  *= f;
		m.x[8]  *= f; m.x[9]  *= f; m.x[10] *= f; m.x[11] *= f;
		m.x[12] *= f; m.x[13] *= f; m.x[14] *= f; m.x[15] *= f;

		return m;
	}

	Vec3 operator*( const Vec3 &v ) const
	{
		return Vec3( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + c[3][0],
		              v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + c[3][1],
		              v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + c[3][2] );
	}

	Vec4 operator*( const Vec4 &v ) const
	{
		return Vec4( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + v.w * c[3][0],
		              v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + v.w * c[3][1],
		              v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + v.w * c[3][2],
		              v.x * c[0][3] + v.y * c[1][3] + v.z * c[2][3] + v.w * c[3][3] );
	}

	Vec3 mult33Vec( const Vec3 &v ) const
	{
		return Vec3( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0],
		              v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1],
		              v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] );
	}
	
	// 变换
	void translate( const float x, const float y, const float z )
	{
		*this = TransMat( x, y, z ) * *this;
	}

	void scale( const float x, const float y, const float z )
	{
		*this = ScaleMat( x, y, z ) * *this;
	}

	void rotate( const float x, const float y, const float z )
	{
		*this = RotMat( x, y, z ) * *this;
	}

	// 置换矩阵
	Matrix4 transposed() const
	{
		Matrix4 m( *this );
		
		for( unsigned int y = 0; y < 4; ++y )
		{
			for( unsigned int x = y + 1; x < 4; ++x ) 
			{
				float tmp = m.c[x][y];
				m.c[x][y] = m.c[y][x];
				m.c[y][x] = tmp;
			}
		}
		return m;
	}
	//求矩阵值
	float determinant() const
	{
		return 
			c[0][3]*c[1][2]*c[2][1]*c[3][0] - c[0][2]*c[1][3]*c[2][1]*c[3][0] - c[0][3]*c[1][1]*c[2][2]*c[3][0] + c[0][1]*c[1][3]*c[2][2]*c[3][0] +
			c[0][2]*c[1][1]*c[2][3]*c[3][0] - c[0][1]*c[1][2]*c[2][3]*c[3][0] - c[0][3]*c[1][2]*c[2][0]*c[3][1] + c[0][2]*c[1][3]*c[2][0]*c[3][1] +
			c[0][3]*c[1][0]*c[2][2]*c[3][1] - c[0][0]*c[1][3]*c[2][2]*c[3][1] - c[0][2]*c[1][0]*c[2][3]*c[3][1] + c[0][0]*c[1][2]*c[2][3]*c[3][1] +
			c[0][3]*c[1][1]*c[2][0]*c[3][2] - c[0][1]*c[1][3]*c[2][0]*c[3][2] - c[0][3]*c[1][0]*c[2][1]*c[3][2] + c[0][0]*c[1][3]*c[2][1]*c[3][2] +
			c[0][1]*c[1][0]*c[2][3]*c[3][2] - c[0][0]*c[1][1]*c[2][3]*c[3][2] - c[0][2]*c[1][1]*c[2][0]*c[3][3] + c[0][1]*c[1][2]*c[2][0]*c[3][3] +
			c[0][2]*c[1][0]*c[2][1]*c[3][3] - c[0][0]*c[1][2]*c[2][1]*c[3][3] - c[0][1]*c[1][0]*c[2][2]*c[3][3] + c[0][0]*c[1][1]*c[2][2]*c[3][3];
	}
	//逆矩阵
	Matrix4 inverted() const
	{
		Matrix4 m( Math::NO_INIT );

		float d = determinant();
		if( d == 0 ) return m;
		d = 1.0f / d;
		
		m.c[0][0] = d * (c[1][2]*c[2][3]*c[3][1] - c[1][3]*c[2][2]*c[3][1] + c[1][3]*c[2][1]*c[3][2] - c[1][1]*c[2][3]*c[3][2] - c[1][2]*c[2][1]*c[3][3] + c[1][1]*c[2][2]*c[3][3]);
		m.c[0][1] = d * (c[0][3]*c[2][2]*c[3][1] - c[0][2]*c[2][3]*c[3][1] - c[0][3]*c[2][1]*c[3][2] + c[0][1]*c[2][3]*c[3][2] + c[0][2]*c[2][1]*c[3][3] - c[0][1]*c[2][2]*c[3][3]);
		m.c[0][2] = d * (c[0][2]*c[1][3]*c[3][1] - c[0][3]*c[1][2]*c[3][1] + c[0][3]*c[1][1]*c[3][2] - c[0][1]*c[1][3]*c[3][2] - c[0][2]*c[1][1]*c[3][3] + c[0][1]*c[1][2]*c[3][3]);
		m.c[0][3] = d * (c[0][3]*c[1][2]*c[2][1] - c[0][2]*c[1][3]*c[2][1] - c[0][3]*c[1][1]*c[2][2] + c[0][1]*c[1][3]*c[2][2] + c[0][2]*c[1][1]*c[2][3] - c[0][1]*c[1][2]*c[2][3]);
		m.c[1][0] = d * (c[1][3]*c[2][2]*c[3][0] - c[1][2]*c[2][3]*c[3][0] - c[1][3]*c[2][0]*c[3][2] + c[1][0]*c[2][3]*c[3][2] + c[1][2]*c[2][0]*c[3][3] - c[1][0]*c[2][2]*c[3][3]);
		m.c[1][1] = d * (c[0][2]*c[2][3]*c[3][0] - c[0][3]*c[2][2]*c[3][0] + c[0][3]*c[2][0]*c[3][2] - c[0][0]*c[2][3]*c[3][2] - c[0][2]*c[2][0]*c[3][3] + c[0][0]*c[2][2]*c[3][3]);
		m.c[1][2] = d * (c[0][3]*c[1][2]*c[3][0] - c[0][2]*c[1][3]*c[3][0] - c[0][3]*c[1][0]*c[3][2] + c[0][0]*c[1][3]*c[3][2] + c[0][2]*c[1][0]*c[3][3] - c[0][0]*c[1][2]*c[3][3]);
		m.c[1][3] = d * (c[0][2]*c[1][3]*c[2][0] - c[0][3]*c[1][2]*c[2][0] + c[0][3]*c[1][0]*c[2][2] - c[0][0]*c[1][3]*c[2][2] - c[0][2]*c[1][0]*c[2][3] + c[0][0]*c[1][2]*c[2][3]);
		m.c[2][0] = d * (c[1][1]*c[2][3]*c[3][0] - c[1][3]*c[2][1]*c[3][0] + c[1][3]*c[2][0]*c[3][1] - c[1][0]*c[2][3]*c[3][1] - c[1][1]*c[2][0]*c[3][3] + c[1][0]*c[2][1]*c[3][3]);
		m.c[2][1] = d * (c[0][3]*c[2][1]*c[3][0] - c[0][1]*c[2][3]*c[3][0] - c[0][3]*c[2][0]*c[3][1] + c[0][0]*c[2][3]*c[3][1] + c[0][1]*c[2][0]*c[3][3] - c[0][0]*c[2][1]*c[3][3]);
		m.c[2][2] = d * (c[0][1]*c[1][3]*c[3][0] - c[0][3]*c[1][1]*c[3][0] + c[0][3]*c[1][0]*c[3][1] - c[0][0]*c[1][3]*c[3][1] - c[0][1]*c[1][0]*c[3][3] + c[0][0]*c[1][1]*c[3][3]);
		m.c[2][3] = d * (c[0][3]*c[1][1]*c[2][0] - c[0][1]*c[1][3]*c[2][0] - c[0][3]*c[1][0]*c[2][1] + c[0][0]*c[1][3]*c[2][1] + c[0][1]*c[1][0]*c[2][3] - c[0][0]*c[1][1]*c[2][3]);
		m.c[3][0] = d * (c[1][2]*c[2][1]*c[3][0] - c[1][1]*c[2][2]*c[3][0] - c[1][2]*c[2][0]*c[3][1] + c[1][0]*c[2][2]*c[3][1] + c[1][1]*c[2][0]*c[3][2] - c[1][0]*c[2][1]*c[3][2]);
		m.c[3][1] = d * (c[0][1]*c[2][2]*c[3][0] - c[0][2]*c[2][1]*c[3][0] + c[0][2]*c[2][0]*c[3][1] - c[0][0]*c[2][2]*c[3][1] - c[0][1]*c[2][0]*c[3][2] + c[0][0]*c[2][1]*c[3][2]);
		m.c[3][2] = d * (c[0][2]*c[1][1]*c[3][0] - c[0][1]*c[1][2]*c[3][0] - c[0][2]*c[1][0]*c[3][1] + c[0][0]*c[1][2]*c[3][1] + c[0][1]*c[1][0]*c[3][2] - c[0][0]*c[1][1]*c[3][2]);
		m.c[3][3] = d * (c[0][1]*c[1][2]*c[2][0] - c[0][2]*c[1][1]*c[2][0] + c[0][2]*c[1][0]*c[2][1] - c[0][0]*c[1][2]*c[2][1] - c[0][1]*c[1][0]*c[2][2] + c[0][0]*c[1][1]*c[2][2]);
		
		return m;
	}
	//矩阵分解
	void decompose( Vec3 &trans, Vec3 &rot, Vec3 &scale ) const
	{
		// 位移
		trans = Vec3( c[3][0], c[3][1], c[3][2] );

		// 缩放
		scale.x = sqrtf( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
		scale.y = sqrtf( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
		scale.z = sqrtf( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );

		if( scale.x == 0 || scale.y == 0 || scale.z == 0 ) return;

		// 如果有缩放为负值，反向任意坐标轴
		if( determinant() < 0 ) scale.x = -scale.x;

		// YXZ 组合旋转矩阵  （欧拉旋转）
		// Cos[y]*Cos[z]+Sin[x]*Sin[y]*Sin[z]   Cos[z]*Sin[x]*Sin[y]-Cos[y]*Sin[z]  Cos[x]*Sin[y]	
		// Cos[x]*Sin[z]                        Cos[x]*Cos[z]                       -Sin[x]
		// -Cos[z]*Sin[y]+Cos[y]*Sin[x]*Sin[z]  Cos[y]*Cos[z]*Sin[x]+Sin[y]*Sin[z]  Cos[x]*Cos[y]

		rot.x = asinf( -c[2][1] / scale.z );
		
		// 特殊情况: Cos[x] == 0 (当 Sin[x] == +/-1)
		float f = fabsf( c[2][1] / scale.z );
		if( f > 0.999f && f < 1.001f )
		{
			// 当z或y值变到0时，产生万向死锁
			rot.y = 0;
			
			// 这时 Cos[x] = 0, Sin[x] = +/-1, Cos[y] = 1, Sin[y] = 0
			// => m[0][0] = Cos[z] 且 m[1][0] = -Sin[z]
			rot.z = atan2f( -c[1][0] / scale.y, c[0][0] / scale.x );
		}
		// 一般情况
		else
		{
			rot.y = atan2f( c[2][0] / scale.z, c[2][2] / scale.z );
			rot.z = atan2f( c[0][1] / scale.x, c[1][1] / scale.y );
		}
	}

	
	void setCol( unsigned int col, const Vec4& v ) 
	{
		x[col * 4 + 0] = v.x;
		x[col * 4 + 1] = v.y;
		x[col * 4 + 2] = v.z;
		x[col * 4 + 3] = v.w;
	}

	Vec4 getCol( unsigned int col ) const
	{
		return Vec4( x[col * 4 + 0], x[col * 4 + 1], x[col * 4 + 2], x[col * 4 + 3] );
	}

	Vec4 getRow( unsigned int row ) const
	{
		return Vec4( x[row + 0], x[row + 4], x[row + 8], x[row + 12] );
	}

	Vec3 getTrans() const
	{
		return Vec3( c[3][0], c[3][1], c[3][2] );
	}
	
	Vec3 getScale() const
	{
		Vec3 scale;
		// 矩阵列的值的长代表缩放参数
		scale.x = sqrtf( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
		scale.y = sqrtf( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
		scale.z = sqrtf( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );
		return scale;
	}
};


// 平面

class Plane
{
public:
	Vec3 normal; 
	float dist;

	Plane() 
	{ 
		normal.x = 0; normal.y = 0; normal.z = 0; dist = 0; 
	};

	explicit Plane( const float a, const float b, const float c, const float d )
	{
		normal = Vec3( a, b, c );
		float invLen = 1.0f / normal.length();
		normal *= invLen;	// 归一
		dist = d * invLen;
	}

	Plane( const Vec3 &v0, const Vec3 &v1, const Vec3 &v2 )
	{
		normal = v1 - v0;
		normal = normal.cross( v2 - v0 );
		normal.normalize();
		dist = -normal.dot( v0 );
	}

	//点到线的距离  D=P*N/N  由于这里N归一 |N|=1 => D=P*N+dist 
	float distToPoint( const Vec3 &v ) const
	{
		return normal.dot( v ) + dist;
	}
};


// 交叉算法

inline bool rayTriangleIntersection( const Vec3 &rayOrig, const Vec3 &rayDir, 
                                     const Vec3 &vert0, const Vec3 &vert1, const Vec3 &vert2,
                                     Vec3 &intsPoint )
{
	// 算法引用论文: Tomas Moeller and Ben Trumbore
	//Fast, Minimum Storage Ray/Triangle Intersection 
	
	// 三角形两边向量
	Vec3 edge1 = vert1 - vert0;
	Vec3 edge2 = vert2 - vert0;

	// P = D*E2
	Vec3 pvec = rayDir.cross( edge2 );

	//P*E1
	float det = edge1.dot( pvec );


	// *** Culling branch ***
	/*if( det < Math::Epsilon )return false;

	// Calculate distance from vert0 to ray origin
	Vec3 tvec = rayOrig - vert0;

	// Calculate U parameter and test bounds
	float u = tvec.dot( pvec );
	if (u < 0 || u > det ) return false;

	// Prepare to test V parameter
	Vec3 qvec = tvec.cross( edge1 );

	// Calculate V parameter and test bounds
	float v = rayDir.dot( qvec );
	if (v < 0 || u + v > det ) return false;

	// Calculate t, scale parameters, ray intersects triangle
	float t = edge2.dot( qvec ) / det;*/


	// *** Non-culling branch ***
	if( det > -Math::Epsilon && det < Math::Epsilon ) return 0;
	float inv_det = 1.0f / det;

	// T = O - V0
	Vec3 tvec = rayOrig - vert0;

	// u = T*P*（1/P*E1）
	float u = tvec.dot( pvec ) * inv_det;
	if( u < 0.0f || u > 1.0f ) return 0;

	// Q = T*E1
	Vec3 qvec = tvec.cross( edge1 );

	// v = D*Q*（1/P*E1）
	float v = rayDir.dot( qvec ) * inv_det;
	if( v < 0.0f || u + v > 1.0f ) return 0;

	// t = E2*Q*（1/P*E1）
	float t = edge2.dot( qvec ) * inv_det;


	//计算交点并测试射线长度和方向
	intsPoint = rayOrig + rayDir * t;
	Vec3 vec = intsPoint - rayOrig;
	if( vec.dot( rayDir ) < 0 || vec.length() > rayDir.length() ) return false;

	return true;
}


inline bool rayAABBIntersection( const Vec3 &rayOrig, const Vec3 &rayDir, 
                                 const Vec3 &mins, const Vec3 &maxs )
{
	// SLAB AABB求交算法
	
	float l1 = (mins.x - rayOrig.x) / rayDir.x;
	float l2 =  (maxs.x - rayOrig.x) / rayDir.x;
	float lmin = minf( l1, l2 );
	float lmax = maxf( l1, l2 );

	l1 = (mins.y - rayOrig.y) / rayDir.y;
	l2 = (maxs.y - rayOrig.y) / rayDir.y;
	lmin = maxf( minf( l1, l2 ), lmin );
	lmax = minf( maxf( l1, l2 ), lmax );
		
	l1 = (mins.z - rayOrig.z) / rayDir.z;
	l2 = (maxs.z - rayOrig.z) / rayDir.z;
	lmin = maxf( minf( l1, l2 ), lmin );
	lmax = minf( maxf( l1, l2 ), lmax );

	if( (lmax >= 0.0f) & (lmax >= lmin) )
	{
		// 比较长度
		const Vec3 rayDest = rayOrig + rayDir;
		Vec3 rayMins( minf( rayDest.x, rayOrig.x), minf( rayDest.y, rayOrig.y ), minf( rayDest.z, rayOrig.z ) );
		Vec3 rayMaxs( maxf( rayDest.x, rayOrig.x), maxf( rayDest.y, rayOrig.y ), maxf( rayDest.z, rayOrig.z ) );
		return 
			(rayMins.x < maxs.x) && (rayMaxs.x > mins.x) &&
			(rayMins.y < maxs.y) && (rayMaxs.y > mins.y) &&
			(rayMins.z < maxs.z) && (rayMaxs.z > mins.z);
	}
	else
		return false;
}

//点到AABB的最短距离
inline float nearestDistToAABB( const Vec3 &pos, const Vec3 &mins, const Vec3 &maxs )
{
	const Vec3 center = (mins + maxs) * 0.5f;
	const Vec3 extent = (maxs - mins) * 0.5f;
	
	Vec3 nearestVec;
	nearestVec.x = maxf( 0, fabsf( pos.x - center.x ) - extent.x );
	nearestVec.y = maxf( 0, fabsf( pos.y - center.y ) - extent.y );
	nearestVec.z = maxf( 0, fabsf( pos.z - center.z ) - extent.z );
	
	return nearestVec.length();
}

}
#endif // _utMath_H_
