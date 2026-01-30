#include "zf_common_headfile.h"

/**
 * @brief 初始化矩阵
 * @param martix 指向要初始化的矩阵的指针
 * @param rows 矩阵的行数
 * @param cols 矩阵的列数
 * @note 使用memset将矩阵数据初始化为0
 */
void Matrix_Init(matrix_t* martix, int rows, int cols)
{
    ASSERT(rows > 0 && cols > 0);  // 断言行数和列数必须大于0
    martix->rows = rows;
    martix->cols = cols;
    memset(martix->data, 0, MAX_SIZE * MAX_SIZE * sizeof(matrix_type));  // 将矩阵数据区全部置0
}

/**
 * @brief 初始化单位矩阵
 * @param matrix 指向要初始化的矩阵的指针
 * @param size 矩阵的维度(行数和列数相同)
 * @note 单位矩阵是对角线元素为1，其余元素为0的方阵
 */
void Matrix_Identity(matrix_t* matrix, int size)
{
    ASSERT(size > 0);  // 断言维度必须大于0
    matrix->rows = size;
    matrix->cols = size;
    memset(matrix->data, 0, sizeof(matrix->data));  // 先将所有元素置0
    for(int i = 0; i < size; i++)
    {
        matrix->data[i][i] = 1.0f;  // 设置对角线元素为1
    }
}

/**
 * @brief 从一维数组创建矩阵
 * @param mat 指向目标矩阵的指针
 * @param array 源一维数组
 * @param rows 目标矩阵的行数
 * @param cols 目标矩阵的列数
 * @note 数组按行优先顺序填充矩阵
 */
void Matrix_From_Array(matrix_t* mat, const matrix_type* array,const int rows,const int cols)
{
    ASSERT(NULL != array);  // 断言源数组不为空
    Matrix_Init(mat, rows, cols);  // 先初始化矩阵
    for(int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            mat->data[i][j] = array[i * cols + j];  // 按行优先顺序填充
        }
    }
}

/**
 * @brief 矩阵转置
 * @param src 指向源矩阵的指针
 * @return 转置后的新矩阵
 * @note 转置矩阵的行数和列数与原矩阵互换
 */
matrix_t Matrix_Transpose(const matrix_t* src)
{
    // 初始化目标矩阵的大小
    matrix_t dest;
    Matrix_Init(&dest, src->cols, src->rows);  // 行列互换
    
    // 执行转置操作
    for(int i = 0; i < src->rows; i++)
    {
        for(int j = 0; j < src->cols; j++)
        {
            dest.data[j][i] = src->data[i][j];  // 行列互换
        }
    }
    return dest;
}

/**
 * @brief 矩阵乘法
 * @param A 指向第一个矩阵的指针
 * @param B 指向第二个矩阵的指针
 * @return 两个矩阵相乘的结果矩阵
 * @note 要求A的列数必须等于B的行数
 */
matrix_t multiply_matrices(const matrix_t* A, const matrix_t* B)
{
    ASSERT(A->cols == B->rows);  // 断言矩阵维度满足乘法要求

    // 初始化结果矩阵
    matrix_t dest;
    Matrix_Init(&dest, A->rows, B->cols);

    // 执行矩阵乘法运算
    for(int i = 0; i < A->rows; i++)
    {
        for(int j = 0; j < B->cols; j++)
        {
            for(int k = 0; k < A->cols; k++)
            {
                dest.data[i][j] += A->data[i][k] * B->data[k][j];  // 矩阵乘法公式
            }
        }
    }

    return dest;  // 返回结果
}

/**
 * @brief 矩阵加法
 * @param A 指向第一个矩阵的指针
 * @param B 指向第二个矩阵的指针
 * @return 两个矩阵相加的结果矩阵
 * @note 要求两个矩阵的行列数必须相同
 */
matrix_t add_matrices(const matrix_t* A, const matrix_t* B)
{
    // 使用 assert 确保两个矩阵的维度相同
    ASSERT(A->rows == B->rows && A->cols == B->cols);

    matrix_t result;
    Matrix_Init(&result, A->rows, A->cols);

    // 执行矩阵加法运算
    for(int i = 0; i < A->rows; i++)
    {
        for(int j = 0; j < A->cols; j++)
        {
            result.data[i][j] = A->data[i][j] + B->data[i][j];  // 对应元素相加
        }
    }

    return result;  // 返回加法结果
}

/**
 * @brief 矩阵减法
 * @param A 指向第一个矩阵的指针
 * @param B 指向第二个矩阵的指针
 * @return 两个矩阵相减的结果矩阵
 * @note 要求两个矩阵的行列数必须相同
 */
matrix_t subtract_matrices(const matrix_t* A, const matrix_t* B)
{
    // 使用 assert 确保两个矩阵的维度相同
    ASSERT(A->rows == B->rows && A->cols == B->cols);

    // 初始化结果矩阵
    matrix_t result;
    Matrix_Init(&result, A->rows, A->cols);

    // 执行矩阵减法运算
    for(int i = 0; i < A->rows; i++)
    {
        for(int j = 0; j < A->cols; j++)
        {
            result.data[i][j] = A->data[i][j] - B->data[i][j];  // 对应元素相减
        }
    }

    return result;  // 返回减法结果
}

/**
 * @brief 高斯消元法求逆矩阵
 * @param A 指向源矩阵的指针
 * @param invA 指向存储逆矩阵的指针
 * @return 成功返回0，失败返回1
 * @note 使用增广矩阵[A|I]通过高斯消元法求逆
 */
int inverse_matrix(matrix_t* A, matrix_t* invA)
{
    ASSERT(A->rows == A->cols);  // 只支持方阵

    const matrix_type THRESHOLD = 1e-6;  // 判断矩阵是否可逆的阈值

    Matrix_Init(invA, A->rows, A->cols);  // 初始化逆矩阵

    int n = A->rows;

    matrix_type augmented[MAX_SIZE][2 * MAX_SIZE];  // 增广矩阵

    // 构建增广矩阵 [A | I]
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            augmented[i][j] = A->data[i][j];
            augmented[i][j + n] = (float)((i == j) ? 1 : 0);  // 构建单位矩阵部分
        }
    }

    // 高斯消元过程
    for(int i = 0; i < n; i++)
    {
        // 找到第 i 列的最大元素
        int max_row = i;
        for(int j = i + 1; j < n; j++)
        {
            if(fabs(augmented[j][i]) > fabs(augmented[max_row][i]))
            {
                max_row = j;
            }
        }

        // 如果主元为 0，说明矩阵不可逆
        if(fabs(augmented[max_row][i]) < THRESHOLD)
        {
            return 1;  // 失败
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

        // 对当前行进行归一化，使主元为 1
        matrix_type pivot = augmented[i][i];
        for(int j = 0; j < 2 * n; j++)
        {
            augmented[i][j] /= pivot;
        }

        // 消去当前列的其他元素
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

    // 提取右半部分 [I | A^-1]
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            invA->data[i][j] = augmented[i][j + n];
        }
    }

    return 0;  // 成功，返回 0
}

/**
 * @brief 快速平方根倒数计算
 * @param x 输入值
 * @return x的平方根倒数
 * @note 使用快速近似算法，比标准库函数快但精度略低
 */
static inline float invSqrt(float x)
{
    float xhalf = 0.5f * x;

    int i = *(int*)&x;  // 将浮点数视为整数

    i = 0x5f375a86 - (i >> 1);  // 魔法常数，用于快速近似

    x = *(float*)&i;  // 将整数转回浮点数

    x = x * (1.5f - xhalf * x * x);  // 牛顿迭代法提高精度

    return x;
}

/**
 * @brief 向量归一化
 * @param v 指向向量矩阵的指针
 * @note 向量可以是行向量(1行n列)或列向量(n行1列)
 */
void normalize_vector(matrix_t *v)
{
    ASSERT(1 == v->cols || 1 == v->rows);  // 断言必须是行向量或列向量

    matrix_type norm = 0;

    // 计算向量的模长
    if(1 == v->rows)  // 行向量情况
    {
        for(int i = 0; i < v->cols; ++i)
        {
            norm += (v->data[0][i] * v->data[0][i]);
        }
    }
    if(1 == v->cols)  // 列向量情况
    {
        for(int i = 0; i < v->rows; ++i)
        {
            norm += (v->data[i][0] * v->data[i][0]);
        }
    }

    norm = invSqrt((float)norm);  // 计算模长的倒数
    
    // 归一化向量
    if(1 == v->rows)  // 行向量情况
    {
        for(int i = 0; i < v->cols; ++i)
        {
            v->data[0][i] *= norm;
        }
    }
    if(1 == v->cols)  // 列向量情况
    {
        for(int i = 0; i < v->rows; ++i)
        {
            v->data[i][0] *= norm;
        }
    }
}

/**
 * @brief 打印矩阵
 * @param matrix 指向要打印的矩阵的指针
 */
void print_matrix(const matrix_t* matrix)
{
    for(int i = 0; i < matrix->rows; i++)
    {
        for(int j = 0; j < matrix->cols; j++)
        {
            printf("%2f ", matrix->data[i][j]);  // 打印每个元素
        }
        printf("\n");  // 每行结束后换行
    }
    printf("\n");  // 矩阵打印完毕后多空一行
}
