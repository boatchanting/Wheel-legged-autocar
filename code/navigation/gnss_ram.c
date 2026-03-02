#include "gnss_ram.h"

//-------------------------------------------------------------------------------------------------------------------
//  全局 RAM 数据实例
//-------------------------------------------------------------------------------------------------------------------
GnssRamData_t gnss_ram_data;

/**
 * @brief  初始化 GNSS RAM 打点模块
 */
void GnssRam_Init(void)
{
    uint16 i;

    gnss_ram_data.plan_type  = 0;
    gnss_ram_data.point_count = 0;

    // 清空点内容，便于调试
    for (i = 0; i < GNSS_RAM_MAX_POINTS; i++)
    {
        gnss_ram_data.points[i].x = 0.0f;
        gnss_ram_data.points[i].y = 0.0f;
        gnss_ram_data.points[i].point_type = 0;
    }
}

/**
 * @brief  设置当前 plan 类型
 * @param  plan plan 类型
 */
void GnssRam_SetPlan(uint8 plan)
{
    gnss_ram_data.plan_type = plan;
}

/**
 * @brief  记录一个 GNSS 点到 RAM
 */
uint8 GnssRam_RecordPoint(uint8 point_type)
{
    uint16 idx;

    // RAM 已满
    if (gnss_ram_data.point_count >= GNSS_RAM_MAX_POINTS)
    {
        return 1;
    }

    // 可选：如果需要严格控制，可以在这里检查 gnss_trans.is_valid 是否为 1 
    // if (!gnss_trans.is_valid) return 2; // 无效数据不记录

    idx = gnss_ram_data.point_count;

    // 直接读取当前 GNSS 高斯投影相对坐标解算结果
    gnss_ram_data.points[idx].x = gnss_trans.x;
    gnss_ram_data.points[idx].y = gnss_trans.y;
    gnss_ram_data.points[idx].point_type = point_type;

    gnss_ram_data.point_count++;

    return 0;
}

/**
 * @brief  获取当前已记录点数量
 */
uint16 GnssRam_GetPointCount(void)
{
    return gnss_ram_data.point_count;
}

/**
 * @brief  根据点类型鸣叫蜂鸣器
 * @param  point_type 点类型 n
 * @note   实际鸣叫次数 = n + 1
 */
void GnssRam_Buzzer_Beep_By_PointType(uint8 point_type)
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