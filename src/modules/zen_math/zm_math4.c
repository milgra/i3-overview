#ifndef zm_math4_h
#define zm_math4_h

#include "zm_math3.c"
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
  #define M_PI_2 3.14159265358979323846 * 2
#endif

typedef struct _v4_t v4_t;
struct _v4_t
{
  float x, y, z, w;
};

v4_t v4_init(float x, float y, float z, float w);
v4_t v4_add(v4_t a, v4_t b);
v4_t v4_sub(v4_t a, v4_t b);
v4_t v4_scale(v4_t a, float f);
void v4_describe(v4_t vector);

typedef struct _m4_t m4_t;
struct _m4_t
{
  float m00, m01, m02, m03;
  float m10, m11, m12, m13;
  float m20, m21, m22, m23;
  float m30, m31, m32, m33;
};

typedef union _matrix4array_t matrix4array_t;
union _matrix4array_t
{
  m4_t  matrix;
  float array[16];
};

m4_t m4_defaultidentity(void);
m4_t m4_defaultscale(float x, float y, float z);
m4_t m4_defaultrotation(float x, float y, float z);
m4_t m4_defaulttranslation(float x, float y, float z);
m4_t m4_defaultortho(float left,
                     float right,
                     float bottom,
                     float top,
                     float near,
                     float far);
m4_t m4_defaultperspective(float fovy, float aspect, float nearz, float farz);
m4_t m4_scale(m4_t matrix, float x, float y, float z);
m4_t m4_rotate(m4_t matrix, float x, float y, float z);
m4_t m4_translate(m4_t other, float x, float y, float z);
m4_t m4_invert(m4_t source, char* success);
m4_t m4_multiply(m4_t a, m4_t b);
m4_t m4_transpose(m4_t matrix);
void m4_describe(m4_t matrix);
void m4_extractangles(m4_t matrix, float* x, float* y, float* z);

v4_t m4_multiply_vector4(m4_t matrix, v4_t vector);
v4_t m4_world_to_screen_coords(m4_t  proj_matrix,
                               v4_t  vector,
                               float width,
                               float height);
v3_t m4_screen_to_world_coords(m4_t  proj_matrix,
                               v4_t  vector,
                               float width,
                               float height);
void m4_extract(m4_t trafo, v3_t* scale, v3_t* rotation, v3_t* translation);
v3_t v4_quadrelativecoors(v4_t ulc, v4_t urc, v4_t llc, v3_t point_is);
v3_t v4_quadlineintersection(v4_t pointul,
                             v4_t pointur,
                             v4_t pointll,
                             v3_t linea,
                             v3_t lineb);

#endif
#if __INCLUDE_LEVEL__ == 0

#include <float.h>
#include <math.h>

void m4_multiplywithnumber(m4_t* matrix, float number);

/* creates vector */

v4_t v4_init(float x, float y, float z, float w)
{
  v4_t result;

  result.x = x;
  result.y = y;
  result.z = z;
  result.w = w;

  return result;
}

/* adds two vectors */

v4_t v4_add(v4_t a, v4_t b)
{
  v4_t result;

  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;
  result.w = a.w + b.w;

  return result;
}

/* substract b from a */

v4_t v4_sub(v4_t a, v4_t b)
{
  v4_t result;

  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;
  result.w = a.w - b.w;

  return result;
}

/* scales vector */

v4_t v4_scale(v4_t a, float f)
{
  v4_t result;

  result.x = a.x * f;
  result.y = a.y * f;
  result.z = a.z * f;
  result.w = a.w * f;

  return result;
}

/* describes vector4 */

void v4_describe(v4_t vector)
{
  printf("x : %f y : %f z : %f w : %f", vector.x, vector.y, vector.z, vector.w);
}

/* creates identity matrix */

m4_t m4_defaultidentity()
{
  m4_t matrix;

  matrix.m00 = 1.0f;
  matrix.m01 = 0.0f;
  matrix.m02 = 0.0f;
  matrix.m03 = 0.0f;
  matrix.m10 = 0.0f;
  matrix.m11 = 1.0f;
  matrix.m12 = 0.0f;
  matrix.m13 = 0.0f;
  matrix.m20 = 0.0f;
  matrix.m21 = 0.0f;
  matrix.m22 = 1.0f;
  matrix.m23 = 0.0f;
  matrix.m30 = 0.0f;
  matrix.m31 = 0.0f;
  matrix.m32 = 0.0f;
  matrix.m33 = 1.0f;

  return matrix;
}

/* creates scale matrix */

m4_t m4_defaultscale(float x, float y, float z)
{
  m4_t matrix = m4_defaultidentity();

  matrix.m00 = x;
  matrix.m11 = y;
  matrix.m22 = z;

  return matrix;
}

/* creates rotation matrix */

m4_t m4_defaultrotation(float x, float y, float z)
{
  float max = fabs(x) > fabs(y) ? x : y;
  max       = fabs(z) > fabs(max) ? z : max;

  if (max == 0.0)
    return m4_defaultidentity();

  x = x / max;
  y = y / max;
  z = z / max;

  float nx, ny, nz, scale, sin, cos, cosp;
  m4_t  matrix;

  // normalize values

  nx = x;
  ny = y;
  nz = z;

  scale = 1.0f / sqrtf(nx * nx + ny * ny + nz * nz);

  nx *= scale;
  ny *= scale;
  nz *= scale;

  // precalc

  sin  = sinf(max);
  cos  = cosf(max);
  cosp = 1.0f - cos;

  // create matrix

  matrix.m00 = cos + cosp * nx * nx;
  matrix.m01 = cosp * nx * ny + nz * sin;
  matrix.m02 = cosp * nx * nz - ny * sin;
  matrix.m03 = 0.0f;
  matrix.m10 = cosp * nx * ny - nz * sin;
  matrix.m11 = cos + cosp * ny * ny;
  matrix.m12 = cosp * ny * nz + nx * sin;
  matrix.m13 = 0.0f;
  matrix.m20 = cosp * nx * nz + ny * sin;
  matrix.m21 = cosp * ny * nz - nx * sin;
  matrix.m22 = cos + cosp * nz * nz;
  matrix.m23 = 0.0f;
  matrix.m30 = 0.0f;
  matrix.m31 = 0.0f;
  matrix.m32 = 0.0f;
  matrix.m33 = 1.0f;

  return matrix;
}

/* creates translation matrix */

m4_t m4_defaulttranslation(float x, float y, float z)
{
  m4_t result;

  result     = m4_defaultidentity();
  result.m00 = 1;

  result.m30 = x;
  result.m31 = y;
  result.m32 = z;

  return result;
}

/* creates ortographic projection */

m4_t m4_defaultortho(float left,
                     float right,
                     float bottom,
                     float top,
                     float near,
                     float far)
{
  float rpl, rml, tpb, tmb, fpn, fmn;
  m4_t  matrix;

  rpl = right + left;
  rml = right - left;
  tpb = top + bottom;
  tmb = top - bottom;
  fpn = far + near;
  fmn = far - near;

  matrix.m00 = 2.0f / rml;
  matrix.m01 = 0.0f;
  matrix.m02 = 0.0f;
  matrix.m03 = 0.0f;
  matrix.m10 = 0.0f;
  matrix.m11 = 2.0f / tmb;
  matrix.m12 = 0.0f;
  matrix.m13 = 0.0f;
  matrix.m20 = 0.0f;
  matrix.m21 = 0.0f;
  matrix.m22 = -2.0f / fmn;
  matrix.m23 = 0.0f;
  matrix.m30 = -rpl / rml;
  matrix.m31 = -tpb / tmb;
  matrix.m32 = -fpn / fmn;
  matrix.m33 = 1.0f;

  return matrix;
}

/* create perspective projection */

m4_t m4_defaultperspective(float fovy, float aspect, float nearz, float farz)
{
  float cotan;
  m4_t  matrix;

  cotan = 1.0f / tanf(fovy / 2.0f);

  matrix.m00 = cotan / aspect;
  matrix.m01 = 0.0f;
  matrix.m02 = 0.0f;
  matrix.m03 = 0.0f;

  matrix.m10 = 0.0f;
  matrix.m11 = cotan;
  matrix.m12 = 0.0f;
  matrix.m13 = 0.0f;

  matrix.m20 = 0.0f;
  matrix.m21 = 0.0f;
  matrix.m22 = (farz + nearz) / (nearz - farz);
  matrix.m23 = -1.0f;

  matrix.m30 = 0.0f;
  matrix.m31 = 0.0f;
  matrix.m32 = (2.0f * farz * nearz) / (nearz - farz);
  matrix.m33 = 0.0f;

  return matrix;
}

/* scales matrix */

m4_t m4_scale(m4_t matrix, float x, float y, float z)
{
  matrix.m00 *= x;
  matrix.m11 *= y;
  matrix.m22 *= z;

  return matrix;
}

/* rotates matrix */

m4_t m4_rotate(m4_t matrix, float x, float y, float z)
{
  m4_t rotation;

  rotation = m4_defaultrotation(x, y, z);
  return m4_multiply(matrix, rotation);
}

/* translates matrix */

m4_t m4_translate(m4_t other, float x, float y, float z)
{
  other.m30 = other.m00 * x + other.m10 * y + other.m20 * z + other.m30;
  other.m31 = other.m01 * x + other.m11 * y + other.m21 * z + other.m31;
  other.m32 = other.m02 * x + other.m12 * y + other.m22 * z + other.m32;

  return other;
}

/* inverts matrix */

m4_t m4_invert(m4_t source, char* success)
{
  float determinant;
  m4_t  inverse;

  inverse.m00 = source.m11 * source.m22 * source.m33 -
                source.m11 * source.m23 * source.m32 -
                source.m21 * source.m12 * source.m33 +
                source.m21 * source.m13 * source.m32 +
                source.m31 * source.m12 * source.m23 -
                source.m31 * source.m13 * source.m22;

  inverse.m10 = -source.m10 * source.m22 * source.m33 +
                source.m10 * source.m23 * source.m32 +
                source.m20 * source.m12 * source.m33 -
                source.m20 * source.m13 * source.m32 -
                source.m30 * source.m12 * source.m23 +
                source.m30 * source.m13 * source.m22;

  inverse.m20 = source.m10 * source.m21 * source.m33 -
                source.m10 * source.m23 * source.m31 -
                source.m20 * source.m11 * source.m33 +
                source.m20 * source.m13 * source.m31 +
                source.m30 * source.m11 * source.m23 -
                source.m30 * source.m13 * source.m21;

  inverse.m30 = -source.m10 * source.m21 * source.m32 +
                source.m10 * source.m22 * source.m31 +
                source.m20 * source.m11 * source.m32 -
                source.m20 * source.m12 * source.m31 -
                source.m30 * source.m11 * source.m22 +
                source.m30 * source.m12 * source.m21;

  inverse.m01 = -source.m01 * source.m22 * source.m33 +
                source.m01 * source.m23 * source.m32 +
                source.m21 * source.m02 * source.m33 -
                source.m21 * source.m03 * source.m32 -
                source.m31 * source.m02 * source.m23 +
                source.m31 * source.m03 * source.m22;

  inverse.m11 = source.m00 * source.m22 * source.m33 -
                source.m00 * source.m23 * source.m32 -
                source.m20 * source.m02 * source.m33 +
                source.m20 * source.m03 * source.m32 +
                source.m30 * source.m02 * source.m23 -
                source.m30 * source.m03 * source.m22;

  inverse.m21 = -source.m00 * source.m21 * source.m33 +
                source.m00 * source.m23 * source.m31 +
                source.m20 * source.m01 * source.m33 -
                source.m20 * source.m03 * source.m31 -
                source.m30 * source.m01 * source.m23 +
                source.m30 * source.m03 * source.m21;

  inverse.m31 = source.m00 * source.m21 * source.m32 -
                source.m00 * source.m22 * source.m31 -
                source.m20 * source.m01 * source.m32 +
                source.m20 * source.m02 * source.m31 +
                source.m30 * source.m01 * source.m22 -
                source.m30 * source.m02 * source.m21;

  inverse.m02 = source.m01 * source.m12 * source.m33 -
                source.m01 * source.m13 * source.m32 -
                source.m11 * source.m02 * source.m33 +
                source.m11 * source.m03 * source.m32 +
                source.m31 * source.m02 * source.m13 -
                source.m31 * source.m03 * source.m12;

  inverse.m12 = -source.m00 * source.m12 * source.m33 +
                source.m00 * source.m13 * source.m32 +
                source.m10 * source.m02 * source.m33 -
                source.m10 * source.m03 * source.m32 -
                source.m30 * source.m02 * source.m13 +
                source.m30 * source.m03 * source.m12;

  inverse.m22 = source.m00 * source.m11 * source.m33 -
                source.m00 * source.m13 * source.m31 -
                source.m10 * source.m01 * source.m33 +
                source.m10 * source.m03 * source.m31 +
                source.m30 * source.m01 * source.m13 -
                source.m30 * source.m03 * source.m11;

  inverse.m32 = -source.m00 * source.m11 * source.m32 +
                source.m00 * source.m12 * source.m31 +
                source.m10 * source.m01 * source.m32 -
                source.m10 * source.m02 * source.m31 -
                source.m30 * source.m01 * source.m12 +
                source.m30 * source.m02 * source.m11;

  inverse.m03 = -source.m01 * source.m12 * source.m23 +
                source.m01 * source.m13 * source.m22 +
                source.m11 * source.m02 * source.m23 -
                source.m11 * source.m03 * source.m22 -
                source.m21 * source.m02 * source.m13 +
                source.m21 * source.m03 * source.m12;

  inverse.m13 = source.m00 * source.m12 * source.m23 -
                source.m00 * source.m13 * source.m22 -
                source.m10 * source.m02 * source.m23 +
                source.m10 * source.m03 * source.m22 +
                source.m20 * source.m02 * source.m13 -
                source.m20 * source.m03 * source.m12;

  inverse.m23 = -source.m00 * source.m11 * source.m23 +
                source.m00 * source.m13 * source.m21 +
                source.m10 * source.m01 * source.m23 -
                source.m10 * source.m03 * source.m21 -
                source.m20 * source.m01 * source.m13 +
                source.m20 * source.m03 * source.m11;

  inverse.m33 = source.m00 * source.m11 * source.m22 -
                source.m00 * source.m12 * source.m21 -
                source.m10 * source.m01 * source.m22 +
                source.m10 * source.m02 * source.m21 +
                source.m20 * source.m01 * source.m12 -
                source.m20 * source.m02 * source.m11;

  determinant = source.m00 * inverse.m00 + source.m01 * inverse.m10 +
                source.m02 * inverse.m20 + source.m03 * inverse.m30;

  if (determinant == 0 && success != NULL)
    *success = 0;

  m4_multiplywithnumber(&inverse, 1.0 / determinant);

  return inverse;
}

/* multiplies matrices */

m4_t m4_multiply(m4_t a, m4_t b)
{
  m4_t matrix;

  matrix.m00 = a.m00 * b.m00 + a.m10 * b.m01 + a.m20 * b.m02 + a.m30 * b.m03;
  matrix.m10 = a.m00 * b.m10 + a.m10 * b.m11 + a.m20 * b.m12 + a.m30 * b.m13;
  matrix.m20 = a.m00 * b.m20 + a.m10 * b.m21 + a.m20 * b.m22 + a.m30 * b.m23;
  matrix.m30 = a.m00 * b.m30 + a.m10 * b.m31 + a.m20 * b.m32 + a.m30 * b.m33;

  matrix.m01 = a.m01 * b.m00 + a.m11 * b.m01 + a.m21 * b.m02 + a.m31 * b.m03;
  matrix.m11 = a.m01 * b.m10 + a.m11 * b.m11 + a.m21 * b.m12 + a.m31 * b.m13;
  matrix.m21 = a.m01 * b.m20 + a.m11 * b.m21 + a.m21 * b.m22 + a.m31 * b.m23;
  matrix.m31 = a.m01 * b.m30 + a.m11 * b.m31 + a.m21 * b.m32 + a.m31 * b.m33;

  matrix.m02 = a.m02 * b.m00 + a.m12 * b.m01 + a.m22 * b.m02 + a.m32 * b.m03;
  matrix.m12 = a.m02 * b.m10 + a.m12 * b.m11 + a.m22 * b.m12 + a.m32 * b.m13;
  matrix.m22 = a.m02 * b.m20 + a.m12 * b.m21 + a.m22 * b.m22 + a.m32 * b.m23;
  matrix.m32 = a.m02 * b.m30 + a.m12 * b.m31 + a.m22 * b.m32 + a.m32 * b.m33;

  matrix.m03 = a.m03 * b.m00 + a.m13 * b.m01 + a.m23 * b.m02 + a.m33 * b.m03;
  matrix.m13 = a.m03 * b.m10 + a.m13 * b.m11 + a.m23 * b.m12 + a.m33 * b.m13;
  matrix.m23 = a.m03 * b.m20 + a.m13 * b.m21 + a.m23 * b.m22 + a.m33 * b.m23;
  matrix.m33 = a.m03 * b.m30 + a.m13 * b.m31 + a.m23 * b.m32 + a.m33 * b.m33;

  return matrix;
}

/* transposes matrix */

m4_t m4_transpose(m4_t matrix)
{
  m4_t result;

  result.m00 = matrix.m00;
  result.m11 = matrix.m11;
  result.m22 = matrix.m22;
  result.m33 = matrix.m33;

  result.m10 = matrix.m01;
  result.m01 = matrix.m10;
  result.m20 = matrix.m02;
  result.m02 = matrix.m20;
  result.m30 = matrix.m03;
  result.m03 = matrix.m30;

  result.m21 = matrix.m12;
  result.m12 = matrix.m21;
  result.m31 = matrix.m13;
  result.m13 = matrix.m31;
  result.m32 = matrix.m23;
  result.m23 = matrix.m32;

  return result;
}

/* describes matrix */

void m4_describe(m4_t matrix)
{
  printf("%.2f %.2f %.2f %.2f | %.2f %.2f %.2f %.2f | %.2f %.2f %.2f %.2f | "
         "%.2f %.2f %.2f %.2f",
         matrix.m00,
         matrix.m01,
         matrix.m02,
         matrix.m03,
         matrix.m10,
         matrix.m11,
         matrix.m12,
         matrix.m13,
         matrix.m20,
         matrix.m21,
         matrix.m22,
         matrix.m23,
         matrix.m30,
         matrix.m31,
         matrix.m32,
         matrix.m33);
}

/* extract rotation angles from matrix */

void m4_extractangles(m4_t matrix, float* x, float* y, float* z)
{
  float y1, y2, x1, x2, z1, z2;
  x1 = x2 = y1 = y2 = z1 = z2 = 0.0;

  if (fabs(matrix.m20) != 1)
  {
    y1 = -asinf(matrix.m20);
    y2 = M_PI - y1;
    x1 = atan2(matrix.m21 / cosf(y1), matrix.m22 / cos(y1));
    x2 = atan2(matrix.m21 / cosf(y2), matrix.m22 / cos(y2));
    z1 = atan2(matrix.m10 / cosf(y1), matrix.m00 / cos(y1));
    z2 = atan2(matrix.m10 / cosf(y2), matrix.m00 / cos(y2));
  }
  else
  {
    z1 = 0;
    if (matrix.m20 == -1.0)
    {
      y1 = M_PI / 2.0;
      x1 = z1 + atan2(matrix.m01, matrix.m02);
    }
    else
    {
      y1 = -M_PI / 2.0;
      x1 = -z1 + atan2(-matrix.m01, -matrix.m02);
    }
  }
  // printf( "angles %f %f %f , %f %f %f" , x1 * 180 / M_PI , y1 * 180 / M_PI,
  // z1* 180 / M_PI , x2* 180 / M_PI , y2* 180 / M_PI , z2* 180 / M_PI );
  *x = x1;
  *y = y1;
  *z = z1;
}

/* internal : multiplies with number */

void m4_multiplywithnumber(m4_t* matrix, float number)
{
  matrix->m00 *= number;
  matrix->m01 *= number;
  matrix->m02 *= number;
  matrix->m03 *= number;
  matrix->m10 *= number;
  matrix->m11 *= number;
  matrix->m12 *= number;
  matrix->m13 *= number;
  matrix->m20 *= number;
  matrix->m21 *= number;
  matrix->m22 *= number;
  matrix->m23 *= number;
  matrix->m30 *= number;
  matrix->m31 *= number;
  matrix->m32 *= number;
  matrix->m33 *= number;
}

/* multiplies matrix4 with vector 4 */

v4_t m4_multiply_vector4(m4_t matrix, v4_t vector)
{
  v4_t result;

  result.x = matrix.m00 * vector.x + matrix.m10 * vector.y +
             matrix.m20 * vector.z + matrix.m30 * vector.w;
  result.y = matrix.m01 * vector.x + matrix.m11 * vector.y +
             matrix.m21 * vector.z + matrix.m31 * vector.w;
  result.z = matrix.m02 * vector.x + matrix.m12 * vector.y +
             matrix.m22 * vector.z + matrix.m32 * vector.w;
  result.w = matrix.m03 * vector.x + matrix.m13 * vector.y +
             matrix.m23 * vector.z + matrix.m33 * vector.w;

  return result;
}

/* projects model space vector4 to screen space */

v4_t m4_world_to_screen_coords(m4_t  matrix,
                               v4_t  srcvector,
                               float width,
                               float height)
{
  v4_t vector;
  vector.x = srcvector.x;
  vector.y = srcvector.y;
  vector.z = 0;
  vector.w = 1;

  vector = m4_multiply_vector4(matrix, vector);

  if (vector.w == 0)
    return vector;

  // perspective divide to normalized device coordinates

  vector.x /= vector.w;
  vector.y /= vector.w;
  vector.z /= vector.w;

  // viewport transformation

  v4_t result;
  result.x = (vector.x + 1.0) * width * 0.5;
  result.y = (vector.y + 1.0) * height * 0.5;
  result.z = vector.z;
  result.w = vector.w;

  return result;
}

/* projects screen space vector4 to model space */

v3_t m4_screen_to_world_coords(m4_t  mvpmatrix,
                               v4_t  scrvector,
                               float width,
                               float height)
{
  // get normalized device coordinates
  // ( src.x - ( width / 2.0 ) ) / ( width / 2.0 )
  // ( src.y - ( height / 2.0 ) ) / ( height / 2.0 )

  v4_t vector;
  vector.x = scrvector.x / width * 2.0 - 1.0;
  vector.y = scrvector.y / height * 2.0 - 1.0;
  vector.z = scrvector.z;
  vector.w = 1.0;

  // invert projection matrix

  char success = 1;
  m4_t invert  = m4_invert(mvpmatrix, &success);

  // multiply transposed inverted projection matrix with vector

  v4_t result = m4_multiply_vector4(invert, vector);

  if (result.w == 0)
    return v3_init(FLT_MAX, FLT_MAX, FLT_MAX);

  // get world space coordinates

  result.w = 1.0 / result.w;
  result.x *= result.w;
  result.y *= result.w;
  result.z *= result.w;

  return v3_init(result.x, result.y, result.z);
}

/* extracts data from matrix4 */

void m4_extract(m4_t trafo, v3_t* scale, v3_t* rotation, v3_t* translation)
{
  v4_t base_o = v4_init(0, 0, 0, 1);
  v4_t base_x = v4_init(1, 0, 0, 1);
  v4_t base_y = v4_init(0, 1, 0, 1);

  base_o = m4_multiply_vector4(trafo, base_o);
  base_x = m4_multiply_vector4(trafo, base_x);
  base_y = m4_multiply_vector4(trafo, base_y);

  v3_t final_o = v3_init(base_o.x, base_o.y, base_o.z);
  v3_t final_x = v3_init(base_x.x, base_x.y, base_x.z);
  v3_t final_y = v3_init(base_y.x, base_y.y, base_y.z);

  translation->x = final_o.x;
  translation->y = final_o.y;
  translation->z = final_o.z;

  final_x = v3_sub(final_x, final_o);
  final_y = v3_sub(final_y, final_o);

  float scale_factor = v3_length(final_x);

  scale->x = scale_factor;
  scale->y = scale_factor;
  scale->z = scale_factor;

  *rotation = v3_getxyunitrotation(final_x, final_y);
}

/* returns point_is coordiantes in the local coordinate system of the quad
 * described by ulc, urc, llc */

v3_t v4_quadrelativecoors(v4_t ulc, v4_t urc, v4_t llc, v3_t point_is)
{
  v3_t plane_a, plane_b, plane_d;
  v3_t vec_ab, vec_ad, vec_n, vec_ai;

  plane_a = v3_init(ulc.x, ulc.y, ulc.z);
  plane_b = v3_init(llc.x, llc.y, llc.z);
  plane_d = v3_init(urc.x, urc.y, urc.z);

  // create plane vectors and normal

  vec_ab = v3_sub(plane_b, plane_a);
  vec_ad = v3_sub(plane_d, plane_a);
  vec_n  = v3_cross(vec_ab, vec_ad);

  // get angle of AI from AB and AC to build up the frame square in its actual
  // position

  vec_ai = v3_sub(point_is, plane_a);

  float angle_ab_ai   = v3_angle(vec_ab, vec_ai);
  float angle_ad_ai   = v3_angle(vec_ad, vec_ai);
  float length_vec_ai = v3_length(vec_ai);

  // get relative coordinates

  v3_t relative;
  relative.x = cos(angle_ad_ai) * length_vec_ai;
  relative.y = -sin(angle_ad_ai) * length_vec_ai;

  // check quarters

  if (angle_ab_ai > M_PI / 2 && angle_ad_ai > M_PI / 2)
    relative.y *= -1;
  else if (angle_ab_ai > M_PI / 2 && angle_ad_ai < M_PI / 2)
    relative.y *= -1;

  if (relative.x > 0.0 && relative.x < v3_length(vec_ad) &&
      relative.y < 0.0 && relative.y > -v3_length(vec_ab))
  {
    return relative;
  }
  else
    return v3_init(FLT_MAX, FLT_MAX, FLT_MAX);
}

/* calculates intersection point of quad and line */

v3_t v4_quadlineintersection(v4_t ulc, v4_t urc, v4_t llc, v3_t linea, v3_t lineb)
{
  v3_t plane_a, plane_b, plane_d;
  v3_t vec_ab, vec_ad, vec_n, vec_ai, point_is;

  plane_a = v3_init(ulc.x, ulc.y, ulc.z);
  plane_b = v3_init(llc.x, llc.y, llc.z);
  plane_d = v3_init(urc.x, urc.y, urc.z);

  // create plane vectors and normal

  vec_ab = v3_sub(plane_b, plane_a);
  vec_ad = v3_sub(plane_d, plane_a);
  vec_n  = v3_cross(vec_ab, vec_ad);

  // get intersection point

  point_is = v3_intersectwithplane(linea, lineb, plane_a, vec_n);

  // get angle of AI from AB and AC to build up the frame square in its actual
  // position

  vec_ai = v3_sub(point_is, plane_a);

  float angle_ab_ai   = v3_angle(vec_ab, vec_ai);
  float angle_ad_ai   = v3_angle(vec_ad, vec_ai);
  float length_vec_ai = v3_length(vec_ai);

  // get relative coordinates

  float x = cos(angle_ad_ai) * length_vec_ai;
  float y = -sin(angle_ad_ai) * length_vec_ai;

  // check quarters

  if (angle_ab_ai > M_PI / 2 && angle_ad_ai > M_PI / 2)
    y *= -1;
  else if (angle_ab_ai > M_PI / 2 && angle_ad_ai < M_PI / 2)
    y *= -1;

  if (x > 0.0 && x < v3_length(vec_ad) && y < 0.0 && y > -v3_length(vec_ab))
    return point_is;
  else
    return v3_init(FLT_MAX, FLT_MAX, FLT_MAX);
}

#endif
