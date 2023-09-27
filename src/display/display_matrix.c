#include "display_matrix.h"
#include <math.h>
#include <string.h>

matrix orthogonal_projection ={ {1,0,0},
								{0,1,0},
								{0,0,1}};

void multiplicateVectorsScalar(vector * a, float b, vector* res) {

	res->x = a->x * b;
	res->y = a->y * b;
	res->z = a->z * b;
}

float multiplicateVectors(vector * a, vector* b) {

	float result = 0;

	result = (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
	return result;
}

void multiplateMatrixVector(matrix* m, vector* v, vector * res) {
	res->x = multiplicateVectors(&m->a, v);
	res->y = multiplicateVectors(&m->b, v);
	res->z = multiplicateVectors(&m->c, v);
}

void rotateZ_vector(vector * v, float angle, vector * res) {

	matrix rotate = { 	{ cosf(angle), -sinf(angle), 0 },
						{ sinf(angle),  cosf(angle), 0 },
						{           0,            0, 1 }};

	multiplateMatrixVector(&rotate, v, res);

}

void rotateX_vector(vector * v, float angle, vector * res) {

	matrix rotate = { 	{ 1,           0,            0 },
						{ 0, cosf(angle), -sinf(angle) },
						{ 0, sinf(angle),  cosf(angle) }};

	multiplateMatrixVector(&rotate, v, res);

}

void rotateY_vector(vector * v, float angle, vector * res) {

	matrix rotate = { 	{  cosf(angle), 0, sinf(angle) },
						{            0, 1,           0 },
						{ -sinf(angle), 0, cosf(angle) }};

	multiplateMatrixVector(&rotate, v, res);

}

void copyVector(vector* src, vector* dest) {
	memcpy((void*)dest, (void*)src, sizeof(vector));
}
