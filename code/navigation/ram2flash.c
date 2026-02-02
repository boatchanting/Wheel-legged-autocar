#include "ram2flash.h"
#include "../common.h"

/**
 * @brief  处理来自中断或外部逻辑的 Flash 读写请求
 */
void NavFlash_ProcessRequests(void)
{
    // 处理保存请求
    if (g_save_flash_request)
    {
        #if DEBUG_LOG_ENABLE
        printf("Flash Task: Saving data...\r\n");
        #endif
        if (NavFlash_SaveRamToFlash() == 0)
        {
            g_save_flash_request = 0; // 执行成功后清除标志位
            #if DEBUG_LOG_ENABLE
            printf("Flash Task: Save Done.\r\n");
            #endif
        }
    }

    // 处理读取请求
    if (g_load_flash_request)
    {
        #if DEBUG_LOG_ENABLE
        printf("Flash Task: Loading data...\r\n");
        #endif
        if (NavFlash_ReadFlashToRam() == 0)
        {
            g_load_flash_request = 0; // 执行成功后清除标志位
            #if DEBUG_LOG_ENABLE
            printf("Flash Task: Load Done.\r\n");
            #endif
        }
        else
        {
            // 如果读取失败（如Flash为空），也清除标志位防止死循环
            g_load_flash_request = 0;
            #if DEBUG_LOG_ENABLE
            printf("Flash Task: Load Failed.\r\n");
            #endif
        }
    }
}

/**
 * @brief  底层保存：将 RAM 数据持久化到 Flash
 */
uint8 NavFlash_SaveRamToFlash(void)
{
    uint16 i;
    uint16 buf_idx = OFF_POINTS_START;

    // 1. 擦除
    if(flash_check(NAV_FLASH_SECTION, NAV_FLASH_PAGE))
    {
        flash_erase_page(NAV_FLASH_SECTION, NAV_FLASH_PAGE);
    }

    flash_buffer_clear();

    // 2. 填充头部
    flash_union_buffer[OFF_MAGIC].uint32_type = NAV_FLASH_MAGIC;
    flash_union_buffer[OFF_COUNT].uint32_type = (uint32)nav_ram_data.point_count;
    flash_union_buffer[OFF_PLAN].uint32_type  = (uint32)nav_ram_data.plan_type;

    // 3. 填充坐标数据 (限制最大 166 个点以符合页大小)
    uint16 safe_count = (nav_ram_data.point_count > 166) ? 166 : nav_ram_data.point_count;
    
    for (i = 0; i < safe_count; i++)
    {
        flash_union_buffer[buf_idx++].float_type  = nav_ram_data.points[i].x;
        flash_union_buffer[buf_idx++].float_type  = nav_ram_data.points[i].y;
        flash_union_buffer[buf_idx++].uint32_type = (uint32)nav_ram_data.points[i].point_type;
        #if DEBUG_LOG_ENABLE
        printf("NavFlash_SaveRamToFlash: %d, %f, %f, %d\r\n", i, nav_ram_data.points[i].x, nav_ram_data.points[i].y, nav_ram_data.points[i].point_type);
        #endif
    }

    // 4. 写入
    uint32 write_len = OFF_POINTS_START + (safe_count * 3);
    return flash_write_page_from_buffer(NAV_FLASH_SECTION, NAV_FLASH_PAGE, write_len);
}

/**
 * @brief  底层读取：解析 Flash 数据到 RAM 内存
 */
uint8 NavFlash_ReadFlashToRam(void)
{
    uint16 i;
    uint16 buf_idx = OFF_POINTS_START;

    // 1. 读取 Flash 全页到缓冲区
    flash_read_page_to_buffer(NAV_FLASH_SECTION, NAV_FLASH_PAGE, FLASH_PAGE_LENGTH);

    // 2. 校验魔数，判断 Flash 是否被有效写过
    if (flash_union_buffer[OFF_MAGIC].uint32_type != NAV_FLASH_MAGIC)
    {
        #if DEBUG_LOG_ENABLE
        printf("Error: No valid Nav data in Flash!\r\n");
        #endif
        return 1;
    }

    // 3. 解析点数和 Plan 类型
    nav_ram_data.point_count = (uint16)flash_union_buffer[OFF_COUNT].uint32_type;
    nav_ram_data.plan_type   = (uint8)flash_union_buffer[OFF_PLAN].uint32_type;
    
    #if DEBUG_LOG_ENABLE
        printf("NavFlash_ReadFlashToRam: point_count = %d, plan_type = %d\r\n", nav_ram_data.point_count, nav_ram_data.plan_type);
    #endif

    // 4. 限制解析范围，防止越界
    if (nav_ram_data.point_count > 166) nav_ram_data.point_count = 166;

    // 5. 循环解析
    for (i = 0; i < nav_ram_data.point_count; i++)
    {
        nav_ram_data.points[i].x          = flash_union_buffer[buf_idx++].float_type;
        nav_ram_data.points[i].y          = flash_union_buffer[buf_idx++].float_type;
        nav_ram_data.points[i].point_type = (uint8)flash_union_buffer[buf_idx++].uint32_type;
        #if DEBUG_LOG_ENABLE
        printf("NavFlash_ReadFlashToRam: %d, %f, %f, %d\r\n", 
               i, nav_ram_data.points[i].x, nav_ram_data.points[i].y, nav_ram_data.points[i].point_type);
        #endif
    }

    return 0;
}