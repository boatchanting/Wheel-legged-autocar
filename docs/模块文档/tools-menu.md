# tools/menu 模块文档

## 1. 模块作用

`menu` 模块提供屏幕菜单状态机，用于车上参数选择、科目选择、标定与动作确认流程。  
对应源码：`code/tools/menu.h`、`code/tools/menu.c`。

## 2. 菜单状态（`MenuState_t`）

- `MENU_STATE_MAIN`：主界面
- `MENU_STATE_SUBJECT`：科目选择
- `MENU_STATE_CALIBRATION`：标定方式选择
- `MENU_STATE_ACTION_SELECT`：操作选择（如 Record/Start）
- `MENU_STATE_ACTION_CONFIRM`：确认界面
- `MENU_STATE_ACTION_RUNNING`：运行中动态显示
- `MENU_STATE_ACTION_COMPLETE`：完成界面

## 3. 全局变量

- `current_state`：当前菜单状态
- `menu_index`：当前项索引
- `menu_values[]`：各层状态值
- `need_redraw`：是否需要重绘
- `g_is_push_mode`：推车模式标志（给底层控制环读取）

## 4. 关键接口

- `Menu_Init()`：菜单初始化
- `Menu_HandleKey()`：按键处理（建议周期调用）
- `Menu_ShowStatic()`：静态内容渲染
- `Menu_ShowDynamic()`：动态数据渲染

## 5. 推荐调用节拍

- 初始化阶段调用 `Menu_Init()`。
- 在主循环中周期调用 `Menu_HandleKey()`。
- 根据 `need_redraw` 决定是否刷新静态界面，并周期刷新动态区域。
