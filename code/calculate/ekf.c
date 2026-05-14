#include "ekf.h"

// ========================================================================
// IMU 硬件抽象层：根据 IMU_CATEGORY 统一访问接口
// ========================================================================
#if IMU_CATEGORY == 1  // IMU660RA
    #define IMU_GET_GYRO()        imu660ra_get_gyro()
    #define IMU_GET_ACC()         imu660ra_get_acc()
    #define IMU_GYRO_X            imu660ra_gyro_x
    #define IMU_GYRO_Y            imu660ra_gyro_y
    #define IMU_GYRO_Z            imu660ra_gyro_z
    #define IMU_ACC_X             imu660ra_acc_x
    #define IMU_ACC_Y             imu660ra_acc_y
    #define IMU_ACC_Z             imu660ra_acc_z
    #define IMU_ACC_X_OFFSET      imu660ra_acc_x_AND
    #define IMU_ACC_Y_OFFSET      imu660ra_acc_y_AND
    #define IMU_ACC_Z_OFFSET      imu660ra_acc_z_AND
    #define IMU_ACC_X_LP          imu660ra_acc_x_l
    #define IMU_ACC_Y_LP          imu660ra_acc_y_l
    #define IMU_ACC_Z_LP          imu660ra_acc_z_l
#elif IMU_CATEGORY == 2  // IMU660RB
    #define IMU_GET_GYRO()        imu660rb_get_gyro()
    #define IMU_GET_ACC()         imu660rb_get_acc()
    #define IMU_GYRO_X            imu660rb_gyro_x
    #define IMU_GYRO_Y            imu660rb_gyro_y
    #define IMU_GYRO_Z            imu660rb_gyro_z
    #define IMU_ACC_X             imu660rb_acc_x
    #define IMU_ACC_Y             imu660rb_acc_y
    #define IMU_ACC_Z             imu660rb_acc_z
    #define IMU_ACC_X_OFFSET      imu660ra_acc_x_AND
    #define IMU_ACC_Y_OFFSET      imu660ra_acc_y_AND
    #define IMU_ACC_Z_OFFSET      imu660ra_acc_z_AND //这里暂时未使用，所以先这样命名
    #define IMU_ACC_X_LP          imu660rb_acc_x_l
    #define IMU_ACC_Y_LP          imu660rb_acc_y_l
    #define IMU_ACC_Z_LP          imu660rb_acc_z_l
#elif IMU_CATEGORY == 3  // IMU963RA
    #define IMU_GET_GYRO()        imu963ra_get_gyro()
    #define IMU_GET_ACC()         imu963ra_get_acc()
    #define IMU_GYRO_X            imu963ra_gyro_x
    #define IMU_GYRO_Y            imu963ra_gyro_y
    #define IMU_GYRO_Z            imu963ra_gyro_z
    #define IMU_ACC_X             imu963ra_acc_x
    #define IMU_ACC_Y             imu963ra_acc_y
    #define IMU_ACC_Z             imu963ra_acc_z
    #define IMU_ACC_X_OFFSET      imu963ra_acc_x_AND
    #define IMU_ACC_Y_OFFSET      imu963ra_acc_y_AND
    #define IMU_ACC_Z_OFFSET      imu963ra_acc_z_AND //这里暂时未使用，所以先这样命名【优化点】
    #define IMU_ACC_X_LP          imu963ra_acc_x_l
    #define IMU_ACC_Y_LP          imu963ra_acc_y_l
    #define IMU_ACC_Z_LP          imu963ra_acc_z_l
#else
    #error "Unsupported IMU_CATEGORY value"
#endif


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
const matrix_type q[4][4] = {{0.001, 0, 0, 0}, {0, 0.001, 0, 0}, {0, 0, 0.001, 0}, {0, 0, 0, 0.001}};
// 测量噪声协方差矩阵R
const matrix_type r[3][3] = {{10000, 0, 0}, {0, 10000, 0}, {0, 0, 10000}};

// 定义两个 R 矩阵的值(这里优化可以这么用测试一下,暂时尚未优化)
// // R_FAST: 启动时用，极小，信任加速度计，瞬间收敛 (10 ~ 50)
// const matrix_type r_fast[3][3] = {{20, 0, 0}, {0, 20, 0}, {0, 0, 20}};

// // R_SLOW: 稳定后用，极大，信任陀螺仪，过滤震动 (你原来的 10000)
// const matrix_type r_slow[3][3] = {{10000, 0, 0}, {0, 10000, 0}, {0, 0, 10000}};

// 初始协方差矩阵P
const matrix_type p[4][4] = {{1000000, 0, 0, 0}, {0, 1000000, 0, 0}, {0, 0, 1000000, 0}, {0, 0, 0, 1000000}};
// 初始四元数 [1, 0, 0, 0]
//const matrix_type ekf[4] = {1, 0, 0, 0};//原先代码中的值
#if IMU_CATEGORY == 1 && CAR_SELECT == 0 //imu660ra
const matrix_type ekf[4]= {0.707107f, 0.0f, -0.707107f, 0.0f};//学习板小车使用的
// 静态矩阵变量
#endif
#if IMU_CATEGORY == 1 && CAR_SELECT == 3 //imu660ra
const matrix_type ekf[4]= {0.707107f, 0.0f, -0.707107f, 0.0f};
// 静态矩阵变量
#endif
#if IMU_CATEGORY == 3 && CAR_SELECT == 0 ///imu963ra
const matrix_type ekf[4]= {0.707107f, 0.0f, -0.707107f, 0.0f};
#endif
#if IMU_CATEGORY == 3&& CAR_SELECT == 3 //imu963ra
const matrix_type ekf[4]= {-0.5, -0.5, 0.5, 0.5};
// 静态矩阵变量
#endif
static matrix_t Q;  // 过程噪声协方差矩阵
static matrix_t R;  // 测量噪声协方差矩阵
static matrix_t P;  // 协方差矩阵
static volatile bool  start_yaw_stability_check = false;//偏航角稳定性检测
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
    start_yaw_stability_check = true;//初始化偏航角稳定性检测
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

    #if IMU_CATEGORY == 1&&CAR_SELECT == 0 //imu660ra
    // 计算翻滚角(roll)
    euler_angle.roll = asin(-2 * q1 * q3 + 2 * q0 * q2) * DEG_TO_RAD;                                  // pitch
    // 计算俯仰角(pitch)
    // euler_angle.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * DEG_TO_RAD;   // roll原来的
    euler_angle.pitch = atan2( -2 * (q2 * q3 + q0 * q1), 2 * q1 * q1 + 2 * q2 * q2 - 1 ) * DEG_TO_RAD; //改为0度为平衡状态的pitch，学习板
    // euler_angle.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * DEG_TO_RAD;//我们的板子1
    // 计算偏航角(yaw)
    euler_angle.yaw = atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * DEG_TO_RAD;    // yaw
    #endif
    #if IMU_CATEGORY == 1&&CAR_SELECT == 3 //imu660ra
    // 计算翻滚角(roll)：新的 roll = -原来的 pitch
    euler_angle.roll = -atan2(-2 * (q2 * q3 + q0 * q1), 2 * q1 * q1 + 2 * q2 * q2 - 1) * DEG_TO_RAD;

    // 计算俯仰角(pitch)：新的 pitch = 原来的 roll
    euler_angle.pitch = asin(-2 * q1 * q3 + 2 * q0 * q2) * DEG_TO_RAD;

    // 计算偏航角(yaw)：新的 yaw = 原来的 yaw + 90°
    euler_angle.yaw = (atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) ) * DEG_TO_RAD + 90.0;
    #endif
    #if IMU_CATEGORY == 3&&CAR_SELECT == 0//imu963ra //这里面根据实际测试使用了面向结果编程，imu换轴的时候使用转轴公式，或者根据上位机波形来判断一下
    // 计算翻滚角(roll)
    euler_angle.roll = atan2( -2 * (q2 * q3 + q0 * q1), 2 * q1 * q1 + 2 * q2 * q2 - 1 ) * DEG_TO_RAD;                                // pitch
    // 计算俯仰角(pitch)
    // euler_angle.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * DEG_TO_RAD;   // roll原来的
    euler_angle.pitch = -asin(-2 * q1 * q3 + 2 * q0 * q2) * DEG_TO_RAD;   //改为0度为平衡状态的pitch，学习板
    // euler_angle.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * DEG_TO_RAD;//我们的板子1
    // 计算偏航角(yaw)
    euler_angle.yaw = atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * DEG_TO_RAD-90;    // yaw
    //if(euler_angle.yaw   < -180.0f) euler_angle.yaw  += 360.0f;
    #endif
    #if IMU_CATEGORY == 3&&CAR_SELECT == 3//imu963ra //这里面根据实际测试使用了面向结果编程，imu换轴的时候使用转轴公式，或者根据上位机波形来判断一下
    // 直接从旧四元数计算新坐标系下的欧拉角
    euler_angle.roll  = -atan2(2.0f * (q0 * q2 - q3 * q1),1.0f - 2.0f * (q1 * q1 + q2 * q2)) * DEG_TO_RAD-180.0f;
    euler_angle.pitch = -asin(2.0f * (q0 * q1 + q2 * q3)) * DEG_TO_RAD;
    euler_angle.yaw   = -atan2(2.0f * (q0 * q3 - q1 * q2),1.0f - 2.0f * (q2 * q2 + q3 * q3))  * DEG_TO_RAD - 90.0f;
    #endif
}

// IMU加速度计低通滤波变量
static int16 IMU_ACC_X_LP = 0;
static int16 IMU_ACC_Y_LP = 0;
static int16 IMU_ACC_Z_LP = 0;

// --- 静态偏移量变量 ---
static float gyro_offset_x = 0.0f;
static float gyro_offset_y = 0.0f;
static float gyro_offset_z = 0.0f;
// 加速度静态偏移量变量
static float acc_offset_x = 0.0f;
static float acc_offset_y = 0.0f;
static float acc_offset_z = 0.0f;
// --- 死区阈值 (根据传感器噪声调整) ---
#define GYRO_DEAD_ZONE 8.0f 
float imu660ra_acc_x_AND=0.0f;//这里暂时未使用，所以先这样命名【优化点】下面几行也要改掉
float imu660ra_acc_y_AND=0.0f;
float imu660ra_acc_z_AND=0.0f;
 

/**
 * @brief 全方位陀螺仪校准
 * @note 必须在静止状态下调用
 */
void IMU_Calibrate_All_Gyro(void)
{
    float sum_x = 0, sum_y = 0, sum_z = 0;
    float sum_x_a = 0, sum_y_a = 0, sum_z_a = 0;
    const int sample_count = 2000;

    for(int i = 0; i < sample_count; i++)
    {   

        IMU_GET_GYRO();
        IMU_GET_ACC();
        if(IMU_GYRO_X==0||IMU_GYRO_Y==0||IMU_GYRO_Z==0||IMU_ACC_X==0||IMU_ACC_Y==0||IMU_ACC_Z==0)
        {
            i--;
            continue;
        }
        sum_x += IMU_GYRO_X;
        sum_y += IMU_GYRO_Y;
        sum_z += IMU_GYRO_Z;
        sum_x_a += IMU_ACC_X;
        sum_y_a += IMU_ACC_Y;
        sum_z_a += IMU_ACC_Z;
        // 简单延时
        // system_delay_us(100); 
    }

    // 计算三轴的平均零偏
    gyro_offset_x = sum_x / sample_count;
    gyro_offset_y = sum_y / sample_count;
    gyro_offset_z = sum_z / sample_count;
    // 计算加速度计的平均零偏
    acc_offset_x = sum_x_a / sample_count;
    acc_offset_y = sum_y_a / sample_count;
    acc_offset_z = sum_z_a / sample_count;
    imu660ra_acc_x_AND=acc_offset_x;//【优化点】暂时未使用，这三行都是
    imu660ra_acc_y_AND=acc_offset_y;
    imu660ra_acc_z_AND=acc_offset_z;
}
/**
 * @brief 获取IMU数据并进行预处理
 * @note 包括加速度计低通滤波和陀螺仪单位转换
 */
void imu_get_values(void)
{
    // 1. 获取原始数据
    IMU_GET_GYRO();
    IMU_GET_ACC();

    // 2. 减去零偏 (这里是报错的地方，已修正为新变量名)
    // 之前报错是因为写成了 gyro_z_offset
    float gx_temp = (float)IMU_GYRO_X - gyro_offset_x; 
    float gy_temp = (float)IMU_GYRO_Y - gyro_offset_y;
    float gz_temp = (float)IMU_GYRO_Z - gyro_offset_z; 
    // 3. 死区处理
    if (fabs(gx_temp) < GYRO_DEAD_ZONE) gx_temp = 0.0f;
    if (fabs(gy_temp) < GYRO_DEAD_ZONE) gy_temp = 0.0f;
    if (fabs(gz_temp) < GYRO_DEAD_ZONE) gz_temp = 0.0f;

    // 4. 单位转换 (使用修正后的 temp 变量)
    #if IMU_CATEGORY == 1//660ra
    imu_data.gyro_x = gx_temp * PI / 180 / 16.384f;
    imu_data.gyro_y = gy_temp * PI / 180 / 16.384f;
    imu_data.gyro_z = gz_temp * PI / 180 / 16.384f;
    #endif
    #if IMU_CATEGORY == 3//963ra
    imu_data.gyro_x = gx_temp * PI / 180 / 14.3f;
    imu_data.gyro_y = gy_temp * PI / 180 / 14.3f;
    imu_data.gyro_z = gz_temp * PI / 180 / 14.3f;
    #endif


    // --- 加速度计部分保持不变 ---
    imu_data.acc_x = K * IMU_ACC_X + (1 - K) * IMU_ACC_X_LP;
    imu_data.acc_y = K * IMU_ACC_Y + (1 - K) * IMU_ACC_Y_LP;
    imu_data.acc_z = K * IMU_ACC_Z + (1 - K) * IMU_ACC_Z_LP;

    IMU_ACC_X_LP = (int16)imu_data.acc_x;
    IMU_ACC_Y_LP = (int16)imu_data.acc_y;
    IMU_ACC_Z_LP = (int16)imu_data.acc_z;
}

/**
 * @brief 扩展卡尔曼滤波更新函数 (已融合重力估计)
 * @note 执行完整的EKF预测、更新步骤以及重力分量提取
 */
void EKF_UpData(void)
{
    // 1. 获取并处理数据
    float gx, gy, gz;
    imu_get_values(); // 获取最新的 acc 和 gyro 数据
    gx = imu_data.gyro_x;
    gy = imu_data.gyro_y;
    gz = imu_data.gyro_z;

    // 2. 测量向量Z (归一化的加速度计数据)
    matrix_t Z;
    Matrix_Init(&Z, 3, 1);
    Z.data[0][0] = (matrix_type)imu_data.acc_x;
    Z.data[1][0] = (matrix_type)imu_data.acc_y;
    Z.data[2][0] = (matrix_type)imu_data.acc_z;
    normalize_vector(&Z); // 归一化，只保留方向信息

    // 3. 状态预测: X = F * X (基于陀螺仪积分)
    matrix_type f[4][4]= {{1, -0.5f * gx * dt, -0.5f * gy * dt, -0.5f * gz * dt},
                          {0.5f * gx * dt, 1, 0.5f * gz * dt, -0.5f * gy * dt},
                          {0.5f * gy * dt, -0.5f * gz * dt, 1, 0.5f * gx * dt},
                          {0.5f * gz * dt, 0.5f * gy * dt, -0.5f * gx * dt, 1}};
    matrix_t F, FT;
    Matrix_From_Array(&F, (const matrix_type*)f, 4, 4);
    FT = Matrix_Transpose(&F);
    exf_x = multiply_matrices(&F, &exf_x);
    normalize_vector(&exf_x); // 预测后归一化四元数

    // 提取预测后的四元数
    float q0 = (exf_x.data[0][0]);
    float q1 = (exf_x.data[1][0]);
    float q2 = (exf_x.data[2][0]);
    float q3 = (exf_x.data[3][0]);

    // 4. 观测矩阵 H (雅可比矩阵)
    // 对应重力向量 [0,0,1] 旋转后的偏导数
    matrix_type h[3][4]={{-2 * q2, 2 * q3, -2 * q0, 2 * q1},
                         {2 * q1, 2 * q0, 2 * q3, 2 * q2},
                         {2 * q0, -2 * q1, -2 * q2, 2 * q3}};
    matrix_t H, HT;
    Matrix_From_Array(&H, (const matrix_type*)h, 3, 4);
    HT = Matrix_Transpose(&H);

    // 5. 协方差预测: P = F * P * FT + Q
    matrix_t PK_;
    PK_ = multiply_matrices(&F, &P);
    PK_ = multiply_matrices(&PK_, &FT);
    P = add_matrices(&PK_, &Q);

    // 6. 计算卡尔曼增益和更新
    matrix_t DK, invDK;
    DK = multiply_matrices(&H, &P);
    DK = multiply_matrices(&DK, &HT);
    DK = add_matrices(&DK, &R);

    if(inverse_matrix(&DK, &invDK)) { quaternion_to_euler(); return; }

    matrix_t EK, EKT;
    EK = multiply_matrices(&H, &exf_x); // h(x) 的线性近似
    EK = subtract_matrices(&Z, &EK);    // 残差
    EKT = Matrix_Transpose(&EK);

    // 误差检查
    error = multiply_matrices(&EKT, &invDK);
    error = multiply_matrices(&error, &EK);
    if(error.data[0][0] > r_yz) { quaternion_to_euler(); return; }

    matrix_t Kk;
    Kk = multiply_matrices(&P, &HT);
    Kk = multiply_matrices(&Kk, &invDK);

    matrix_t temp;
    temp = multiply_matrices(&Kk, &EK);
    exf_x = add_matrices(&exf_x, &temp); // 状态更新
    normalize_vector(&exf_x);            // 更新后再次归一化

    // 更新协方差
    matrix_t I;
    Matrix_Identity(&I, 4);
    temp = multiply_matrices(&Kk, &H);
    temp = subtract_matrices(&I, &temp);
    P = multiply_matrices(&temp, &P);

    // =========================================================================
    // 融合部分：重力在传感器坐标系下的三轴分量估计
    // =========================================================================
    // 必须使用更新后的最新四元数
    q0 = (exf_x.data[0][0]);
    q1 = (exf_x.data[1][0]);
    q2 = (exf_x.data[2][0]);
    q3 = (exf_x.data[3][0]);

    // 核心原理：将世界坐标系的重力 [0, 0, 1] 旋转到传感器坐标系
    // 公式源自 Madgwick 算法中的 rotateAndScaleVector 函数优化版
    // 对应旋转矩阵 R 的转置矩阵 R^T 的第三列
    
    // X轴分量: 2 * (q1*q3 - q0*q2)
    imu_data.grav_x = 2.0f * (q1 * q3 - q0 * q2);
    
    // Y轴分量: 2 * (q0*q1 + q2*q3)
    imu_data.grav_y = 2.0f * (q0 * q1 + q2 * q3);
    
    // Z轴分量: 1 - 2*(q1*q1 + q2*q2) (Madgwick 写法为 2 * (0.5 - q1^2 - q2^2))
    imu_data.grav_z = 2.0f * (0.5f - q1 * q1 - q2 * q2);
    /* 
       应用说明：
       现在 imu_data.grav_x/y/z 表示的是传感器处于当前姿态时，
       重力加速度(1g)在XYZ三个轴上的理论投影分量（单位化向量）。
       
       如果想获得不含重力的"运动加速度"(Linear Acceleration)，
       可以用原始加速度数据(需先换算成 g 单位) 减去 这个分量。
    */
    // =========================================================================

    // 7. 转换为欧拉角供控制使用
    quaternion_to_euler();
}

// ================== 偏航角零点初始化模块实现开始 ==================

// --- 全局变量定义 ---
// 这是变量的实体，内存会在这里分配
volatile float g_initial_yaw = 0.0f;
volatile bool  g_yaw_initialized = false;
static volatile bool  start_yaw_stability_check;

// --- 内部静态变量 ---
// static 关键字使这些变量仅在 ekf.c 文件内部可见，实现了信息隐藏
static uint32_t yaw_init_start_time = 0;
static float    yaw_at_init_start = 0.0f;

/**
 * @brief  检查车模是否稳定，如果稳定则记录初始偏航角作为零点 (实现)
 * @param  current_tick 当前的中断计数值 (来自 loop_counter)
 */
void record_initial_yaw_task(uint32_t current_tick)
{
    // 如果已经初始化完成，或者主函数还没允许开始，则直接返回
    if (g_yaw_initialized || !start_yaw_stability_check)
    {
        return;
    }

    if (yaw_init_start_time == 0)
    {
        // 第一次进入或重置后，开始新的100ms计时窗口
        yaw_init_start_time = current_tick;
        yaw_at_init_start = euler_angle.yaw; // 假设 euler_angle 在此文件可见
    }
    else if (current_tick - yaw_init_start_time >= 100) // 检查是否已过去至少100ms
    {
        float current_yaw = euler_angle.yaw;
        float yaw_change = fabsf(current_yaw - yaw_at_init_start);

        if (yaw_change < 1.0f)
        {
            // 条件满足，记录当前角度为初始零度角
            g_initial_yaw = current_yaw;
            g_yaw_initialized = true; // 设置成功标志
        }
        else
        {
            // 不稳定，重置计时器，下一周期重新开始检测
            yaw_init_start_time = 0;
        }
    }
}
// ======================== 偏航角零点初始化模块实现结束 ==================


#if IMU_CATEGORY == 3  // IMU963RA的磁力计模块
//【优化点】在嵌入式中，建议使用 sinf, cosf, atan2f（带f后缀的），它们是针对 float 类型的，效率更高。
volatile float heading = 0.0f;
float mag_x = 0.0f;
float mag_y = 0.0f;
float mag_z = 0.0f;
static float mag_raw_lpf_x = 0.0f;
static float mag_raw_lpf_y = 0.0f;
static float mag_raw_lpf_z = 0.0f;
static uint8_t mag_raw_lpf_inited = 0;
#define MAG_RAW_LPF_ALPHA 0.15f
// =========================================================================
// 2. 磁力计校准参数 (常量定义在函数外部)
// =========================================================================
// 硬铁偏移：从原始读数中减去
const float MAG_OFFSET_X = -236.46f;
const float MAG_OFFSET_Y = -167.84f;
const float MAG_OFFSET_Z = 274.64f;

// 软铁校正矩阵
const float MAG_SOFT_IRON[3][3] = {
    {0.988940f, 0.000863f, 0.007377f},
    {0.000863f, 0.989113f, 0.005068f},
    {0.007377f, 0.005068f, 1.021947f}
};

// =========================================================================
// 3. 磁力计校准函数
// =========================================================================
void mag_calibrate(float raw_x, float raw_y, float raw_z, float *mag_x, float *mag_y, float *mag_z)
{
    // 1. 减去硬铁偏移
    float x = raw_x - MAG_OFFSET_X;
    float y = raw_y - MAG_OFFSET_Y;
    float z = raw_z - MAG_OFFSET_Z;
    
    // 2. 应用软铁校正矩阵
    *mag_x = MAG_SOFT_IRON[0][0] * x + MAG_SOFT_IRON[0][1] * y + MAG_SOFT_IRON[0][2] * z;
    *mag_y = MAG_SOFT_IRON[1][0] * x + MAG_SOFT_IRON[1][1] * y + MAG_SOFT_IRON[1][2] * z;
    *mag_z = MAG_SOFT_IRON[2][0] * x + MAG_SOFT_IRON[2][1] * y + MAG_SOFT_IRON[2][2] * z;
}
void EKF_Update_Heading(void)
{
    // 1. 获取物理值 (局部变量可以在函数内直接初始化)
    imu963ra_get_mag();
    float raw_mag_x = (float)imu963ra_mag_x;
    float raw_mag_y = (float)imu963ra_mag_y;
    float raw_mag_z = (float)imu963ra_mag_z;

    if (!mag_raw_lpf_inited)
    {
        mag_raw_lpf_x = raw_mag_x;
        mag_raw_lpf_y = raw_mag_y;
        mag_raw_lpf_z = raw_mag_z;
        mag_raw_lpf_inited = 1;
    }
    else
    {
        mag_raw_lpf_x += MAG_RAW_LPF_ALPHA * (raw_mag_x - mag_raw_lpf_x);
        mag_raw_lpf_y += MAG_RAW_LPF_ALPHA * (raw_mag_y - mag_raw_lpf_y);
        mag_raw_lpf_z += MAG_RAW_LPF_ALPHA * (raw_mag_z - mag_raw_lpf_z);
    }
    // ---------------------------------------------------------
    // 步骤1：获取磁力计原始数据，并进行校准
    // ---------------------------------------------------------
    mag_calibrate(mag_raw_lpf_x, 
                  mag_raw_lpf_y, 
                  mag_raw_lpf_z, 
                  &mag_x, &mag_y, &mag_z);

    // ---------------------------------------------------------
    // 步骤2：将校准后的原始数据转换为物理单位 (高斯)
    // 注意：这里复用了头文件中的转换宏，传入的是校准后的变量
    // ---------------------------------------------------------
    float m_x = imu963ra_mag_transition(mag_x);
    float m_y = imu963ra_mag_transition(mag_y);
    float m_z = imu963ra_mag_transition(mag_z);

    // ---------------------------------------------------------
    // 步骤3：提取欧拉角并转换为弧度制
    // ---------------------------------------------------------
    // 【重要提示】：
    // 标准航空坐标系中，Pitch 是“车头向上为正”。
    // 而你的定义是“车头向下倒为正”。因此，为了匹配标准的倾角补偿公式，
    // 我们在这里加上负号 (-euler_angle.pitch) 将其翻转。
    // 如果实际测试时发现车头上下俯仰时航向乱飘，可以尝试去掉这个负号。
    float roll_rad  = euler_angle.roll  * (3.14159265f / 180.0f);
    float pitch_rad = -euler_angle.pitch * (3.14159265f / 180.0f);

    // ---------------------------------------------------------
    // 步骤4：倾角补偿计算 (保留中间变量，防止写反，不损耗算力！)
    // ---------------------------------------------------------
    float mag_x_h = m_x * cosf(pitch_rad) + m_z * sinf(pitch_rad);
    float mag_y_h = m_x * sinf(roll_rad) * sinf(pitch_rad) + m_y * cosf(roll_rad) - m_z * sinf(roll_rad) * cosf(pitch_rad);

    // ---------------------------------------------------------
    // 步骤5：计算航向 (注意：atan2f 必须是 Y 在前，X 在后)
    // ---------------------------------------------------------
    float temp_heading = atan2f(mag_y_h, mag_x_h) * (180.0f / 3.14159265f);

    // 5. 磁偏角修正及范围归一化
    temp_heading += -5.5f; // 这里的 -5.5 替换为你当地的磁偏角，上海是 -5.5 度【优化点】写在头文件里面，最好应该写在config里面，这样换地方了要注意

    if (temp_heading < 0.0f) {
        temp_heading += 360.0f;
    } else if (temp_heading >= 360.0f) {
        temp_heading -= 360.0f;
    }

    // 6. 赋值给全局变量
    heading = temp_heading;
}
#endif
