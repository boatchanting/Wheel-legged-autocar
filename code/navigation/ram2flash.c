#include "ram2flash.h"

// 全局标志位
uint8 g_flash_save_finished = 0;

// 临时页缓冲区 (与 zf_driver_flash.h 中的定义保持一致，通常是一页的大小)
// 假设 FLASH_PAGE_LENGTH 为一页的字(uint32)数
static uint32 page_data_buffer[FLASH_PAGE_LENGTH]; 

// 数据转换辅助宏：将 float 的二进制直接视为 uint32，不进行数值转换
#define FLOAT_BITS_TO_UINT32(f_val) (*((uint32*)&(f_val)))
#define UINT32_BITS_TO_FLOAT(u_val) (*((float*)&(u_val)))

/**
 * @brief 初始化
 */
void Ram2Flash_Init(void) {
    flash_init();
    g_flash_save_finished = 0;
}

/**
 * @brief 核心函数：压缩轨迹并保存到 Flash
 * 
 * 逻辑：
 * 1. 遍历 nav_ram 中的所有点
 * 2. 判断是否为关键帧（起点、终点、曲线点、直线转折点）
 * 3. 将关键帧写入 Flash buffer，满了就写入物理 Flash
 */
uint8 Ram2Flash_SaveCompressed(void) {
    uint16 total_raw_records = NAV_RAM_GetRecordCount();
    if (total_raw_records < 2) return 0; // 点太少没必要存

    g_flash_save_finished = 0;
    
    // 状态变量
    uint16 raw_index = 0;
    uint16 saved_count = 0;       // 实际保存的点数
    uint32 current_page = FLASH_PAGE_DATA_START;
    uint32 buffer_index = 0;      // 当前页buffer的写入位置
    
    // 临时变量用于算法判断
    NavPoint_t p_prev, p_curr, p_next;
    float yaw_diff_curr, yaw_diff_next;
    uint8 is_key_frame = 0;

    // 清空缓冲区
    memset(page_data_buffer, 0xFF, sizeof(page_data_buffer));
    
    // 擦除数据区域 (从11页擦除到90页)
    // 注意：实际应用中为了速度，可以边写边擦，这里为了安全先全擦
    // 或者依靠 flash_write_page 内部的检查机制
    
    printf("Start Saving... Raw points: %d\r\n", total_raw_records);

    // --- 遍历所有点进行筛选 ---
    for (raw_index = 0; raw_index < total_raw_records; raw_index++) {
        
        // 获取当前点
        NAV_RAM_GetRecord(raw_index, &p_curr);
        
        is_key_frame = 0;

        // 【规则1】起点和终点必须保存
        if (raw_index == 0 || raw_index == total_raw_records - 1) {
            is_key_frame = 1;
        }
        else {
            // 获取前后点用于计算
            NAV_RAM_GetRecord(raw_index - 1, &p_prev);
            NAV_RAM_GetRecord(raw_index + 1, &p_next);

            // 计算偏航角变化 (取绝对值)
            // 简单处理：当前点与上一点的差
            yaw_diff_curr = fabsf(p_curr.yaw - p_prev.yaw);
            // 下一点与当前点的差
            yaw_diff_next = fabsf(p_next.yaw - p_curr.yaw);

            // 处理角度跳变 (例如 359度 -> 1度)
            if (yaw_diff_curr > 180.0f) yaw_diff_curr = 360.0f - yaw_diff_curr;
            if (yaw_diff_next > 180.0f) yaw_diff_next = 360.0f - yaw_diff_next;

            // 【规则2】曲线判定：如果角度变化大，说明在转弯，保存（保留100ms精度）
            if (yaw_diff_curr > COMPRESS_YAW_DIFF) {
                is_key_frame = 1; 
            }
            // 【规则3】直线结束判定：当前是直线，但下一点是曲线（入弯点），必须保存
            // 否则全速跑直线时会冲出弯道
            else if (yaw_diff_curr <= COMPRESS_YAW_DIFF && yaw_diff_next > COMPRESS_YAW_DIFF) {
                is_key_frame = 1;
            }
            // 否则：是直线中间的点，跳过（压缩）
        }

        // --- 如果判定为关键帧，写入 Buffer ---
        if (is_key_frame) {
            // 检查当前页是否满了 (一个点占3个uint32: x, y, yaw)
            if (buffer_index + 3 > FLASH_PAGE_LENGTH) {
                // 写入 Flash
                flash_write_page(0, current_page, page_data_buffer, FLASH_PAGE_LENGTH);
                current_page++;
                
                // 复位 Buffer
                buffer_index = 0;
                memset(page_data_buffer, 0xFF, sizeof(page_data_buffer));

                // 检查页溢出
                if (current_page > FLASH_PAGE_DATA_END) {
                    printf("Error: Flash Full! (Page %d)\r\n", FLASH_PAGE_DATA_END);
                    break; 
                }
            }

            // 将 float 转换为 uint32 存入数组 (禁止结构体指针，使用位拷贝)
            page_data_buffer[buffer_index++] = FLOAT_BITS_TO_UINT32(p_curr.x);
            page_data_buffer[buffer_index++] = FLOAT_BITS_TO_UINT32(p_curr.y);
            page_data_buffer[buffer_index++] = FLOAT_BITS_TO_UINT32(p_curr.yaw);
            
            if(saved_count == 0) {
                printf("[SAVE-DEBUG] First key point to save: x=%.1f, y=%.1f, yaw=%.1f\r\n", p_curr.x, p_curr.y, p_curr.yaw);
            }
            saved_count++;
        }
    }

    // 写入最后一页剩余的数据
    if (buffer_index > 0 && current_page <= FLASH_PAGE_DATA_END) {
        // 【FIX 1】: 修正 len 参数：传入实际写入的字数 buffer_index
        printf("[SAVE-DEBUG] Writing final data page %d with %d words.\r\n", current_page, buffer_index);
        flash_write_page(0, current_page, page_data_buffer, buffer_index);
    }

    // --- 写入信息页 (Page 10) ---
    memset(page_data_buffer, 0xFF, sizeof(page_data_buffer));
    page_data_buffer[0] = FLASH_MAGIC_NUM;      // 魔数
    page_data_buffer[1] = (uint32)saved_count;  // 压缩后的点数
    printf("[SAVE-DEBUG] Writing info page: Magic=0x%08X, Count=%d\r\n", page_data_buffer[0], page_data_buffer[1]);
    
    // 擦除并写入信息页
    if(flash_check(0, FLASH_PAGE_INFO)) {
        flash_erase_page(0, FLASH_PAGE_INFO);
    }
    // 【FIX 2】: 修正 len 参数：信息页只写入 2 个字
    flash_write_page(0, FLASH_PAGE_INFO, page_data_buffer, 2); 

    // 完成
    g_flash_save_finished = 1;
    printf("Save Done. Raw: %d -> Compressed: %d points.\r\n", total_raw_records, saved_count);
    
    return 1;
}

/**
 * @brief 从 Flash 读取数据回填到 NAV_RAM
 */
uint8 Ram2Flash_Load(void) {
    if (!Ram2Flash_CheckValid()) {
        printf("Flash Empty or Invalid.\r\n");
        return 0;
    }

    // ==========================================================
    // 【编译错误修复】将局部变量声明放在函数开头 调试用
    // ==========================================================
    uint32 magic_read;
    float x, y, yaw; // 声明 x, y, yaw

    // 1. 读取信息页
    flash_read_page(0, FLASH_PAGE_INFO, page_data_buffer, FLASH_PAGE_LENGTH);
    uint32 saved_count = page_data_buffer[1];

    printf("[LOAD-DEBUG] Read info page: Magic=0x%08X, Count=%d\r\n", magic_read, saved_count);

    if (saved_count == 0 || saved_count > 100000) return 0; // 异常保护

    // 2. 清空 RAM 准备接收
    NAV_RAM_ClearRecords();
    
    uint32 current_page = FLASH_PAGE_DATA_START;
    uint16 points_read = 0;
    uint32 buffer_idx = 0;

    // 预读取第一页数据
    flash_read_page(0, current_page, page_data_buffer, FLASH_PAGE_LENGTH);

    // 3. 循环读取
    while (points_read < saved_count) {
        // 检查 buffer 是否读完
        if (buffer_idx + 3 > FLASH_PAGE_LENGTH) {
            current_page++;
            if (current_page > FLASH_PAGE_DATA_END) break;

            // 读取下一页
            flash_read_page(0, current_page, page_data_buffer, FLASH_PAGE_LENGTH);
            buffer_idx = 0;
        }

        // 【DEBUG】打印每个还原的点
            printf("[LOAD-DEBUG] Point %d: x=%.1f, y=%.1f, yaw=%.1f\r\n", points_read, x, y, yaw);

            // 【DEBUG】检查数据是否异常 (NaN或无穷大)
            if(isnan(x) || isnan(y) || isnan(yaw)){
                printf("[LOAD-DEBUG] Error: Corrupted data (NaN) detected!\r\n");
                return 0;
            }

        // 还原数据
        float x = UINT32_BITS_TO_FLOAT(page_data_buffer[buffer_idx++]);
        float y = UINT32_BITS_TO_FLOAT(page_data_buffer[buffer_idx++]);
        float yaw = UINT32_BITS_TO_FLOAT(page_data_buffer[buffer_idx++]);

        // 添加到 RAM (注意：此时 RAM 里存的是压缩后的轨迹)
        // 之后的控制算法遍历这个 RAM 时，会发现点之间的物理距离可能很远（直线段）
        NAV_RAM_ForceAddRecord(x, y, yaw);
        
        points_read++;
    }
    
    printf("Loaded %d points to RAM.\r\n", points_read);
    return 1;
}

/**
 * @brief 检查 Flash 中是否有有效数据
 */
uint8 Ram2Flash_CheckValid(void) {
    // 读取信息页第一个字
    flash_read_page(0, FLASH_PAGE_INFO, page_data_buffer, FLASH_PAGE_LENGTH);
    
    if (page_data_buffer[0] == FLASH_MAGIC_NUM) {
        return 1;
    }
    return 0;
}

/**
 * @brief 清除 Flash 数据标志
 */
void Ram2Flash_Clear(void) {
    // 只需要擦除信息页，数据就不可见了
    flash_erase_page(0, FLASH_PAGE_INFO);
    g_flash_save_finished = 0;
}