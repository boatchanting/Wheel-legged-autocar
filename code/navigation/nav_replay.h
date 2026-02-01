#ifndef _NAV_REPLAY_H_
#define _NAV_REPLAY_H_

#include "zf_common_headfile.h"
#include "nav_ram.h"      // 复用 NavPoint_t 定义
#include "ram2flash.h"    // 调用 Flash 读取接口

//-------------------------------------------------------------------------------------------------------------------
//  @brief      复现控制参数配置
//-------------------------------------------------------------------------------------------------------------------

// 速度设定 (负数为向前)
#define REPLAY_SPEED_STRAIGHT     -150.0f   // 直线段全速
#define REPLAY_SPEED_CURVE        -80.0f   // 曲线段速度
#define REPLAY_SPEED_STOP         0.0f     // 停止

// 路径点切换阈值
#define REPLAY_WAYPOINT_DIST_MM   200.0f   // 距离目标点小于此距离(mm)时，切换到下一个点
#define REPLAY_FINISH_DIST_MM     100.0f   // 距离终点小于此距离时，认为复现完成

// 最大支持的复现节点数 (取决于RAM大小，这里假设最大2000个关键点)
// 注意：这比原始记录的点少得多，因为直线被压缩了
#define REPLAY_MAX_NODES          2000

//-------------------------------------------------------------------------------------------------------------------
//  @brief      数据结构定义
//-------------------------------------------------------------------------------------------------------------------

// 复现状态
typedef enum {
    REPLAY_STATE_IDLE,        // 空闲
    REPLAY_STATE_LOADING,     // 正在从Flash加载
    REPLAY_STATE_READY,       // 加载完成，等待开始
    REPLAY_STATE_RUNNING,     // 正在复现
    REPLAY_STATE_FINISHED,    // 复现完成
    REPLAY_STATE_ERROR        // 错误
} ReplayState_t;

// 复现节点类型
typedef enum {
    NODE_TYPE_CURVE = 0,
    NODE_TYPE_LINE  = 1
} NodeType_t;

// 复现路径节点 (RAM中存储的精简结构)
typedef struct {
    float x;            // 目标点X
    float y;            // 目标点Y
    uint8_t type;       // 节点类型 (决定速度)
} ReplayNode_t;

// 复现管理器
typedef struct {
    ReplayState_t status;       // 当前状态
    uint16_t total_nodes;       // 总节点数
    uint16_t current_index;     // 当前目标节点索引
    uint8_t  data_valid_flag;   // 数据加载成功标志
} ReplayManager_t;

// 引用外部变量
extern volatile float target_speed_set;
extern volatile float err_degree;

// 全局管理器声明
extern ReplayManager_t replay_manager;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      函数声明
//-------------------------------------------------------------------------------------------------------------------

void NAV_REPLAY_Init(void);
uint8_t NAV_REPLAY_LoadFromFlash(void);
void NAV_REPLAY_Start(void);
void NAV_REPLAY_Stop(void);
void NAV_REPLAY_Task(void); // 放在定时中断或主循环中调用

#endif // _NAV_REPLAY_H_