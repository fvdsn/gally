#ifndef __GALLY_MATH_H__
#define __GALLY_MATH_H__

typedef struct vec_s{
	float x;
	float y;
	float z;
	float w;
}vec_t;

typedef struct mat_s{
	vec_t x;	/*rows*/
	vec_t y;
	vec_t z;
	vec_t w;
}mat_t;

vec_t vec_new(float x, float y, float z, float w);
vec_t vec_parse(const char *line);
vec_t vec_add(vec_t a, vec_t b);
vec_t vec_delta(vec_t a, vec_t b);
vec_t vec_sub(vec_t a, vec_t b);
vec_t vec_cross(vec_t a, vec_t b);
vec_t vec_scale(float s, vec_t a);
vec_t vec_norm(vec_t a);
vec_t vec_wnorm(vec_t a);
vec_t vec_neg(vec_t a);
vec_t vec_abs(vec_t a);
vec_t vec_vec(vec_t a);
vec_t vec_point(vec_t a);
float vec_len(vec_t a);
float vec_dot(vec_t a, vec_t b);
vec_t vec_print(vec_t a);
int   vec_equal(vec_t a, vec_t b);
/* fast operations */
inline void  vec_fsub(vec_t *d, const vec_t *a, const vec_t *b);
inline void  vec_fcross(vec_t *d, const vec_t *a, const vec_t *b);
inline void  vec_fscale(float f, vec_t *a);
inline void  vec_fnorm(vec_t *a);
inline float vec_fdot(const vec_t *a, const vec_t *b);

mat_t *mat_new_zero(void);
mat_t *mat_new(	float xx, float yx, float zx, float wx,
		float xy, float yy, float zy, float wy,
		float xz, float yz, float zz, float wz,
		float xw, float yw, float zw, float ww);
mat_t *mat_dup(const mat_t *m);
const mat_t *mat_print(const mat_t *m);
void   mat_free(mat_t *m);
mat_t *mat_add(mat_t *a,  const mat_t *b);
mat_t *mat_mult(mat_t *a, const mat_t *b);
vec_t  mat_vmult(const mat_t *a, vec_t b);
mat_t *mat_cpy(mat_t *a,  const mat_t *b);
mat_t *mat_scale(float s, mat_t *a);
mat_t *mat_set_id(mat_t *a);
mat_t *mat_set_rot(int naxis, float angle,mat_t *a);
mat_t *mat_set_trans(vec_t t, mat_t *a);
mat_t *mat_set_scale(vec_t t, mat_t *a);
mat_t *mat_set_col(int ncol, vec_t c, mat_t *a);
vec_t  mat_get_col(int ncol, const mat_t *a);
vec_t  mat_get_row(int nrow, const mat_t *a);


#endif
