
#include "Sphere.h"


bool Sphere::hit(const Ray & r, HitRec & rec) const {	
	
	Vec3f v = c - r.o;
	float s = v.dot(r.d);
	float vLenSq = v.dot(v);
	float radSq = this->r * this->r; 
	if (s < 0 && vLenSq > radSq) {
		return false;
	}
	float mSq = vLenSq - s * s;
	if (mSq > radSq) { // no hit
		return false;
	}

	// calculate closest hit
	float thc = sqrt((double) radSq - mSq);

	float t0 = s - thc;
	float t1 = s + thc;

	if (t0 < t1 && t0 >= 0) {
		rec.tHit = t0;
	}
	else if(t1 >= 0) {
		rec.tHit = t1;
	}
	else {
		return false;
	}

	rec.anyHit = true;
	return true;
}


void Sphere::computeSurfaceHitFields(const Ray & r, HitRec & rec) const {
	rec.p = r.o + r.d * rec.tHit;
	rec.n = (rec.p - c).normalize();
}
