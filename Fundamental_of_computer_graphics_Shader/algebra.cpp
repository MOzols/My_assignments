#define _USE_MATH_DEFINES // To get M_PI defined
#include <math.h>
#include <stdio.h>
#include "algebra.h"


Vector Add(Vector a, Vector b) {
	Vector v = { a.x + b.x, a.y + b.y, a.z + b.z };
	return v;
}
Vector CrossProduct(Vector a, Vector b) {
	Vector v = { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
	return v;
}
float DotProduct(Vector a, Vector b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}
Vector Homogenize(HomVector h) {
	Vector a;
	if (h.w == 0.0) {
		fprintf(stderr, "Homogenize: w = 0\n");
		a.x = a.y = a.z = 9999999;
		return a;
	}
	a.x = h.x / h.w;
	a.y = h.y / h.w;
	a.z = h.z / h.w;
	return a;
}
float Length(Vector a) {
	return (float)sqrt((double)a.x * (double)a.x + (double)a.y * (double)a.y + (double)a.z * (double)a.z);
}
Matrix MatMatMul(Matrix a, Matrix b) {
	Matrix c;
	int i, j, k;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			c.e[j * 4 + i] = 0.0;
			for (k = 0; k < 4; k++)
				c.e[j * 4 + i] += a.e[k * 4 + i] * b.e[j * 4 + k];
		}
	}
	return c;
}
HomVector MatVecMul(Matrix a, Vector b) {
	HomVector h;
	h.x = b.x * a.e[0] + b.y * a.e[4] + b.z * a.e[8] + a.e[12];
	h.y = b.x * a.e[1] + b.y * a.e[5] + b.z * a.e[9] + a.e[13];
	h.z = b.x * a.e[2] + b.y * a.e[6] + b.z * a.e[10] + a.e[14];
	h.w = b.x * a.e[3] + b.y * a.e[7] + b.z * a.e[11] + a.e[15];
	return h;
}
Vector Normalize(Vector a) {
	float len = Length(a);
	Vector v = { a.x / len, a.y / len, a.z / len };
	return v;
}
void PrintHomVector(char* name, HomVector a) {
	printf("%s: %6.5lf %6.5lf %6.5lf %6.5lf\n", name, a.x, a.y, a.z, a.w);
}
void PrintMatrix(char* name, Matrix a) {
	int i, j;

	printf("%s:\n", name);
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf("%6.5lf ", a.e[j * 4 + i]);
		}
		printf("\n");
	}
}
void PrintVector(char* name, Vector a) {
	printf("%s: %6.5lf %6.5lf %6.5lf\n", name, a.x, a.y, a.z);
}
Vector ScalarVecMul(float t, Vector a) {
	Vector b = { t * a.x, t * a.y, t * a.z };
	return b;
}
Vector Subtract(Vector a, Vector b) {
	Vector v = { a.x-b.x, a.y-b.y, a.z-b.z };
	return v;
}    

//Assignment functions
//Calculates cotangens with help of math.h functions cos() and sin()
double cotan(double angle)
{
	double radians = angle * M_PI / 180.0;
	double c = cos(radians);
	double s = sin(radians);

	return c/s;
}
//Creates and returns perspective projection made with fov matrix
Matrix FovProjection(double fovy, double aspect, double near, double far)
{
	Matrix FOV;
	double cot = cotan(fovy / 2);

	FOV.e[0] = cot/aspect;		FOV.e[4] = 0.0f;	FOV.e[8] = 0.0f;					   FOV.e[12] = 0.0f;
	FOV.e[1] = 0.0f;			FOV.e[5] = cot;		FOV.e[9] = 0.0f;					   FOV.e[13] = 0.0f;
	FOV.e[2] = 0.0f;			FOV.e[6] = 0.0f;	FOV.e[10] = (far + near) / (near-far); FOV.e[14] = (2 * far * near) / (near-far);
	FOV.e[3] = 0.0f;			FOV.e[7] = 0.0f;	FOV.e[11] = -1.0f;					   FOV.e[15] = 0.0f;

	return FOV;
}
//Creates and returns perspective projection made with frustum matrix
Matrix FrustumProjection(float left, float right, float bottom, float top, float near, float far)
{
	Matrix Fru;

	Fru.e[0] = (2.0f * near) / (right - left); Fru.e[4] = 0.0f;							  Fru.e[8] = (right + left) / (right - left); Fru.e[12] = 0.0f;
	Fru.e[1] = 0.0f;						   Fru.e[5] = (2.0f * near) / (top - bottom); Fru.e[9] = (top + bottom) / (top - bottom); Fru.e[13] = 0.0f;
	Fru.e[2] = 0.0f;						   Fru.e[6] = 0.0f;							  Fru.e[10] = -((far + near) / (far - near)); Fru.e[14] = -((2.0f * far * near) / (far - near));
	Fru.e[3] = 0.0f;						   Fru.e[7] = 0.0f;							  Fru.e[11] = -1.0f;						  Fru.e[15] = 0.0f;

	return Fru;

}
//Creates and returns parallel projection made with orthogonal matrix
Matrix OrthogonalProjection(float left, float right, float bottom, float top, float near, float far)
{
	Matrix Ort;

	Ort.e[0] = 2.0f / (right - left); Ort.e[4] = 0.0f;					Ort.e[8] = 0.0f;				 Ort.e[12] = -((right + left) / (right - left));
	Ort.e[1] = 0.0f;				  Ort.e[5] = 2.0f / (top - bottom);	Ort.e[9] = 0.0f;				 Ort.e[13] = -((top + bottom) / (top - bottom));
	Ort.e[2] = 0.0f;				  Ort.e[6] = 0.0f;					Ort.e[10] = 2.0f / (near - far); Ort.e[14] = -((far + near) / (far - near));
	Ort.e[3] = 0.0f;				  Ort.e[7] = 0.0f;					Ort.e[11] = 0.0f;		 		 Ort.e[15] = 1.0f;

	return Ort;
}
//Allows to rotate camera or model relative to x axel
Matrix Rotate_x(float x_rotation)
{
	Matrix Rx;
	double radians = x_rotation * M_PI / 180.0;
	double c = cos(radians);
	double s = sin(radians);

	Rx.e[0] = 1.0f; Rx.e[4] = 0.0f;	Rx.e[8] = 0.0f;	 Rx.e[12] = 0.0f;
	Rx.e[1] = 0.0f; Rx.e[5] = c;	Rx.e[9] = -s;    Rx.e[13] = 0.0f;
	Rx.e[2] = 0.0f; Rx.e[6] = s;	Rx.e[10] = c;    Rx.e[14] = 0.0f;
	Rx.e[3] = 0.0f; Rx.e[7] = 0.0f;	Rx.e[11] = 0.0f; Rx.e[15] = 1.0f;

	return Rx;
}
//Allows to rotate camera or model relative to y axel
Matrix Rotate_y(float y_rotation)
{
	Matrix Ry;
	double radians = y_rotation * M_PI / 180.0;
	double c = cos(radians);
	double s = sin(radians);

	Ry.e[0] = c;	Ry.e[4] = 0.0f; Ry.e[8] = s;	 Ry.e[12] = 0.0f;
	Ry.e[1] = 0.0f;	Ry.e[5] = 1.0f; Ry.e[9] = 0.0f;	 Ry.e[13] = 0.0f;
	Ry.e[2] = -s;	Ry.e[6] = 0.0f; Ry.e[10] = c;	 Ry.e[14] = 0.0f;
	Ry.e[3] = 0.0f;	Ry.e[7] = 0.0f; Ry.e[11] = 0.0f; Ry.e[15] = 1.0f;

	return Ry;
}
//Allows to rotate camera or model relative to z axel
Matrix Rotate_z(float z_rotation)
{
	Matrix Rz;
	double radians = z_rotation * M_PI / 180.0;
	double c = cos(radians);
	double s = sin(radians);

	Rz.e[0] = c;	Rz.e[4] = -s;	Rz.e[8] = 0.0f;  Rz.e[12] = 0.0f;
	Rz.e[1] = s;	Rz.e[5] = c;	Rz.e[9] = 0.0f;  Rz.e[13] = 0.0f;
	Rz.e[2] = 0.0f; Rz.e[6] = 0.0f; Rz.e[10] = 1.0f; Rz.e[14] = 0.0f;
	Rz.e[3] = 0.0f; Rz.e[7] = 0.0f; Rz.e[11] = 0.0f; Rz.e[15] = 1.0f;

	return Rz;
}
//Allows to scale model
Matrix Scale(Vector p)
{
	Matrix S;

	S.e[0] = p.x;  S.e[4] = 0.0f; S.e[8] = 0.0f;  S.e[12] = 0.0f;
	S.e[1] = 0.0f; S.e[5] = p.y;  S.e[9] = 0.0f;  S.e[13] = 0.0f;
	S.e[2] = 0.0f; S.e[6] = 0.0f; S.e[10] = p.z;  S.e[14] = 0.0f;
	S.e[3] = 0.0f; S.e[7] = 0.0f; S.e[11] = 0.0f; S.e[15] = 1.0f;

	return S;
}
//Allows to to move camera or model
Matrix Translate(Vector p)
{
	Matrix T;

	T.e[0] = 1.0f; T.e[4] = 0.0f; T.e[8] = 0.0f;  T.e[12] = p.x;
	T.e[1] = 0.0f; T.e[5] = 1.0f; T.e[9] = 0.0f;  T.e[13] = p.y;
	T.e[2] = 0.0f; T.e[6] = 0.0f; T.e[10] = 1.0f; T.e[14] = p.z;
	T.e[3] = 0.0f; T.e[7] = 0.0f; T.e[11] = 0.0f; T.e[15] = 1.0f;

	return T;
}
