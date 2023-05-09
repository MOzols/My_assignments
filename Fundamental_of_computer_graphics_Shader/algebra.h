#ifndef _ALGEBRA_H_
#define _ALGEBRA_H_

typedef struct { float x, y, z; } Vector;
typedef struct { float x, y, z, w; } HomVector;

/* Column-major order are used for the matrices here to be compatible with OpenGL.
** The indices used to access elements in the matrices are shown below.
**  _                _
** |                  |
** |   0   4   8  12  |
** |                  |
** |   1   5   9  13  |
** |                  |
** |   2   6  10  14  |
** |                  |
** |   3   7  11  15  |
** |_                _|
*/
typedef struct matrix { float e[16]; } Matrix;


Vector Add(Vector a, Vector b);
Vector CrossProduct(Vector a, Vector b);
float DotProduct(Vector a, Vector b);
Vector Homogenize(HomVector a);
float Length(Vector a);
Matrix MatMatMul(Matrix a, Matrix b);
HomVector MatVecMul(Matrix a, Vector b);
Vector Normalize(Vector a);
void PrintHomVector(char* name, HomVector h);
void PrintMatrix(char* name, Matrix m);
void PrintVector(char* name, Vector v);
Vector ScalarVecMul(float t, Vector a);
Vector Subtract(Vector a, Vector b);
//Assignment functions
double cotan(double angle);
Matrix FovProjection(double fovy, double aspect, double near, double far);
Matrix FrustumProjection(float left, float right, float bottom, float top, float near, float far);
Matrix OrthogonalProjection(float left, float right, float bottom, float top, float near, float far);
Matrix Rotate_x(float x_rotation);
Matrix Rotate_y(float y_rotation);
Matrix Rotate_z(float z_rotation);
Matrix Scale(Vector p);
Matrix Translate(Vector p);


#endif

