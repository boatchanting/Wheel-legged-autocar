#include "wifi_protocol.h"

// ---------------------------------------------------------
// 内部变量
// ---------------------------------------------------------
static uint8_t tx_buf[WIFI_TX_BUFFER_SIZE];
static uint16_t idx = 0; // 当前缓冲区的写入位置索引

// ---------------------------------------------------------
// 辅助序列化函数 (确保小端模式传输，且无填充)
// ---------------------------------------------------------

// 写入 1 字节
static void write_u8(uint8_t val) {
    if (idx < WIFI_TX_BUFFER_SIZE) tx_buf[idx++] = val;
}

// 写入 1 字节 (从 int8 转换)
static void write_i8(int8_t val) {
    if (idx < WIFI_TX_BUFFER_SIZE) tx_buf[idx++] = (uint8_t)val;
}

// 写入 2 字节 (uint16)
static void write_u16(uint16_t val) {
    if (idx + 2 <= WIFI_TX_BUFFER_SIZE) {
        tx_buf[idx++] = (uint8_t)(val & 0xFF);
        tx_buf[idx++] = (uint8_t)((val >> 8) & 0xFF);
    }
}

// 写入 4 字节 (uint32 或 float)
static void write_u32_or_float(void *val_ptr) {
    if (idx + 4 <= WIFI_TX_BUFFER_SIZE) {
        uint8_t *p = (uint8_t *)val_ptr;
        // ARM Cortex-M 通常是小端模式，直接拷贝即可
        // 如果你的上位机是大端，需要在这里反转顺序，但通常 PC 也是小端
        tx_buf[idx++] = p[0];
        tx_buf[idx++] = p[1];
        tx_buf[idx++] = p[2];
        tx_buf[idx++] = p[3];
    }
}

// 写入 8 字节 (double) - 关键函数
static void write_double(double *val_ptr) {
    if (idx + 8 <= WIFI_TX_BUFFER_SIZE) {
        uint8_t *p = (uint8_t *)val_ptr;
        for (int i = 0; i < 8; i++) {
            tx_buf[idx++] = p[i];
        }
    }
}

// ---------------------------------------------------------
// 核心发送函数
// ---------------------------------------------------------
void wifi_protocol_send_data(void) {
    idx = 0; // 重置缓冲区索引

    // -----------------------------------------------------
    // 1. 帧头与控制信息
    // -----------------------------------------------------
    write_u8(WIFI_FRAME_HEAD1);     // 0x5A
    write_u8(WIFI_FRAME_HEAD2);     // 0xA5
    write_u8(WIFI_CMD_DATA_PACKET);  // 0x01 (功能字)
    
    // 预留长度位 (索引 3)，稍后计算完负载长度再回填
    uint16_t len_pos = idx;
    write_u8(0x00); 

    // -----------------------------------------------------
    // 2. 数据负载 (Payload) - 严格按顺序打包
    // -----------------------------------------------------
    
    // --- A. 系统信息 ---
    write_u32_or_float(&loop_counter);      // 4 bytes

    // --- B. 惯导信息 (InertialNav_t) ---
    write_u32_or_float(&inertial_nav.x);    // 4 bytes (float)
    write_u32_or_float(&inertial_nav.y);    // 4 bytes (float)
    write_u32_or_float(&inertial_nav.vx_body);   // 4 bytes (float)
    write_u32_or_float(&inertial_nav.vy_body);   // 4 bytes (float)
    // 如果需要 vx_body, slip_flag 等，在这里继续添加 write_xx

    // --- C. GNSS 信息 (gnss_info_struct) ---
    // 为了无视结构体填充，我们逐个成员写入

    // C.1 时间 (gps_time_struct) - 共 7 bytes
    write_u16(gnss.time.year);
    write_u8(gnss.time.month);
    write_u8(gnss.time.day);
    write_u8(gnss.time.hour);
    write_u8(gnss.time.minute);
    write_u8(gnss.time.second);

    // C.2 状态
    write_u8(gnss.state);                   // 1 byte

    // C.3 整型经纬度 (用于备份或校验) - 共 12 bytes
    write_u16(gnss.latitude_degree);
    write_u16(gnss.latitude_cent);
    write_u16(gnss.latitude_second);
    write_u16(gnss.longitude_degree);
    write_u16(gnss.longitude_cent);
    write_u16(gnss.longitude_second);

    // C.4 双精度经纬度 (核心数据) - 共 16 bytes
    // 这是一个原子操作，传输内存中真实的 IEEE 754 8字节数据
    write_double(&gnss.latitude);           
    write_double(&gnss.longitude);

    // C.5 半球信息
    write_i8(gnss.ns);                      // 1 byte
    write_i8(gnss.ew);                      // 1 byte

    // C.6 速度与方向
    write_u32_or_float(&gnss.speed);        // 4 bytes (float)
    write_u32_or_float(&gnss.direction);    // 4 bytes (float)

    // C.7 天线信息
    write_u8(gnss.antenna_direction_state);     // 1 byte
    write_u32_or_float(&gnss.antenna_direction);// 4 bytes (float)

    // C.8 其他
    write_u8(gnss.satellite_used);          // 1 byte
    write_u32_or_float(&gnss.height);       // 4 bytes (float)

    // -----------------------------------------------------
    // 3. 计算长度与校验
    // -----------------------------------------------------
    
    // 计算负载长度 (当前索引 - 长度位索引 - 1)
    // 如果负载超过 255 字节，协议需要修改长度位为 uint16，目前看大约 70 字节，足够
    uint8_t payload_len = idx - (len_pos + 1);
    tx_buf[len_pos] = payload_len; // 回填长度

    // 计算校验和 (CheckSum)
    // 简单累加校验：从帧头开始，一直加到负载结束
    uint8_t check_sum = 0;
    for (int i = 0; i < idx; i++) {
        check_sum += tx_buf[i];
    }
    write_u8(check_sum); // 写入校验位

    // 写入帧尾
    write_u8(WIFI_FRAME_TAIL);      // 0xED

    // -----------------------------------------------------
    // 4. 发送数据
    // -----------------------------------------------------
    // 调用 zf_device_wifi_spi.h 中的发送函数
    wifi_spi_send_buffer(tx_buf, idx);
}