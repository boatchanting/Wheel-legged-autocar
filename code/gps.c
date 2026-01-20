/*********************************************************************************************************************
 * GPS 地图记录模块 - 实现文件
 * 
 * 功能描述：
 *   实现HDOP自适应卡尔曼滤波、坐标转换、滑动窗口和地图记录功能
 *
 * 作者：自动驾驶车队
 * 日期：2026年1月
*********************************************************************************************************************/

#include "gps.h"
#include "zf_common_headfile.h"

// ================= 常数定义 =================
#define EARTH_R 6378137.0        // 地球半径 (米)
#define DEG2RAD 0.01745329252    // 角度转弧度系数

// ================= 全局变量定义 =================
// 坐标系原点 (参考点的经纬度，所有坐标相对此点)
double map_lat0 = 0, map_lon0 = 0;

// GPS地图点数组
MapPoint map[100];
int point_cnt = 0;

// 卡尔曼滤波器 (分别用于纬度和经度)
Kalman kf_lat, kf_lon;
uint8 kalman_ready = 0;

// 滑动窗口 (用于稳定性检测)
static SlidingWindow sw;

// 稳定计数器 (用于防抖动)
static int stable_counter = 0;

// ================= 卡尔曼滤波实现 =================

/**
 * @brief 初始化卡尔曼滤波器
 * 
 * 参数选择建议：
 * - q值小(如1e-6)：表示对模型的信任度高，输出会更平滑
 * - base_r值小(如1e-4)：表示对测量值的信任度高，响应会更灵敏
 */
void kalman_init(Kalman *k, double q, double base_r, double init)
{
    k->q = q;               // 过程噪声
    k->base_r = base_r;     // 保存基准测量噪声
    k->r = base_r;          // 初始化为基准值
    k->p = 1.0;             // 初始协方差，取值较大表示初始不确定性高
    k->x = init;            // 初始状态值
}

/**
 * @brief 卡尔曼滤波更新 - 核心算法
 * 
 * 工作流程：
 * 1. 根据HDOP值动态调整测量噪声R（HDOP^2倍关系）
 * 2. 预测步骤：更新协方差 p = p + q
 * 3. 计算卡尔曼增益：k = p / (p + r)
 *    - 当r大(信号差)时，k小，更相信模型预测
 *    - 当r小(信号好)时，k大，更相信测量值
 * 4. 更新状态：x = x + k * (测量值 - 估计值)
 * 5. 更新协方差：p = (1 - k) * p
 */
double kalman_update(Kalman *k, double z, float hdop)
{
    // ===== 安全检查 =====
    // 防止 hdop 为0或异常值
    if(hdop < 0.5f) hdop = 1.0f;      // 最小值
    if(hdop > 10.0f) hdop = 10.0f;    // 最大值

    // ===== 动态调整测量噪声 =====
    // 原理：HDOP越大表示信号越差，越应该降低对测量值的信信任度
    // 使用平方关系，让惩罚更明显
    double scale = hdop * hdop;
    k->r = k->base_r * scale;

    // ===== 标准卡尔曼滤波步骤 =====
    // 1. 预测：更新估计的不确定性(协方差增加)
    k->p += k->q;
    
    // 2. 计算卡尔曼增益
    k->k = k->p / (k->p + k->r);
    
    // 3. 更新状态估计
    k->x += k->k * (z - k->x);
    
    // 4. 更新估计的不确定性(协方差减少)
    k->p = (1.0 - k->k) * k->p;
    
    return k->x;
}

// ================= 坐标转换实现 =================

/**
 * @brief GPS经纬度转换为平面直角坐标
 * 
 * 使用简化的墨卡托投影，适用于小范围区域
 * 
 * 公式：
 *   Y = (lat - lat0) * DEG2RAD * R   (南北方向)
 *   X = (lon - lon0) * DEG2RAD * R * cos(lat0)   (东西方向)
 * 
 * 坐标系定义：
 *   原点：(map_lat0, map_lon0)
 *   X轴：东方为正
 *   Y轴：北方为正
 */
void gps_to_plane(double lat, double lon, float *x, float *y)
{
    // 计算纬度差异导致的Y坐标差(南北方向，单位：米)
    *y = (float)((lat - map_lat0) * DEG2RAD * EARTH_R);
    
    // 计算经度差异导致的X坐标差
    // 需要乘以cos(纬度)来补偿地球曲率
    *x = (float)((lon - map_lon0) * DEG2RAD * EARTH_R * cos(map_lat0 * DEG2RAD));
}

// ================= 滑动窗口实现 =================

/**
 * @brief 向滑动窗口中添加一个数据点
 * 
 * 使用环形缓冲区实现，自动覆盖最旧的数据
 * 
 * 工作原理：
 *   - head指针循环移动 (0 -> 1 -> ... -> WINDOW_SIZE-1 -> 0)
 *   - count记录当前有效数据个数(从0增长到WINDOW_SIZE)
 */
void win_push(double lat, double lon)
{
    // 移动队列头指针 (环形)
    sw.head = (sw.head + 1) % WINDOW_SIZE;
    
    // 存储数据
    sw.lat[sw.head] = lat;
    sw.lon[sw.head] = lon;
    
    // 更新有效数据计数
    if(sw.count < WINDOW_SIZE)
        sw.count++;
}

/**
 * @brief 检测滑动窗口内的数据是否稳定
 * 
 * 稳定性判断标准：
 * 1. 窗口已满(count == WINDOW_SIZE)
 * 2. 所有数据点转换为平面坐标后，范围都在STABLE_RADIUS内
 *    即：max_x - min_x <= STABLE_RADIUS 且 max_y - min_y <= STABLE_RADIUS
 * 
 * @return 1表示稳定，0表示不稳定
 * @note 返回时同时输出窗口内所有点的平均坐标
 */
int win_check_stable(double *out_avg_lat, double *out_avg_lon)
{
    // 检查窗口是否已满
    if(sw.count < WINDOW_SIZE)
        return 0;

    // ===== 扫描所有窗口内的点，找出范围 =====
    float min_x = 1e9, max_x = -1e9;
    float min_y = 1e9, max_y = -1e9;
    double sum_lat = 0, sum_lon = 0;
    
    for(int i = 0; i < WINDOW_SIZE; i++) {
        // 将GPS坐标转换为平面坐标
        float tx, ty;
        gps_to_plane(sw.lat[i], sw.lon[i], &tx, &ty);
        
        // 更新范围
        if(tx < min_x) min_x = tx;
        if(tx > max_x) max_x = tx;
        if(ty < min_y) min_y = ty;
        if(ty > max_y) max_y = ty;
        
        // 累加以计算平均值
        sum_lat += sw.lat[i];
        sum_lon += sw.lon[i];
    }

    // ===== 稳定性判定 =====
    // 如果范围超过阈值，则判定为不稳定
    if((max_x - min_x) > STABLE_RADIUS || (max_y - min_y) > STABLE_RADIUS)
        return 0;

    // ===== 返回平均坐标 =====
    *out_avg_lat = sum_lat / WINDOW_SIZE;
    *out_avg_lon = sum_lon / WINDOW_SIZE;
    
    return 1;  // 稳定
}

/**
 * @brief 初始化GPS模块
 * 
 * 重置滑动窗口状态
 */
void gps_init(void)
{
    sw.head = -1;
    sw.count = 0;
    stable_counter = 0;
}

// ================= GPS数据处理主函数 =================

/**
 * @brief GPS数据处理核心函数
 * 
 * 处理GNSS接收到的数据，进行以下操作：
 * 1. 计算HDOP值
 * 2. 初始化卡尔曼滤波器
 * 3. 进行HDOP自适应滤波
 * 4. 滑动窗口稳定性检测
 * 5. 原点校准
 * 6. GPS点记录
 * 
 * @return 1表示有新点被记录，0表示没有
 */
int gps_process(void)
{
    // 检查GNSS标志
    if (!gnss_flag)
        return 0;

    // ===== 1. 计算模拟HDOP值 =====
    // 根据卫星数量计算信号质量
    float fake_hdop = (gnss.satellite_used > 12) ? 0.8f : (15.0f - gnss.satellite_used) * 0.3f;
    if(fake_hdop < 0.8f) fake_hdop = 0.8f;
    
    // 清除GNSS标志，允许接收下一组数据
    gnss_flag = 0;
    
    // 解析GNSS数据 (提取经纬度、卫星数等)
    gnss_data_parse();

    // ===== 2. 数据有效性检查 =====
    // 跳过无效数据 (纬度接近0表示还未定位)
    if(fabs(gnss.latitude) < 0.001)
        return 0;

    // ===== 3. 卡尔曼滤波器初始化 =====
    // 第一次收到有效数据时初始化
    if (!kalman_ready) {
        // Q值：过程噪声 = 1e-6
        //   表示对GPS轨迹平滑性的信任度，越小越平滑
        // Base_R值：基准测量噪声 = 1e-4
        //   会根据HDOP值动态调整，HDOP好时R变小，HDOP差时R变大
        kalman_init(&kf_lat, 1e-6, 1e-4, gnss.latitude);
        kalman_init(&kf_lon, 1e-6, 1e-4, gnss.longitude);
        kalman_ready = 1;
        
        // 设置坐标系原点为第一个GPS点
        map_lat0 = gnss.latitude;
        map_lon0 = gnss.longitude;
        
        printf("[INIT] Kalman filter initialized.\r\n");
        return 0;  // 跳过第一个数据点
    }

    // ===== 4. HDOP自适应卡尔曼滤波 =====
    // 根据HDOP值动态调整滤波参数
    double f_lat = kalman_update(&kf_lat, gnss.latitude, fake_hdop);
    double f_lon = kalman_update(&kf_lon, gnss.longitude, fake_hdop);
    
    // (可选) 调试打印：观察R值是否随HDOP变化
    // if(point_cnt < 5)
    //     printf("HDOP: %.2f, R: %.6f, Sat: %d\r\n", fake_hdop, kf_lat.r, gnss.satellite_used);

    // ===== 5. 将滤波后的数据推入滑动窗口 =====
    // 用于稳定性检测
    win_push(f_lat, f_lon);

    // ===== 6. 原点校准逻辑 =====
    // 等待第一个点的数据稳定后，用平均值更新原点
    if(point_cnt == 0) {
        double avg_lat, avg_lon;
        
        // 检查滑动窗口内的数据是否稳定
        if(win_check_stable(&avg_lat, &avg_lon)) {
            // 更新坐标系原点为稳定位置的平均值
            map_lat0 = avg_lat;
            map_lon0 = avg_lon;
            
            // 记录第一个点 (原点)
            map[0].lat = avg_lat;
            map[0].lon = avg_lon;
            map[0].x = 0.0f;
            map[0].y = 0.0f;
            point_cnt = 1;
            
            printf("[ORIGIN] Fixed at Lat:%.8f Lon:%.8f (HDOP:%.1f)\r\n",
                   avg_lat, avg_lon, fake_hdop);
        }
        return 0;  // 继续等待稳定
    }

    // ===== 7. 后续点记录逻辑 =====
    // 实现防抖动：需要连续5次稳定才记录一个新点
    
    double avg_lat, avg_lon;
    
    if (win_check_stable(&avg_lat, &avg_lon)) {
        // 数据稳定，计数器加1
        stable_counter++;
    } else {
        // 数据不稳定，重置计数器
        stable_counter = 0;
    }

    // 当稳定计数足够时，检查距离条件
    if (stable_counter > 5) {
        // 转换为平面坐标
        float curr_x, curr_y;
        gps_to_plane(avg_lat, avg_lon, &curr_x, &curr_y);

        // 计算与前一点的距离
        float dx = curr_x - map[point_cnt - 1].x;
        float dy = curr_y - map[point_cnt - 1].y;
        float dist = sqrtf(dx * dx + dy * dy);

        // 距离超过最小值时记录新点
        if (dist > MIN_RECORD_DIST) {
            // 保存点的信息
            map[point_cnt].lat = avg_lat;
            map[point_cnt].lon = avg_lon;
            map[point_cnt].x = curr_x;
            map[point_cnt].y = curr_y;
            
            // 输出调试信息
            printf("[SAVED] P%d: X=%.2f, Y=%.2f, Dist=%.2f (Sat:%d, HDOP:%.1f)\r\n",
                   point_cnt, curr_x, curr_y, dist, gnss.satellite_used, fake_hdop);
            
            // 更新点计数
            point_cnt++;
            if(point_cnt >= 100)  // 数组最多100个点
                point_cnt = 99;
            
            // 重置稳定计数，等待下一个新点
            stable_counter = 0;
            
            return 1;  // 有新点被记录
        }
    }
    
    return 0;  // 没有新点
}

// ================= 调试输出函数 =================

/**
 * @brief 输出当前记录的所有GPS地图点
 * 
 * 用于调试，显示已记录的点的坐标信息
 */
void gps_debug_print_map(void)
{
    printf("\n========== GPS Map Points ==========\r\n");
    printf("Total: %d points\r\n", point_cnt);
    printf("Origin: Lat:%.8f Lon:%.8f\r\n", map_lat0, map_lon0);
    printf("----- Map Data -----\r\n");
    
    for(int i = 0; i < point_cnt; i++) {
        printf("P%d: X=%.2f, Y=%.2f | Lat=%.8f, Lon=%.8f\r\n",
               i,
               map[i].x, map[i].y,
               map[i].lat, map[i].lon);
    }
    printf("====================================\r\n\n");
}
