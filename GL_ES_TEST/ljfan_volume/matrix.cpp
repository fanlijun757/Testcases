/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009 - 2011 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/*
 * matrix.c
 * Matrix manipulation functions.
 */

#include "matrix.h"

int Matrix_Inv(float *r,float *m,int n)
{
	 int i,j,k,l;
	 float max,tmp,t;

	 /* identitée dans r */
	 for(i=0;i<n*n;i++) r[i]=0;
	 for(i=0;i<n;i++) r[i*n+i]=1;
	 
	 for(j=0;j<n;j++) {
			
			/* recherche du nombre de plus grand module sur la colonne j */
			max=m[j*n+j];
			k=j;
			for(i=j+1;i<n;i++)
				if (fabs(m[i*n+j])>fabs(max)) {
					 k=i;
					 max=m[i*n+j];
				}

      /* non intersible matrix */
      if (max==0) return 1;

			
			/* permutation des lignes j et k */
			if (k!=j) {
				 for(i=0;i<n;i++) {
						tmp=m[j*n+i];
						m[j*n+i]=m[k*n+i];
						m[k*n+i]=tmp;
						
						tmp=r[j*n+i];
						r[j*n+i]=r[k*n+i];
						r[k*n+i]=tmp;
				 }
			}
			
			/* multiplication de la ligne j par 1/max */
			max=1/max;
			for(i=0;i<n;i++) {
				 m[j*n+i]*=max;
				 r[j*n+i]*=max;
			}
			
			
			for(l=0;l<n;l++) if (l!=j) {
				 t=m[l*n+j];
				 for(i=0;i<n;i++) {
						m[l*n+i]-=m[j*n+i]*t;
						r[l*n+i]-=r[j*n+i]*t;
				 }
			}
	 }

	 return 0;
}
/* 
 * Simulates desktop's glRotatef. The matrix is returned in column-major 
 * order. 
 */
void rotate_matrix(double angle, double x, double y, double z, float *R) {
    double radians, c, s, c1, u[3], length;
    int i, j;

    radians = (angle * M_PI) / 180.0;

    c = cos(radians);
    s = sin(radians);

    c1 = 1.0 - cos(radians);

    length = sqrt(x * x + y * y + z * z);

    u[0] = x / length;
    u[1] = y / length;
    u[2] = z / length;

    for (i = 0; i < 16; i++) {
        R[i] = 0.0;
    }

    R[15] = 1.0;

    for (i = 0; i < 3; i++) {
        R[i * 4 + (i + 1) % 3] = u[(i + 2) % 3] * s;
        R[i * 4 + (i + 2) % 3] = -u[(i + 1) % 3] * s;
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            R[i * 4 + j] += c1 * u[i] * u[j] + (i == j ? c : 0.0);
        }
    }
}

/* 
 * Simulates gluPerspectiveMatrix 
 */
void perspective_matrix(double fovy, double aspect, double znear, double zfar, float *P) {
    int i;
    double f;

    f = 1.0/tan(fovy * 0.5);

    for (i = 0; i < 16; i++) {
        P[i] = 0.0;
    }

    P[0] = f / aspect;
    P[5] = f;
    P[10] = (znear + zfar) / (znear - zfar);
    P[11] = -1.0;
    P[14] = (2.0 * znear * zfar) / (znear - zfar);
    P[15] = 0.0;
}

/* 
 * Multiplies A by B and writes out to C. All matrices are 4x4 and column
 * major. In-place multiplication is supported.
 */
void multiply_matrix(float *A, float *B, float *C) {
	int i, j, k;
    float aTmp[16];

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            aTmp[j * 4 + i] = 0.0;

            for (k = 0; k < 4; k++) {
                aTmp[j * 4 + i] += A[k * 4 + i] * B[j * 4 + k];
            }
        }
    }

    for (i = 0; i < 16; i++) {
        C[i] = aTmp[i];
    }
}

void invert4(float* mInv, float* m) {
	int mOffset = 0;
	int mInvOffset = 0;

    // Invert a 4 x 4 matrix using Cramer's Rule

    // transpose matrix
    float src0  = m[mOffset +  0];
    float src4  = m[mOffset +  1];
    float src8  = m[mOffset +  2];
    float src12 = m[mOffset +  3];

    float src1  = m[mOffset +  4];
    float src5  = m[mOffset +  5];
    float src9  = m[mOffset +  6];
    float src13 = m[mOffset +  7];

    float src2  = m[mOffset +  8];
    float src6  = m[mOffset +  9];
    float src10 = m[mOffset + 10];
    float src14 = m[mOffset + 11];

    float src3  = m[mOffset + 12];
    float src7  = m[mOffset + 13];
    float src11 = m[mOffset + 14];
    float src15 = m[mOffset + 15];

    // calculate pairs for first 8 elements (cofactors)
    float atmp0  = src10 * src15;
    float atmp1  = src11 * src14;
    float atmp2  = src9  * src15;
    float atmp3  = src11 * src13;
    float atmp4  = src9  * src14;
    float atmp5  = src10 * src13;
    float atmp6  = src8  * src15;
    float atmp7  = src11 * src12;
    float atmp8  = src8  * src14;
    float atmp9  = src10 * src12;
    float atmp10 = src8  * src13;
    float atmp11 = src9  * src12;

    // calculate first 8 elements (cofactors)
    float dst0  = (atmp0 * src5 + atmp3 * src6 + atmp4  * src7)
                      - (atmp1 * src5 + atmp2 * src6 + atmp5  * src7);
    float dst1  = (atmp1 * src4 + atmp6 * src6 + atmp9  * src7)
                      - (atmp0 * src4 + atmp7 * src6 + atmp8  * src7);
    float dst2  = (atmp2 * src4 + atmp7 * src5 + atmp10 * src7)
                      - (atmp3 * src4 + atmp6 * src5 + atmp11 * src7);
    float dst3  = (atmp5 * src4 + atmp8 * src5 + atmp11 * src6)
                      - (atmp4 * src4 + atmp9 * src5 + atmp10 * src6);
    float dst4  = (atmp1 * src1 + atmp2 * src2 + atmp5  * src3)
                      - (atmp0 * src1 + atmp3 * src2 + atmp4  * src3);
    float dst5  = (atmp0 * src0 + atmp7 * src2 + atmp8  * src3)
                      - (atmp1 * src0 + atmp6 * src2 + atmp9  * src3);
    float dst6  = (atmp3 * src0 + atmp6 * src1 + atmp11 * src3)
                      - (atmp2 * src0 + atmp7 * src1 + atmp10 * src3);
    float dst7  = (atmp4 * src0 + atmp9 * src1 + atmp10 * src2)
                      - (atmp5 * src0 + atmp8 * src1 + atmp11 * src2);

    // calculate pairs for second 8 elements (cofactors)
    float btmp0  = src2 * src7;
    float btmp1  = src3 * src6;
    float btmp2  = src1 * src7;
    float btmp3  = src3 * src5;
    float btmp4  = src1 * src6;
    float btmp5  = src2 * src5;
    float btmp6  = src0 * src7;
    float btmp7  = src3 * src4;
    float btmp8  = src0 * src6;
    float btmp9  = src2 * src4;
    float btmp10 = src0 * src5;
    float btmp11 = src1 * src4;

    // calculate second 8 elements (cofactors)
    float dst8  = (btmp0  * src13 + btmp3  * src14 + btmp4  * src15)
                      - (btmp1  * src13 + btmp2  * src14 + btmp5  * src15);
    float dst9  = (btmp1  * src12 + btmp6  * src14 + btmp9  * src15)
                      - (btmp0  * src12 + btmp7  * src14 + btmp8  * src15);
    float dst10 = (btmp2  * src12 + btmp7  * src13 + btmp10 * src15)
                      - (btmp3  * src12 + btmp6  * src13 + btmp11 * src15);
    float dst11 = (btmp5  * src12 + btmp8  * src13 + btmp11 * src14)
                      - (btmp4  * src12 + btmp9  * src13 + btmp10 * src14);
    float dst12 = (btmp2  * src10 + btmp5  * src11 + btmp1  * src9 )
                      - (btmp4  * src11 + btmp0  * src9  + btmp3  * src10);
    float dst13 = (btmp8  * src11 + btmp0  * src8  + btmp7  * src10)
                      - (btmp6  * src10 + btmp9  * src11 + btmp1  * src8 );
    float dst14 = (btmp6  * src9  + btmp11 * src11 + btmp3  * src8 )
                      - (btmp10 * src11 + btmp2  * src8  + btmp7  * src9 );
    float dst15 = (btmp10 * src10 + btmp4  * src8  + btmp9  * src9 )
                      - (btmp8  * src9  + btmp11 * src10 + btmp5  * src8 );

    // calculate determinant
    float det =
            src0 * dst0 + src1 * dst1 + src2 * dst2 + src3 * dst3;
	float invdet;

    if (det == 0.0f) {
        return;//false;
    }

    // calculate matrix inverse
    invdet = 1.0f / det;
    mInv[     mInvOffset] = dst0  * invdet;
    mInv[ 1 + mInvOffset] = dst1  * invdet;
    mInv[ 2 + mInvOffset] = dst2  * invdet;
    mInv[ 3 + mInvOffset] = dst3  * invdet;

    mInv[ 4 + mInvOffset] = dst4  * invdet;
    mInv[ 5 + mInvOffset] = dst5  * invdet;
    mInv[ 6 + mInvOffset] = dst6  * invdet;
    mInv[ 7 + mInvOffset] = dst7  * invdet;

    mInv[ 8 + mInvOffset] = dst8  * invdet;
    mInv[ 9 + mInvOffset] = dst9  * invdet;
    mInv[10 + mInvOffset] = dst10 * invdet;
    mInv[11 + mInvOffset] = dst11 * invdet;

    mInv[12 + mInvOffset] = dst12 * invdet;
    mInv[13 + mInvOffset] = dst13 * invdet;
    mInv[14 + mInvOffset] = dst14 * invdet;
    mInv[15 + mInvOffset] = dst15 * invdet;

    return ;//true;
}
/*
void invert3(float* mInv, float* m) {


	Matrix_Inv(mInv, m3x3, 3);
}*/

void invert3(float* mInv, float* m) {
	int mOffset = 0;
	int mInvOffset = 0;

    // Invert a 4 x 4 matrix using Cramer's Rule

    // transpose matrix
    float src0  = m[mOffset +  0];
    float src4  = m[mOffset +  1];
    float src8  = m[mOffset +  2];
    float src12 = m[mOffset +  3];

    float src1  = m[mOffset +  4];
    float src5  = m[mOffset +  5];
    float src9  = m[mOffset +  6];
    float src13 = m[mOffset +  7];

    float src2  = m[mOffset +  8];
    float src6  = m[mOffset +  9];
    float src10 = m[mOffset + 10];
    float src14 = m[mOffset + 11];

    float src3  = m[mOffset + 12];
    float src7  = m[mOffset + 13];
    float src11 = m[mOffset + 14];
    float src15 = m[mOffset + 15];

    // calculate pairs for first 8 elements (cofactors)
    float atmp0  = src10 * src15;
    float atmp1  = src11 * src14;
    float atmp2  = src9  * src15;
    float atmp3  = src11 * src13;
    float atmp4  = src9  * src14;
    float atmp5  = src10 * src13;
    float atmp6  = src8  * src15;
    float atmp7  = src11 * src12;
    float atmp8  = src8  * src14;
    float atmp9  = src10 * src12;
    float atmp10 = src8  * src13;
    float atmp11 = src9  * src12;

    // calculate first 8 elements (cofactors)
    float dst0  = (atmp0 * src5 + atmp3 * src6 + atmp4  * src7)
                      - (atmp1 * src5 + atmp2 * src6 + atmp5  * src7);
    float dst1  = (atmp1 * src4 + atmp6 * src6 + atmp9  * src7)
                      - (atmp0 * src4 + atmp7 * src6 + atmp8  * src7);
    float dst2  = (atmp2 * src4 + atmp7 * src5 + atmp10 * src7)
                      - (atmp3 * src4 + atmp6 * src5 + atmp11 * src7);
    float dst3  = (atmp5 * src4 + atmp8 * src5 + atmp11 * src6)
                      - (atmp4 * src4 + atmp9 * src5 + atmp10 * src6);
    float dst4  = (atmp1 * src1 + atmp2 * src2 + atmp5  * src3)
                      - (atmp0 * src1 + atmp3 * src2 + atmp4  * src3);
    float dst5  = (atmp0 * src0 + atmp7 * src2 + atmp8  * src3)
                      - (atmp1 * src0 + atmp6 * src2 + atmp9  * src3);
    float dst6  = (atmp3 * src0 + atmp6 * src1 + atmp11 * src3)
                      - (atmp2 * src0 + atmp7 * src1 + atmp10 * src3);
    float dst7  = (atmp4 * src0 + atmp9 * src1 + atmp10 * src2)
                      - (atmp5 * src0 + atmp8 * src1 + atmp11 * src2);

    // calculate pairs for second 8 elements (cofactors)
    float btmp0  = src2 * src7;
    float btmp1  = src3 * src6;
    float btmp2  = src1 * src7;
    float btmp3  = src3 * src5;
    float btmp4  = src1 * src6;
    float btmp5  = src2 * src5;
    float btmp6  = src0 * src7;
    float btmp7  = src3 * src4;
    float btmp8  = src0 * src6;
    float btmp9  = src2 * src4;
    float btmp10 = src0 * src5;
    float btmp11 = src1 * src4;

    // calculate second 8 elements (cofactors)
    float dst8  = (btmp0  * src13 + btmp3  * src14 + btmp4  * src15)
                      - (btmp1  * src13 + btmp2  * src14 + btmp5  * src15);
    float dst9  = (btmp1  * src12 + btmp6  * src14 + btmp9  * src15)
                      - (btmp0  * src12 + btmp7  * src14 + btmp8  * src15);
    float dst10 = (btmp2  * src12 + btmp7  * src13 + btmp10 * src15)
                      - (btmp3  * src12 + btmp6  * src13 + btmp11 * src15);
    float dst11 = (btmp5  * src12 + btmp8  * src13 + btmp11 * src14)
                      - (btmp4  * src12 + btmp9  * src13 + btmp10 * src14);
    float dst12 = (btmp2  * src10 + btmp5  * src11 + btmp1  * src9 )
                      - (btmp4  * src11 + btmp0  * src9  + btmp3  * src10);
    float dst13 = (btmp8  * src11 + btmp0  * src8  + btmp7  * src10)
                      - (btmp6  * src10 + btmp9  * src11 + btmp1  * src8 );
    float dst14 = (btmp6  * src9  + btmp11 * src11 + btmp3  * src8 )
                      - (btmp10 * src11 + btmp2  * src8  + btmp7  * src9 );
    float dst15 = (btmp10 * src10 + btmp4  * src8  + btmp9  * src9 )
                      - (btmp8  * src9  + btmp11 * src10 + btmp5  * src8 );
                      
    (void)dst12;
    (void)dst11;
    (void)dst15;
    (void)dst14;
    (void)dst7;
    (void)dst13;

    // calculate determinant
    float det =
            src0 * dst0 + src1 * dst1 + src2 * dst2 + src3 * dst3;
	float invdet;

    if (det == 0.0f) {
        return;//false;
    }

    // calculate matrix inverse
    invdet = 1.0f / det;
    mInv[     mInvOffset] = dst0  * invdet;
    mInv[ 1 + mInvOffset] = dst1  * invdet;
    mInv[ 2 + mInvOffset] = dst2  * invdet;

    mInv[ 3 + mInvOffset] = dst4  * invdet;
    mInv[ 4 + mInvOffset] = dst5  * invdet;
    mInv[ 5 + mInvOffset] = dst6  * invdet;

    mInv[ 6 + mInvOffset] = dst8  * invdet;
    mInv[ 7 + mInvOffset] = dst9  * invdet;
    mInv[ 8 + mInvOffset] = dst10 * invdet;

    return ;//true;
}


//--------------------------------------------------------------------------------------
__inline float DET2X2( float m00, float m01, 
                         float m10, float m11 )
{
    return m00 * m11 - m01 * m10;
}


//--------------------------------------------------------------------------------------
// Name: DET3X3()
// Desc: Helper function to compute the determinant of a 3x3 matrix
//--------------------------------------------------------------------------------------
__inline float DET3X3( float m00, float m01, float m02,
                         float m10, float m11, float m12,
                         float m20, float m21, float m22 )
{
    return m00 * DET2X2( m11, m12, m21, m22 ) -
           m10 * DET2X2( m01, m02, m21, m22 ) +
           m20 * DET2X2( m01, m02, m11, m12 );
}

void object2noraml(float* mInv, float* m)
{

    // Compute the matrix' determinant
    float fDeterminant = DET3X3( m[0], m[1], m[2], 
                                   m[4], m[5], m[6], 
                                   m[8], m[9], m[10] );
    float fScale = 1.0f / fDeterminant;

    // Divide the adjoint of the matrix by its determinant and transpose the result
    mInv[0] = +DET2X2( m[5], m[6], m[9], m[10] ) * fScale;
    mInv[1] = -DET2X2( m[4], m[6], m[8], m[10] ) * fScale;
    mInv[2] = +DET2X2( m[4], m[5], m[8], m[9]  ) * fScale;
    
    mInv[3] = -DET2X2( m[1], m[2], m[9], m[10] ) * fScale;
    mInv[4] = +DET2X2( m[0], m[2], m[8], m[10] ) * fScale;
    mInv[5] = -DET2X2( m[0], m[1], m[8], m[9]  ) * fScale;
    
    mInv[6] = +DET2X2( m[1], m[2], m[5], m[6]  ) * fScale;
    mInv[7] = -DET2X2( m[0], m[2], m[4], m[6]  ) * fScale;
    mInv[8] = +DET2X2( m[0], m[1], m[4], m[5]  ) * fScale;
}

void setIdentity(float* sm) {
	int smOffset = 0;
	int i= 0;
    for (i=0 ; i<16 ; i++) {
        sm[smOffset + i] = 0;
    }
    for(i = 0; i < 16; i += 5) {
        sm[smOffset + i] = 1.0f;
    }
}

void translateM(float* m, float x, float y, float z) {
	int i = 0;
    for (i=0 ; i<4 ; i++) {
        int mi = i;
        m[12 + mi] += m[mi] * x + m[4 + mi] * y + m[8 + mi] * z;
    }
}
void setTranslate(float* m, float x, float y, float z) {
	setIdentity(m);
	m[12 + 0] = x;
	m[12 + 1] = y;
	m[12 + 2] = z;
}

void ortho(float* m, float l, float r, float t, float b, float n, float f) {
	float tx = -(r + l) / (r - l);
	float ty = -(t + b) / (t - b);
	float tz = -(f + n) / (f - n);
	setIdentity(m);
	m[0 + 0] = 2 / (r - l);
	m[0 + 1] = 0;
	m[0 + 2] = 0;
	m[0 + 3] = 0;

	m[4 + 0] = 0;
	m[4 + 1] = 2 / (t - b);
	m[4 + 2] = 0;
	m[4 + 3] = 0;

	m[8 + 0] = 0;
	m[8 + 1] = 0;
	m[8 + 2] = -2 / (f - n);
	m[8 + 3] = 0;
	
	m[12 + 0] = tx;
	m[12 + 1] = ty;
	m[12 + 2] = tz;
	m[12 + 2] = 1;
}


void setScaling(float* m, float x, float y, float z)
{
	setIdentity(m);
	m[0]  = x;
	m[5]  = y;
	m[10] = z;
}

float length(float x, float y, float z) {
    return (float) sqrt(x * x + y * y + z * z);
}

void setLookAt(float* rm, 
		float eyeX, float eyeY, float eyeZ,
        float centerX, float centerY, float centerZ, 
		float upX, float upY, float upZ) 
{
	int rmOffset = 0;

    // See the OpenGL GLUT documentation for gluLookAt for a description
    // of the algorithm. We implement it in a straightforward way:
    float sx, sy, sz, rls, ux, uy, uz;

    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;

    // Normalize f
    float rlf = 1.0f / length(fx, fy, fz);
    fx *= rlf;
    fy *= rlf;
    fz *= rlf;

    // compute s = f x up (x means "cross product")
    sx = fy * upZ - fz * upY;
    sy = fz * upX - fx * upZ;
    sz = fx * upY - fy * upX;

    // and normalize s
    rls = 1.0f / length(sx, sy, sz);
    sx *= rls;
    sy *= rls;
    sz *= rls;

    // compute u = s x f
    ux = sy * fz - sz * fy;
    uy = sz * fx - sx * fz;
    uz = sx * fy - sy * fx;

    rm[rmOffset + 0] = sx;
    rm[rmOffset + 1] = ux;
    rm[rmOffset + 2] = -fx;
    rm[rmOffset + 3] = 0.0f;

    rm[rmOffset + 4] = sy;
    rm[rmOffset + 5] = uy;
    rm[rmOffset + 6] = -fy;
    rm[rmOffset + 7] = 0.0f;

    rm[rmOffset + 8] = sz;
    rm[rmOffset + 9] = uz;
    rm[rmOffset + 10] = -fz;
    rm[rmOffset + 11] = 0.0f;

    rm[rmOffset + 12] = 0.0f;
    rm[rmOffset + 13] = 0.0f;
    rm[rmOffset + 14] = 0.0f;
    rm[rmOffset + 15] = 1.0f;

    translateM(rm, -eyeX, -eyeY, -eyeZ);
}