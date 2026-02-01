#include "ram2flash.h"

// 全局标志位
uint8 g_flash_save_finished = 0;

// 【修改点 1】声明外部缓冲区 (定义在 zf_driver_flash.c 中)
extern flash_data_union flash_union_buffer[FLASH_PAGE_LENGTH];

// 【修改点 2】移除本地静态缓冲区
// static uint32 page_data_buffer[FLASH_PAGE_LENGTH]; 

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
    uint32 buffer_index = 0;      // 当前页buffer的写入位置 (在 flash_union_buffer 内的索引)
    
    // 临时变量用于算法判断
    NavPoint_t p_prev, p_curr, p_next;
    float yaw_diff_curr, yaw_diff_next;
    uint8 is_key_frame = 0;

    // 【修改点 3】 清空全局缓冲区，防止残留垃圾数据
    flash_buffer_clear(); 
    
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
                // 【修改点 4】 使用高级写入接口 flash_write_page_from_buffer
                printf("[SAVE-DEBUG] Writing data page %d (FULL) from buffer...\r\n", current_page);
                flash_write_page_from_buffer(0, current_page, FLASH_PAGE_LENGTH); // 写入一整页
                current_page++;
                
                // 复位 Buffer
                buffer_index = 0;
                flash_buffer_clear(); // 写入后清空全局缓冲区

                // 检查页溢出
                if (current_page > FLASH_PAGE_DATA_END) {
                    printf("Error: Flash Full! (Page %d)\r\n", FLASH_PAGE_DATA_END);
                    break; 
                }
            }

            // 【修改点 5】 将数据写入全局缓冲区
            flash_union_buffer[buffer_index++].uint32_type = FLOAT_BITS_TO_UINT32(p_curr.x);
            flash_union_buffer[buffer_index++].uint32_type = FLOAT_BITS_TO_UINT32(p_curr.y);
            flash_union_buffer[buffer_index++].uint32_type = FLOAT_BITS_TO_UINT32(p_curr.yaw);
            
            if(saved_count == 0) {
                printf("[SAVE-DEBUG] First key point to save: x=%.1f, y=%.1f, yaw=%.1f\r\n", p_curr.x, p_curr.y, p_curr.yaw);
            }
            saved_count++;
        }
    }

    // 写入最后一页剩余的数据
    if (buffer_index > 0 && current_page <= FLASH_PAGE_DATA_END) {
        // 【修改点 6】 写入实际使用的字数
        printf("[SAVE-DEBUG] Writing final data page %d with %d words from buffer.\r\n", current_page, buffer_index);
        flash_write_page_from_buffer(0, current_page, buffer_index);
    }

    // --- 写入信息页 (Page 10) ---
    flash_buffer_clear(); 
    flash_union_buffer[0].uint32_type = FLASH_MAGIC_NUM;      // 魔数
    flash_union_buffer[1].uint32_type = (uint32)saved_count;  // 压缩后的点数
    printf("[SAVE-DEBUG] Writing info page: Magic=0x%08X, Count=%d\r\n", flash_union_buffer[0].uint32_type, flash_union_buffer[1].uint32_type);
    
    // 写入信息页
    // flash_write_page_from_buffer 内部会检查是否需要擦除
    flash_write_page_from_buffer(0, FLASH_PAGE_INFO, 2); // 只需要写入 2 个字

    // 完成
    g_flash_save_finished = 1;
    printf("Save Done. Raw: %d -> Compressed: %d points.\r\n", total_raw_records, saved_count);
    
    return 1;
}

/**
 * @brief 从 Flash 读取数据回填到 NAV_RAM
 */
uint8 Ram2Flash_Load(void) {
    // 1. 检查数据有效性（必须先读取到全局缓冲区）
    flash_read_page_to_buffer(0, FLASH_PAGE_INFO, 2); // 读取魔数和点数
    uint32 magic_read = flash_union_buffer[0].uint32_type;

    if (magic_read != FLASH_MAGIC_NUM) {
        // 【修改点 7】现在判断是否有效的逻辑更简单
        printf("Flash Empty or Invalid Magic (Read: 0x%08X, Expected: 0x%08X).\r\n", magic_read, FLASH_MAGIC_NUM);
        return 0;
    }

    // 读取点数
    uint32 saved_count = flash_union_buffer[1].uint32_type;

    printf("[LOAD-DEBUG] Read info page: Magic=0x%08X, Count=%d\r\n", magic_read, saved_count);
    
    if (saved_count == 0 || saved_count > NAV_MAX_RECORDS) return 0; // 异常保护

    // 2. 清空 RAM 准备接收
    NAV_RAM_ClearRecords();
    
    uint32 current_page = FLASH_PAGE_DATA_START;
    uint16 points_read = 0;
    uint32 buffer_idx = 0;

    printf("Loading %d points from Flash...\r\n", saved_count);

    // 3. 循环读取数据
    while (points_read < saved_count) {
        
        // 检查是否需要加载新页
        if (buffer_idx + 3 > FLASH_PAGE_LENGTH || buffer_idx == 0) { 
            // 如果 buffer 读完，或者这是第一次循环，需要从 Flash 读取新页到 buffer
            if (current_page > FLASH_PAGE_DATA_END) break;

            uint32 remaining_points = saved_count - points_read;
            uint32 words_to_read = remaining_points * 3;
            // 一页最多读 FLASH_PAGE_LENGTH 个字
            if (words_to_read > FLASH_PAGE_LENGTH) {
                words_to_read = FLASH_PAGE_LENGTH;
            }

            printf("[LOAD-DEBUG] Reading data page %d, %d words...\r\n", current_page, words_to_read);
            
            // 【修改点 8】使用高级读取接口
            flash_read_page_to_buffer(0, current_page, words_to_read); 
            buffer_idx = 0;
            current_page++;
        }

        // 还原数据
        float x = UINT32_BITS_TO_FLOAT(flash_union_buffer[buffer_idx++].uint32_type);
        float y = UINT32_BITS_TO_FLOAT(flash_union_buffer[buffer_idx++].uint32_type);
        float yaw = UINT32_BITS_TO_FLOAT(flash_union_buffer[buffer_idx++].uint32_type);
        
        // 【DEBUG】检查数据是否异常 (NaN或无穷大)
        if(isnan(x) || isnan(y) || isnan(yaw)){
            printf("[LOAD-DEBUG] Error: Corrupted data (NaN) detected at point %d!\r\n", points_read);
            // 遇到坏数据时可以决定是停止还是跳过
            return 0; // 选择停止加载
        }
        
        // 【DEBUG】打印每个还原的点
        // printf("[LOAD-DEBUG] Point %d: x=%.1f, y=%.1f, yaw=%.1f\r\n", points_read, x, y, yaw);

        // 添加到 RAM (注意：此时 RAM 里存的是压缩后的轨迹)
        // 之后的控制算法遍历这个 RAM 时，会发现点之间的物理距离可能很远（直线段）
        NAV_RAM_ForceAddRecord(x, y, yaw);
        
        points_read++;
    }
    
    printf("Loaded %d points to RAM.\r\n", NAV_RAM_GetRecordCount());
    return 1;
}

/**
 * @brief 检查 Flash 中是否有有效数据
 */
uint8 Ram2Flash_CheckValid(void) {
    // 【修改点 9】 必须读取到全局缓冲区才能判断
    flash_read_page_to_buffer(0, FLASH_PAGE_INFO, 1);
    
    if (flash_union_buffer[0].uint32_type == FLASH_MAGIC_NUM) {
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
    printf("Flash storage cleared.\r\n");
}