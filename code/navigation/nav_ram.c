#include "nav_ram.h"

// 全局变量定义
NavRecordManager_t nav_manager;

// 静态变量 - 使用内存池存储轨迹数据
static NavPoint_t nav_buffer[NAV_MAX_RECORDS];

/**
 * @brief 初始化导航数据存储模块
 */
void NAV_RAM_Init(void) {
    // 初始化管理器
    nav_manager.buffer = nav_buffer;
    nav_manager.write_index = 0;
    nav_manager.read_index = 0;
    nav_manager.record_count = 0;
    nav_manager.status = NAV_STATUS_IDLE;
    nav_manager.last_record_time = 0;
    nav_manager.overflow_flag = 0;
    
    // 清空缓冲区
    memset(nav_buffer, 0, sizeof(nav_buffer));
    
    // printf("NAV_RAM: Initialized with %d records capacity (%d KB)\r\n", 
    //        NAV_MAX_RECORDS, NAV_MAX_RAM_SIZE_KB);
}

/**
 * @brief 添加轨迹记录点
 * @param x X坐标(mm)
 * @param y Y坐标(mm)
 * @param yaw 偏航角(度)
 * @return 成功标志: 0=失败, 1=成功
 */
uint8_t NAV_RAM_AddRecord(float x, float y, float yaw) {
    // 检查是否已满
    if (nav_manager.overflow_flag || nav_manager.record_count >= NAV_MAX_RECORDS) {
        nav_manager.status = NAV_STATUS_FULL;
        nav_manager.overflow_flag = 1;
        return 0;
    }
    
    // 检查时间间隔(如果不是在中断中调用，需要保证100ms间隔)
    // uint32_t current_time = systick_get_ms();
    // if (current_time - nav_manager.last_record_time < NAV_RECORD_INTERVAL_MS) {
    //     return 0;
    // }
    
    // 更新记录时间
    // nav_manager.last_record_time = current_time;
    
    // 写入数据
    nav_manager.buffer[nav_manager.write_index].x = x;
    nav_manager.buffer[nav_manager.write_index].y = y;
    nav_manager.buffer[nav_manager.write_index].yaw = yaw;
    
    // 更新索引和计数
    nav_manager.write_index++;
    nav_manager.record_count++;
    
    // 环形缓冲区处理
    if (nav_manager.write_index >= NAV_MAX_RECORDS) {
        nav_manager.write_index = 0;
    }
    
    // 检查是否即将写满
    if (nav_manager.record_count >= NAV_MAX_RECORDS) {
        nav_manager.status = NAV_STATUS_FULL;
        nav_manager.overflow_flag = 1;
        // printf("NAV_RAM: Storage full! Records: %d\r\n", nav_manager.record_count);
    } else {
        nav_manager.status = NAV_STATUS_RECORDING;
    }
    
    // 定期打印状态
    if (nav_manager.record_count % 100 == 0) {
        float percent = ((float)nav_manager.record_count / NAV_MAX_RECORDS) * 100.0f;
        // printf("NAV_RAM: %d records (%.1f%% used)\r\n", 
        //        nav_manager.record_count, percent);
    }
    
    return 1;
}

/**
 * @brief 获取指定索引的记录点
 * @param index 记录索引(0开始)
 * @param point 返回的记录点指针
 * @return 成功标志: 0=失败, 1=成功
 */
uint8_t NAV_RAM_GetRecord(uint16_t index, NavPoint_t* point) {
    if (index >= nav_manager.record_count || point == NULL) {
        return 0;
    }
    
    // 计算实际缓冲区位置
    uint16_t buffer_index = index;
    if (nav_manager.write_index >= nav_manager.record_count) {
        // 未发生环形回绕
        *point = nav_manager.buffer[index];
    } else {
        // 发生环形回绕，需要调整索引
        buffer_index = (nav_manager.write_index + index) % NAV_MAX_RECORDS;
        *point = nav_manager.buffer[buffer_index];
    }
    
    return 1;
}

/**
 * @brief 获取当前记录数量
 * @return 记录数量
 */
uint16_t NAV_RAM_GetRecordCount(void) {
    return nav_manager.record_count;
}

/**
 * @brief 清空所有记录
 */
void NAV_RAM_ClearRecords(void) {
    nav_manager.write_index = 0;
    nav_manager.read_index = 0;
    nav_manager.record_count = 0;
    nav_manager.status = NAV_STATUS_IDLE;
    nav_manager.overflow_flag = 0;
    
    // printf("NAV_RAM: All records cleared\r\n");
}

/**
 * @brief 获取存储状态
 * @return 当前状态
 */
NavRecordStatus_t NAV_RAM_GetStatus(void) {
    return nav_manager.status;
}

/**
 * @brief 检查存储是否已满
 * @return 已满标志: 0=未满, 1=已满
 */
uint8_t NAV_RAM_IsFull(void) {
    return nav_manager.overflow_flag;
}

/**
 * @brief 获取存储使用百分比
 * @return 使用百分比(0.0-100.0)
 */
float NAV_RAM_GetUsedPercentage(void) {
    return ((float)nav_manager.record_count / NAV_MAX_RECORDS) * 100.0f;
}

/**
 * @brief 获取剩余存储空间(记录条数)
 * @return 剩余空间
 */
uint16_t NAV_RAM_GetFreeSpace(void) {
    return NAV_MAX_RECORDS - nav_manager.record_count;
}