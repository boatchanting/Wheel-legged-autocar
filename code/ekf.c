#include "zf_common_headfile.h"

// 扩展卡尔曼滤波状态变量 (四元数)
matrix_t exf_x;
// 误差变量
matrix_t error;
// 欧拉角结构体
EulerAngles euler_angle;
// IMU数据结构体，初始化为0
imu_t imu_data = {0, 0, 0, 0, 0, 0};
// 误差阈值
matrix_type r_yz = 0.001f;

// 过程噪声协方差矩阵Q
const matrix_type q[4][4] = {{0.005, 0, 0, 0}, {0, 0.005, 0, 0}, {0, 0, 0.005, 0}, {0, 0, 0, 0.005}};
// 测量噪声协方差矩阵R
const matrix_type r[3][3] = {{10000, 0, 0}, {0, 10000, 0}, {0, 0, 10000}};
// 初始协方差矩阵P
const matrix_type p[4][4] = {{1000000, 0, 0, 0}, {0, 1000000, 0, 0}, {0, 0, 1000000, 0}, {0, 0, 0, 1000000}};
// 初始四元数 [1, 0, 0, 0]
const matrix_type ekf[4] = {1, 0, 0, 0};

// 静态矩阵变量
static matrix_t Q;  // 过程噪声协方差矩阵
static matrix_t R;  // 测量噪声协方差矩阵
static matrix_t P;  // 协方差矩阵

/**
 * @brief 初始化扩展卡尔曼滤波器
 * @note 将静态数组转换为矩阵类型
 */
void EKF_Init(void)
{
    // 初始化状态向量exf_x
    Matrix_From_Array(&exf_x, (const matrix_type*)ekf, 4, 1);
    // 初始化过程噪声协方差矩阵Q
    Matrix_From_Array(&Q, (const matrix_type*)q, 4, 4);
    // 初始化测量噪声协方差矩阵R
    Matrix_From_Array(&R, (const matrix_type*)r, 3, 3);
    // 初始化协方差矩阵P
    Matrix_From_Array(&P, (const matrix_type*)p, 4, 4);
}

/**
 * @brief 将四元数转换为欧拉角
 * @note 从exf_x中提取四元数元素并转换为pitch、roll、yaw
 */
static inline void quaternion_to_euler(void)
{
    // 提取四元数元素
    float q0 = (exf_x.data[0][0]);
    float q1 = (exf_x.data[1][0]);
    float q2 = (exf_x.data[2][0]);
    float q3 = (exf_x.data[3][0]);

    // 计算俯仰角(pitch)
    euler_angle.pitch = asin(-2 * q1 * q3 + 2 * q0 * q2) * DEG_TO_RAD;                                  // pitch
    // 计算横滚角(roll)
    // euler_angle.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * DEG_TO_RAD;   // roll原来的
    euler_angle.roll = atan2( -2 * (q2 * q3 + q0 * q1), 2 * q1 * q1 + 2 * q2 * q2 - 1 ) * DEG_TO_RAD; //改为0度为平衡状态的pitch
    // 计算偏航角(yaw)
    euler_angle.yaw = atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * DEG_TO_RAD;    // yaw
}

// IMU加速度计低通滤波变量
static int16 imu660ra_acc_x_l = 0;
static int16 imu660ra_acc_y_l = 0;
static int16 imu660ra_acc_z_l = 0;

/**
 * @brief 获取IMU数据并进行预处理
 * @note 包括加速度计低通滤波和陀螺仪单位转换
 */
void imu_get_values(void)
{
    // 获取陀螺仪数据
    imu660ra_get_gyro();
    // 获取加速度计数据
    imu660ra_get_acc();

    // 一阶低通滤波，计算结果是浮点数，先存入 imu_data (假设 imu_t 成员是 float)
    imu_data.acc_x = K * imu660ra_acc_x + (1 - K) * imu660ra_acc_x_l;
    imu_data.acc_y = K * imu660ra_acc_y + (1 - K) * imu660ra_acc_y_l;
    imu_data.acc_z = K * imu660ra_acc_z + (1 - K) * imu660ra_acc_z_l;

    // 更新滤波器状态 (将浮点数转回 int16 以便下次计算使用)
    imu660ra_acc_x_l = (int16)imu_data.acc_x;  // 强制转换为int16，避免编译警告
    imu660ra_acc_y_l = (int16)imu_data.acc_y;  // 
    imu660ra_acc_z_l = (int16)imu_data.acc_z;  // 
    // --- 修改结束 ---

    // 陀螺仪角度转弧度
    imu_data.gyro_x = imu660ra_gyro_x * PI / 180 / 16.384f;
    imu_data.gyro_y = imu660ra_gyro_y * PI / 180 / 16.384f;
    imu_data.gyro_z = imu660ra_gyro_z * PI / 180 / 16.384f;
}

/**
 * @brief 扩展卡尔曼滤波更新函数
 * @note 执行完整的EKF预测和更新步骤
 */
void EKF_UpData(void)
{
    // 陀螺仪数据
    float gx, gy, gz;
    // 获取IMU数据
    imu_get_values();
    gx = imu_data.gyro_x;
    gy = imu_data.gyro_y;
    gz = imu_data.gyro_z;

    // 测量向量Z (归一化的加速度计数据)
    matrix_t Z;
    Matrix_Init(&Z, 3, 1);

    Z.data[0][0] = (matrix_type)imu_data.acc_x;
    Z.data[1][0] = (matrix_type)imu_data.acc_y;
    Z.data[2][0] = (matrix_type)imu_data.acc_z;

    // 归一化测量向量
    normalize_vector(&Z);

    // 状态转移矩阵F (基于陀螺仪数据)
    matrix_type f[4][4]= {{1, -0.5f * gx * dt, -0.5f * gy * dt, -0.5f * gz * dt},
                          {0.5f * gx * dt, 1, 0.5f * gz * dt, -0.5f * gy * dt},
                          {0.5f * gy * dt, -0.5f * gz * dt, 1, 0.5f * gx * dt},
                          {0.5f * gz * dt, 0.5f * gy * dt, -0.5f * gx * dt, 1}};

    // 状态转移矩阵及其转置
    matrix_t F,FT;
    Matrix_From_Array(&F, (const matrix_type*)f, 4, 4);
    FT = Matrix_Transpose(&F);

    // 状态预测: X = F * X
    exf_x = multiply_matrices(&F, &exf_x);
    // 归一化四元数
    normalize_vector(&exf_x);

    // 提取四元数元素
    float q0 = (exf_x.data[0][0]);
    float q1 = (exf_x.data[1][0]);
    float q2 = (exf_x.data[2][0]);
    float q3 = (exf_x.data[3][0]);

    // 观测矩阵H (将四元数转换为重力方向)
    matrix_type h[3][4]={{-2 * q2, 2 * q3, -2 * q0, 2 * q1},
                         {2 * q1, 2 * q0, 2 * q3, 2 * q2},
                         {2 * q0, -2 * q1, -2 * q2, 2 * q3}};

    // 观测矩阵及其转置
    matrix_t H, HT;
    Matrix_From_Array(&H, (const matrix_type*)h, 3, 4);
    HT = Matrix_Transpose(&H);
    matrix_t PK_;

    // 预测协方差: P = F * P * FT + Q
    PK_ = multiply_matrices(&F, &P);       //F * P;
    PK_ = multiply_matrices(&PK_, &FT);    //F * P * FT;
    P = add_matrices(&PK_, &Q);            //F * P * FT + Q;

    // 计算新息协方差: DK = H * P * HT + R
    matrix_t DK, invDK;
    DK = multiply_matrices(&H, &P);
    DK = multiply_matrices(&DK, &HT);
    DK = add_matrices(&DK, &R);

    // 检查矩阵是否可逆
    if(inverse_matrix(&DK, &invDK))
    {
    	// 矩阵不可逆，直接转换四元数为欧拉角并返回
    	quaternion_to_euler();
    	return;
    }

    // 计算新息: EK = Z - H * X
    matrix_t EK, EKT;
    EK = multiply_matrices(&H, &exf_x);     //H * X;
    EK = subtract_matrices(&Z, &EK);        //Z - HX;
    EKT = Matrix_Transpose(&EK);

    // 计算误差: r = EKT * invDK * EK
    error = multiply_matrices(&EKT, &invDK);
    error = multiply_matrices(&error, &EK);

    // 检查误差是否过大
    if(error.data[0][0] > r_yz)
    {
    	// 误差过大，直接转换四元数为欧拉角并返回
    	quaternion_to_euler();
    	return;
    }

    // 计算卡尔曼增益: Kk = P * HT * invDK
    matrix_t Kk;
    Kk = multiply_matrices(&P, &HT);
    Kk = multiply_matrices(&Kk, &invDK);

    // 更新状态: X = X + Kk * EK
    matrix_t temp;
    temp = multiply_matrices(&Kk, &EK);
    exf_x = add_matrices(&exf_x, &temp);
    // 归一化四元数
    normalize_vector(&exf_x);

    // 更新协方差: P = (I - Kk * H) * P
    matrix_t I;
    Matrix_Identity(&I, 4);
    temp = multiply_matrices(&Kk, &H);
    temp = subtract_matrices(&I, &temp);
    P = multiply_matrices(&temp, &P);
    
    // 转换四元数为欧拉角
    quaternion_to_euler();
    
    // 调试输出 (已注释)
    //printf("%f,%f,%f\n",dt,euler_angle.pitch,euler_angle.roll);
}
