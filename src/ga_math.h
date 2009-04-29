#ifndef __GALLY_MATH_H__
#define __GALLY_MATH_H__

#define RAND_NORM 1.0f/(float)RAND_MAX
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
/** 
 * Creates a new vector from a line of text. The vector 
 * components must be separated by whitespace and tabs.
 * Missing components are initialized to 0
 */
vec_t vec_parse(const char *line);
vec_t vec_add(vec_t a, vec_t b);
/**
 * Returns a vector from a to b
 */
vec_t vec_delta(vec_t a, vec_t b);
/* returns a - b */
vec_t vec_sub(vec_t a, vec_t b);
/**
 * Returns the cross product a x b
 */
vec_t vec_cross(vec_t a, vec_t b);
/**
 * Returns the multiplication component by component
 * of x,y,z
 */
vec_t vec_mult(vec_t a, vec_t b);
/**
 * Returns the division a/b component by compoent of x,y,z
 */
vec_t vec_div(vec_t a, vec_t b);
/**
 * Returns the minimum component by component of x,y,z
 */
vec_t vec_min(vec_t a, vec_t b);
/**
 * Returns the maximum component by component of x,y,z
 */
vec_t vec_max(vec_t a, vec_t b);
/**
 * Multiplies x,y,z component of a by s
 */
vec_t vec_scale(float s, vec_t a);
/**
 * Return a normalized in x,y,z components
 * if a is zero, returns [1 0 0 0]
 */
vec_t vec_norm(vec_t a);
/**
 * Divides x,y,z components by w if it is different from zero.
 */
vec_t vec_homog(vec_t a);
/**
 * Returns the vector with opposite x,y,z components.
 */
vec_t vec_neg(vec_t a);
/**
 * Returns the vector with absolute value of x,y,z components
 */
vec_t vec_abs(vec_t a);
/**
 * Homogenize a, and sets w to 0
 */
vec_t vec_vec(vec_t a);
/**
 * Homogenize a and sets w to 1
 */
vec_t vec_point(vec_t a);
/**
 * Returns the norm 2 of x,y,z vector, doesn't take w into account.
 */
float vec_len(vec_t a);
/**
 * Returns a component of a : i=0 -> x, i=1 -> y, i=2 -> z, i=3 -> w
 */
float vec_idx(vec_t a, int i);
/**
 * Returns the dot product of a and b, doesn't take w into account.
 */
float vec_dot(vec_t a, vec_t b);
/**
 * Prints the vector to the console.
 */
vec_t vec_print(vec_t a);
/**
 * Returns 1 if the vector are equals when homogenized.
 */
int   vec_equal(vec_t a, vec_t b);
/**
 * Makes p1 and p2 perpendicular to each other and to a.
 */
void  vec_fperp(const vec_t *a, vec_t *p1, vec_t *p2);

/* fast operations */
/**
 * Sets d to a - b.
 */
inline void  vec_fsub(vec_t *d, const vec_t *a, const vec_t *b);
/**
 * sets a to vec_min(*a,*b)
 */
inline void  vec_fmin(vec_t *a, const vec_t *b);
/**
 * *a = vec_max(*a,*b)
 */
inline void  vec_fmax(vec_t *a, const vec_t *b);
/**
 * *a = vec_add(*a,*b)
 */
inline void  vec_fadd(vec_t *a, const vec_t *b);
/**
 * *a = vec_add(*a,vec_scale(f,*b))
 */
inline void  vec_ffadd(vec_t *a, float f, const vec_t *b);

/**
 * Sets d to cross product of a and b
 */
inline void  vec_fcross(vec_t *d, const vec_t *a, const vec_t *b);
/** 
 * multiplies x,y,z components of a by f
 */
inline void  vec_fscale(float f, vec_t *a);
/**
 * Normalizes a
 */
inline void  vec_fnorm(vec_t *a);
/**
 * Returns the dot product of a and b. doesn't take w into account.
 */
inline float vec_fdot(const vec_t *a, const vec_t *b);
/**
 * Returns a component i of a : 0 ->x, 1 -> y, 2 -> z, 3 -> w
 */
inline float vec_fidx(const vec_t *a, int i);
/**
 * Returns 1 if *a is the zero vector
 */
inline int   vec_fzero(const vec_t *a);
/**
 * Returns the squared distance of a and b
 */
inline float vec_fsqdist(const vec_t *a, const vec_t *b);

/**
 * Allocates a matrix with all components set to zero
 */
mat_t *mat_new_zero(void);
mat_t *mat_new(	float xx, float yx, float zx, float wx,
		float xy, float yy, float zy, float wy,
		float xz, float yz, float zz, float wz,
		float xw, float yw, float zw, float ww);
/**
 * Allocates a matrix equal to m
 */
mat_t *mat_dup(const mat_t *m);
/**
 * Prints m to the console
 */
const mat_t *mat_print(const mat_t *m);
/**
 * frees memory allocated by m.
 */
void   mat_free(mat_t *m);
/**
 * Sets a to the addition of a and b
 */
mat_t *mat_add(mat_t *a,  const mat_t *b);
/**
 * Sets a to the matrix multiplication of a and b.
 */
mat_t *mat_mult(mat_t *a, const mat_t *b);
mat_t *mat_mult2(const mat_t*a, mat_t*b);
/**
 * returns the multiplication of matrix a and vector b
 */
vec_t  mat_vmult(const mat_t *a, vec_t b);
/**
 * Copies the matrix b into matrix a.
 */
mat_t *mat_cpy(mat_t *a,  const mat_t *b);
/**
 * Multiplies all component of matrix a by s.
 */
mat_t *mat_scale(float s, mat_t *a);
/**
 * Sets the matrix a to identity matrix.
 */
mat_t *mat_set_id(mat_t *a);
/**
 * Sets the matrix a to a 2d image projection matrix with
 * image size nx and ny
 */
mat_t *mat_set_2d(int nx, int ny, mat_t *a);
/**
 * Sets the matrix a to the orthographic projection matrix
 * with lbn and rtf bounding box vectors.
 */
mat_t *mat_set_norm(mat_t *n, const mat_t *m);
mat_t *mat_set_ortho(vec_t lbn, vec_t rtf, mat_t *a);
/**
 * Sets the matrix a to the custom camera position projection
 * matrix with camera position in eye, and u,v,w camera vectors
 */
mat_t *mat_set_view(vec_t eye, vec_t u, vec_t v, vec_t w, mat_t *a);
/**
 * Sets a to the perspective matrix with n and f fields values.
 */
mat_t *mat_set_persp(float n, float f,mat_t*a);
/**
 * sets the matrix to rotation matrix perpendicular to selected axis. 
 * angle in degrees.	TODO NOT IMPLEMENTED 
 */
mat_t *mat_set_rot(vec_t dir, float angle,mat_t *a);
/**
 * sets the matrix to translation matrix of vector t
 */
mat_t *mat_set_trans(vec_t t, mat_t *a);
/**
 * sets the matrix to scaling matrix with coefficent t
 */
mat_t *mat_set_scale(vec_t t, mat_t *a);
/**
 * replace a collumn in the matrix by a vector 
 */
mat_t *mat_set_col(int ncol, vec_t c, mat_t *a);
/**
 * replace a row in the matrix by a vector 
 */
mat_t *mat_set_row(int ncol, vec_t c, mat_t *a);
/**
 * returns a collumn of the matrix
 */
vec_t  mat_get_col(int ncol, const mat_t *a);
/**
 * returns a row of the matrix.
 */
vec_t  mat_get_row(int nrow, const mat_t *a);


#endif
