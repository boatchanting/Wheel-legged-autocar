# Plan1 简化 LQR 路径跟踪调参说明

这版不是完整 LQR，不在线解 Riccati。它先用离线路径表里的切线航向和曲率，再叠加横向误差、航向误差，最后输出到底盘原来的 `err_degree`。

控制律：

```text
err_raw = LQR_SIGN * (LQR_K_CURV * curvature * speed_sign
                    + LQR_K_LATERAL * e_y
                    + LQR_K_HEADING * e_psi)
```

速度仍然只用路径表里的 `target_speed`，再经过 `NavReplay_SpeedSlew_Update()` 做斜率限制。负速度表示前进。

## 关键参数

- `LQR_PREVIEW_POINTS`：参考航向和曲率往前看几个点。大一点更稳，小一点更贴线。
- `LQR_SHARP_CURVATURE_TH`：急弯判断阈值。调大时只有更急的弯才软化；调小时普通弯也会更保守。
- `LQR_SHARP_PREVIEW_POINTS`：急弯附近的预瞄点数。调大入弯更主动；调小入弯更稳，不容易突然崴脚。
- `LQR_SEARCH_RANGE_NORMAL`：正常最近点向前搜索窗口。太小会跟不上，太大可能跨到后面相似路段。
- `LQR_SEARCH_RANGE_RECOVER`：特殊动作恢复后的搜索窗口。恢复时可比正常大。
- `LQR_K_CURV`：曲率前馈。主要负责弯道提前给角。
- `LQR_K_LATERAL`：横向误差反馈。车离线越远，纠偏越强。
- `LQR_K_HEADING`：航向误差反馈。车头和路径切线差得越多，纠偏越强。
- `LQR_ERR_MAX_DEG`：最大输出角度，防止一下打太狠。
- `LQR_ERR_SLEW_DEG`：单周期最大角度变化，防止绕桩抽搐。
- `LQR_SHARP_ERR_SLEW_DEG`：急弯段单周期最大角度变化。调大更跟手；调小更柔，能减少急弯入口颤一下。
- `LQR_FILTER_ALPHA`：低通滤波系数。大一点反应快，小一点更稳。
- `LQR_SHARP_FILTER_ALPHA`：急弯段低通滤波系数。调大急弯更灵；调小急弯更顺，但太小会转得慢。
- `LQR_LATERAL_ERR_LIMIT_MM`：横向误差限幅，防止离线很远时输出爆掉。
- `LQR_SIGN`：总方向符号。实车方向整体反了，优先改它。
- `LQR_FORWARD_SPEED_IS_NEGATIVE`：本车负速度为前进，默认设为 1。

## 先确认方向

先低速跑，暂时不要追求快。

1. 让 `LQR_K_CURV` 小一点，甚至先设 0。
2. 只看车偏离路径时，会不会往路径方向修。
3. 如果整体反着打，先把 `LQR_SIGN` 从 `1.0f` 改成 `-1.0f`。
4. 如果只有横向修正反了，改 `LQR_K_LATERAL` 的正负。
5. 如果只有弯道提前量反了，改 `LQR_K_CURV` 的正负。

方向没确认前，不要加速度。

## 推荐调参顺序

1. `LQR_K_HEADING`：先让车头能顺着路径切线走，不明显左右摆。
2. `LQR_K_LATERAL`：再让车能回到路径中心线，不要为了贴线把它调得太大。
3. `LQR_K_CURV`：最后加曲率前馈，让进弯不滞后、绕桩不切弯。
4. `LQR_ERR_SLEW_DEG`：如果绕桩有抽搐，先降这个。
5. `LQR_FILTER_ALPHA`：如果仍然抖，再适当降低；如果反应太慢，再提高。
6. `target_speed`：控制稳定后再提高路径表速度。

## 常见现象

### 掉头回来第一下蹩脚

按顺序查：

1. 最近点索引是否跳太远。
2. `LQR_SEARCH_RANGE_NORMAL` 是否太大。
3. `LQR_K_HEADING` 是否太大。
4. `LQR_ERR_SLEW_DEG` 是否太大。
5. 掉头出口处 `curvature` 是否突变。

### 绕桩左右抽搐

按顺序查：

1. `LQR_K_LATERAL` 是否太大。
2. `LQR_K_CURV` 是否太大。
3. `LQR_ERR_SLEW_DEG` 是否太大。
4. `LQR_FILTER_ALPHA` 是否太大。
5. `LQR_PREVIEW_POINTS` 是否太小。

### 车很稳但切弯撞桩

按顺序查：

1. `LQR_K_CURV` 是否太小或符号反了。
2. `LQR_PREVIEW_POINTS` 是否太大，导致参考点看得太远。
3. 路径本身和桩筒安全距离是否不够。
4. 该段 `target_speed` 是否太高。

## 建议记录的数据

调车时如果能打日志，优先记录：

- `g_target_idx`
- 参考点 `x/y`
- `target_speed_set`
- `e_y`
- `e_psi`
- `curvature`
- 限幅前 `err_raw`
- 最终 `err_degree`

这些量能快速判断问题是在路径、符号、增益，还是输出滤波。
