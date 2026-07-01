#include "gnss_transform.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define RAD_PER_DEG (M_PI / 180.0f)

// ==================== 全局变量实例 ====================
gnss_transform_struct gnss_trans = {0};

// ==================== 内部静态变量 ====================
static double s_origin_x_abs = 0.0; // 原点高斯 X (北向)
static double s_origin_y_abs = 0.0; // 原点高斯 Y (东向)

// ==================== WGS84 椭球参数 ====================
#define WGS84_A         6378137.0
#define WGS84_F         (1.0 / 298.257223563)
#define WGS84_E2        (2.0 * WGS84_F - WGS84_F * WGS84_F)
#define WGS84_E12       (WGS84_E2 / (1.0 - WGS84_E2))

// ==================== 内部辅助函数 ====================

static double _get_central_meridian(double lon_deg)
{
    int zone = (int)((lon_deg + 1.5) / 3.0);
    return (double)zone * 3.0;
}

static void _gauss_project(double lat, double lon, double *out_x, double *out_y)
{
    double B = lat * RAD_PER_DEG;
    double L = lon * RAD_PER_DEG;
    double L0 = _get_central_meridian(lon) * RAD_PER_DEG;
    double l = L - L0;

    double sinB = sin(B);
    double cosB = cos(B);
    double tanB = tan(B);
    double N = WGS84_A / sqrt(1.0 - WGS84_E2 * sinB * sinB);
    double eta2 = WGS84_E12 * cosB * cosB;

    double m0 = WGS84_A * (1.0 - WGS84_E2 / 4.0 - 3.0 * WGS84_E2 * WGS84_E2 / 64.0);
    double m2 = WGS84_A * (3.0 * WGS84_E2 / 8.0 + 3.0 * WGS84_E2 * WGS84_E2 / 32.0);
    double m4 = WGS84_A * (15.0 * WGS84_E2 * WGS84_E2 / 256.0);
    double m6 = WGS84_A * (35.0 * WGS84_E2 * WGS84_E2 * WGS84_E2 / 3072.0);

    double X = m0 * B - m2 * sin(2.0 * B) + m4 * sin(4.0 * B) - m6 * sin(6.0 * B);

    double t = tanB;
    double l2 = l * l;
    double l4 = l2 * l2;
    double l6 = l4 * l2;

    double x_val = X + N * t * l2 / 2.0
                 + N * t * (5.0 - t * t + 9.0 * eta2 + 4.0 * eta2 * eta2) * l4 / 24.0
                 + N * t * (61.0 - 58.0 * t * t + t * t * t * t) * l6 / 720.0;

    double y_val = N * l
                 + N * (1.0 - t * t + eta2) * l * l2 / 6.0
                 + N * (5.0 - 18.0 * t * t + t * t * t * t + 14.0 * eta2 - 58.0 * eta2 * t * t) * l * l4 / 120.0;

    *out_x = x_val;
    *out_y = y_val;
}

static double _dms_to_decimal(uint16_t deg, uint16_t min, uint16_t sec_x100)
{
    return (double)deg + (double)min / 60.0 + (double)sec_x100 / 100.0 / 3600.0;
}

// ==================== API 接口实现 ====================

void Gnss_Transform_Init(void)
{
    gnss_trans.x = 0.0f;
    gnss_trans.y = 0.0f;
    gnss_trans.ground_x = 0.0f;
    gnss_trans.ground_y = 0.0f;
    gnss_trans.origin_lat = 0.0;
    gnss_trans.origin_lon = 0.0;
    gnss_trans.current_lat = 0.0;
    gnss_trans.current_lon = 0.0;
    gnss_trans.is_origin_set = 0;
    gnss_trans.is_valid = 0;
    gnss_trans.update_count = 0;

    s_origin_x_abs = 0.0;
    s_origin_y_abs = 0.0;
}

void Gnss_Transform_Reset_Origin(void)
{
    gnss_trans.is_origin_set = 0;
    gnss_trans.x = 0.0f;
    gnss_trans.y = 0.0f;
    gnss_trans.ground_x = 0.0f;
    gnss_trans.ground_y = 0.0f;
    s_origin_x_abs = 0.0;
    s_origin_y_abs = 0.0;
}

void Gnss_Transform_SetOriginDirect(double lat, double lon)
{
    _gauss_project(lat, lon, &s_origin_x_abs, &s_origin_y_abs);
    gnss_trans.origin_lat = lat;
    gnss_trans.origin_lon = lon;
    gnss_trans.is_origin_set = 1;
    gnss_trans.x = 0.0f;
    gnss_trans.y = 0.0f;
}

void Gnss_Transform_Update(void)
{
    // 1. 检查 GNSS 数据有效性
    if (gnss.state != 1 || gnss.satellite_used < 4)
    {
        gnss_trans.is_valid = 0;
        return;
    }
    gnss_trans.is_valid = 1;
    gnss_trans.update_count++;

    // 2. 获取经纬度
    double lat = gnss.latitude;
    double lon = gnss.longitude;

    if (lat == 0.0 && lon == 0.0)
    {
        lat = _dms_to_decimal(gnss.latitude_degree, gnss.latitude_cent, gnss.latitude_second);
        lon = _dms_to_decimal(gnss.longitude_degree, gnss.longitude_cent, gnss.longitude_second);
        if (gnss.ns == 'S' || gnss.ns == -1) lat = -lat;
        if (gnss.ew == 'W' || gnss.ew == -1) lon = -lon;
    }

    // 3. 始终更新当前经纬度 (供手动锁定使用)
    gnss_trans.current_lat = lat;
    gnss_trans.current_lon = lon;

    // 4. 高斯投影
    double current_x_abs, current_y_abs;
    _gauss_project(lat, lon, &current_x_abs, &current_y_abs);

    // 5. 仅在原点已锁定时计算相对坐标
    if (!gnss_trans.is_origin_set)
    {
        gnss_trans.x = 0.0f;
        gnss_trans.y = 0.0f;
        return;
    }

    gnss_trans.x = (float)(current_y_abs - s_origin_y_abs); // 东向 (米)
    gnss_trans.y = (float)(current_x_abs - s_origin_x_abs); // 北向 (米)

    // 6. ground_x/ground_y 由 Fusion 模块通过 Gnss_Transform_ComputeGround() 计算
}
