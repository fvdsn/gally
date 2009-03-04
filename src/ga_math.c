#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <error.h>
#include <math.h>
#include "ga_math.h"

vec_t vec_new(float x, float y, float z, float w){
	vec_t v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}
vec_t vec_parse(const char *line){
	vec_t v = vec_new(0,0,0,0);
	float *vf = (float*)&v; 
	const char *lptr = line;
	char *sptr = NULL;
	int i = 0;
	while(i < 4){
		vf[i] = (float)strtod(lptr,&sptr);
		if(lptr == sptr){
			break;
		}else{
			lptr = sptr;
			i++;
		}
	}
	return v;
}	
vec_t vec_add(vec_t a, vec_t b){
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}
vec_t vec_delta(vec_t a, vec_t b){
	b.x -= a.x;
	b.y -= a.y;
	b.z -= a.y;
	return b;
}
vec_t vec_cross(vec_t a, vec_t b){
	vec_t v;
	v.x = a.y * b.z - a.z * b.y;
	v.y = a.z * b.x - a.x * b.z;
	v.z = a.x * b.y - a.y * b.x;
	return v;
}
vec_t vec_scale(float s, vec_t a){
	a.x *= s;
	a.y *= s;
	a.z *= s;
	return a;
}
vec_t vec_norm(vec_t a){
	float len = vec_len(a);
	if(len == 0.0f){
		return vec_new(1.0f,0.0f,0.0f,0.0f);
	}else{
		return vec_scale(1.0f/len,a);
	}
}
vec_t vec_wnorm(vec_t a){
	if(a.w == 0.0f){
		return a;
	}else{
		a = vec_scale(1.0/a.w,a);
		a.w = 1.0;
		return a;
	}
}
vec_t vec_neg(vec_t a){
	a.x = -a.x;
	a.y = -a.y;
	a.z = -a.z;
	return a;
}
vec_t vec_abs(vec_t a){
	if(a.x < 0.0f){ a.x = - a.x; }
	if(a.y < 0.0f){ a.y = - a.y; }
	if(a.z < 0.0f){ a.z = - a.z; }
	return a;
}
vec_t vec_vec(vec_t a){
	if (a.w == 0.0f){
		return a;
	}else{
		a = vec_scale(1.0f/a.w,a);
		a.w = 0.0f;
		return a;
	}
}
vec_t vec_point(vec_t a){
	if(a.w == 0.0){
		a.w = 1.0;
	}
	return a;
}
float vec_len(vec_t a){
	return sqrtf( a.x*a.x + a.y*a.y + a.z*a.z);
}
float vec_dot(vec_t a, vec_t b){
	return a.x*b.x + a.y*b.y + a.z*b.z;
}
static float vec_wdot(vec_t a, vec_t b){
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}
vec_t vec_print(vec_t a){
	printf("[ %f %f %f %f ]\n",a.x,a.y,a.z,a.w);
	return a;
}
int vec_equal(vec_t a, vec_t b){
	a = vec_wnorm(a);
	b = vec_wnorm(b);
	return (a.x == b.x && a.y == b.y && a.z == b.z);
}
#define V_I(v,i)    ((float*)(&v))[i]
#define M_IJ(m,i,j) ((float*)(m))[i*4+j]

mat_t *mat_new_zero(void){
	mat_t *m = (mat_t*)malloc(sizeof(mat_t));
	if(m){
		memset(m,0,sizeof(mat_t));
	}else{
		error_at_line(1,0,__FILE__,__LINE__,"not enough memory\n");
	}
	return m;
}
mat_t *mat_new(	float xx, float yx, float zx, float wx,
		float xy, float yy, float zy, float wy,
		float xz, float yz, float zz, float wz,
		float xw, float yw, float zw, float ww){
	mat_t *m = (mat_t*)malloc(sizeof(mat_t));
	if(m){
		m->x = vec_new(xx,yx,zx,wx);
		m->y = vec_new(xy,yy,zy,wy);
		m->z = vec_new(xz,yz,zz,wz); 
		m->w = vec_new(xw,yw,zw,ww);
	}else{
		error_at_line(1,0,__FILE__,__LINE__,"not enough memory\n");
	}
	return m;
}
mat_t *mat_dup(const mat_t *m){
	mat_t *m2 = (mat_t*)malloc(sizeof(mat_t));
	if(m2){
		memcpy(m2,m,sizeof(mat_t));
	}else{
		error_at_line(1,0,__FILE__,__LINE__,"not enough memory\n");
	}
	return m2;
}
void   mat_free(mat_t *m){
	free(m);
}
const mat_t *mat_print(const mat_t *m){
	printf("[");vec_print(m->x);
	printf(" ");vec_print(m->y);
	printf(" ");vec_print(m->z);
	printf(" ");vec_print(m->w);
	printf("]\n");
	return m;
}
mat_t *mat_add(mat_t *a,  const mat_t *b){
	a->x = vec_add(a->x,b->x);
	a->y = vec_add(a->y,b->y);
	a->z = vec_add(a->z,b->x);
	return a;
}
mat_t *mat_mult(mat_t *a, const mat_t *b){
	int i = 4;
	int j = 4;
	mat_t *m = mat_new_zero();
	while(i--){
		j = 4;
		while(j--){
			M_IJ(m,i,j) = vec_wdot(	mat_get_row(i,a),
						mat_get_col(j,b)	);
		}
	}
	mat_cpy(a,m);
	mat_free(m);
	return a;
}
vec_t mat_vmult(const mat_t *a, vec_t b){
	vec_t r;
	int i = 4;
	while(i--){
		V_I(r,i) = vec_wdot(mat_get_row(i,a),b);
	}
	return r;
}
mat_t *mat_cpy(mat_t *a,  const mat_t *b){
	memcpy(a,b,sizeof(mat_t));
	return a;
}
mat_t *mat_scale(float s, mat_t *a){
	a->x = vec_scale(s,a->x);
	a->y = vec_scale(s,a->y);
	a->z = vec_scale(s,a->z);
	a->w = vec_scale(s,a->w);
	return a;
}
mat_t *mat_set_id(mat_t *a){
	memset(a,0,sizeof(mat_t));
	a->x.x = 1;
	a->y.y = 1;
	a->z.z = 1;
	a->w.w = 1;
	return a;
}
mat_t *mat_set_rot(int naxis, float angle,mat_t *a){
	return NULL;
}
mat_t *mat_set_trans(vec_t t, mat_t *a){
	mat_set_id(a);
	a->x.w = t.x;
	a->y.w = t.y;
	a->z.w = t.z;
	return a;
}
mat_t *mat_set_scale(vec_t t, mat_t *a){
	memset(a,0,sizeof(mat_t));
	a->x.x = t.x;
	a->y.y = t.y;
	a->z.z = t.z;
	a->w.w = 1;
	return a;
}

mat_t *mat_set_col(int ncol, vec_t c,mat_t*a){
	M_IJ(a,0,ncol) = c.x;
	M_IJ(a,1,ncol) = c.y;
	M_IJ(a,2,ncol) = c.z;
	M_IJ(a,3,ncol) = c.w;
	return a;
}
mat_t *mat_set_row(int nrow, vec_t c, mat_t*a){
	vec_t *A = (vec_t*)a;
	A[nrow] = c;
	return a;
}	
vec_t mat_get_col(int ncol, const mat_t *a){
	return vec_new(	M_IJ(a,0,ncol),
			M_IJ(a,1,ncol),
			M_IJ(a,2,ncol),
			M_IJ(a,3,ncol)	);
}
vec_t mat_get_row(int nrow, const mat_t *a){
	const vec_t *A = (const vec_t*)a;
	return A[nrow];
}
/*
int main(int argc, char **argv){
	vec_t vx = vec_new(1,0,0,0);
	vec_t vy = vec_new(0,1,0,0);
	vec_t vz = vec_new(0,0,1,0);
	vec_t v  = vec_new(42,69,66.6,0);
	mat_t *tr = mat_print(mat_set_trans(vec_new(10,0,20,0),mat_new_zero()));
	vec_print(vec_vec(mat_vmult(tr,vec_point(v))));
	mat_print(mat_set_scale(vec_new(0.1,1,2,0),tr));
	vec_print(vec_vec(mat_vmult(tr,vec_point(v))));
	return 1;
}*/
