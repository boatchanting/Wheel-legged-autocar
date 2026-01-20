#include "zf_common_headfile.h"
#ifndef CODE_MATRIX_H_
#define CODE_MATRIX_H_


#define __weak                __attribute__((weak))
#define clip(x, min, max)    (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#define ABS(x)               (((x) > 0) ? (x) : (-(x)))
#define clip2(x, num)        (clip((x), (-ABS(num)), (ABS(num))))
#define MAX(a, b)            (((a) > (b)) ? (a) : (b))
#define MIN(a, b)            (((a) < (b)) ? (a) : (b))



#define MAX_SIZE (4)
#define ASSERT(x) zf_assert(x)


typedef float matrix_type;
typedef struct
{
    int rows;
    int cols;
    matrix_type data[MAX_SIZE][MAX_SIZE];
}matrix_t;
extern matrix_t error;
extern matrix_t exf_x;

typedef struct
{
    matrix_type roll, pitch, yaw;
}EulerAngles;
extern EulerAngles euler_angle;


void Matrix_Init(matrix_t*martix,int rows,int col);//创建0矩阵

void Matrix_From_Array(matrix_t* mat, const matrix_type* array,const int rows,const int cols);

void Matrix_Identity(matrix_t* matrix, int size);//创建单位矩阵

matrix_t Matrix_Transpose(const matrix_t* src);//矩阵转置

matrix_t multiply_matrices(const matrix_t* A, const matrix_t* B);//矩阵乘法

matrix_t add_matrices(const matrix_t* A, const matrix_t* B);//矩阵加法

matrix_t subtract_matrices(const matrix_t* A, const matrix_t* B);//矩阵减法

int inverse_matrix(matrix_t *A, matrix_t *invA);//矩阵求逆

void normalize_vector(matrix_t *v);//向量归一化

void print_matrix(const matrix_t* matrix);


#endif /* CODE_MATRIX_H_ */
