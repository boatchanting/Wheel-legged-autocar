#include "ram2flash.h"

// 模块内部状态
static R2F_Status_t s_status = R2F_STATUS_IDLE;
// 静态缓冲区页长度（通常为 512/4 = 128 words）
#define PAGE_WORD_LENGTH   (FLASH_PAGE_LENGTH) 

// Flash 读写辅助结构体
typedef struct {
    uint16_t current_page;
    uint16_t word_offset; // 在当前页中的偏移 (以4字节为单位)
} FlashCursor_t;


/**
 * @brief  计算一个点到两点确定直线的垂直距离
 * @param  p  待测点
 * @param  p1 直线起点
 * @param  p2 直线终点
 * @return float 垂直距离(mm)
 */
static float point_to_line_distance(NavPoint_t p, NavPoint_t p1, NavPoint_t p2) {
    float A = p2.y - p1.y;
    float B = p1.x - p2.x;
    float C = p2.x * p1.y - p1.x * p2.y;
    
    if (A == 0 && B == 0) {
        // p1 和 p2 是同一个点，计算 p 到 p1 的距离
        return sqrtf(powf(p.x - p1.x, 2) + powf(p.y - p1.y, 2));
    }
    
    return fabsf(A * p.x + B * p.y + C) / sqrtf(A * A + B * B);
}

//  Flash 缓冲区填充进度
static uint16_t s_flash_buffer_fill_words = 0; // 当前 flash_union_buffer 中已填充的字数 (4字节单位)

/**
 * @brief  向Flash写入数据，自动处理换页
 * @param  cursor   Flash读写指针
 * @param  data     数据源指针
 * @param  words    要写入的数据长度 (以4字节为单位)
 * @return 1=成功, 0=失败(空间不足)
 */
//-------------------------------------------------------------------------------------------------------------------
//  @brief      向Flash写入数据，自动处理换页 (不使用偏移量参数，而是通过填充缓冲区实现)
//  @param      cursor      Flash读写指针 (仅用于跟踪当前页码)
//  @param      data        数据源指针
//  @param      words       要写入的数据总长度 (以4字节为单位)
//  @return     1=成功, 0=失败(空间不足)
//-------------------------------------------------------------------------------------------------------------------
static uint8_t write_to_flash(FlashCursor_t* cursor, void* data, uint16_t words) {
    uint8_t* data_ptr = (uint8_t*)data;
    uint16_t words_remaining = words; // 剩余需要写入的字数

    while (words_remaining > 0) {
        // 1. 检查是否超出 Flash 范围
        if (cursor->current_page > R2F_FLASH_END_PAGE) {
            s_status = R2F_STATUS_FULL;
            return 0; // Flash 空间不足
        }

        // 2. 计算当前页还能填充多少字
        uint16_t free_words_in_current_page = (FLASH_PAGE_SIZE / 4) - s_flash_buffer_fill_words;

        // 3. 计算本次实际可以填充的字数
        uint16_t words_to_fill_now = (words_remaining > free_words_in_current_page) ? free_words_in_current_page : words_remaining;

        // 4. 将数据从源拷贝到 flash_union_buffer
        memcpy(&flash_union_buffer[s_flash_buffer_fill_words], data_ptr, words_to_fill_now * 4);

        // 5. 更新指针和剩余字数
        data_ptr += words_to_fill_now * 4;
        words_remaining -= words_to_fill_now;
        s_flash_buffer_fill_words += words_to_fill_now;

        // 6. 如果当前页已满，则将缓冲区写入 Flash，然后准备下一页
        if (s_flash_buffer_fill_words >= (FLASH_PAGE_SIZE / 4)) {
            // 缓冲区已满，将填充的内容写入当前页
            flash_buffer_clear(); // 先清空一次，确保写入的是干净数据（虽然memcpy会覆盖）
            memcpy(flash_union_buffer, data, words * 4); // 重新拷贝一次，确保全部数据都在buffer中
            
            // 调用 Flash 写入函数，写入 s_flash_buffer_fill_words 个字
            // 注意：这里的 offset_words 参数被移除，意味着总是从页首写入
            flash_write_page_from_buffer(R2F_FLASH_SECTOR, cursor->current_page, s_flash_buffer_fill_words); 

            // 更新 cursor 到下一页，重置缓冲区填充进度
            cursor->current_page++;
            s_flash_buffer_fill_words = 0; // 重置，准备下一页
        }
    }
    return 1;
}


/**
 * @brief 初始化Ram2Flash模块
 */
void R2F_Init(void) {
    // flash_init() 应该在主函数中被调用过了
    s_status = R2F_STATUS_IDLE;
    printf("RAM2FLASH: Initialized. Flash pages %d-%d assigned.\r\n", R2F_FLASH_START_PAGE, R2F_FLASH_END_PAGE);
}

/**
 * @brief 擦除所有存储的轨迹数据
 * @return 操作状态
 */
R2F_Status_t R2F_EraseTrajectory(void) {
    s_status = R2F_STATUS_BUSY;
    printf("RAM2FLASH: Erasing pages %d to %d...\r\n", R2F_FLASH_START_PAGE, R2F_FLASH_END_PAGE);
    for (uint16_t page = R2F_FLASH_START_PAGE; page <= R2F_FLASH_END_PAGE; ++page) {
        flash_erase_page(R2F_FLASH_SECTOR, page);
    }
    printf("RAM2FLASH: Erase complete.\r\n");
    s_status = R2F_STATUS_SUCCESS;
    return s_status;
}


/**
 * @brief 从RAM压缩并保存轨迹到Flash
 * @return 操作状态
 */
R2F_Status_t R2F_SaveTrajectoryFromRAM(void) {
    uint16_t total_points = NAV_RAM_GetRecordCount();
    if (total_points < R2F_MIN_LINE_POINTS) {
        printf("RAM2FLASH: Error, not enough points in RAM to save (%d < %d).\r\n", total_points, R2F_MIN_LINE_POINTS);
        return R2F_STATUS_NO_RAM_DATA;
    }

    s_status = R2F_STATUS_BUSY;
    printf("RAM2FLASH: Starting trajectory save process...\r\n");

    // 1. 擦除Flash区域
    R2F_EraseTrajectory();

    FlashCursor_t cursor = {R2F_METADATA_PAGE + 1, 0}; // 数据从元数据页的下一页开始
    uint16_t current_ram_index = 0;
    uint16_t segment_count = 0;
    uint32_t checksum = 0;

    // 2. 循环压缩和写入
    while (current_ram_index < total_points) {
        // --- 尝试寻找最长的直线段 ---
        uint16_t line_end_index = 0;
        for (uint16_t j = current_ram_index + R2F_MIN_LINE_POINTS - 1; j < total_points; ++j) {
            uint8_t is_line = 1;
            NavPoint_t p_start, p_end;
            NAV_RAM_GetRecord(current_ram_index, &p_start);
            NAV_RAM_GetRecord(j, &p_end);
            
            // 检查中间点
            for (uint16_t k = current_ram_index + 1; k < j; ++k) {
                NavPoint_t p_mid;
                NAV_RAM_GetRecord(k, &p_mid);
                if (point_to_line_distance(p_mid, p_start, p_end) > R2F_LINE_DEVIATION_MM) {
                    is_line = 0;
                    break;
                }
            }
            
            if (is_line) {
                line_end_index = j; // 发现一条更长的直线，更新终点
            } else {
                break; // 直线中断，以上一条记录的 end_index 为准
            }
        }

        // --- 根据查找结果决定写入曲线还是直线 ---
        R2F_SegmentHeader_t header;
        if (line_end_index > 0) { // 找到了直线段
            header.type = SEGMENT_TYPE_LINE;
            header.point_count = 2;
            
            NavPoint_t p_start, p_end;
            NAV_RAM_GetRecord(current_ram_index, &p_start);
            NAV_RAM_GetRecord(line_end_index, &p_end);
            
            if(!write_to_flash(&cursor, &header, sizeof(header)/4)) return s_status;
            if(!write_to_flash(&cursor, &p_start, sizeof(p_start)/4)) return s_status;
            if(!write_to_flash(&cursor, &p_end, sizeof(p_end)/4)) return s_status;

            // 更新校验和与索引
            checksum += (*(uint32_t*)&p_start.x) + (*(uint32_t*)&p_end.x);
            current_ram_index = line_end_index; // 跳到直线段末尾
            
        } else { // 未找到足够长的直线，作为曲线段处理
            // 确定曲线段的长度 (到下一条潜在直线的起点)
            uint16_t curve_end_index = current_ram_index;
            // (此处可添加更智能的曲线切分逻辑，为简化，我们一次只存一个点作为曲线)
            // 简化处理：将单个点作为曲线段
            header.type = SEGMENT_TYPE_CURVE;
            header.point_count = 1;
            
            NavPoint_t point;
            NAV_RAM_GetRecord(current_ram_index, &point);

            if(!write_to_flash(&cursor, &header, sizeof(header)/4)) return s_status;
            if(!write_to_flash(&cursor, &point, sizeof(point)/4)) return s_status;

            // 更新校验和与索引
            checksum += (*(uint32_t*)&point.x);
            current_ram_index++;
        }
        segment_count++;
    }

    // 3. 写入元数据
    R2F_Metadata_t metadata;
    metadata.magic_number = R2F_MAGIC_NUMBER;
    metadata.total_segments = segment_count;
    metadata.checksum = checksum;
    metadata.reserved = 0;
    
    flash_buffer_clear();
    memcpy(flash_union_buffer, &metadata, sizeof(metadata));
    flash_write_page_from_buffer(R2F_FLASH_SECTOR, R2F_METADATA_PAGE, sizeof(metadata)/4);

    printf("RAM2FLASH: Save complete. Total segments: %d\r\n", segment_count);
    s_status = R2F_STATUS_SUCCESS;
    return s_status;
}

/**
 * @brief 检查Flash中是否有有效的轨迹数据
 * @return 1=有, 0=无
 */
uint8_t R2F_HasValidTrajectory(void) {
    R2F_Metadata_t metadata;
    flash_read_page_to_buffer(R2F_FLASH_SECTOR, R2F_METADATA_PAGE, sizeof(metadata)/4);
    memcpy(&metadata, flash_union_buffer, sizeof(metadata));
    
    if(metadata.magic_number == R2F_MAGIC_NUMBER) {
        return 1;
    }
    return 0;
}

/**
 * @brief 从Flash加载轨迹信息 (不加载具体数据)
 * @param segment_count 用于返回总段数的指针
 * @return 操作状态
 */
R2F_Status_t R2F_LoadTrajectoryInfo(uint16_t* segment_count) {
    if (!R2F_HasValidTrajectory()) {
        *segment_count = 0;
        return R2F_STATUS_NO_DATA;
    }
    
    R2F_Metadata_t metadata;
    flash_read_page_to_buffer(R2F_FLASH_SECTOR, R2F_METADATA_PAGE, sizeof(metadata)/4);
    memcpy(&metadata, flash_union_buffer, sizeof(metadata));
    
    *segment_count = metadata.total_segments;
    return R2F_STATUS_SUCCESS;
}

/**
 * @brief 获取指定的轨迹段数据 (兼容不支持结构体指针的编译器)
 * @param segment_index 要获取的段索引 (从0开始)
 * @param header_buffer 返回段头信息的缓冲区 (至少 sizeof(R2F_SegmentHeader_t) 字节)
 * @param points_buffer 返回轨迹点数据的缓冲区 (至少 buffer_size_bytes 字节)
 * @param buffer_size_bytes 轨迹点缓冲区大小(字节)
 * @return 操作状态
 */
R2F_Status_t R2F_GetSegment(uint16_t segment_index, uint8_t* type_out, uint16_t* count_out, float* coords_buffer, uint16_t buffer_size_f) {
    uint16_t total_segments;
    if (R2F_LoadTrajectoryInfo(&total_segments) != R2F_STATUS_SUCCESS) return R2F_STATUS_NO_DATA;
    if (segment_index >= total_segments || type_out == NULL || count_out == NULL || coords_buffer == NULL) return R2F_STATUS_DATA_ERROR;

    s_status = R2F_STATUS_BUSY;
    
    FlashCursor_t cursor = {R2F_METADATA_PAGE + 1, 0}; 
    uint8_t current_page_loaded = 0; 

    for (uint16_t i = 0; i <= segment_index; ++i) {
        
        // 检查是否需要换页或加载新页
        if (cursor.word_offset == 0 || !current_page_loaded) {
            if (cursor.current_page > R2F_FLASH_END_PAGE) {
                return R2F_STATUS_DATA_ERROR; 
            }
            // 读取整个页到缓冲区 (3 参数调用)
            flash_read_page_to_buffer(R2F_FLASH_SECTOR, cursor.current_page, PAGE_WORD_LENGTH);
            current_page_loaded = 1;
        }

        // 2. 从缓冲区中读取段头
        R2F_SegmentHeader_t temp_header;
        if (cursor.word_offset + sizeof(R2F_SegmentHeader_t)/4 > PAGE_WORD_LENGTH) {
            return R2F_STATUS_DATA_ERROR; // 段头跨页，简化代码不支持
        }
        memcpy(&temp_header, &flash_union_buffer[cursor.word_offset], sizeof(R2F_SegmentHeader_t));
        
        // 计算整个段的字数
        uint16_t point_floats = temp_header.point_count * 3; // 每个点 3 个 float
        uint32_t data_words = point_floats * (sizeof(float) / 4); // 4字节对齐
        uint32_t header_words = sizeof(R2F_SegmentHeader_t) / 4;
        uint32_t words_to_skip = header_words + data_words;
        
        // 3. 如果找到了目标段
        if (i == segment_index) {
            *type_out = temp_header.type;
            *count_out = temp_header.point_count;
            
            // 缓冲区大小检查：点数 * 3 (float)
            if (buffer_size_f < point_floats) { 
                s_status = R2F_STATUS_DATA_ERROR; // 提供的 float 缓冲区太小
                return s_status;
            }

            // 更新光标位置到数据开始处
            cursor.word_offset += header_words;
            
            // 检查数据是否跨页
            if (cursor.word_offset + data_words > PAGE_WORD_LENGTH) {
                 s_status = R2F_STATUS_DATA_ERROR; // 数据跨页，超出当前页容量 (简化处理)
                 return s_status;
            }

            // 从缓冲区中拷贝数据到 coords_buffer (float 数组)
            // 拷贝的字节数 = 点数 * 12 字节
            uint32_t data_bytes = data_words * 4;
            
            // flash_union_buffer[cursor.word_offset] 是联合体数组，我们取出其 uint32_type 的地址
            memcpy(coords_buffer, &flash_union_buffer[cursor.word_offset], data_bytes);
            
            s_status = R2F_STATUS_SUCCESS;
            return s_status;
        }

        // 4. 跳过当前段的数据，准备读取下一段
        cursor.word_offset += words_to_skip;
        
        // 检查是否需要换页
        if (cursor.word_offset >= PAGE_WORD_LENGTH) {
            cursor.word_offset -= PAGE_WORD_LENGTH;
            cursor.current_page++;
            current_page_loaded = 0; // 下一循环需要加载新页
        }
    }
    
    return R2F_STATUS_DATA_ERROR; // 未找到目标段
}