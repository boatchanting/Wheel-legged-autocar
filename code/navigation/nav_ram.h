#ifndef _NAV_RAM_H_
#define _NAV_RAM_H_

#include "zf_common_headfile.h"

//-------------------------------------------------------------------------------------------------------------------
//  @brief      惯导打点 RAM 管理模块
//  @note       1. 仅负责“打点 -> 存 RAM”，不涉及 Flash。
//              2. 点坐标来源于全局 inertial_nav.x / inertial_nav.y。
//              3. 点类型、plan 类型由外部逻辑（遥控器）控制。
//              4. 不引入任何时间相关参数。
//-------------------------------------------------------------------------------------------------------------------

// ========================= 配置 =========================
#define NAV_RAM_MAX_POINTS      5000     // RAM 中最多允许存储的惯导点数，后面正好能写下一页flash

// ========================= 点类型定义 =========================
typedef enum
{
    NAV_POINT_PATH = 0,     // 普通路径点
    NAV_POINT_CIRCLE = 1,   // 转圈点
    NAV_POINT_SLOPE = 2,    // 上坡点
    NAV_POINT_JUMP = 3,     // 跳跃点
    NAV_POINT_BRIDGE = 4,   // 单边桥点
    NAV_POINT_BUMP = 5      // 颠簸路段点
} NavPointType_e;

// ========================= plan 类型 =========================
typedef enum
{
    NAV_PLAN_1 = 1,
    NAV_PLAN_2 = 2,
    NAV_PLAN_3 = 3
} NavPlanType_e;

// ========================= 单个惯导点 =========================
typedef struct
{
    float x;                // 惯导 X 坐标 (mm)
    float y;                // 惯导 Y 坐标 (mm)
    uint8 point_type;       // 点类型 (NavPointType_e)
} NavRamPoint_t;

// ========================= RAM 总结构 =========================
typedef struct
{
    uint8 plan_type;                        // 当前 plan
    uint16 point_count;                     // 已记录点数量
    NavRamPoint_t points[NAV_RAM_MAX_POINTS];
} NavRamData_t;

// ========================= 全局变量 =========================
extern NavRamData_t nav_ram_data;

// ========================= 接口函数 =========================

/**
 * @brief  初始化惯导 RAM 打点模块
 * @note   清空所有 RAM 数据，一般在进入打点模式时调用
 */
void NavRam_Init(void);

/**
 * @brief  设置当前 plan 类型
 * @param  plan plan 类型 (NavPlanType_e)
 */
void NavRam_SetPlan(uint8 plan);

/**
 * @brief  记录一个惯导点到 RAM
 * @param  point_type 点类型 (NavPointType_e)
 * @return 0: 成功
 *         1: RAM 已满，记录失败
 * @note   点坐标自动从 inertial_nav.x / inertial_nav.y 读取
 */
uint8 NavRam_RecordPoint(uint8 point_type);

/**
 * @brief  获取当前已记录点数量
 * @return 点数量
 */
uint16 NavRam_GetPointCount(void);

/**
 * @brief  根据点类型鸣叫蜂鸣器
 * @param  point_type 点类型 n
 * @note   实际鸣叫次数 = n + 1
 */
void Buzzer_Beep_By_PointType(uint8 point_type);

#endif  // _NAV_RAM_H_
