#include "zf_common_headfile.h"




void Matrix_Init(matrix_t* martix, int rows, int cols)
{
    ASSERT(rows > 0 && cols > 0);
    martix->rows = rows;
    martix->cols = cols;
    memset(martix->data, 0, MAX_SIZE * MAX_SIZE * sizeof(matrix_type));
}



void Matrix_Identity(matrix_t* matrix, int size)
{
    ASSERT(size > 0);
    matrix->rows = size;
    matrix->cols = size;
    memset(matrix->data, 0, sizeof(matrix->data));
    for(int i = 0; i < size; i++)
    {
        matrix->data[i][i] = 1.0f;
    }
}



void Matrix_From_Array(matrix_t* mat, const matrix_type* array,const int rows,const int cols)
{
    ASSERT(NULL != array);
    Matrix_Init(mat, rows, cols);
    for(int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            mat->data[i][j] = array[i * cols + j];
        }
    }
}




matrix_t Matrix_Transpose(const matrix_t* src)
{
    // 设置目标矩阵的大小
    matrix_t dest;
    Matrix_Init(&dest, src->cols, src->rows);
    // 进行转置操作
    for(int i = 0; i < src->rows; i++)
    {
        for(int j = 0; j < src->cols; j++)
        {
            dest.data[j][i] = src->data[i][j];
        }
    }
    return dest;
}




// 矩阵乘法
matrix_t multiply_matrices(const matrix_t* A, const matrix_t* B)
{
    ASSERT(A->cols == B->rows);

    // 初始化结果矩阵
    matrix_t dest;
    Matrix_Init(&dest, A->rows, B->cols);

    // 进行矩阵乘法运算
    for(int i = 0; i < A->rows; i++)
    {
        for(int j = 0; j < B->cols; j++)
        {
            for(int k = 0; k < A->cols; k++)
            {
                dest.data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }

    return dest;  // 成功
}



// 矩阵加法
matrix_t add_matrices(const matrix_t* A, const matrix_t* B)
{
    // 使用 assert 来确保两个矩阵的维度相同
    ASSERT(A->rows == B->rows && A->cols == B->cols);

    matrix_t result;
    Matrix_Init(&result, A->rows, A->cols);

    // 进行矩阵加法运算
    for(int i = 0; i < A->rows; i++)
    {
        for(int j = 0; j < A->cols; j++)
        {
            result.data[i][j] = A->data[i][j] + B->data[i][j];
        }
    }

    return result;  // 返回加法结果矩阵
}




// 矩阵减法
matrix_t subtract_matrices(const matrix_t* A, const matrix_t* B)
{
    // 使用 assert 来确保两个矩阵的维度相同
    ASSERT(A->rows == B->rows && A->cols == B->cols);

    // 初始化结果矩阵
    matrix_t result;
    Matrix_Init(&result, A->rows, A->cols);

    // 进行矩阵减法运算
    for(int i = 0; i < A->rows; i++)
    {
        for(int j = 0; j < A->cols; j++)
        {
            result.data[i][j] = A->data[i][j] - B->data[i][j];
        }
    }

    return result;  // 返回减法结果矩阵
}




// 高斯消元法求矩阵的逆
int inverse_matrix(matrix_t* A, matrix_t* invA)
{
    ASSERT(A->rows == A->cols);  // 仅支持方阵

    const matrix_type THRESHOLD = 1e-6;  // 根据具体情况调整阈值

    Matrix_Init(invA, A->rows, A->cols);

    int n = A->rows;

    matrix_type augmented[MAX_SIZE][2 * MAX_SIZE];

    // 构造增广矩阵 [A | I]
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            augmented[i][j] = A->data[i][j];
            augmented[i][j + n] = (float)((i == j) ? 1 : 0);  // 设置单位矩阵部分
        }
    }

    // 高斯消元过程
    for(int i = 0; i < n; i++)
    {
        // 找到第 i 列的主元素
        int max_row = i;
        for(int j = i + 1; j < n; j++)
        {
            if(fabs(augmented[j][i]) > fabs(augmented[max_row][i]))
            {
                max_row = j;
            }
        }

        // 如果主元素为 0，说明矩阵不可逆
        if(fabs(augmented[max_row][i]) < THRESHOLD)
        {
            return 1;  // 不可逆
        }

        // 交换当前行和最大行
        if(max_row != i)
        {
            for(int j = 0; j < 2 * n; j++)
            {
                matrix_type temp = augmented[i][j];
                augmented[i][j] = augmented[max_row][j];
                augmented[max_row][j] = temp;
            }
        }

        // 对当前行进行归一化，使得主元素为 1
        matrix_type pivot = augmented[i][i];
        for(int j = 0; j < 2 * n; j++)
        {
            augmented[i][j] /= pivot;
        }

        // 消去当前列其他行的元素
        for(int j = 0; j < n; j++)
        {
            if(j != i)
            {
                matrix_type factor = augmented[j][i];
                for(int k = 0; k < 2 * n; k++)
                {
                    augmented[j][k] -= factor * augmented[i][k];
                }
            }
        }
    }

    // 提取逆矩阵部分 [I | A^-1]
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            invA->data[i][j] = augmented[i][j + n];
        }
    }

    return 0;  // 可逆，返回 0
}





static inline float invSqrt(float x)
{
    float xhalf = 0.5f * x;

    int i = *(int*)&x;

    i = 0x5f375a86 - (i >> 1);

    x = *(float*)&i;

    x = x * (1.5f - xhalf * x * x);

    return x;
}




void normalize_vector(matrix_t *v)
{
    ASSERT(1 == v->cols || 1 == v->rows);

    matrix_type norm = 0;

    if(1 == v->rows)
    {
        for(int i = 0; i < v->cols; ++i)
        {
            norm += (v->data[0][i] * v->data[0][i]);
        }
    }
    if(1 == v->cols)
    {
        for(int i = 0; i < v->rows; ++i)
        {
            norm += (v->data[i][0] * v->data[i][0]);
        }
    }

    norm = invSqrt((float)norm);
    if(1 == v->rows)
    {
        for(int i = 0; i < v->cols; ++i)
        {
            v->data[0][i] *= norm;
        }
    }
    if(1 == v->cols)
    {
        for(int i = 0; i < v->rows; ++i)
        {
            v->data[i][0] *= norm;
        }
    }
}





void print_matrix(const matrix_t* matrix)
{
    for(int i = 0; i < matrix->rows; i++)
    {
        for(int j = 0; j < matrix->cols; j++)
        {
            printf("%2f ", matrix->data[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}



