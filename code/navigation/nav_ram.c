#include "nav_ram.h"

//-------------------------------------------------------------------------------------------------------------------
//  全局 RAM 数据实例
//-------------------------------------------------------------------------------------------------------------------
NavRamData_t nav_ram_data;

/**
 * @brief  初始化惯导 RAM 打点模块
 */
void NavRam_Init(void)
{
    uint16 i;

    nav_ram_data.plan_type  = 0;
    nav_ram_data.point_count = 0;

    // 可选：清空点内容，便于调试
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
 * @brief  设置当前 plan 类型
 * @param  plan plan 类型
 */
void NavRam_SetPlan(uint8 plan)
{
    nav_ram_data.plan_type = plan;
}

/**
 * @brief  记录一个惯导点到 RAM
 */
uint8 NavRam_RecordPoint(uint8 point_type)
{
    uint16 idx;

    // RAM 已满
    if (nav_ram_data.point_count >= NAV_RAM_MAX_POINTS)
    {
        return 1;
    }

    idx = nav_ram_data.point_count;

    // 直接读取当前惯导解算结果
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
 * @brief  获取当前已记录点数量
 */
uint16 NavRam_GetPointCount(void)
{
    return nav_ram_data.point_count;
}

/**
 * @brief  根据点类型鸣叫蜂鸣器
 * @param  point_type 点类型 n
 * @note   实际鸣叫次数 = n + 1
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
