#ifndef DISPLAY_MATRIX_H
#define DISPLAY_MATRIX_H

typedef struct{
	float x;
	float y;
	float z;
}vector;

typedef struct{
	vector a;
	vector b;
	vector c;
}matrix;



void multiplicateVectorsScalar(vector * a, float b, vector* res);
float multiplicateVectors(vector * a, vector* b);
void multiplateMatrixVector(matrix* m, vector* v, vector * res);
void rotateZ_vector(vector * v, float angle, vector * res);
void rotateX_vector(vector * v, float angle, vector * res);
void rotateY_vector(vector * v, float angle, vector * res);
void copyVector(vector* src, vector* dest);


#endif
