#ifndef _VEC3_H_
#define _VEC3_H_

#include "math.h"

template<class T>
class Vec3 {
public:
	union {
		T arr[3];
		struct { T x, y, z; };
		struct { T r, g, b; };
	};

	Vec3() { }
	Vec3(T xc, T yc, T zc) : x(xc), y(yc), z(zc) { }
	Vec3(const T a[3]){ arr[0]=a[0]; arr[1]=a[1]; arr[2]=a[2]; }

	void set(T xc, T yc, T zc) { x = xc; y = yc; z = zc; }
	void set(const T a[3]) { arr[0]=a[0]; arr[1]=a[1]; arr[2]=a[2]; }
	void set(const Vec3 &v){ arr[0]=v.x; arr[1]=v.y; arr[2]=v.z; }
	
	T len(void) const { return (T) sqrt(x * x + y * y + z * z); }
	T lenSq(void) const { return x * x + y * y + z * z; }
	
	Vec3<T>& normalize(void) { T invLen = T(1.0) / this->len(); x *= invLen; y *= invLen; z *= invLen; return *this; }
	Vec3<T> getNormalized(void) { Vec3<T> c(*this); return c.normalize(); }         
	bool normalizeIf(void);
	
	Vec3<T> operator+(const Vec3 &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); } 
	void operator+=(const Vec3 &v) { x += v.x; y += v.y; z += v.z; }


	Vec3<T> operator-(const Vec3 &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
	void operator-=(const Vec3 &v) { x -= v.x; y -= v.y; z -= v.z; }
	
	Vec3<T> operator-(void) const { return Vec3<T>(-x, -y, -z); }
	void flip(void) { x = -x; y = -y; z = -z;  }

	//component wise vector multiplication
	Vec3<T> operator*(const Vec3& v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }

	Vec3<T> operator*(T t) const { return Vec3<T>(x * t, y * t, z * t); }
	void operator*=(T t)  { x *= t; y *= t; z *= t; }
	
	T dot(const Vec3 &v) const { return x * v.x + y * v.y + z * v.z; }
	Vec3<T> cross(const Vec3 &v) const { return Vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
	
	Vec3<T> multCoordwise(const Vec3 &v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
};

template<class T>
bool Vec3<T>::normalizeIf(void) { 
	T length = len(); 
	
	if (length > 0.0000001) {
		T invLen = T(1.0) / length;
		x *= invLen; 
		y *= invLen; 
		z *= invLen;
		return true;
	} else {
		return false;
	}
}

typedef Vec3<float> Vec3f;

#endif