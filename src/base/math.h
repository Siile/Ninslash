

#ifndef BASE_MATH_H
#define BASE_MATH_H

#include <stdlib.h>

const float invsqrt2 = 0.7071067811865475244f;

template <typename T>
inline T clamp(T val, T min, T max)
{
	if(val < min)
		return min;
	if(val > max)
		return max;
	return val;
}

inline float sign(float f)
{
	return f<0.0f?-1.0f:1.0f;
}

inline int round_to_int(float f)
{
	if(f > 0)
		return (int)(f+0.5f);
	return (int)(f-0.5f);
}

template<typename T, typename TB>
inline T mix(const T a, const T b, TB amount)
{
	return a + (b-a)*amount;
}

inline float frandom() { return rand()/(float)(RAND_MAX); }

// float to fixed
inline int f2fx(float v) { return (int)(v*(float)(1<<10)); }
inline float fx2f(int v) { return v*(1.0f/(1<<10)); }

inline int gcd(int a, int b)
{
	while(b != 0)
	{
		int c = a % b;
		a = b;
		b = c;
	}
	return a;
}

class fxp
{
	int value;
public:
	void set(int v) { value = v; }
	int get() const { return value; }
	fxp &operator = (int v) { value = v<<10; return *this; }
	fxp &operator = (float v) { value = (int)(v*(float)(1<<10)); return *this; }
	operator float() const { return value/(float)(1<<10); }
};

const float pi = 3.1415926535897932384626433f;

template <typename T> inline T min(T a, T b) { return a<b?a:b; }
template <typename T> inline T max(T a, T b) { return a>b?a:b; }
template <typename T> inline T absolute(T a) { return a<T(0)?-a:a; }

// MapGen: Perlin Noise
inline double fade(double t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

inline double lerp(double t, double a, double b)
{
    return a + t * (b - a);
}

inline double grad(int hash, double x, double y, double z)
{
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

#endif // BASE_MATH_H
