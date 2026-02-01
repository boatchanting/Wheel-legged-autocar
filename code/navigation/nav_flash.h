#ifndef _NAV_FLASH_H_
#define _NAV_FLASH_H_
#include "zf_common_headfile.h"

// ======================= Flash 配置 =======================
// 根据 flash.h: 192KB 总大小, 96页, 每页 2KB (512个 uint32/float)
// 我们预留前 10 页给系统或其他参数，从第 10 页开始用
#define NAV_FLASH_SECTION           (0)             // 默认扇区
#define NAV_FLASH_PAGE_HEADER       (10)            // 【头信息页】存储点数总数等元数据
#define NAV_FLASH_PAGE_DATA_START   (11)            // 【数据页】轨迹数据开始的页码
#define NAV_FLASH_MAX_PAGES         (80)            // 最大使用 80 页 (11 ~ 90)

// ======================= 记录逻辑配置 =======================
#define NAV_MEM_BUFFER_SIZE         (300)           // RAM 缓存大小: 300点 * 10ms = 3.0秒
#define NAV_LOG_DECIMATION          (10)            // 曲线抽稀系数: 10ms记录 -> 100ms存储 (10倍)
#define NAV_CURVE_THRESHOLD         (2.0f)          // 曲线阈值(度): 3s内任意两点偏航角差值超过此值，视为曲线段
#define NAV_POINT_SIZE_WORDS        (3)             // 每个点占用 3 个 float (x, y, yaw)

// ======================= 数据结构定义 =======================

// 1. 轨迹点结构体 (Flash 中实际存储的格式)
typedef struct {
    float x;
    float y;
    float yaw;
} NavPoint_t;

// 2. 导航记录器状态枚举
typedef enum {
    NAV_STATE_IDLE = 0,     // 空闲 / 停止
    NAV_STATE_RECORDING,    // 正在记录
} NavState_e;

// 3. RAM 原始数据缓存 (用于 3s 逻辑判断)
typedef struct {
    NavPoint_t raw_points[NAV_MEM_BUFFER_SIZE];
    uint16_t   count;       // 当前缓存的点数 (0-300)
    uint8_t    is_curved;   // 曲线标志位
} NavMemoryBuffer_t;

// ======================= 全局变量声明 =======================
extern NavState_e nav_recorder_state;    // 记录器状态
extern uint32_t   nav_total_saved_points;// 已保存到 Flash 的总点数

// ======================= 函数接口声明 =======================

/**
 * @brief 初始化导航 Flash 模块 (上电调用)
 * @note  会读取 Header 页，恢复上次记录的总点数，准备复现
 */
void NavFlash_Init(void);

/**
 * @brief 清除 Flash 中的所有轨迹数据
 * @note  会擦除 Header 页和所有 Data 页，耗时较长，请在停车时调用
 */
void NavFlash_Clear_All(void);

/**
 * @brief 开始记录轨迹
 * @note  会自动擦除旧数据，重置计数器，开始新的记录
 */
void NavFlash_Start_Record(void);

/**
 * @brief 停止记录轨迹
 * @note  1. 强制处理 RAM 中剩余的缓存数据(最后0-3s)
 *        2. 将 Flash 写入缓冲区中未满一页的数据写入物理 Flash
 *        3. 更新 Header 页的总点数
 */
void NavFlash_Stop_Record(void);

/**
 * @brief 周期性记录函数 (需 10ms 调用一次)
 * @param x 当前小车的 x 坐标 (mm)
 * @param y 当前小车的 y 坐标 (mm)
 * @param yaw 当前小车的航向角 (度)
 * 修改点：直接传递 x, y, yaw，避免结构体未定义错误
 */
void NavFlash_Record_Task_10ms(float x, float y, float yaw);

/**
 * @brief 读取指定索引的轨迹点 (用于复现)
 * @param index 点的索引 (0 ~ nav_total_saved_points-1)
 * @param out_point 输出参数，指向存放读取数据的结构体
 * @return 1:读取成功 0:失败(索引越界)
 */
uint8_t NavFlash_Read_Point(uint32_t index, NavPoint_t* out_point);

#endif // _NAV_FLASH_H_