

#ifndef BASE_VMATH_H
#define BASE_VMATH_H

#include <math.h>
#include "math.h"

// ------------------------------------

template<typename T>
class vector2_base
{
public:
	union { T x,u; };
	union { T y,v; };

	vector2_base() {}
	vector2_base(float nx, float ny)
	{
		x = nx;
		y = ny;
	}

	vector2_base operator -() const { return vector2_base(-x, -y); }
	vector2_base operator -(const vector2_base &v) const { return vector2_base(x-v.x, y-v.y); }
	vector2_base operator +(const vector2_base &v) const { return vector2_base(x+v.x, y+v.y); }
	vector2_base operator *(const T v) const { return vector2_base(x*v, y*v); }
	vector2_base operator *(const vector2_base &v) const { return vector2_base(x*v.x, y*v.y); }
	vector2_base operator /(const T v) const { return vector2_base(x/v, y/v); }
	vector2_base operator /(const vector2_base &v) const { return vector2_base(x/v.x, y/v.y); }

	const vector2_base &operator +=(const vector2_base &v) { x += v.x; y += v.y; return *this; }
	const vector2_base &operator -=(const vector2_base &v) { x -= v.x; y -= v.y; return *this; }
	const vector2_base &operator *=(const T v) { x *= v; y *= v; return *this;	}
	const vector2_base &operator *=(const vector2_base &v) { x *= v.x; y *= v.y; return *this; }
	const vector2_base &operator /=(const T v) { x /= v; y /= v; return *this;	}
	const vector2_base &operator /=(const vector2_base &v) { x /= v.x; y /= v.y; return *this; }

	bool operator ==(const vector2_base &v) const { return x == v.x && y == v.y; } //TODO: do this with an eps instead

	operator const T* () { return &x; }
};


template<typename T>
inline T length(const vector2_base<T> &a)
{
	return sqrtf(a.x*a.x + a.y*a.y);
}

template<typename T>
inline T distance(const vector2_base<T> a, const vector2_base<T> &b)
{
	return length(a-b);
}

template<typename T>
inline T dot(const vector2_base<T> a, const vector2_base<T> &b)
{
	return a.x*b.x + a.y*b.y;
}

template<typename T>
inline vector2_base<T> normalize(const vector2_base<T> &v)
{
	T l = (T)(1.0f/sqrtf(v.x*v.x + v.y*v.y));
	return vector2_base<T>(v.x*l, v.y*l);
}

typedef vector2_base<float> vec2;
typedef vector2_base<bool> bvec2;
typedef vector2_base<int> ivec2;

template<typename T>
inline vector2_base<T> closest_point_on_line(vector2_base<T> line_point0, vector2_base<T> line_point1, vector2_base<T> target_point)
{
	vector2_base<T> c = target_point - line_point0;
	vector2_base<T> v = (line_point1 - line_point0);
	v = normalize(v);
	T d = length(line_point0-line_point1);
	T t = dot(v, c)/d;
	return mix(line_point0, line_point1, clamp(t, (T)0, (T)1));
	/*
	if (t < 0) t = 0;
	if (t > 1.0f) return 1.0f;
	return t;*/
}

inline float nlerp(float a, float b, float amount)
{
	a *= pi/180.0f; b *= pi/180.0f;
	vec2 VecA(cosf(a), sinf(a));
	vec2 VecB(cosf(b), sinf(b));

	vec2 out = normalize(mix(VecA, VecB, amount));
	return (atan2f(out.y, out.x)) * 180.0f/pi;
}

// ------------------------------------
template<typename T>
class vector3_base
{
public:
	union { T x,r,h; };
	union { T y,g,s; };
	union { T z,b,v,l; };

	vector3_base() {}
	vector3_base(float nx, float ny, float nz)
	{
		x = nx;
		y = ny;
		z = nz;
	}

	vector3_base operator -(const vector3_base &v) const { return vector3_base(x-v.x, y-v.y, z-v.z); }
	vector3_base operator -() const { return vector3_base(-x, -y, -z); }
	vector3_base operator +(const vector3_base &v) const { return vector3_base(x+v.x, y+v.y, z+v.z); }
	vector3_base operator *(const T v) const { return vector3_base(x*v, y*v, z*v); }
	vector3_base operator *(const vector3_base &v) const { return vector3_base(x*v.x, y*v.y, z*v.z); }
	vector3_base operator /(const T v) const { return vector3_base(x/v, y/v, z/v); }
	vector3_base operator /(const vector3_base &v) const { return vector3_base(x/v.x, y/v.y, z/v.z); }

	const vector3_base &operator +=(const vector3_base &v) { x += v.x; y += v.y; z += v.z; return *this; }
	const vector3_base &operator -=(const vector3_base &v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	const vector3_base &operator *=(const T v) { x *= v; y *= v; z *= v; return *this;	}
	const vector3_base &operator *=(const vector3_base &v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
	const vector3_base &operator /=(const T v) { x /= v; y /= v; z /= v; return *this;	}
	const vector3_base &operator /=(const vector3_base &v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

	bool operator ==(const vector3_base &v) const { return x == v.x && y == v.y && z == v.z; } //TODO: do this with an eps instead

	operator const T* () { return &x; }
};

template<typename T>
inline T length(const vector3_base<T> &a)
{
	return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}

template<typename T>
inline T distance(const vector3_base<T> &a, const vector3_base<T> &b)
{
	return length(a-b);
}

template<typename T>
inline T dot(const vector3_base<T> &a, const vector3_base<T> &b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

template<typename T>
inline vector3_base<T> normalize(const vector3_base<T> &v)
{
	T l = (T)(1.0f/sqrtf(v.x*v.x + v.y*v.y + v.z*v.z));
	return vector3_base<T>(v.x*l, v.y*l, v.z*l);
}

template<typename T>
inline vector3_base<T> cross(const vector3_base<T> &a, const vector3_base<T> &b)
{
	return vector3_base<T>(
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x);
}

typedef vector3_base<float> vec3;
typedef vector3_base<bool> bvec3;
typedef vector3_base<int> ivec3;

// ------------------------------------

template<typename T>
class vector4_base
{
public:
	union { T x,r; };
	union { T y,g; };
	union { T z,b; };
	union { T w,a; };

	vector4_base() {}
	vector4_base(float nx, float ny, float nz, float nw)
	{
		x = nx;
		y = ny;
		z = nz;
		w = nw;
	}

	vector4_base operator +(const vector4_base &v) const { return vector4_base(x+v.x, y+v.y, z+v.z, w+v.w); }
	vector4_base operator -(const vector4_base &v) const { return vector4_base(x-v.x, y-v.y, z-v.z, w-v.w); }
	vector4_base operator -() const { return vector4_base(-x, -y, -z, -w); }
	vector4_base operator *(const vector4_base &v) const { return vector4_base(x*v.x, y*v.y, z*v.z, w*v.w); }
	vector4_base operator *(const T v) const { return vector4_base(x*v, y*v, z*v, w*v); }
	vector4_base operator /(const vector4_base &v) const { return vector4_base(x/v.x, y/v.y, z/v.z, w/v.w); }
	vector4_base operator /(const T v) const { return vector4_base(x/v, y/v, z/v, w/v); }

	const vector4_base &operator +=(const vector4_base &v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
	const vector4_base &operator -=(const vector4_base &v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	const vector4_base &operator *=(const T v) { x *= v; y *= v; z *= v; w *= v; return *this;	}
	const vector4_base &operator *=(const vector4_base &v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
	const vector4_base &operator /=(const T v) { x /= v; y /= v; z /= v; w /= v; return *this;	}
	const vector4_base &operator /=(const vector4_base &v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }

	bool operator ==(const vector4_base &v) const { return x == v.x && y == v.y && z == v.z && w == v.w; } //TODO: do this with an eps instead

	operator const T* () { return &x; }
};

typedef vector4_base<float> vec4;
typedef vector4_base<bool> bvec4;
typedef vector4_base<int> ivec4;

// ------------------------------------

template<typename T>
class matrix22_base
{
public:
	T m00, m01;
	T m10, m11;

	matrix22_base() {}
	matrix22_base(T n00, T n01, T n10, T n11)
	{
		m00 = n00;
		m01 = n01;
		m10 = n10;
		m11 = n11;
	}

	matrix22_base operator +(const matrix22_base &m) const { return matrix22_base(m00+m.m00, m01+m.m01, m10+m.m10, m11+m.m11); }
	matrix22_base operator -(const matrix22_base &m) const { return matrix22_base(m00-m.m00, m01-m.m01, m10-m.m10, m11-m.m11); }
	matrix22_base operator -() const { return matrix22_base(-m00, -m01, -m10, -m11); }
	matrix22_base operator *(const T s) const { return matrix22_base(s*m00, s*m01, s*m10, s*m11); }
	vector2_base<T> operator *(const vector2_base<T> &v) const { return vector2_base<T>(m00*v.x + m01*v.y, m10*v.x + m11*v.y); }
	matrix22_base operator *(const matrix22_base &m) const
	{
		return matrix22_base(
				m00*m.m00 + m01*m.m10, m00*m.m01 + m01*m.m11,
				m10*m.m00 + m11*m.m10, m10*m.m01 + m11*m.m11
			);
	}

	const matrix22_base &operator =(const matrix22_base &m) { m00 = m.m00; m01 = m.m01; m10 = m.m10; m11 = m.m11; return *this; }

	const matrix22_base &operator +=(const matrix22_base &m) { m00 += m.m00; m01 += m.m01; m10 += m.m10; m11 += m.m11; return *this; }
	const matrix22_base &operator -=(const matrix22_base &m) { m00 -= m.m00; m01 -= m.m01; m10 -= m.m10; m11 -= m.m11; return *this; }
	const matrix22_base &operator *=(const T &s) { m00 *= s; m01 *= s; m10 *= s; m11 *= s; return *this; }
	const matrix22_base &operator *=(const matrix22_base &m)
	{
		m00 = m00*m.m00 + m01*m.m10;
		m01 = m00*m.m01 + m01*m.m11;
		m10 = m10*m.m00 + m11*m.m10;
		m11 = m10*m.m01 + m11*m.m11;
		return *this;
	}

	bool operator ==(const matrix22_base &m) const { return m00 = m.m00 && m01 = m.m01 && m10 = m.m10 && m11 = m.m11; }


	operator const T* () { return &m00; }
};

typedef matrix22_base<float> mat2;

// ------------------------------------
template<typename T>
class matrix33_base
{
public:
	T m00, m01, m02;
	T m10, m11, m12;
	T m20, m21, m22;

	matrix33_base() {}
	matrix33_base(
		T n00, T n01, T n02, 
		T n10, T n11, T n12,
		T n20, T n21, T n22)
	{
		m00 = n00; m01 = n01; m02 = n02;
		m10 = n10; m11 = n11; m12 = n12;
		m20 = n20; m21 = n21; m22 = n22;
	}

	static
	matrix33_base<T> identity()
	{
		return matrix33_base<T>(
				T(1), 0, 0,
				0, T(1), 0,
				0, 0, T(1)
			);
	}

	vector3_base<T> operator *(const vector3_base<T> &v) const
	{
		return vector3_base<T>(
				m00*v.x + m01*v.y + m02*v.z,
				m10*v.x + m11*v.y + m12*v.z,
				m20*v.x + m21*v.y + m22*v.z
			);
	}

	matrix33_base<T> operator *(const matrix33_base<T> &m) const
	{
		return matrix33_base<T>(
				m00*m.m00 + m01*m.m10 + m02*m.m20, m00*m.m01 + m01*m.m11 + m02*m.m21, m00*m.m02 + m01*m.m12 + m02*m.m22,
				m10*m.m00 + m11*m.m10 + m12*m.m20, m10*m.m01 + m11*m.m11 + m12*m.m21, m10*m.m02 + m11*m.m12 + m12*m.m22,
				m20*m.m00 + m21*m.m10 + m22*m.m20, m20*m.m01 + m21*m.m11 + m22*m.m21, m20*m.m02 + m21*m.m12 + m22*m.m22
			);
	}

	const matrix33_base &operator =(const matrix33_base &m)
	{
		m00 = m.m00; m01 = m.m01; m02 = m.m02;
		m10 = m.m10; m11 = m.m11; m12 = m.m12;
		m20 = m.m20; m21 = m.m21; m22 = m.m22;
		return *this;
	}

	operator const T* () { return &m00; }
};

typedef matrix33_base<float> mat33;

inline float mix_angle(float a, float b, float amount)
{
	float diff = b - a;
	if(diff > 180.0f)
		a += 360.0f;
	if(diff < -180.0f)
		a -= 360.0f;
	return mix(a, b, amount);
}

#endif
