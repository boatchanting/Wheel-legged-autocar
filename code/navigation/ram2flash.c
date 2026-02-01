#include "ram2flash.h"
#include "zf_driver_flash.h" 

// 全局变量
uint8_t g_save_finished_flag = 0;

// 本地缓存 buffer，大小与 Flash 页长度一致 (定义在 zf_driver_flash.h 中)
// 假设 FLASH_PAGE_LENGTH 是以 uint32 为单位的长度
static uint32_t page_buffer[FLASH_PAGE_LENGTH];

/**
 * @brief 初始化 Flash 模块
 */
void Ram2Flash_Init(void) {
    flash_init();
    g_save_finished_flag = 0;
}

/**
 * @brief 检查 Flash 中是否有有效的轨迹数据
 * @return 1=有效, 0=无效
 */
uint8_t Ram2Flash_IsDataValid(void) {
    // 读取信息页 (Page 10)
    flash_read_page(0, FLASH_INFO_PAGE, page_buffer, FLASH_PAGE_LENGTH);
    
    // 检查第一个字是否为魔数
    if (page_buffer[0] == TRAJECTORY_MAGIC_NUM) {
        return 1;
    }
    return 0;
}

/**
 * @brief 将 RAM 中的轨迹数据保存到 Flash (Page 11-90)
 * @return 1=成功, 0=失败(空间不足或错误)
 */
uint8_t Ram2Flash_Save(void) {
    uint16_t total_points = NAV_RAM_GetRecordCount();
    uint16_t current_point_idx = 0;
    uint32_t current_flash_page = FLASH_DATA_START_PAGE;
    
    g_save_finished_flag = 0;

    if (total_points == 0) {
        return 0; // 没有数据可存
    }

    // 1. 计算 Flash 容量是否足够
    // 每个点由 x, y, yaw 组成，占用 3 个 uint32
    // FLASH_PAGE_LENGTH 是每一页能存的 uint32 数量
    uint32_t points_per_page = FLASH_PAGE_LENGTH / 3;
    uint32_t max_pages = FLASH_DATA_END_PAGE - FLASH_DATA_START_PAGE + 1;
    uint32_t max_points = max_pages * points_per_page;

    if (total_points > max_points) {
        printf("Error: Too many points (%d) for Flash storage (Max %d)\r\n", total_points, max_points);
        return 0;
    }

    printf("Start Saving %d points to Flash...\r\n", total_points);

    // 2. 循环写入数据页
    while (current_point_idx < total_points) {
        // 清空缓冲区
        memset(page_buffer, 0xFF, sizeof(page_buffer));
        
        uint32_t buffer_idx = 0;
        
        // 填充一页数据
        // 条件：还有点没存 且 缓冲区还能再放下一个点(3个字)
        while ((current_point_idx < total_points) && ((buffer_idx + 3) <= FLASH_PAGE_LENGTH)) {
            NavPoint_t temp_point;
            
            // 获取 RAM 中的点
            NAV_RAM_GetRecord(current_point_idx, &temp_point);
            
            // 转换为 uint32 存入 buffer (X, Y, Yaw 顺序)
            // 严禁使用结构体指针直接强转 Flash 地址，这里使用值拷贝
            page_buffer[buffer_idx++] = FLOAT_TO_UINT32(temp_point.x);
            page_buffer[buffer_idx++] = FLOAT_TO_UINT32(temp_point.y);
            page_buffer[buffer_idx++] = FLOAT_TO_UINT32(temp_point.yaw);
            
            current_point_idx++;
        }
        
        // 写入当前页
        flash_write_page(0, current_flash_page, page_buffer, FLASH_PAGE_LENGTH);
        current_flash_page++;

        // 边界保护
        if (current_flash_page > FLASH_DATA_END_PAGE) {
            break;
        }
    }

    // 3. 写入信息页 (Page 10)
    // 必须最后写，这样如果中间掉电，魔数不会被写入，数据视为无效
    memset(page_buffer, 0xFF, sizeof(page_buffer));
    page_buffer[0] = TRAJECTORY_MAGIC_NUM; // 标记有效
    page_buffer[1] = (uint32_t)total_points; // 记录点数
    
    flash_write_page(0, FLASH_INFO_PAGE, page_buffer, FLASH_PAGE_LENGTH);

    g_save_finished_flag = 1;
    printf("Save Complete! Used pages: %d to %d\r\n", FLASH_DATA_START_PAGE, current_flash_page - 1);
    
    return 1;
}

/**
 * @brief 从 Flash 读取数据回填到 RAM
 * @return 1=成功, 0=无效数据或失败
 */
uint8_t Ram2Flash_Load(void) {
    // 1. 检查数据有效性
    if (!Ram2Flash_IsDataValid()) {
        printf("No valid trajectory in Flash.\r\n");
        return 0;
    }

    // 读取点数
    flash_read_page(0, FLASH_INFO_PAGE, page_buffer, FLASH_PAGE_LENGTH);
    uint16_t total_points = (uint16_t)page_buffer[1];

    // 清空当前 RAM
    NAV_RAM_ClearRecords();

    printf("Loading %d points from Flash...\r\n", total_points);

    uint16_t points_loaded = 0;
    uint32_t current_flash_page = FLASH_DATA_START_PAGE;

    // 2. 循环读取数据
    while (points_loaded < total_points) {
        // 读取一页
        flash_read_page(0, current_flash_page, page_buffer, FLASH_PAGE_LENGTH);
        
        uint32_t buffer_idx = 0;
        
        // 解析一页
        while ((points_loaded < total_points) && ((buffer_idx + 3) <= FLASH_PAGE_LENGTH)) {
            float x, y, yaw;
            
            // 从 buffer 还原 float
            x = UINT32_TO_FLOAT(page_buffer[buffer_idx++]);
            y = UINT32_TO_FLOAT(page_buffer[buffer_idx++]);
            yaw = UINT32_TO_FLOAT(page_buffer[buffer_idx++]);
            
            // 写入 RAM
            NAV_RAM_AddRecord(x, y, yaw);
            
            points_loaded++;
        }
        
        current_flash_page++;
        if (current_flash_page > FLASH_DATA_END_PAGE) break;
    }

    printf("Load Complete.\r\n");
    return 1;
}

/**
 * @brief 清除 Flash 中的轨迹标记 (擦除第10页即可)
 */
void Ram2Flash_ClearStorage(void) {
    flash_erase_page(0, FLASH_INFO_PAGE);
    printf("Flash storage cleared.\r\n");
}