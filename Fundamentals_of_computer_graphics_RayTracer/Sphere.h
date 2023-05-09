#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "Vec3.h"
#include "Ray.h"

struct Material {
	Vec3f ambient;
	Vec3f diffuse;
	Vec3f specular;
	float shininess;
	float reflectiveness;
};

struct LightPoint {
	Vec3f position;
	Vec3f ambient;
	Vec3f diffuse;
	Vec3f specular;
};

class Sphere {
public:
	Vec3f c;
	float r;
	Material mat;
public:
	Sphere(const Vec3f & cen, float rad, Material material) : c(cen), r(rad), mat(material) { }

	bool hit(const Ray & r, HitRec & rec) const;
	void computeSurfaceHitFields(const Ray & r, HitRec & rec) const;

};

#endif