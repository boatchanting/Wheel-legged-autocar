
#include "zf_common_headfile.h"

// ==========================================
// 1. 全局变量与矩阵定义
// ==========================================
nav_state_t nav_result;
#define g 9.80665f; // 重力加速度常数
// 卡尔曼核心矩阵
static matrix_t X;  // 状态向量 [px, py, vx, vy]^T
static matrix_t P;  // 协方差矩阵 4x4
static matrix_t Q;  // 过程噪声矩阵 4x4 (信任IMU程度)
static matrix_t R;  // 测量噪声矩阵 2x2 (信任编码器程度)
static matrix_t F;  // 状态转移矩阵 4x4
static matrix_t H;  // 观测矩阵 2x4
static matrix_t B;  // 控制矩阵 4x2 (加速度输入)
float ax=0.0f, ay=0.0f; // 全局加速度变量
float ax_world = 0.0f, ay_world=0.0f; // 世界系加速度变量
// 临时计算变量 (避免栈溢出，定义为静态)
static matrix_t U;       // 输入向量 [ax, ay]^T
static matrix_t Z;       // 观测向量 [vx_enc, vy_enc]^T
static matrix_t X_pred;  // 预测状态
static matrix_t P_pred;  // 预测协方差
static matrix_t K_gain;       // 卡尔曼增益
static matrix_t Temp1, Temp2, Temp3, Temp4, Temp5, Temp6; // 通用临时矩阵
int32_t loop_count = 0; // 全局循环计数器
// ==========================================
// 2. 参数配置 (根据实际情况调整)
// ==========================================
// IMU 加速度计噪声方差 (越小越信IMU)
#define Q_ACC_VAR       0.01f   
// 编码器速度噪声方差 (越小越信编码器)
#define R_ENC_VAR       0.005f  

// ==========================================
// 3. 初始化函数
// ==========================================
void Navigation_EKF_Init(void)
{
    // 1. 初始化状态 X (全0)
    Matrix_Init(&X, NAV_STATE_DIM, 1);
    
    // 2. 初始化协方差 P (对角线设为较大值，表示初始不确定)
    Matrix_Identity(&P, NAV_STATE_DIM);
    for(int i=0; i<NAV_STATE_DIM; i++) P.data[i][i] = 1.0f;

    // 3. 初始化 Q (过程噪声)
    // 匀速模型中，不确定性主要来自加速度的积分
    // 这里简单设为对角阵，也可以用更严谨的 G*G^T * sigma_a 形式
    Matrix_Init(&Q, NAV_STATE_DIM, NAV_STATE_DIM);
    Q.data[0][0] = 0.0001f; Q.data[1][1] = 0.0001f; // 位置噪声小一点
    Q.data[2][2] = Q_ACC_VAR; Q.data[3][3] = Q_ACC_VAR; // 速度噪声取决于加速度计

    // 4. 初始化 R (测量噪声)
    Matrix_Init(&R, NAV_MEAS_DIM, NAV_MEAS_DIM);
    R.data[0][0] = R_ENC_VAR;
    R.data[1][1] = R_ENC_VAR;

    // 5. 初始化 F (状态转移矩阵 - 匀速模型)
    // [1 0 dt 0]
    // [0 1 0 dt]
    // [0 0 1  0]
    // [0 0 0  1]
    Matrix_Init(&F, NAV_STATE_DIM, NAV_STATE_DIM);
    F.data[0][0]=1; F.data[1][1]=1; F.data[2][2]=1; F.data[3][3]=1;
    F.data[0][2]=NAV_DT; F.data[1][3]=NAV_DT;

    // 6. 初始化 H (观测矩阵 - 观测速度)
    // [0 0 1 0]
    // [0 0 0 1]
    Matrix_Init(&H, NAV_MEAS_DIM, NAV_STATE_DIM);
    H.data[0][2] = 1.0f;
    H.data[1][3] = 1.0f;

    // 7. 初始化 B (控制矩阵 - 加速度输入)
    // [0.5*dt^2   0     ]
    // [0          0.5*dt^2]
    // [dt         0     ]
    // [0          dt    ]
    Matrix_Init(&B, NAV_STATE_DIM, NAV_INPUT_DIM);
    float half_dt2 = 0.5f * NAV_DT * NAV_DT;
    B.data[0][0] = half_dt2;
    B.data[1][1] = half_dt2;
    B.data[2][0] = NAV_DT;
    B.data[3][1] = NAV_DT;
    
    // 初始化其他临时矩阵
    Matrix_Init(&U, NAV_INPUT_DIM, 1);
    Matrix_Init(&Z, NAV_MEAS_DIM, 1);
}
// 简化版智能滤波
static void fast_smart_filter_xy(float ax, float ay, float *out_x, float *out_y)
{
    // 静态变量（保持状态）
    static float mean_x = 0, mean_y = 0;
    static float var_est_x = 0.01f, var_est_y = 0.01f;  // 方差估计
    static int anomaly_x = 0, anomaly_y = 0;
    static int sample_count = 0;
    static float last_outx = 0, last_outy = 0;
    
    sample_count++;
    
    // 1. 在线更新均值
    float alpha_mean = 0.1f;  // 均值更新系数
    mean_x = mean_x * (1 - alpha_mean) + ax * alpha_mean;
    mean_y = mean_y * (1 - alpha_mean) + ay * alpha_mean;
    
    // 2. 计算偏差和更新方差估计
    float diff_x = ax - mean_x;
    float diff_y = ay - mean_y;
    
    float alpha_var = 0.05f;  // 方差更新系数
    var_est_x = (1 - alpha_var) * var_est_x + alpha_var * (diff_x * diff_x);
    var_est_y = (1 - alpha_var) * var_est_y + alpha_var * (diff_y * diff_y);
    
    // 3. 简化的异常检测
    float sigma_x = fabsf(diff_x) / (sqrtf(var_est_x) + 0.001f);
    float sigma_y = fabsf(diff_y) / (sqrtf(var_est_y) + 0.001f);
    
    // 4. 快速异常处理
    float outx = ax, outy = ay;
    
    // 使用绝对阈值+相对阈值组合
    const float ABS_THRESH = 2.0f;   // 绝对阈值 2 m/s²
    const float REL_THRESH = 3.0f;   // 相对阈值 3σ
    
    // X轴检测
    if(fabsf(diff_x) > ABS_THRESH || sigma_x > REL_THRESH) {
        anomaly_x++;
        if(anomaly_x <= 2) {
            outx = mean_x;  // 偶尔异常用均值
        } else {
            mean_x = 0.8f * mean_x + 0.2f * ax;  // 适应
            anomaly_x = 0;
        }
    } else {
        anomaly_x = 0;
    }
    
    // Y轴检测
    if(fabsf(diff_y) > ABS_THRESH || sigma_y > REL_THRESH) {
        anomaly_y++;
        if(anomaly_y <= 2) {
            outy = mean_y;
        } else {
            mean_y = 0.8f * mean_y + 0.2f * ay;
            anomaly_y = 0;
        }
    } else {
        anomaly_y = 0;
    }
    
    // 5. 快速平滑
    const float SMOOTH = 0.7f;
    *out_x = SMOOTH * outx + (1 - SMOOTH) * last_outx;
    *out_y = SMOOTH * outy + (1 - SMOOTH) * last_outy;
    last_outx = *out_x;
    last_outy = *out_y;
    
    // 6. 调试输出（每300次输出一次）
    // if(sample_count % 300 == 0) {
    //     printf("快速滤波: X=%.2f±%.2f(m/s²), Y=%.2f±%.2f(m/s²)\n", 
    //            mean_x, sqrtf(var_est_x), mean_y, sqrtf(var_est_y));
    // }
}

// 4. 核心更新函数Navigation_EKF_Update(imu660ra_acc_x, imu660ra_acc_y, euler_angle.yaw, current_actual_speed/60.0f*WHEEL_CIRCUMFERENCE);
//加速度单位 m/s²，偏航角单位 角度，编码器速度单位 m/s
// ==========================================
void Navigation_EKF_Update(float imu_ax, float imu_ay, float yaw_rad, float enc_vel_mps)
{
    
    loop_count++;
        // --- 新增：零速检测 ---
    // static int stationary_count = 0;
    // const float ZERO_SPEED_THRESHOLD = 0.01f;  // 0.01 m/s
    // const int STATIONARY_SAMPLES = 20;         // 连续20次认为静止
    // // 检查是否静止
    // if(fabsf(enc_vel_mps) < ZERO_SPEED_THRESHOLD) 
    // {
    //     stationary_count++;
    //     if(stationary_count >= STATIONARY_SAMPLES) 
    //     {
    //         // if(loop_count % 100 == 0)
    //         // {
    //         //  printf("pos_x: %.3f, pos_y: %.3f, vel_x: %.3f, vel_y: %.3f\n", nav_result.pos_x, nav_result.pos_y, nav_result.vel_x, nav_result.vel_y);
    //         // }
    //         // 直接返回，不进行EKF更新
    //         return;
    //     }
    // } 
    // else 
    // {
    //     stationary_count = 0;
    // }
    // 去除加速度计零偏
    float raw_ax = (imu_ax - imu660ra_acc_x_AND) / 4096.0f * 9.80665f;  // 转为m/s²
    float raw_ay = (imu_ay*cosf(euler_angle.yaw-ANG_MECH_ZERO) - imu660ra_acc_y_AND*pitch_initialization) / 4096.0f * 9.80665f; // IMU加速度，单位 m/s²*pitch_initialization  //*cosf(euler_angle.yaw-ANG_MECH_ZERO)
    float filtered_ax=0.0f, filtered_ay=0.0f;
    fast_smart_filter_xy(raw_ax, raw_ay, &filtered_ax, &filtered_ay);
    // ax = raw_ax; 
    // ay =raw_ay;
    ax= filtered_ax;
    ay= filtered_ay;
    if(ax<0.09f && ax>-0.09f)
        ax=0.0f;
    if(ay<0.09f && ay>-0.09f)
        ay=0.0f;
    // if(loop_count % 100 == 0)
    // {
    //     // printf("imu_ay: %.3f, imu660ra_acc_y_AND: %.3f, imu660ra_acc_x_AND: %.3f\n", imu_ay, imu660ra_acc_y_AND, imu660ra_acc_x_AND);
    //     if (ax!=0||ay!=0)
    //     printf("ax: %.3f, ay: %.3f\n", ax, ay);
    //     // printf("vx_enc: %.3f, vy_enc: %.3f\n", vx_enc, vy_enc);
    // }
    float enc_vel=enc_vel_mps/60.0f*WHEEL_CIRCUMFERENCE;
    // if(g_yaw_initialized==false )
    // {
    //     // 如果偏航角稳定性检测未完成，直接返回
    //     return;
    // }
    // --- Step 1: 数据预处理 (坐标旋转) ---
    // 将 IMU 的车体坐标系加速度 -> 转换到 -> 世界坐标系 (Assuming Z-axis rotation)
    float yaw = -yaw_rad+g_initial_yaw;

    while(1)
    {
        if(yaw > 180)
            yaw -= 360.0f;
        else if(yaw < -180) 
            yaw += 360.0f;
        else break;
    }
    // if(loop_count % 100 == 0)
    // {
    // printf("yaw: %.3f, g_initial_yaw: %.3f\n", yaw, g_initial_yaw);
    // }
    /* 偏航角（世界系，0 对应 +Y） */
    float c = cosf(yaw);
    float s = sinf(yaw);

    /* 加速度：车体系 -> 世界系 */
    ax_world = -ax * c + ay * s;
    ay_world = ay * c + ax * s;
    // 将编码器速度 -> 转换到 -> 世界坐标系
    float vx_enc = enc_vel * c;
    float vy_enc = enc_vel * s;
    // if(loop_count % 100 == 0)
    // {
    //     if (ax!=0||ay!=0)
    //     printf("ax_world: %.3f, ay_world: %.3f\n", ax_world, ay_world);
    //     // printf("vx_enc: %.3f, vy_enc: %.3f\n", vx_enc, vy_enc);
    // }
    // 填入输入向量 U 和 观测向量 Z
    U.data[0][0] = ax_world;
    U.data[1][0] = ay_world;
    Z.data[0][0] = vx_enc;
    Z.data[1][0] = vy_enc;
    // 保存调试数据
    nav_result.acc_world_x = ax_world;
    nav_result.acc_world_y = ay_world;
    // Debug print
    // if(loop_count % 100 == 0)
    // {
    // printf("ax_world: %.3f, ay_world: %.3f, vx_enc: %.3f, vy_enc: %.3f\n", ax_world, ay_world, vx_enc, vy_enc);
    // }
    // --- Step 2: 预测 (Prediction) ---
    
    // 2.1 状态预测: X_pred = F*X + B*U
    // 先算 F*X
    Temp1 = multiply_matrices(&F, &X);
    // 再算 B*U
    Temp2 = multiply_matrices(&B, &U);
    // 相加
    X_pred = add_matrices(&Temp1, &Temp2);

    // 2.2 协方差预测: P_pred = F*P*F^T + Q
    // 先算 F*P
    Temp1 = multiply_matrices(&F, &P);
    // 这里的 F^T 其实对于此简单矩阵可以直接手写，但为了通用性调用转置
    matrix_t FT = Matrix_Transpose(&F);
    // 再乘 F^T
    Temp2 = multiply_matrices(&Temp1, &FT);
    // 加 Q
    P_pred = add_matrices(&Temp2, &Q);

    // --- Step 3: 更新 (Correction / Update) ---

    // 3.1 计算卡尔曼增益 K = P_pred * H^T * (H * P_pred * H^T + R)^-1
    matrix_t HT = Matrix_Transpose(&H);
    
    // 算分母部分 S = H * P_pred * H^T + R
    Temp1 = multiply_matrices(&H, &P_pred);
    Temp2 = multiply_matrices(&Temp1, &HT);
    matrix_t S = add_matrices(&Temp2, &R);
    
    // 求逆 S^-1
    matrix_t invS;
    if(inverse_matrix(&S, &invS) != 0)
    {
        // 如果求逆失败（奇异矩阵），放弃本次更新，保留预测值
        X = X_pred;
        P = P_pred;
         // 重要：必须更新导航结果！
        nav_result.pos_x = X.data[0][0];
        nav_result.pos_y = X.data[1][0];
        nav_result.vel_x = X.data[2][0];
        nav_result.vel_y = X.data[3][0];
    
        // 可以添加调试信息
        static int inv_fail_count = 0;
        if(++inv_fail_count % 50 == 0) {
        printf("矩阵求逆失败（第%d次），使用预测值\n", inv_fail_count);
        }
       return   ;
    }

    // 算 K_gain
    Temp1 = multiply_matrices(&P_pred, &HT);
    K_gain = multiply_matrices(&Temp1, &invS);

    // 3.2 更新状态 X = X_pred +K_gain*(Z - H*X_pred)
    // 算预测的观测值 H*X_pred
    Temp1 = multiply_matrices(&H, &X_pred);
    // 算残差 Innovation = Z - H*X_pred
    matrix_t Innov = subtract_matrices(&Z, &Temp1);
    // 算修正量 K_gain * Innov
    Temp2 = multiply_matrices(&K_gain, &Innov);
    // 更新 X
    X = add_matrices(&X_pred, &Temp2);

    // 3.3 更新协方差 P = (I - K*H) * P_pred
    matrix_t I;
    Matrix_Identity(&I, NAV_STATE_DIM);

    Temp1 = multiply_matrices(&K_gain, &H);          // K*H
    Temp2 = subtract_matrices(&I, &Temp1);      // I-KH
    Temp3 = multiply_matrices(&Temp2, &P_pred);
    matrix_t Temp2T = Matrix_Transpose(&Temp2);
    Temp4 = multiply_matrices(&Temp3, &Temp2T);

    Temp5 = multiply_matrices(&K_gain, &R);
    matrix_t K_gainT = Matrix_Transpose(&K_gain);
    Temp6 = multiply_matrices(&Temp5, &K_gainT);

    P = add_matrices(&Temp4, &Temp6);

    // --- Step 4: 输出结果 ---
    nav_result.pos_x = X.data[0][0];
    nav_result.pos_y = X.data[1][0];
    nav_result.vel_x = X.data[2][0];
    nav_result.vel_y = X.data[3][0];
    // if(loop_count % 100 == 0)
    // {
    // printf("pos_x: %.3f, pos_y: %.3f, vel_x: %.3f, vel_y: %.3f\n", nav_result.pos_x, nav_result.pos_y, nav_result.vel_x, nav_result.vel_y);
    // }
}

void Navigation_Reset(void)
{
    // 将状态清零，通常在发车前调用
    Matrix_Init(&X, NAV_STATE_DIM, 1);
    // P 矩阵重置为初始不确定度
    Matrix_Identity(&P, NAV_STATE_DIM);
    nav_result.pos_x = 0; nav_result.pos_y = 0;
    nav_result.vel_x = 0; nav_result.vel_y = 0;
    g_initial_yaw = euler_angle.yaw;
}
