/*
 * matrix_annotated.h
 * 注释版头文件：解释 `matrix.h` 中的类型与接口，便于新手阅读矩阵工具
 */
#ifndef CODE_MATRIX_H_
#define CODE_MATRIX_H_

#include "zf_common_headfile.h"

// 辅助宏说明：clip/ABS/MAX/MIN 等用于数值防护和比较
#define __weak                __attribute__((weak))
#define clip(x, min, max)    (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#define ABS(x)               (((x) > 0) ? (x) : (-(x)))
#define clip2(x, num)        (clip((x), (-ABS(num)), (ABS(num))))
#define MAX(a, b)            (((a) > (b)) ? (a) : (b))
#define MIN(a, b)            (((a) < (b)) ? (a) : (b))

// 矩阵最大尺寸（此项目仅使用小矩阵）
#define MAX_SIZE (6)
#define ASSERT(x) zf_assert(x)

typedef float matrix_type;
typedef struct
{
    int rows;
    int cols;
    matrix_type data[MAX_SIZE][MAX_SIZE];
}matrix_t;

// 常用外部矩阵实例（在 ekf/imu 等文件中使用）
extern matrix_t error;
extern matrix_t exf_x;

typedef struct
{
    matrix_type roll, pitch, yaw;
}EulerAngles;
extern EulerAngles euler_angle;

// 接口：初始化、转置、乘法、加减、求逆、归一化、打印
void Matrix_Init(matrix_t*martix,int rows,int col);// 初始化矩阵并置零
void Matrix_From_Array(matrix_t* mat, const matrix_type* array,const int rows,const int cols);
void Matrix_Identity(matrix_t* matrix, int size);// 生成单位矩阵
matrix_t Matrix_Transpose(const matrix_t* src);// 矩阵转置
matrix_t multiply_matrices(const matrix_t* A, const matrix_t* B);// 矩阵乘法
matrix_t add_matrices(const matrix_t* A, const matrix_t* B);// 矩阵加法
matrix_t subtract_matrices(const matrix_t* A, const matrix_t* B);// 矩阵减法
int inverse_matrix(matrix_t *A, matrix_t *invA);// 计算矩阵逆，返回 0 表示成功
void normalize_vector(matrix_t *v);// 向量归一化（行向量或列向量）
void print_matrix(const matrix_t* matrix);

#endif /* CODE_MATRIX_ANNOTATED_H_ */
