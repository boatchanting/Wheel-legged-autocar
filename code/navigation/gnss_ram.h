#ifndef _GNSS_RAM_H_
#define _GNSS_RAM_H_

#include "zf_common_headfile.h"
#include "gnss_transform.h" // 引入 GNSS 转换结构体

//-------------------------------------------------------------------------------------------------------------------
//  @brief      GNSS 打点 RAM 管理模块
//  @note       1. 仅负责“打点 -> 存 RAM”，不涉及 Flash。
//              2. 点坐标来源于全局 gnss_trans.x / gnss_trans.y。
//              3. 点类型、plan 类型由外部逻辑（遥控器）控制。
//              4. 不引入任何时间相关参数。
//-------------------------------------------------------------------------------------------------------------------

// ========================= 配置 =========================
#define GNSS_RAM_MAX_POINTS      166     // RAM 中最多允许存储的GNSS点数，后面正好能写下一页flash

// ========================= 点类型定义 =========================
typedef enum
{
    GNSS_POINT_PATH = 0,     // 普通路径点
    GNSS_POINT_CIRCLE = 1,   // 转圈点
    GNSS_POINT_SLOPE = 2,    // 上坡点
    GNSS_POINT_JUMP = 3,     // 跳跃点
    GNSS_POINT_BRIDGE = 4,   // 单边桥点
    GNSS_POINT_BUMP = 5      // 颠簸路段点
} GnssPointType_e;

// ========================= plan 类型 =========================
typedef enum
{
    GNSS_PLAN_1 = 1,
    GNSS_PLAN_2 = 2,
    GNSS_PLAN_3 = 3
} GnssPlanType_e;

// ========================= 单个 GNSS 点 =========================
typedef struct
{
    float x;                // GNSS 相对 X 坐标 (米) (东向)
    float y;                // GNSS 相对 Y 坐标 (米) (北向)
    uint8 point_type;       // 点类型 (GnssPointType_e)
} GnssRamPoint_t;

// ========================= RAM 总结构 =========================
typedef struct
{
    uint8 plan_type;                        // 当前 plan
    uint16 point_count;                     // 已记录点数量
    GnssRamPoint_t points[GNSS_RAM_MAX_POINTS];
} GnssRamData_t;

// ========================= 全局变量 =========================
extern GnssRamData_t gnss_ram_data;

// ========================= 接口函数 =========================

/**
 * @brief  初始化 GNSS RAM 打点模块
 * @note   清空所有 RAM 数据，一般在进入打点模式时调用
 */
void GnssRam_Init(void);

/**
 * @brief  设置当前 plan 类型
 * @param  plan plan 类型 (GnssPlanType_e)
 */
void GnssRam_SetPlan(uint8 plan);

/**
 * @brief  记录一个 GNSS 点到 RAM
 * @param  point_type 点类型 (GnssPointType_e)
 * @return 0: 成功
 *         1: RAM 已满，记录失败
 * @note   点坐标自动从 gnss_trans.x / gnss_trans.y 读取
 */
uint8 GnssRam_RecordPoint(uint8 point_type);

/**
 * @brief  获取当前已记录点数量
 * @return 点数量
 */
uint16 GnssRam_GetPointCount(void);

/**
 * @brief  根据点类型鸣叫蜂鸣器
 * @param  point_type 点类型 n
 * @note   实际鸣叫次数 = n + 1
 */
void GnssRam_Buzzer_Beep_By_PointType(uint8 point_type);

#endif  // _GNSS_RAM_H_