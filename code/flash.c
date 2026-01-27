#include "zf_common_headfile.h" 
 
// ==========================================
//函数实现
// ==========================================

void param_read_from_flash(void)
{
    // 1. 从 Flash 读取数据到缓冲区
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX, PARAM_NUM);

    // 2. 判断 Flash 是否为空
    // 判断方法：检查第一个参数是否为 0.0 (Flash 擦除后通常数据不可用或为特定值)
    // 注意：浮点数判等通常需要 epsilon，但在检测“是否从未写入过”这种粗略场景下，直接判 0.0f 是常用的工程手段
    // --- 速度环 (Speed Loop) ---
    // 逻辑：如果 Flash 中的值不等于 0.0f，则使用 Flash 的值，否则使用 SPD_KP
    pid_speed.kp = (flash_union_buffer[IDX_SPD_P].float_type != 0.0f) ? flash_union_buffer[IDX_SPD_P].float_type : SPD_KP;
    pid_speed.ki = (flash_union_buffer[IDX_SPD_I].float_type != 0.0f) ? flash_union_buffer[IDX_SPD_I].float_type : SPD_KI;
    pid_speed.kd = (flash_union_buffer[IDX_SPD_D].float_type != 0.0f) ? flash_union_buffer[IDX_SPD_D].float_type : SPD_KD;

    // --- 角度环 (Angle Loop) ---
    pid_angle.kp = (flash_union_buffer[IDX_ANG_P].float_type != 0.0f) ? flash_union_buffer[IDX_ANG_P].float_type : ANG_KP;
    pid_angle.ki = (flash_union_buffer[IDX_ANG_I].float_type != 0.0f) ? flash_union_buffer[IDX_ANG_I].float_type : ANG_KI;
    pid_angle.kd = (flash_union_buffer[IDX_ANG_D].float_type != 0.0f) ? flash_union_buffer[IDX_ANG_D].float_type : ANG_KD;

    // --- 角速度环 (Gyro Loop) ---
    pid_gyro.kp  = (flash_union_buffer[IDX_GYR_P].float_type != 0.0f) ? flash_union_buffer[IDX_GYR_P].float_type : GYR_KP;
    pid_gyro.ki  = (flash_union_buffer[IDX_GYR_I].float_type != 0.0f) ? flash_union_buffer[IDX_GYR_I].float_type : GYR_KI;
    pid_gyro.kd  = (flash_union_buffer[IDX_GYR_D].float_type != 0.0f) ? flash_union_buffer[IDX_GYR_D].float_type : GYR_KD;
}

// 将当前参数保存到 Flash (Save)
void param_save_to_flash(void)
{
    // 1. 擦除页 (Flash 特性：必须先擦除才能写入)
    if(flash_check(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX))
    {
        flash_erase_page(FLASH_SECTION_INDEX,FLASH_PAGE_INDEX);
    }
    
    // 2. 清空缓冲区，防止残留垃圾数据
    flash_buffer_clear();

    // 3. 将当前的全局变量填入缓冲区
    flash_union_buffer[IDX_SPD_P].float_type = pid_speed.kp;
    flash_union_buffer[IDX_SPD_I].float_type = pid_speed.ki;
    flash_union_buffer[IDX_SPD_D].float_type = pid_speed.kd;

    flash_union_buffer[IDX_ANG_P].float_type = pid_angle.kp;
    flash_union_buffer[IDX_ANG_I].float_type = pid_angle.ki;
    flash_union_buffer[IDX_ANG_D].float_type = pid_angle.kd;

    flash_union_buffer[IDX_GYR_P].float_type = pid_gyro.kp;
    flash_union_buffer[IDX_GYR_I].float_type = pid_gyro.ki;
    flash_union_buffer[IDX_GYR_D].float_type = pid_gyro.kd;

    // 4. 写入 Flash
    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX, PARAM_NUM);
    
    printf("Parameters Saved to Flash.\r\n");
}