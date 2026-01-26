```c
/*********************************************************************************************************************
* CYT4BB Opensourec Library 即（ CYT4BB 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 CYT4BB 开源库的一部分
*
* CYT4BB 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          main_cm7_0
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT4BB
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-4       pudding            first version
********************************************************************************************************************/

#include "zf_common_headfile.h"

// 打开新的工程或者工程移动了位置务必执行以下操作
// 第一步 关闭上面所有打开的文件
// 第二步 project->clean  等待下方进度条走完

// *************************** 例程硬件连接说明 ***************************
// 使用逐飞科技 CMSIS-DAP 调试下载器连接
//      直接将下载器正确连接在核心板的调试下载接口即可
// 使用 USB-TTL 模块连接
//      模块管脚            单片机管脚
//      USB-TTL-RX          查看 zf_common_debug.h 文件中 DEBUG_UART_TX_PIN 宏定义的引脚 默认 P14_0
//      USB-TTL-TX          查看 zf_common_debug.h 文件中 DEBUG_UART_RX_PIN 宏定义的引脚 默认 P14_1
//      USB-TTL-GND         核心板电源地 GND
//      USB-TTL-3V3         核心板 3V3 电源


// *************************** 例程测试说明 ***************************
// 1.核心板烧录完成本例程，单独使用核心板与调试下载器或者 USB-TTL 模块，在断电情况下完成连接
// 2.将调试下载器或者 USB-TTL 模块连接电脑，完成上电
// 3.电脑上使用串口助手打开对应的串口，串口波特率为 zf_common_debug.h 文件中 DEBUG_UART_BAUDRATE 宏定义 默认 115200，核心板按下复位按键
// 4.可以在串口助手上看到如下串口信息：
//      FLASH_SECTION_INDEX: 127, FLASH_PAGE_INDEX: 3, origin data is :
//      ...
// 如果发现现象与说明严重不符 请参照本文件最下方 例程常见问题说明 进行排查

// **************************** 代码区域 ****************************

#define FLASH_SECTION_INDEX       (0)                                 // 存储数据用的扇区
#define FLASH_PAGE_INDEX          (0)                                // 存储数据用的页码 倒数第一个页码

int main(void)
{
    clock_init(SYSTEM_CLOCK_250M); 	// 时钟配置及系统初始化<务必保留>
    debug_init();                   // 调试串口信息初始化
    // 此处编写用户代码 例如外设初始化代码等

    flash_init();                                                               // 使用flash前先调用flash初始化
    
    if(flash_check(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX))                      // 判断是否有数据
        flash_erase_page(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);                // 擦除这一页

    printf("\r\n");
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX, 7);        // 将数据从 flash 读取到缓冲区
    printf("\r\n FLASH_SECTION_INDEX: %d, FLASH_PAGE_INDEX: %d, origin data is :", FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    printf("\r\n float_type : %f", flash_union_buffer[0].float_type);           // 将缓冲区第 0 个位置的数据以 float  格式输出
    printf("\r\n uint32_type: %d", flash_union_buffer[1].uint32_type);          // 将缓冲区第 1 个位置的数据以 uint32 格式输出
    printf("\r\n int32_type : %d", flash_union_buffer[2].int32_type);           // 将缓冲区第 2 个位置的数据以 int32  格式输出
    printf("\r\n uint16_type: %u", flash_union_buffer[3].uint16_type);          // 将缓冲区第 3 个位置的数据以 uint16 格式输出
    printf("\r\n int16_type : %d", flash_union_buffer[4].int16_type);           // 将缓冲区第 4 个位置的数据以 int16  格式输出
    printf("\r\n uint8_type : %u", flash_union_buffer[5].uint8_type);           // 将缓冲区第 5 个位置的数据以 uint8  格式输出
    printf("\r\n int8_type  : %d", flash_union_buffer[6].int8_type);            // 将缓冲区第 6 个位置的数据以 int8   格式输出
    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据
    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据
    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据

    // 例如 flash_data_union_buffer[0] 写入 int8_type 那么只能以 int8_type 读取
    // 同样 flash_data_union_buffer[0] 写入 float_type 那么只能以 float_type 读取
    printf("\r\n");
    printf("\r\n Data property display :");
    printf("\r\n flash_data_union_buffer[0] write int8 data type:");
    flash_union_buffer[0].int8_type   = -128;                                   // 向缓冲区第 0 个位置写入     int8   数据
    printf("\r\n float_type : %f-data error", flash_union_buffer[0].float_type);// 将缓冲区第 0 个位置的数据以  float  格式输出 数据将不正确
    printf("\r\n int8_type  : %d", flash_union_buffer[0].int8_type);            // 将缓冲区第 0 个位置的数据以  int8   格式输出 得到正确写入数据

    printf("\r\n flash_data_union_buffer[0] write int8 data type:");
    flash_union_buffer[0].int8_type  -= 1;                                      // 向缓冲区第 0 个位置写入     int8   数据
    printf("\r\n float_type : %f-data error", flash_union_buffer[0].float_type);// 将缓冲区第 0 个位置的数据以  float  格式输出 数据将不正确
    printf("\r\n int8_type  : %d", flash_union_buffer[0].int8_type);            // 将缓冲区第 0 个位置的数据以  int8   格式输出 得到正确写入数据

    printf("\r\n flash_data_union_buffer[0] write float data type:");
    flash_union_buffer[0].float_type  = 16.625;                                 // 向缓冲区第 0 个位置写入     float  数据
    printf("\r\n float_type : %f", flash_union_buffer[0].float_type);           // 将缓冲区第 0 个位置的数据以  float  格式输出 得到正确写入数据
    printf("\r\n int8_type  : %d-data error", flash_union_buffer[0].int8_type); // 将缓冲区第 0 个位置的数据以  int8   格式输出 数据将不正确

    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据
    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据
    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据

    flash_buffer_clear();                                                       // 清空缓冲区
    flash_union_buffer[0].float_type  = 3.1415926;                              // 向缓冲区第 0 个位置写入 float  数据
    flash_union_buffer[1].uint32_type = 4294967294;                             // 向缓冲区第 1 个位置写入 uint32 数据
    flash_union_buffer[2].int32_type  = -2147483648;                            // 向缓冲区第 2 个位置写入 int32  数据
    flash_union_buffer[3].uint16_type = 65535;                                  // 向缓冲区第 3 个位置写入 uint16 数据
    flash_union_buffer[4].int16_type  = -32768;                                 // 向缓冲区第 4 个位置写入 int16  数据
    flash_union_buffer[5].uint8_type  = 255;                                    // 向缓冲区第 5 个位置写入 uint8  数据
    flash_union_buffer[6].int8_type   = -128;                                   // 向缓冲区第 6 个位置写入 int8   数据
    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX, 7);     // 向指定 Flash 扇区的页码写入缓冲区数据

    printf("\r\n");
    flash_buffer_clear();                                                       // 清空缓冲区
    printf("\r\n Flash data buffer default data is :");
    printf("\r\n float_type : %f", flash_union_buffer[0].float_type);           // 将缓冲区第 0 个位置的数据以 float  格式输出
    printf("\r\n uint32_type: %d", flash_union_buffer[1].uint32_type);          // 将缓冲区第 1 个位置的数据以 uint32 格式输出
    printf("\r\n int32_type : %d", flash_union_buffer[2].int32_type);           // 将缓冲区第 2 个位置的数据以 int32  格式输出
    printf("\r\n uint16_type: %u", flash_union_buffer[3].uint16_type);          // 将缓冲区第 3 个位置的数据以 uint16 格式输出
    printf("\r\n int16_type : %d", flash_union_buffer[4].int16_type);           // 将缓冲区第 4 个位置的数据以 int16  格式输出
    printf("\r\n uint8_type : %u", flash_union_buffer[5].uint8_type);           // 将缓冲区第 5 个位置的数据以 uint8  格式输出
    printf("\r\n int8_type  : %d", flash_union_buffer[6].int8_type);            // 将缓冲区第 6 个位置的数据以 int8   格式输出
    system_delay_ms(200);
    printf("\r\n");
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX, 7);        // 将数据从 flash 读取到缓冲区
    printf("\r\n FLASH_SECTION_INDEX: %d, FLASH_PAGE_INDEX: %d, new data is :", FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    printf("\r\n float_type : %f", flash_union_buffer[0].float_type);           // 将缓冲区第 0 个位置的数据以 float  格式输出
    printf("\r\n uint32_type: %d", flash_union_buffer[1].uint32_type);          // 将缓冲区第 1 个位置的数据以 uint32 格式输出
    printf("\r\n int32_type : %d", flash_union_buffer[2].int32_type);           // 将缓冲区第 2 个位置的数据以 int32  格式输出
    printf("\r\n uint16_type: %u", flash_union_buffer[3].uint16_type);          // 将缓冲区第 3 个位置的数据以 uint16 格式输出
    printf("\r\n int16_type : %d", flash_union_buffer[4].int16_type);           // 将缓冲区第 4 个位置的数据以 int16  格式输出
    printf("\r\n uint8_type : %u", flash_union_buffer[5].uint8_type);           // 将缓冲区第 5 个位置的数据以 uint8  格式输出
    printf("\r\n int8_type  : %d", flash_union_buffer[6].int8_type);            // 将缓冲区第 6 个位置的数据以 int8   格式输出
    
    
    // 此处编写用户代码 例如外设初始化代码等
    while(true)
    {
        // 此处编写需要循环执行的代码


      
        
        // 此处编写需要循环执行的代码
    }
}

// **************************** 代码区域 ****************************

// *************************** 例程常见问题说明 ***************************
// 遇到问题时请按照以下问题检查列表检查
// 问题1：串口没有数据
//      查看串口助手打开的是否是正确的串口，检查打开的 COM 口是否对应的是调试下载器或者 USB-TTL 模块的 COM 口
//      如果是使用逐飞科技 CMSIS-DAP 调试下载器连接，那么检查下载器线是否松动，检查核心板串口跳线是否已经焊接，串口跳线查看核心板原理图即可找到
//      如果是使用 USB-TTL 模块连接，那么检查连线是否正常是否松动，模块 TX 是否连接的核心板的 RX，模块 RX 是否连接的核心板的 TX
//      如果上述检查都正常，但依旧没有输出数据，就进入调试看看是否进入断言，如果进入断言就更换扇区和页码尝试，如果依旧不行，那么联系技术支持
// 问题2：串口数据乱码
//      查看串口助手设置的波特率是否与程序设置一致，程序中 zf_common_debug.h 文件中 DEBUG_UART_BAUDRATE 宏定义为 debug uart 使用的串口波特率
// 问题3：数据不对
//      更换扇区和页码尝试，如果数据依旧读出不正确，联系技术支持

...

这段代码是一个 **Flash（闪存）读写测试例程**。

它的核心功能是：**演示如何将数据（如 PID 参数、传感器校准值）写入单片机的 Flash 存储区，并在断电重启后读取出来。**

在智能车开发中，这是**“参数保存”**功能的底层实现。如果没有这个功能，你每次给车换电池或重启，辛苦调好的 PID 参数就会丢失，必须重新烧录或重新输入。

下面我为你详细拆解代码的逻辑和涉及的关键知识点：

### 1. 核心概念：RAM vs Flash

*   **RAM (随机存取存储器)**：代码中定义的普通变量（如 `int a = 0;`）都存在 RAM 里。**特点是读写快，但掉电数据丢失。**
*   **Flash (闪存)**：用来存储程序代码的地方。逐飞库允许我们划出一小块区域专门存数据。**特点是掉电数据不丢失（像硬盘一样），但写入速度慢，且有寿命限制。**

### 2. 代码逻辑拆解

#### A. 配置存储位置
```c
#define FLASH_SECTION_INDEX  (0) // 扇区索引
#define FLASH_PAGE_INDEX     (0) // 页索引
```
*   这定义了数据要把存到 Flash 的哪个“房间”里。
*   **注意**：在实际项目中，不要随意修改这两个值，以免覆盖掉单片机里的程序代码（把自己“删”了）。通常库文件会指定安全的区域。

#### B. 擦除旧数据 (Erase)
```c
flash_init(); // 初始化驱动

if(flash_check(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX))
    flash_erase_page(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
```
*   **原理**：Flash 存储器有一个特性，**只能把 1 写成 0，不能把 0 写成 1**。
*   **必须擦除**：要想写入新数据，必须先“擦除”这一页（将所有位变为 1）。这就好比黑板写满了，必须先擦干净才能写新字。

#### C. 数据缓冲区与联合体 (`flash_union_buffer`) —— 代码的核心黑科技
代码中使用了 `flash_union_buffer`，这是一个 **联合体 (Union)** 数组。
*   **什么是联合体？**：它是一块共用的内存空间。你可以把它当成 `float` 用，也可以当成 `int` 用。
*   **代码演示的“类型错误”**：
    ```c
    flash_union_buffer[0].int8_type = -128; // 写入整数 -128
    printf("... %f ...", flash_union_buffer[0].float_type); // 尝试用浮点数读出来
    ```
    *   代码故意演示了这一点：**你存进去是什么类型，读出来就必须是什么类型**。如果你存了整数，强行按浮点数读，结果就是乱码。

#### D. 写入数据 (Save)
```c
flash_buffer_clear(); // 1. 清空内存里的缓冲区（防止有垃圾数据）

// 2. 填入各种类型的数据（模拟你的PID参数、目标速度等）
flash_union_buffer[0].float_type  = 3.1415926;
flash_union_buffer[1].uint32_type = 4294967294;
// ...

// 3. 将缓冲区的数据真正“烧写”进 Flash 芯片
flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX, 7);
```

#### E. 读取与验证 (Load)
```c
flash_buffer_clear(); // 1. 先清空内存，假装我们什么都不知道

// 2. 从 Flash 芯片读取数据到内存
flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX, 7);

// 3. 打印验证
printf("... %f ...", flash_union_buffer[0].float_type); // 应该输出 3.141593
```

---

### 3. 如何在智能车项目中使用？

这个例程是你实现 **“参数掉电保存”** 功能的基础。在实际比赛代码中，你会封装成两个函数：

#### 伪代码示例：

```c
// 假设用来存直立环 PID
#define IDX_KP 0
#define IDX_KI 1
#define IDX_KD 2

// 【开机时调用】读取参数
void Load_PID_Params() {
    flash_read_page_to_buffer(0, 0, 10);
    
    // 检查是不是第一次运行（空数据）
    if(flash_union_buffer[IDX_KP].float_type == 0 && flash_union_buffer[IDX_KI].float_type == 0) {
        //如果是空的，赋默认值
        Kp = 10.0; Ki = 0.0; Kd = 0.5;
    } else {
        // 如果有数据，读出来
        Kp = flash_union_buffer[IDX_KP].float_type;
        Ki = flash_union_buffer[IDX_KI].float_type;
        Kd = flash_union_buffer[IDX_KD].float_type;
    }
}

// 【收到串口指令或按键确认后调用】保存参数
void Save_PID_Params() {
    flash_erase_page(0, 0); // 必须先擦除！
    
    flash_union_buffer[IDX_KP].float_type = Kp;
    flash_union_buffer[IDX_KI].float_type = Ki;
    flash_union_buffer[IDX_KD].float_type = Kd;
    
    flash_write_page_from_buffer(0, 0, 3);
}
```

### 4. ⚠️ 致命注意事项

1.  **寿命限制**：Flash 的擦写次数是有限的（通常是 1万次到 10万次）。
    *   **绝对禁止**：把 `flash_write` 放在 `while(true)` 循环里或者定时器中断里。那样几分钟你的芯片就废了。
    *   **正确做法**：只有在参数修改并确认保存时，才调用一次写入函数。
2.  **写前必擦**：调用 `write` 之前，务必确保该页已经被 `erase` 过。
3.  **不要在这个例程的 `while(true)` 里写代码**：因为 Flash 操作耗时较长（毫秒级），且通常会暂停 CPU 的其他操作，不适合在车辆高速行驶时频繁操作。

### 总结
这段代码演示了**单片机的“存盘”功能**。它是智能车调试阶段的神器，让你不必每次上电都重新输入参数。