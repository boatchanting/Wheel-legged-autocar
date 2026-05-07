#include "nav_ram.h"

//--------------------------------------------------------------------------------------------------
// 全局 RAM 轨迹数据实例：打点、回放与存储模块共用
//--------------------------------------------------------------------------------------------------
NavRamData_t nav_ram_data;

/**
 * @brief 初始化导航 RAM 打点模块
 * @note 调用位置：进入打点模式前或整车流程复位时调用
 */
void NavRam_Init(void)
{
    uint16 i;

    nav_ram_data.plan_type = 0;
    nav_ram_data.point_count = 0;

    // 清空点缓存，防止历史数据干扰当前任务
    for (i = 0; i < NAV_RAM_MAX_POINTS; i++)
    {
        nav_ram_data.points[i].x = 0.0f;
        nav_ram_data.points[i].y = 0.0f;
        nav_ram_data.points[i].target_yaw_deg = 0.0f;
        nav_ram_data.points[i].heading_deg = 0.0f;
        nav_ram_data.points[i].target_speed = 0.0f;
        nav_ram_data.points[i].point_type = 0;
    }
}

/**
 * @brief 设置当前导航 plan 类型
 * @param plan 科目类型（见 NavPlanType_e）
 * @note 调用位置：上层切换科目前调用
 */
void NavRam_SetPlan(uint8 plan)
{
    nav_ram_data.plan_type = plan;
}

/**
 * @brief 记录一个导航点到 RAM
 * @param point_type 点类型（普通点、圆环点等）
 * @return 0 成功；1 RAM 已满
 * @note 调用位置：打点流程周期调用；坐标来自当前惯导输出
 */
uint8 NavRam_RecordPoint(uint8 point_type)
{
    uint16 idx;

    // RAM 已满，拒绝继续写入
    if (nav_ram_data.point_count >= NAV_RAM_MAX_POINTS)
    {
        return 1;
    }

    idx = nav_ram_data.point_count;

    // 直接读取当前惯导位姿；target_speed 后续由离线规划脚本写入
    nav_ram_data.points[idx].x = inertial_nav.x;
    nav_ram_data.points[idx].y = inertial_nav.y;
    nav_ram_data.points[idx].target_yaw_deg = inertial_nav.relative_yaw;
#if IMU_CATEGORY == 3
    nav_ram_data.points[idx].heading_deg = heading;
#else
    nav_ram_data.points[idx].heading_deg = 0.0f;
#endif
    nav_ram_data.points[idx].target_speed = 0.0f;
    nav_ram_data.points[idx].point_type = point_type;

    nav_ram_data.point_count++;

    return 0;
}

/**
 * @brief 获取当前已记录点数量
 * @return 当前点数
 * @note 调用位置：上层显示、保存和回放前检查
 */
uint16 NavRam_GetPointCount(void)
{
    return nav_ram_data.point_count;
}

/**
 * @brief 根据点类型进行蜂鸣器提示
 * @param point_type 点类型枚举值
 * @note 调用位置：打点成功反馈；实际鸣叫次数 = point_type + 1
 */
void Buzzer_Beep_By_PointType(uint8 point_type)
{
    uint8 beep_times = point_type + 1;

    for (uint8 i = 0; i < beep_times; i++)
    {
        gpio_set_level(BUZZER_PIN, 1);
        system_delay_ms(100);
        gpio_set_level(BUZZER_PIN, 0);

        if (i < beep_times - 1)
        {
            system_delay_ms(50);
        }
    }
}
