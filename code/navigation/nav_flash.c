#include "nav_flash.h"

// ======================= 私有变量定义 =======================

// 1. 记录器状态
NavState_e nav_recorder_state = NAV_STATE_IDLE;

// 2. RAM 原始数据缓存 (3s 窗口)
static NavMemoryBuffer_t s_ram_buf;

// 3. Flash 写入控制变量
uint32_t nav_total_saved_points = 0;       // 总共已存点数
static uint32_t s_current_flash_page = 0;  // 当前正在写入的物理页码
static uint32_t s_flash_union_idx = 0;     // 当前 flash_union_buffer 的写入索引 (0 ~ 511)

// ======================= 私有函数声明 =======================
static void NavFlash_Push_Point_To_Write_Buffer(NavPoint_t* p);
static void NavFlash_Flush_Ram_Buf_Logic(void);
static void NavFlash_Write_Physical_Page(void);

// ======================= 接口实现 =======================

/**
 * @brief 初始化
 */
void NavFlash_Init(void) {
    flash_init();
    
    // 读取 Header 页 (Page 10)，恢复之前的记录信息
    // 假设 Header 页第 0 个字存储了总点数
    if (flash_check(NAV_FLASH_SECTION, NAV_FLASH_PAGE_HEADER)) {
        flash_read_page_to_buffer(NAV_FLASH_SECTION, NAV_FLASH_PAGE_HEADER, 1);
        nav_total_saved_points = flash_union_buffer[0].uint32_type;
    } else {
        nav_total_saved_points = 0;
    }

    nav_recorder_state = NAV_STATE_IDLE;
    s_ram_buf.count = 0;
    s_flash_union_idx = 0;
}

/**
 * @brief 清除所有数据
 */
void NavFlash_Clear_All(void) {
    nav_recorder_state = NAV_STATE_IDLE;
    nav_total_saved_points = 0;
    
    // 1. 擦除 Header 页
    flash_erase_page(NAV_FLASH_SECTION, NAV_FLASH_PAGE_HEADER);
    
    // 2. 擦除所有 Data 页 (从 11 到 11+Max)
    // 注意：擦除操作耗时，循环擦除可能会导致看门狗复位，建议在安全环境下调用
    for (uint32_t i = 0; i < NAV_FLASH_MAX_PAGES; i++) {
        flash_erase_page(NAV_FLASH_SECTION, NAV_FLASH_PAGE_DATA_START + i);
    }
}

/**
 * @brief 开始记录
 */
void NavFlash_Start_Record(void) {
    // 先清除旧数据，保证从头开始写
    // 为了节省时间，Start 时可以只擦除 Header 和第一页，后续写到哪擦到哪
    // 但为了逻辑简单安全，这里调用全擦除，或者你可以只重置索引
    // 考虑到 192KB 擦除也很快，这里重置状态即可，擦除在 Write 时按页进行更高效
    
    // 重置 RAM 状态
    s_ram_buf.count = 0;
    s_ram_buf.is_curved = 0;
    
    // 重置 Flash 写入索引
    nav_total_saved_points = 0;
    s_current_flash_page = NAV_FLASH_PAGE_DATA_START;
    s_flash_union_idx = 0;
    flash_buffer_clear(); // 清空全局 union buffer
    
    // 擦除第一个数据页，准备写入
    flash_erase_page(NAV_FLASH_SECTION, s_current_flash_page);

    nav_recorder_state = NAV_STATE_RECORDING;
}

/**
 * @brief 停止记录
 */
void NavFlash_Stop_Record(void) {
    if (nav_recorder_state != NAV_STATE_RECORDING) return;
    
    // 1. 强制处理 RAM 中剩余的数据 (比如最后不足 3s 的部分)
    if (s_ram_buf.count > 0) {
        NavFlash_Flush_Ram_Buf_Logic();
    }
    
    // 2. 如果 Flash 写入缓冲区里还有残余数据 (未满一页)，强制写入物理 Flash
    if (s_flash_union_idx > 0) {
        flash_write_page_from_buffer(NAV_FLASH_SECTION, s_current_flash_page, s_flash_union_idx);
    }
    
    // 3. 更新 Header 页，保存总点数
    flash_buffer_clear();
    flash_union_buffer[0].uint32_type = nav_total_saved_points;
    flash_erase_page(NAV_FLASH_SECTION, NAV_FLASH_PAGE_HEADER);
    flash_write_page_from_buffer(NAV_FLASH_SECTION, NAV_FLASH_PAGE_HEADER, 1);
    
    nav_recorder_state = NAV_STATE_IDLE;
}

/**
 * @brief 10ms 周期任务
 */
void NavFlash_Record_Task_10ms(float x, float y, float yaw){
    if (nav_recorder_state != NAV_STATE_RECORDING) return;

    // 1. 存入 RAM 缓存
    uint16_t idx = s_ram_buf.count;
    if (idx < NAV_MEM_BUFFER_SIZE) {
         // 直接赋值
        s_ram_buf.raw_points[idx].x = x;
        s_ram_buf.raw_points[idx].y = y;
        s_ram_buf.raw_points[idx].yaw = yaw;
        
        // 2. 实时判断是否曲线 (与前一点比较)
        if (idx > 0) {
            float diff = fabsf(s_ram_buf.raw_points[idx].yaw - s_ram_buf.raw_points[idx-1].yaw);
            // 简单的角度跳变处理 (如 179 -> -179)
            if (diff > 180.0f) diff = 360.0f - diff;
            
            if (diff > NAV_CURVE_THRESHOLD) {
                s_ram_buf.is_curved = 1;
            }
        }
        s_ram_buf.count++;
    }

    // 3. 检查 RAM 缓存是否已满 (3s 数据到位)
    if (s_ram_buf.count >= NAV_MEM_BUFFER_SIZE) {
        NavFlash_Flush_Ram_Buf_Logic(); // 执行压缩并移动到 Flash 缓冲区
    }
}

// ======================= 核心逻辑函数 =======================

/**
 * @brief 核心逻辑：分析 RAM 中的 3s 数据，压缩后推送到 Flash 写入队列
 */
static void NavFlash_Flush_Ram_Buf_Logic(void) {
    if (s_ram_buf.count == 0) return;

    if (s_ram_buf.is_curved == 0) {
        // --- 纯直线逻辑：只存首尾 ---
        // 存起点
        NavFlash_Push_Point_To_Write_Buffer(&s_ram_buf.raw_points[0]);
        // 存终点 (如果点数大于1)
        if (s_ram_buf.count > 1) {
            NavFlash_Push_Point_To_Write_Buffer(&s_ram_buf.raw_points[s_ram_buf.count - 1]);
        }
    } 
    else {
        // --- 曲线逻辑：按 100ms 抽稀存储 ---
        // NAV_LOG_DECIMATION = 10, 即每隔 10 个点存一次
        for (uint16_t i = 0; i < s_ram_buf.count; i += NAV_LOG_DECIMATION) {
            NavFlash_Push_Point_To_Write_Buffer(&s_ram_buf.raw_points[i]);
        }
        // 确保把最后一个点存进去，保证轨迹连续性
        if ((s_ram_buf.count - 1) % NAV_LOG_DECIMATION != 0) {
             NavFlash_Push_Point_To_Write_Buffer(&s_ram_buf.raw_points[s_ram_buf.count - 1]);
        }
    }

    // 清空 RAM 缓存状态，准备下个 3s
    s_ram_buf.count = 0;
    s_ram_buf.is_curved = 0;
}

/**
 * @brief 将单个点推送到 Flash 写入缓冲区 (flash_union_buffer)
 *        如果缓冲区满了，自动写入物理 Flash 并换页
 */
static void NavFlash_Push_Point_To_Write_Buffer(NavPoint_t* p) {
    // 检查 Flash 剩余空间
    if (s_current_flash_page >= NAV_FLASH_PAGE_DATA_START + NAV_FLASH_MAX_PAGES) {
        return; // Flash 满了，丢弃数据
    }

    // 每个点占用 3 个 float 空间
    // 检查当前页是否写得下 (FLASH_PAGE_LENGTH = 512)
    if (s_flash_union_idx + NAV_POINT_SIZE_WORDS > FLASH_PAGE_LENGTH) {
        NavFlash_Write_Physical_Page(); // 缓冲区满了，写入物理页
    }

    // 写入全局 union buffer
    flash_union_buffer[s_flash_union_idx].float_type     = p->x;
    flash_union_buffer[s_flash_union_idx + 1].float_type = p->y;
    flash_union_buffer[s_flash_union_idx + 2].float_type = p->yaw;

    s_flash_union_idx += NAV_POINT_SIZE_WORDS;
    nav_total_saved_points++;
}

/**
 * @brief 将填满的 union buffer 写入物理 Flash 页，并准备下一页
 */
static void NavFlash_Write_Physical_Page(void) {
    // 1. 写入当前页
    // 注意：flash_write_page_from_buffer 会把 flash_union_buffer 的前 len 个数据写入
    flash_write_page_from_buffer(NAV_FLASH_SECTION, s_current_flash_page, s_flash_union_idx);

    // 2. 准备下一页
    s_current_flash_page++;
    
    // 如果没有越界，擦除下一页，为后续写入做准备
    if (s_current_flash_page < NAV_FLASH_PAGE_DATA_START + NAV_FLASH_MAX_PAGES) {
        flash_erase_page(NAV_FLASH_SECTION, s_current_flash_page);
    }
    
    // 3. 重置缓冲区索引
    s_flash_union_idx = 0;
    flash_buffer_clear(); // 这是一个好习惯，防止残留数据
}

/**
 * @brief [读取功能] 读取指定索引的点
 */
uint8_t NavFlash_Read_Point(uint32_t index, NavPoint_t* out_point) {
    if (index >= nav_total_saved_points) return 0; // 越界

    // 计算该点位于哪个页，以及页内的哪个偏移
    // 每页能存多少个点? 512 / 3 = 170 个点 (余 2 个字)
    uint32_t points_per_page = FLASH_PAGE_LENGTH / NAV_POINT_SIZE_WORDS; // 170
    
    uint32_t target_page_offset = index / points_per_page;
    uint32_t target_point_idx_in_page = index % points_per_page;
    
    uint32_t phys_page = NAV_FLASH_PAGE_DATA_START + target_page_offset;

    // 读取该页到 buffer
    // 优化：如果系统 RAM 够大，可以不用每次读 Flash，但在单片机上通常按需读取
    flash_read_page_to_buffer(NAV_FLASH_SECTION, phys_page, FLASH_PAGE_LENGTH);
    
    uint32_t buf_start_idx = target_point_idx_in_page * NAV_POINT_SIZE_WORDS;
    
    out_point->x   = flash_union_buffer[buf_start_idx].float_type;
    out_point->y   = flash_union_buffer[buf_start_idx+1].float_type;
    out_point->yaw = flash_union_buffer[buf_start_idx+2].float_type;
    
    return 1;
}