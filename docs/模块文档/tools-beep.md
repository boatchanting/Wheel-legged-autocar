# tools/beep 模块文档

## 1. 模块作用

`beep` 模块用于蜂鸣器硬件初始化与鸣叫控制，适合做开机提示、状态告警。  
对应源码：`code/tools/beep.h`、`code/tools/beep.c`。

## 2. 关键接口

- `void beep_init(void);`
  - 初始化蜂鸣器 GPIO。

## 3. 关键宏

- `BUZZER_PIN`：蜂鸣器引脚，当前定义为 `P19_4`。

## 4. 使用方式

```c
#include "tools/beep.h"

int main(void)
{
    beep_init();
    // 后续可通过底层 gpio 控制蜂鸣器电平
}
```

## 5. 注意事项

- 该模块本身仅负责初始化，具体“鸣叫节奏”通常在上层状态机或定时任务里实现。
- 如更换主板引脚，请同步修改 `BUZZER_PIN` 宏。
