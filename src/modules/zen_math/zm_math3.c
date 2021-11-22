#ifndef zm_math3_h
#define zm_math3_h

#include <math.h>
#include <stdio.h>

typedef struct _v3_t v3_t;
struct _v3_t
{
  float x, y, z;
};

typedef struct _line3_t line3_t;
struct _line3_t
{
  v3_t v1;
  v3_t v2;
};

v3_t  v3_init(float x, float y, float z);
v3_t  v3_add(v3_t a, v3_t b);
v3_t  v3_sub(v3_t a, v3_t b);
v3_t  v3_scale(v3_t a, float f);
v3_t  v3_cross(v3_t left, v3_t right);
v3_t  v3_normalize(v3_t matrix);
v3_t  v3_rotatearoundx(v3_t vector, float the_angle);
v3_t  v3_rotatearoundy(v3_t vector, float the_angle);
v3_t  v3_rotatearoundz(v3_t vector, float the_angle);
v3_t  v3_getxyunitrotation(v3_t vx, v3_t vy);
v3_t  v3_intersectwithplane(v3_t linev1, v3_t linev2, v3_t planev, v3_t planen);
float v3_dot(v3_t a, v3_t b);
float v3_angle(v3_t a, v3_t b);
float v3_length(v3_t a);
void  v3_toarray(v3_t* vector, float* result);
float v3_distance(v3_t vectorA, v3_t vectorB);

typedef struct _m3_t m3_t;
struct _m3_t
{
  float m00, m01, m02;
  float m10, m11, m12;
  float m20, m21, m22;
};

m3_t m3_defaultidentity(void);
m3_t m3_defaultscale(float x, float y);
m3_t m3_defaulttranslation(float x, float y);
m3_t m3_defaultrotationx(float radian);
m3_t m3_defaultrotationy(float radian);
m3_t m3_defaultrotationz(float radian);
m3_t m3_multiply(m3_t a, m3_t b);
m3_t m3_invert(m3_t source, char* success);
m3_t m3_transpose(m3_t matrix);
v3_t m3_multiply_vector3(m3_t matrix, v3_t vector);
//    v3_t    m3_multiply_v3_transposed( m3_t matrix , v3_t vector );
void m3_multiplywithnumber(m3_t* matrix, float number);
void m3_describe(m3_t matrix);

typedef struct _transformation_t transformation_t;
struct _transformation_t
{
  v3_t translation;
  v3_t rotation;
  v3_t scale;
};

transformation_t
transformation_alloc(void);

typedef struct _vox_t vox_t;
struct _vox_t
{
  float x;
  float y;
  float z;

  float w;
  float h;
  float d;
};

#endif
#if __INCLUDE_LEVEL__ == 0

/* inits vector3 */

v3_t v3_init(float x, float y, float z)
{
  v3_t result;

  result.x = x;
  result.y = y;
  result.z = z;

  return result;
}

/* add two vectors */

v3_t v3_add(v3_t a, v3_t b)
{
  v3_t result;

  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;

  return result;
}

/* substracts b from a */

v3_t v3_sub(v3_t a, v3_t b)
{
  v3_t result;

  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;

  return result;
}

/* scales vector */

v3_t v3_scale(v3_t a, float f)
{
  v3_t result;

  result.x = a.x * f;
  result.y = a.y * f;
  result.z = a.z * f;

  return result;
}

/* creates cross product of two vectors */

v3_t v3_cross(v3_t left, v3_t right)
{
  v3_t v;

  v.x = left.y * right.z - left.z * right.y;
  v.y = left.z * right.x - left.x * right.z;
  v.z = left.x * right.y - left.y * right.x;

  return v;
}

/* normalizes vector */

v3_t v3_normalize(v3_t matrix)
{
  float scale;

  scale = 1.0f / sqrtf(matrix.x * matrix.x + matrix.y * matrix.y +
                       matrix.z * matrix.z);

  matrix.x *= scale;
  matrix.y *= scale;
  matrix.z *= scale;

  return matrix;
}

/* rotates vector around it x axis */

v3_t v3_rotatearoundx(v3_t vector, float the_angle)
{
  float epsilon = 0.00001;
  if (fabs(vector.y) > epsilon || fabs(vector.z) > epsilon)
  {
    float angle  = atan2(vector.z, vector.y);
    float length = sqrtf(vector.y * vector.y + vector.z * vector.z);

    vector.z = sin(angle + the_angle) * length;
    vector.y = cos(angle + the_angle) * length;
  }
  return vector;
}

/* rotates vector around it y axis */

v3_t v3_rotatearoundy(v3_t vector, float the_angle)
{
  float epsilon = 0.00001;
  if (fabs(vector.x) > epsilon || fabs(vector.z) > epsilon)
  {
    float angle  = atan2(vector.z, vector.x);
    float length = sqrtf(vector.x * vector.x + vector.z * vector.z);

    vector.z = sin(angle + the_angle) * length;
    vector.x = cos(angle + the_angle) * length;
  }
  return vector;
}

/* rotates vector around it z axis */

v3_t v3_rotatearoundz(v3_t vector, float the_angle)
{
  float epsilon = 0.00001;
  if (fabs(vector.y) > epsilon || fabs(vector.x) > epsilon)
  {
    float angle  = atan2(vector.y, vector.x);
    float length = sqrtf(vector.x * vector.x + vector.y * vector.y);

    vector.y = sin(angle + the_angle) * length;
    vector.x = cos(angle + the_angle) * length;
  }
  return vector;
}

/* rotates back plane to origo, returns rotation vector */

v3_t v3_getxyunitrotation(v3_t vx, v3_t vy)
{
  v3_t rotation;

  float angle;
  float epsilon = 0.00001;

  // rotate back final_x to base_x on Z axis

  if (fabs(vx.x) > epsilon || fabs(vx.y) > epsilon)
  {

    angle = atan2(vx.y, vx.x);

    rotation.z = angle;

    vx = v3_rotatearoundz(vx, -angle);
    vy = v3_rotatearoundz(vy, -angle);
  }
  else
    rotation.z = 0.0;

  // rotate back final_x to base_x on Y axis

  if (fabs(vx.x) > epsilon || fabs(vx.z) > epsilon)
  {

    angle = atan2(vx.z, vx.x);

    rotation.y = -angle;

    vx = v3_rotatearoundy(vx, -angle);
    vy = v3_rotatearoundy(vy, -angle);
  }
  else
    rotation.y = 0.0;

  // finally rotate back final_y to base_y on X axis

  if (fabs(vy.y) > epsilon || fabs(vy.z) > epsilon)
  {

    angle = atan2(vy.z, vy.y);

    rotation.x = angle;

    vx = v3_rotatearoundx(vx, -angle);
    vy = v3_rotatearoundx(vy, -angle);
  }
  else
    rotation.x = 0.0;

  return rotation;
}

/* intersects vector3 with plane */

v3_t v3_intersectwithplane(v3_t linev1, v3_t linev2, v3_t planev, v3_t planen)
{
  v3_t  u, w, scale, result;
  float div;

  u      = v3_sub(linev2, linev1);
  w      = v3_sub(planev, linev1);
  div    = v3_dot(planen, w) / v3_dot(planen, u);
  scale  = v3_scale(u, div);
  result = v3_add(linev1, scale);

  return result;
}

/* creates dot product of two vectors */

float v3_dot(v3_t a, v3_t b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

/* calculates angle between two vectors */

float v3_angle(v3_t a, v3_t b)
{
  return acosf(v3_dot(a, b) / (v3_length(a) * v3_length(b)));
}

/* calculates vector length */

float v3_length(v3_t a)
{
  return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

/* converts vector to array */

void v3_toarray(v3_t* vector, float* result)
{
  result[0] = vector->x;
  result[1] = vector->y;
  result[2] = vector->z;
}

/* calculates distance of two vectors */

float v3_distance(v3_t vectorA, v3_t vectorB)
{
  float dx, dy, dz;

  dx = vectorB.x - vectorA.x;
  dy = vectorB.y - vectorA.y;
  dz = vectorB.z - vectorA.z;

  return sqrtf(dx * dx + dy * dy + dz * dz);
}

/* creates identity matrix */

m3_t m3_defaultidentity()
{
  m3_t matrix;

  matrix.m00 = 1.0f;
  matrix.m01 = 0.0f;
  matrix.m02 = 0.0f;
  matrix.m10 = 0.0f;
  matrix.m11 = 1.0f;
  matrix.m12 = 0.0f;
  matrix.m20 = 0.0f;
  matrix.m21 = 0.0f;
  matrix.m22 = 1.0f;

  return matrix;
}

/* creates identity matrix */

m3_t m3_defaultscale(float x, float y)
{
  m3_t matrix = m3_defaultidentity();

  matrix.m00 = x;
  matrix.m11 = y;

  return matrix;
}

/* creates translation matrix */

m3_t m3_defaulttranslation(float x, float y)
{
  m3_t result;

  result     = m3_defaultidentity();
  result.m02 = x;
  result.m12 = y;

  return result;
}

/* creates rotationx matrix */

m3_t m3_defaultrotationx(float rad)
{
  m3_t result;

  result     = m3_defaultidentity();
  result.m11 = cosf(rad);
  result.m12 = -sinf(rad);
  result.m21 = sinf(rad);
  result.m22 = cosf(rad);

  return result;
}

/* creates rotationy matrix */

m3_t m3_defaultrotationy(float rad)
{
  m3_t result;

  result     = m3_defaultidentity();
  result.m00 = cosf(rad);
  result.m02 = sinf(rad);
  result.m20 = -sinf(rad);
  result.m22 = cosf(rad);

  return result;
}

/* creates rotationz matrix */

m3_t m3_defaultrotationz(float rad)
{
  m3_t result;

  result     = m3_defaultidentity();
  result.m00 = cosf(rad);
  result.m01 = sinf(rad);
  result.m10 = -sinf(rad);
  result.m11 = cosf(rad);

  return result;
}

/* multiplies two matrixes */

m3_t m3_multiply(m3_t a, m3_t b)
{
  m3_t matrix;

  matrix.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20;
  matrix.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20;
  matrix.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20;

  matrix.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21;
  matrix.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21;
  matrix.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21;

  matrix.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22;
  matrix.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22;
  matrix.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22;

  return matrix;
}

/* inverts matrix */

m3_t m3_invert(m3_t source, char* success)
{
  float determinant;
  m3_t  inverse;

  inverse.m00 = source.m11 * source.m22 - source.m12 * source.m21;

  inverse.m10 = -1 * (source.m01 * source.m22 - source.m02 * source.m21);

  inverse.m20 = source.m01 * source.m12 - source.m02 * source.m11;

  inverse.m01 = -1 * (source.m10 * source.m22 - source.m12 * source.m20);

  inverse.m11 = source.m00 * source.m22 - source.m02 * source.m20;

  inverse.m21 = -1 * (source.m00 * source.m12 - source.m02 * source.m10);

  inverse.m02 = source.m10 * source.m21 - source.m11 * source.m20;

  inverse.m12 = -1 * (source.m00 * source.m21 - source.m01 * source.m20);

  inverse.m22 = source.m00 * source.m11 - source.m01 * source.m10;

  determinant = source.m00 * inverse.m00 + source.m01 * inverse.m01 +
                source.m02 * inverse.m02;

  if (determinant == 0 && success != NULL)
    *success = 0;

  m3_multiplywithnumber(&inverse, 1.0 / determinant);

  return inverse;
}

/* transposes matrix */

m3_t m3_transpose(m3_t matrix)
{
  m3_t result;

  result.m00 = matrix.m00;
  result.m11 = matrix.m11;
  result.m22 = matrix.m22;

  result.m10 = matrix.m01;
  result.m01 = matrix.m10;
  result.m20 = matrix.m02;
  result.m02 = matrix.m20;

  result.m21 = matrix.m12;
  result.m12 = matrix.m21;

  return result;
}

/* multiplies matrix4 with vector 3 */

v3_t m3_multiply_vector3(m3_t matrix, v3_t vector)
{
  v3_t result;

  result.x =
      matrix.m00 * vector.x + matrix.m01 * vector.y + matrix.m02 * vector.z;
  result.y =
      matrix.m10 * vector.x + matrix.m11 * vector.y + matrix.m12 * vector.z;
  result.z =
      matrix.m20 * vector.x + matrix.m21 * vector.y + matrix.m22 * vector.z;

  return result;
}

//	/* multiplies transposed matrix with vector = multiplies vector with
// matrix */
//
//	v3_t m3_multiply_v3_transposed( m3_t matrix , v3_t vector )
//	{
//		v3_t result;
//
//    	result.x = matrix.m00 * vector.x + matrix.m10  * vector.y + matrix.m20 *
//    vector.z;
//		result.y = matrix.m01 * vector.x + matrix.m11  * vector.y +
// matrix.m21 * vector.z; 		result.z = matrix.m02 * vector.x +
// matrix.m12  * vector.y + matrix.m22 * vector.z;
//
//	    return result;
//	}

/* multiplies matrix with number */

void m3_multiplywithnumber(m3_t* matrix, float number)
{
  matrix->m00 *= number;
  matrix->m01 *= number;
  matrix->m02 *= number;
  matrix->m10 *= number;
  matrix->m11 *= number;
  matrix->m12 *= number;
  matrix->m20 *= number;
  matrix->m21 *= number;
  matrix->m22 *= number;
}

/* describes matrix */

void m3_describe(m3_t matrix)
{
  printf("%.2f %.2f %.2f | %.2f %.2f %.2f | %.2f %.2f %.2f",
         matrix.m00,
         matrix.m01,
         matrix.m02,
         matrix.m10,
         matrix.m11,
         matrix.m12,
         matrix.m20,
         matrix.m21,
         matrix.m22);
}

/* transformation default state */

transformation_t
transformation_alloc()
{
  transformation_t result;
  result.translation = v3_init(0.0, 0.0, 0.0);
  result.rotation    = v3_init(0.0, 0.0, 0.0);
  result.scale       = v3_init(1.0, 1.0, 1.0);

  return result;
}

#endif
