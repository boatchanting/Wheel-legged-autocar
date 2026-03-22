# tools/flash 模块文档

## 1. 模块作用

`flash` 模块负责将控制参数（PID 等）在 Flash 中持久化，支持上电读取与运行时保存。  
对应源码：`code/tools/flash.h`、`code/tools/flash.c`。

## 2. 关键接口

- `void param_read_from_flash(void);`
  - 从指定扇区/页读取参数到当前运行变量。
- `void param_save_to_flash(void);`
  - 将当前参数写入 Flash。

## 3. 存储布局（header 中定义）

- 扇区/页：`FLASH_SECTION_INDEX`、`FLASH_PAGE_INDEX`
- 参数索引：
  - 速度环：`IDX_SPD_P/I/D`
  - 角度环：`IDX_ANG_P/I/D`
  - 角速度环：`IDX_GYR_P/I/D`
  - 舵机环：`IDX_SERVO_P/I/D`
- 总参数量：`PARAM_NUM = 12`

## 4. 使用方式

```c
#include "tools/flash.h"

void app_init(void)
{
    param_read_from_flash();   // 启动时恢复参数
}

void on_user_save(void)
{
    param_save_to_flash();     // 用户触发保存
}
```

## 5. 注意事项

- 必须确认扇区/页不覆盖程序区。
- 写 Flash 有擦写次数限制，避免高频调用保存接口。
