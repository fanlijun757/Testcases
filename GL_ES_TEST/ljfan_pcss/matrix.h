/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009 - 2011 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

void rotate_matrix(double angle, double x, double y, double z, float *R);
void perspective_matrix(double fovy, double aspect, double znear, double zfar, float *P);
void multiply_matrix(float *A, float *B, float *C);
void invert4(float* mInv, float* m);
void invert3(float* mInv, float* m);
void object2noraml(float* mInv, float* m);
void setIdentity(float* sm);
void setTranslate(float* m, float x, float y, float z);
void setScaling(float* m, float x, float y, float z);
void ortho(float* m, float l, float r, float t, float b, float n, float f);
void setLookAt(float* rm, float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ);


#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif /* M_PI */

#endif
