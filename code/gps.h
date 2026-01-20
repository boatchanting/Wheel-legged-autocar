/*********************************************************************************************************************
 * GPS 地图记录模块 - 头文件
 * 
 * 功能描述：
 *   - HDOP 自适应卡尔曼滤波：根据GPS信号质量动态调整滤波参数
 *   - 坐标转换：WGS84经纬度 -> 平面直角坐标
 *   - 滑动窗口：用于GPS数据稳定性检测
 *   - 地图点记录：保存经过滤波和验证的GPS位置
 *
 * 算法原理：
 *   1. 卡尔曼滤波：通过HDOP值动态调整测量噪声R，HDOP越大表示信号越差，R越大
 *   2. 稳定性检测：在一个固定大小的滑动窗口内，当所有点都在阈值范围内时认为稳定
 *   3. 防抖动制：需要连续5次稳定才记录一个新点，距离要超过最小值
 *
 * 作者：自动驾驶车队
 * 日期：2026年1月
*********************************************************************************************************************/

#ifndef __GPS_H__
#define __GPS_H__

#include <math.h>

// ================= 参数配置 =================
#define WINDOW_SIZE     20      // 滑动窗口大小，用于稳定性检测
#define STABLE_RADIUS   0.4f    // 稳定性阈值 (0.4m)，窗口内所有点在此范围内视为稳定
#define MIN_RECORD_DIST 1.5f    // 最小记录距离 (1.5m)，新记录点与前一点距离要超过此值

// ================= 卡尔曼滤波结构体 =================
/**
 * @brief HDOP自适应卡尔曼滤波器结构
 * 
 * 该滤波器根据HDOP值(水平精度因子)动态调整测量噪声，使得：
 * - HDOP值小(信号好)时：更相信GPS测量值
 * - HDOP值大(信号差)时：更相信内部模型预测值
 */
typedef struct {
    double x;       // 状态值：滤波后的估计值
    double p;       // 估计协方差：表示当前估计的不确定性
    double q;       // 过程噪声：模型的不完美性(固定值)
    double r;       // 当前测量噪声：根据HDOP动态调整
    double base_r;  // 基准测量噪声：初始设定值，作为动态调整的基准
    double k;       // 卡尔曼增益：决定有多相信新的测量值
} Kalman;

// ================= 滑动窗口结构体 =================
/**
 * @brief 用于GPS数据稳定性检测的环形缓冲区
 * 
 * 保存最近WINDOW_SIZE个GPS点的经纬度数据，用环形队列实现
 */
typedef struct {
    double lat[WINDOW_SIZE];  // 纬度缓冲区
    double lon[WINDOW_SIZE];  // 经度缓冲区
    int head;                 // 队列头指针
    int count;                // 当前有效数据个数
} SlidingWindow;

// ================= 地图点结构体 =================
/**
 * @brief 记录的地图点信息
 * 
 * 存储GPS坐标和对应的平面直角坐标
 */
typedef struct {
    double lat;   // 纬度 (WGS84)
    double lon;   // 经度 (WGS84)
    float x;      // 平面直角坐标X (相对原点，单位：米)
    float y;      // 平面直角坐标Y (相对原点，单位：米)
} MapPoint;

// ================= 全局变量声明 =================
extern MapPoint map[100];           // GPS地图点数组，最多保存100个点
extern int point_cnt;               // 当前记录的点数

extern Kalman kf_lat, kf_lon;       // 纬度和经度的卡尔曼滤波器
extern uint8 kalman_ready;          // 卡尔曼滤波器初始化标志

extern double map_lat0, map_lon0;   // 坐标系原点的经纬度

// ================= 函数声明 =================

/**
 * @brief 初始化卡尔曼滤波器
 * 
 * @param k        指向卡尔曼滤波器的指针
 * @param q        过程噪声 (推荐1e-6，越小越相信模型)
 * @param base_r   基准测量噪声 (推荐1e-4，越小越相信测量值)
 * @param init     初始状态值
 */
void kalman_init(Kalman *k, double q, double base_r, double init);

/**
 * @brief 卡尔曼滤波更新
 * 
 * 根据HDOP值动态调整测量噪声，然后执行标准卡尔曼滤波步骤
 * 
 * @param k    指向卡尔曼滤波器的指针
 * @param z    新的测量值
 * @param hdop HDOP值，范围通常0.5-10.0，决定信号质量
 * @return 滤波后的状态值
 */
double kalman_update(Kalman *k, double z, float hdop);

/**
 * @brief GPS经纬度转换为平面直角坐标
 * 
 * 使用墨卡托投影的简化版本，适用于局部小范围坐标转换
 * 原点为(map_lat0, map_lon0)，转换后的坐标相对于原点
 * 
 * @param lat 纬度 (WGS84)
 * @param lon 经度 (WGS84)
 * @param x   指向X坐标的指针，返回结果(单位：米)
 * @param y   指向Y坐标的指针，返回结果(单位：米)
 */
void gps_to_plane(double lat, double lon, float *x, float *y);

/**
 * @brief 向滑动窗口中添加一个GPS数据点
 * 
 * 使用环形缓冲区，自动覆盖最旧的数据
 * 
 * @param lat 纬度
 * @param lon 经度
 */
void win_push(double lat, double lon);

/**
 * @brief 检测滑动窗口内的GPS数据是否稳定
 * 
 * 判断标准：
 * 1. 窗口已满(有WINDOW_SIZE个数据点)
 * 2. 所有点转换为平面坐标后，最大值-最小值都不超过STABLE_RADIUS
 * 
 * @param out_avg_lat 指向输出平均纬度的指针
 * @param out_avg_lon 指向输出平均经度的指针
 * @return 1表示稳定，0表示不稳定或窗口未满
 */
int win_check_stable(double *out_avg_lat, double *out_avg_lon);

/**
 * @brief 初始化GPS模块(滑动窗口)
 * 
 * 重置滑动窗口状态
 */
void gps_init(void);

/**
 * @brief GPS数据处理主函数
 * 
 * 处理GNSS数据，包括滤波、稳定性检测和点记录
 * 应在每次接收到GNSS数据时调用
 * 
 * @return 1表示有新点被记录，0表示没有
 */
int gps_process(void);

/**
 * @brief 输出当前记录的所有GPS地图点
 * 
 * 用于调试，显示已记录的点的坐标信息
 */
void gps_debug_print_map(void);

#endif // __GPS_H__
