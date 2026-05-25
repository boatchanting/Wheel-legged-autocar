---
name: iar编译命令
description: 使用 IAR Embedded Workbench 命令行工具编译并验证此逐飞（Seekfree）CYT4BB 双核项目。适用于运行 iarbuild、验证 CM7_0/CM7_1 Debug 版本编译、检查编译产物或解析此仓库的 IAR 编译器警告与错误。
---

## boatchanting
文件添加到工程环境
iar\project_config\cyt4bb7_cm_7_0.ewp
iar\project_config\cyt4bb7_cm_7_0.ewt
iar\project_config\cyt4bb7_cm_7_1.ewp
iar\project_config\cyt4bb7_cm_7_1.ewt

写完之后在沙箱外跑编译命令 
在沙箱外跑"D:\Softwares\iar\common\bin\iarbuild.exe" ".\iar\project_config\cyt4bb7_cm_7_0.ewp" -make Debug  
 "D:\Softwares\iar\common\bin\iarbuild.exe" ".\iar\project_config\cyt4bb7_cm_7_1.ewp" -make Debug

## willfulDU
写完之后在沙箱外跑编译命令 
在沙箱外跑
 "C:\Program Files\IAR Systems\Embedded Workbench 9.2\common\bin\iarbuild.exe" ".\iar\project_config\cyt4bb7_cm_7_0.ewp" -make Debug  
 "C:\Program Files\IAR Systems\Embedded Workbench 9.2\common\bin\iarbuild.exe" ".\iar\project_config\cyt4bb7_cm_7_1.ewp" -make Debug