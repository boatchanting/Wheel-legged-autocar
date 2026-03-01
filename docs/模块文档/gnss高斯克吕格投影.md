这是根据你的需求编写的 `gnss_transform.c` 和 `gnss_transform.h`。

代码特点：
1.  **高斯-克吕格投影**：实现了完整的 WGS84 椭球参数投影算法。
2.  **相对坐标输出**：以第一次有效定位点为原点 `(0,0)`，输出相对平面坐标（单位：米），避免了浮点数精度损失，且直接对应你的 INS 相对坐标系。
3.  **全局变量更新**：符合你的代码风格，直接操作全局结构体，不使用返回值。
4.  **容错处理**：包含经纬度格式转换（度分秒转十进制）。

### gnss_transform.h

```c
#ifndef __GNSS_TRANSFORM_H
#define __GNSS_TRANSFORM_H

#include <stdint.h>

// --- 依赖的外部结构体定义 (与你的定义保持一致) ---

typedef struct
{
    uint16_t year;  
    uint8_t  month; 
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
} gps_time_struct;

typedef struct
{
    gps_time_struct time;
    uint8_t  state;             // 有效状态 1：定位有效 0：定位无效
    uint16_t latitude_degree;   // 度
    uint16_t latitude_cent;     // 分
    uint16_t latitude_second;   // 秒 (放大100倍)
    uint16_t longitude_degree;  // 度
    uint16_t longitude_cent;    // 分
    uint16_t longitude_second;  // 秒 (放大100倍)
    double   latitude;          // 纬度 (十进制度)
    double   longitude;         // 经度 (十进制度)
    int8_t   ns;                // N/S
    int8_t   ew;                // E/W
    float    speed;             // km/h
    float    direction;         // 航向
    uint8_t  antenna_direction_state;
    float    antenna_direction;
    uint8_t  satellite_used;
    float    height;
} gnss_info_struct;

// --- 输出结构体 ---

typedef struct
{
    // 相对坐标 (单位: 米)
    // 以初始化时的 GPS 点为原点 (0,0)
    // x: 东向位移 (对应高斯投影东向)
    // y: 北向位移 (对应高斯投影北向)
    float    x;             
    float    y;             
    
    // 速度 (单位: m/s)
    float    speed_ms;      
    
    // 原点记录 (用于调试或绝对坐标恢复)
    double   origin_lat;    
    double   origin_lon;    
    
    // 状态标志
    uint8_t  is_origin_set; // 原点是否已设置 (0: 未初始化, 1: 已初始化)
    uint8_t  is_valid;      // 当前数据是否有效 (基于 state 和卫星数)
    
    // 统计信息
    uint32_t update_count;  // 更新计数
    
} gnss_transform_struct;

// --- 全局变量声明 ---
extern gnss_transform_struct gnss_trans;

// --- API 接口 ---

/**
 * @brief 初始化转换模块
 * @note  清除原点，重置状态
 */
void Gnss_Transform_Init(void);

/**
 * @brief 主处理函数，需在主循环中调用
 * @note  读取全局 gnss 数据，进行投影转换，结果写入全局 gnss_trans
 */
void Gnss_Transform_Update(void);

/**
 * @brief 强制重置原点
 * @note  用于手动设置新的参考原点
 */
void Gnss_Transform_Reset_Origin(void);

#endif // __GNSS_TRANSFORM_H
```

### gnss_transform.c

```c
#include "gnss_transform.h"
#include <math.h>

// ==================== 全局变量实例 ====================
gnss_transform_struct gnss_trans = {0};

// ==================== 内部静态变量 (原点的绝对投影坐标) ====================
static double s_origin_x_abs = 0.0; // 原点高斯 X (北向)
static double s_origin_y_abs = 0.0; // 原点高斯 Y (东向)

// ==================== WGS84 椭球参数 ====================
#define WGS84_A         6378137.0           // 长半轴
#define WGS84_F         (1.0 / 298.257223563) // 扁率
#define WGS84_E2        (2.0 * WGS84_F - WGS84_F * WGS84_F) // 第一偏心率平方
#define WGS84_E12       (WGS84_E2 / (1.0 - WGS84_E2))       // 第二偏心率平方
#define PI              3.14159265358979323846
#define RAD_PER_DEG     (PI / 180.0)

// ==================== 内部辅助函数 ====================

/**
 * @brief 计算中央子午线 (3度带)
 */
static double _get_central_meridian(double lon_deg)
{
    // 3度带计算: Zone = floor((Lon + 1.5) / 3)
    // 中央经线 = Zone * 3
    int zone = (int)((lon_deg + 1.5) / 3.0);
    return (double)zone * 3.0;
}

/**
 * @brief 高斯-克吕格投影核心算法 (绝对坐标)
 * @param lat 纬度 (度)
 * @param lon 经度 (度)
 * @param out_x 输出北向坐标 N (米)
 * @param out_y 输出东向坐标 E (米)
 */
static void _gauss_project(double lat, double lon, double *out_x, double *out_y)
{
    double B = lat * RAD_PER_DEG;
    double L = lon * RAD_PER_DEG;
    
    // 计算中央经线
    double L0 = _get_central_meridian(lon) * RAD_PER_DEG;
    double l = L - L0; // 经差

    // 计算辅助参数
    double sinB = sin(B);
    double cosB = cos(B);
    double tanB = tan(B);
    
    // 卯酉圈曲率半径 N
    double N = WGS84_A / sqrt(1.0 - WGS84_E2 * sinB * sinB);
    
    // 第二偏心率相关的 eta2
    double eta2 = WGS84_E12 * cosB * cosB;

    // 子午线弧长计算系数
    // 使用泰勒级数展开简化计算
    double m0 = WGS84_A * (1.0 - WGS84_E2 / 4.0 - 3.0 * WGS84_E2 * WGS84_E2 / 64.0);
    double m2 = WGS84_A * (3.0 * WGS84_E2 / 8.0 + 3.0 * WGS84_E2 * WGS84_E2 / 32.0);
    double m4 = WGS84_A * (15.0 * WGS84_E2 * WGS84_E2 / 256.0);
    double m6 = WGS84_A * (35.0 * WGS84_E2 * WGS84_E2 * WGS84_E2 / 3072.0);

    // 子午线弧长 X (从赤道起算)
    double X = m0 * B - m2 * sin(2.0 * B) + m4 * sin(4.0 * B) - m6 * sin(6.0 * B);

    // 高斯投影正算公式
    // 北向坐标 x (通常在测绘中x指北，y指东)
    double t = tanB;
    double l2 = l * l;
    double l4 = l2 * l2;
    double l6 = l4 * l2;

    double x_val = X + N * t * l2 / 2.0 
                 + N * t * (5.0 - t * t + 9.0 * eta2 + 4.0 * eta2 * eta2) * l4 / 24.0 
                 + N * t * (61.0 - 58.0 * t * t + t * t * t * t) * l6 / 720.0;

    // 东向坐标 y
    double y_val = N * l 
                 + N * (1.0 - t * t + eta2) * l * l2 / 6.0 
                 + N * (5.0 - 18.0 * t * t + t * t * t * t + 14.0 * eta2 - 58.0 * eta2 * t * t) * l * l4 / 120.0;

    // 去掉伪偏移 (不加 500000 和 带号)，保持纯数学坐标
    *out_x = x_val; // 北
    *out_y = y_val; // 东
}

/**
 * @brief 将度分秒(DMS)格式转换为十进制度
 * @note  你的数据中秒放大了100倍
 */
static double _dms_to_decimal(uint16_t deg, uint16_t min, uint16_t sec_x100)
{
    return (double)deg + (double)min / 60.0 + (double)sec_x100 / 100.0 / 3600.0;
}

// ==================== API 接口实现 ====================

void Gnss_Transform_Init(void)
{
    // 清零全局输出
    gnss_trans.x = 0.0f;
    gnss_trans.y = 0.0f;
    gnss_trans.speed_ms = 0.0f;
    gnss_trans.origin_lat = 0.0;
    gnss_trans.origin_lon = 0.0;
    gnss_trans.is_origin_set = 0;
    gnss_trans.is_valid = 0;
    gnss_trans.update_count = 0;
    
    // 清零内部绝对坐标原点
    s_origin_x_abs = 0.0;
    s_origin_y_abs = 0.0;
}

void Gnss_Transform_Reset_Origin(void)
{
    gnss_trans.is_origin_set = 0;
    s_origin_x_abs = 0.0;
    s_origin_y_abs = 0.0;
}

void Gnss_Transform_Update(void)
{
    // 1. 检查 GNSS 数据有效性 (假设 extern 变量名为 gnss)
    // 这里判断 state=1 且 卫星数>=4 作为有效依据
    if (gnss.state != 1 || gnss.satellite_used < 4)
    {
        gnss_trans.is_valid = 0;
        return;
    }
    gnss_trans.is_valid = 1;
    gnss_trans.update_count++;

    // 2. 获取经纬度 (优先使用 double 成员，若为0则尝试转换 DMS)
    double lat = gnss.latitude;
    double lon = gnss.longitude;

    // 如果 double 成员未解析，使用整数成员计算
    if (lat == 0.0 && lon == 0.0)
    {
        lat = _dms_to_decimal(gnss.latitude_degree, gnss.latitude_cent, gnss.latitude_second);
        lon = _dms_to_decimal(gnss.longitude_degree, gnss.longitude_cent, gnss.longitude_second);
        
        // 处理南纬和西经
        if (gnss.ns == 'S' || gnss.ns == -1) lat = -lat;
        if (gnss.ew == 'W' || gnss.ew == -1) lon = -lon;
    }

    // 3. 高斯投影计算当前绝对坐标
    double current_x_abs, current_y_abs;
    _gauss_project(lat, lon, &current_x_abs, &current_y_abs);

    // 4. 原点初始化 (仅第一次有效时执行)
    if (!gnss_trans.is_origin_set)
    {
        s_origin_x_abs = current_x_abs;
        s_origin_y_abs = current_y_abs;
        
        gnss_trans.origin_lat = lat;
        gnss_trans.origin_lon = lon;
        gnss_trans.is_origin_set = 1;
        
        // 原点时刻，相对坐标强制为0
        gnss_trans.x = 0.0f;
        gnss_trans.y = 0.0f;
    }
    else
    {
        // 5. 计算相对坐标 (当前绝对 - 原点绝对)
        // 结果单位为米
        gnss_trans.x = (float)(current_y_abs - s_origin_y_abs); // 东向位移 (X 通常对应东)
        gnss_trans.y = (float)(current_x_abs - s_origin_x_abs); // 北向位移 (Y 通常对应北)
    }

    // 6. 速度转换 (km/h -> m/s)
    gnss_trans.speed_ms = gnss.speed / 3.6f;
}

```

### 如何在你的代码中使用

在主循环或GPS处理任务中：

```c
// 1. 初始化 (系统启动时调用一次)
Gnss_Transform_Init();

// 2. 主循环 (例如 50Hz)
while(1)
{
    // ... 接收解析 GPS 数据填充 gnss 结构体 ...
    
    // 当 gnss 数据更新后，调用此函数
    if (gnss_flag == 1) 
    {
        gnss_flag = 0; // 清除标志
        Gnss_Transform_Update(); // 自动更新 gnss_trans 全局变量
    }
    
    // 3. 在融合算法中使用结果
    if (gnss_trans.is_valid && gnss_trans.is_origin_set)
    {
        float gps_x = gnss_trans.x; // 相对原点的东向位移 (米)
        float gps_y = gnss_trans.y; // 相对原点的北向位移 (米)
        
        // 此时 gps_x, gps_y 已经可以直接和你的 INS 的 nav_x, nav_y 进行比较了
        // 注意：INS 的坐标系定义需要确认是否也是 东/北 向
        // 如果 INS 是 前/右 (Body系)，需要做旋转。
    }
}
```

### 关键点说明

1.  **坐标系定义**：输出 `gnss_trans.x` 对应高斯投影的东向，`gnss_trans.y` 对应北向。这与标准测绘一致，也符合通常的笛卡尔坐标系习惯。
2.  **INS 坐标系对齐**：你的 INS 数据 `nav_x` (后) 和 `nav_y` (右) 是车身坐标系。在使用时，你需要用 `relative_yaw` 将 `nav_x/nav_y` 旋转到 东/北 坐标系，或者将这里的 `gnss_trans.x/y` 旋转到车身坐标系进行比较。
3.  **精度问题**：内部计算全程使用 `double`，最后赋值给 `float` 输出。由于我们做的是相对坐标减法，数值通常很小（几百米内），因此 `float` 完全足够存储，不会有精度损失。