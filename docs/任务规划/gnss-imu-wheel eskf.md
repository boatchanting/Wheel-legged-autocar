**车辆系速度作为状态**，二维地面车 ESKF。

这套模型特别适合你现在的场景：**两轮车、IMU 高频传播、轮速约束前向速度、NHC 约束侧向速度、GPS 做全局修正、heading 做绝对航向约束**。这种把 wheel odometry 和非完整约束用于抑制地面车 INS 漂移的做法是很常见的，而在线估计这类慢参数又依赖合适的运动激励，尤其是转弯。

下面默认你已经做了两件事：

1. **IMU 加速度已经用 roll/pitch 做了水平化并去除了重力**，所以 (a_x,a_y) 是**车体系水平面加速度**。
2. 滤波器内部统一使用：

   * 导航系 (n)：(x) 向东，(y) 向北
   * 车体系 (b)：(x_b) 向前，(y_b) 向左

---

# 1. 状态定义

## 1.1 名义状态

取 11 维状态：

[
x=
\begin{bmatrix}
p_x & p_y & v_f & v_{lat} & \psi & b_g & b_{ax} & b_{ay} & s_l & s_r & \psi_{off}
\end{bmatrix}^T
]

各量含义：

* (p_x,p_y)：导航系位置
* (v_f)：车体系前向速度
* (v_{lat})：车体系侧向速度
* (\psi)：车体航向角
* (b_g)：陀螺 z 轴零偏
* (b_{ax},b_{ay})：车体系水平加速度零偏
* (s_l,s_r)：左右轮尺度因子
* (\psi_{off})：绝对航向传感器相对车体前向的 yaw 安装偏差

---

## 1.2 误差状态

[
\delta x=
\begin{bmatrix}
\delta p_x & \delta p_y & \delta v_f & \delta v_{lat} & \delta\psi & \delta b_g & \delta b_{ax} & \delta b_{ay} & \delta s_l & \delta s_r & \delta\psi_{off}
\end{bmatrix}^T
]

---

# 2. 基本矩阵

定义

[
c=\cos\psi,\qquad s=\sin\psi
]

二维旋转矩阵

[
R(\psi)=
\begin{bmatrix}
c & -s\
s & c
\end{bmatrix}
]

二维反对称矩阵

[
J=
\begin{bmatrix}
0 & -1\
1 & 0
\end{bmatrix}
]

车体系速度向量记为

[
v_b=
\begin{bmatrix}
v_f\
v_{lat}
\end{bmatrix}
]

则导航系速度为

[
v_n=R(\psi)v_b
==============

\begin{bmatrix}
c v_f - s v_{lat}\
s v_f + c v_{lat}
\end{bmatrix}
]

---

# 3. 名义状态传播模型

## 3.1 传感器输入

设 IMU 测得

[
\omega_m = gyro_z
]

[
a_{xm},\ a_{ym}
]

去偏置后

[
\omega = \omega_m - b_g
]

[
a_x = a_{xm} - b_{ax}
]

[
a_y = a_{ym} - b_{ay}
]

写成向量

[
a_b=
\begin{bmatrix}
a_x\
a_y
\end{bmatrix}
]

---

## 3.2 位置传播

因为位置导数就是导航系速度：

[
\dot p_x = c,v_f - s,v_{lat}
]

[
\dot p_y = s,v_f + c,v_{lat}
]

---

## 3.3 车体系速度传播

在旋转坐标系下，车体系速度满足

[
\dot v_b = a_b - \omega J v_b
]

展开就是

[
\dot v_f = a_x + \omega v_{lat}
]

[
\dot v_{lat} = a_y - \omega v_f
]

也就是

[
\dot v_f = (a_{xm}-b_{ax}) + (\omega_m-b_g),v_{lat}
]

[
\dot v_{lat} = (a_{ym}-b_{ay}) - (\omega_m-b_g),v_f
]

---

## 3.4 航向传播

[
\dot\psi = \omega_m - b_g
]

---

## 3.5 慢变量传播

这些参数按随机游走处理：

[
\dot b_g = 0,\qquad
\dot b_{ax}=0,\qquad
\dot b_{ay}=0
]

[
\dot s_l = 0,\qquad
\dot s_r = 0,\qquad
\dot\psi_{off}=0
]

实际在误差模型里会给它们过程噪声。

---

## 3.6 离散传播

采样周期 (\Delta t) 时，最简单的一阶离散写法为：

[
p_{x,k+1}=p_{x,k} + (c v_f - s v_{lat})\Delta t
]

[
p_{y,k+1}=p_{y,k} + (s v_f + c v_{lat})\Delta t
]

[
v_{f,k+1}=v_{f,k} + \left[(a_{xm}-b_{ax}) + (\omega_m-b_g)v_{lat}\right]\Delta t
]

[
v_{lat,k+1}=v_{lat,k} + \left[(a_{ym}-b_{ay}) - (\omega_m-b_g)v_f\right]\Delta t
]

[
\psi_{k+1}=\mathrm{wrap}!\left(\psi_k + (\omega_m-b_g)\Delta t\right)
]

[
b_{g,k+1}=b_{g,k},\quad
b_{ax,k+1}=b_{ax,k},\quad
b_{ay,k+1}=b_{ay,k}
]

[
s_{l,k+1}=s_{l,k},\quad
s_{r,k+1}=s_{r,k},\quad
\psi_{off,k+1}=\psi_{off,k}
]

---

# 4. 连续误差状态方程

误差状态方程写为

[
\delta \dot x = F,\delta x + G,w
]

---

## 4.1 误差传播逐项推导

### 位置误差

[
\delta \dot p = \delta (R v_b)
]

所以

[
\delta \dot p = R,\delta v_b + R J v_b,\delta\psi
]

展开：

[
\delta \dot p_x
===============

c,\delta v_f - s,\delta v_{lat}
+
(-s v_f - c v_{lat}),\delta\psi
]

[
\delta \dot p_y
===============

s,\delta v_f + c,\delta v_{lat}
+
(c v_f - s v_{lat}),\delta\psi
]

---

### 车体系速度误差

名义模型：

[
\dot v_b = a_b - \omega J v_b
]

对误差线性化：

[
\delta \dot v_b
===============

-\omega J,\delta v_b
+
J v_b,\delta b_g
----------------

## \delta b_a

n_a
+
J v_b,n_g
]

其中

[
\delta b_a=
\begin{bmatrix}
\delta b_{ax}\
\delta b_{ay}
\end{bmatrix}
]

展开成标量：

[
\delta \dot v_f
===============

## \omega,\delta v_{lat}

## v_{lat},\delta b_g

## \delta b_{ax}

## n_{ax}

v_{lat},n_g
]

[
\delta \dot v_{lat}
===================

-\omega,\delta v_f
+
v_f,\delta b_g
--------------

## \delta b_{ay}

n_{ay}
+
v_f,n_g
]

注意这里

[
\omega=\omega_m-b_g
]

---

### 航向误差

[
\delta \dot\psi = -\delta b_g - n_g
]

---

### 偏置、尺度因子、安装偏差误差

[
\delta \dot b_g = n_{wg}
]

[
\delta \dot b_{ax} = n_{wax}
]

[
\delta \dot b_{ay} = n_{way}
]

[
\delta \dot s_l = n_{sl}
]

[
\delta \dot s_r = n_{sr}
]

[
\delta \dot \psi_{off} = n_{\psi off}
]

---

# 5. 状态矩阵 (F)

按误差状态顺序

[
\delta x=
[
\delta p_x,\delta p_y,\delta v_f,\delta v_{lat},\delta\psi,\delta b_g,\delta b_{ax},\delta b_{ay},\delta s_l,\delta s_r,\delta\psi_{off}
]^T
]

则 (F) 为

[
F=
\begin{bmatrix}
0&0&c&-s&-s v_f-c v_{lat}&0&0&0&0&0&0\
0&0&s&c&c v_f-s v_{lat}&0&0&0&0&0&0\
0&0&0&\omega&0&-v_{lat}&-1&0&0&0&0\
0&0&-\omega&0&0&v_f&0&-1&0&0&0\
0&0&0&0&0&-1&0&0&0&0&0\
0&0&0&0&0&0&0&0&0&0&0\
0&0&0&0&0&0&0&0&0&0&0\
0&0&0&0&0&0&0&0&0&0&0\
0&0&0&0&0&0&0&0&0&0&0\
0&0&0&0&0&0&0&0&0&0&0\
0&0&0&0&0&0&0&0&0&0&0
\end{bmatrix}
]

这里最关键的几项你可以直接记：

* 位置对车体系速度的偏导：(R(\psi))
* 位置对航向的偏导：(R(\psi)Jv_b)
* 车体系速度对自身的偏导：(-\omega J)
* 车体系速度对陀螺偏置的偏导：(Jv_b)
* 车体系速度对加计偏置的偏导：(-I)

---

# 6. 噪声矩阵 (G)

定义噪声向量

[
w=
\begin{bmatrix}
n_{ax} & n_{ay} & n_g & n_{wg} & n_{wax} & n_{way} & n_{sl} & n_{sr} & n_{\psi off}
\end{bmatrix}^T
]

则

[
G=
\begin{bmatrix}
0&0&0&0&0&0&0&0&0\
0&0&0&0&0&0&0&0&0\
-1&0&-v_{lat}&0&0&0&0&0&0\
0&-1&v_f&0&0&0&0&0&0\
0&0&-1&0&0&0&0&0&0\
0&0&0&1&0&0&0&0&0\
0&0&0&0&1&0&0&0&0\
0&0&0&0&0&1&0&0&0\
0&0&0&0&0&0&1&0&0\
0&0&0&0&0&0&0&1&0\
0&0&0&0&0&0&0&0&1
\end{bmatrix}
]

---

# 7. 连续过程噪声协方差 (Q_c)

[
Q_c
===

\mathrm{diag}
\left(
\sigma_{ax}^2,,
\sigma_{ay}^2,,
\sigma_g^2,,
\sigma_{wg}^2,,
\sigma_{wax}^2,,
\sigma_{way}^2,,
\sigma_{sl}^2,,
\sigma_{sr}^2,,
\sigma_{\psi off}^2
\right)
]

---

# 8. 离散化

一阶近似就够工程用了：

[
\Phi_k \approx I + F_k \Delta t
]

[
Q_{d,k} \approx G_k Q_c G_k^T \Delta t
]

协方差传播：

[
P_{k+1}^- = \Phi_k P_k^+ \Phi_k^T + Q_{d,k}
]

---

# 9. 量测模型

下面把每种观测都按这套状态完整写出来。

---

## 9.1 GPS 位置量测

GPS 转局部平面后：

[
z_p=
\begin{bmatrix}
x_{gps}\
y_{gps}
\end{bmatrix}
]

量测函数：

[
h_p(x)=
\begin{bmatrix}
p_x\
p_y
\end{bmatrix}
]

残差：

[
r_p = z_p - h_p(x)
]

雅可比：

[
H_p=
\begin{bmatrix}
1&0&0&0&0&0&0&0&0&0&0\
0&1&0&0&0&0&0&0&0&0&0
\end{bmatrix}
]

---

## 9.2 GPS 速度量测

GPS 若给 `speed` 和 `direction`，转成导航系速度：

[
v_{gps,x}=v_g\sin\chi,\qquad
v_{gps,y}=v_g\cos\chi
]

其中

[
v_g = \frac{\text{speed}}{3.6}
]

[
\chi = \text{direction}
]

则

[
z_v=
\begin{bmatrix}
v_{gps,x}\
v_{gps,y}
\end{bmatrix}
]

量测函数：

[
h_v(x)=
\begin{bmatrix}
c v_f - s v_{lat}\
s v_f + c v_{lat}
\end{bmatrix}
]

残差：

[
r_v=z_v-h_v(x)
]

雅可比：

[
H_v=
\begin{bmatrix}
0&0&c&-s&-s v_f-c v_{lat}&0&0&0&0&0&0\
0&0&s&c&c v_f-s v_{lat}&0&0&0&0&0&0
\end{bmatrix}
]

---

## 9.3 轮速前向速度量测

原始左右轮速度为 (v_l,v_r)。

尺度修正后的轮速：

[
v_l^{corr}=(1+s_l)v_l,\qquad
v_r^{corr}=(1+s_r)v_r
]

前向速度观测定义成

[
z_w=\frac{v_l+v_r}{2}
]

量测函数写成

[
h_w(x)=
v_f - \frac12(s_l v_l + s_r v_r)
]

这样残差就是

[
r_w = z_w - h_w(x)
]

直观上：

* (z_w) 是原始平均轮速
* (h_w) 是“车体前向速度 + 尺度修正项”

雅可比：

[
H_w=
\begin{bmatrix}
0&0&1&0&0&0&0&0&-\frac{v_l}{2}&-\frac{v_r}{2}&0
\end{bmatrix}
]

---

## 9.4 非完整约束 NHC

两轮地面车正常行驶时，侧向速度近似为 0：

[
z_{nhc}=0
]

[
h_{nhc}(x)=v_{lat}
]

残差：

[
r_{nhc}=z_{nhc}-h_{nhc}(x)= -v_{lat}
]

雅可比：

[
H_{nhc}=
\begin{bmatrix}
0&0&0&1&0&0&0&0&0&0&0
\end{bmatrix}
]

这个量测是整套模型里非常关键的一个。地面车组合导航文献中，wheel odometry 和 NHC 都是经典约束项。

---

## 9.5 轮速差分角速度量测

这个量测专门用来帮助估计：

* (b_g)
* (s_l,s_r)

轮速差分给出的原始角速度：

[
z_\omega = \frac{v_r-v_l}{b}
]

其中 (b) 是轮距。

尺度修正后的编码器角速度应为

[
\omega_{odo}^{corr}
===================

# \frac{(1+s_r)v_r-(1+s_l)v_l}{b}

\frac{v_r-v_l}{b} + \frac{s_r v_r - s_l v_l}{b}
]

而状态给出的车体 yaw rate 是

[
\omega_{imu} = \omega_m - b_g
]

于是量测函数定义为

[
h_\omega(x)=
(\omega_m-b_g)-\frac{s_r v_r - s_l v_l}{b}
]

残差：

[
r_\omega = z_\omega - h_\omega(x)
]

如果 IMU yaw rate 和轮速差分 yaw rate 一致，残差就接近 0。

雅可比：

[
H_\omega=
\begin{bmatrix}
0&0&0&0&0&-1&0&0&\frac{v_l}{b}&-\frac{v_r}{b}&0
\end{bmatrix}
]

这里要注意符号：

* 对 (b_g) 的偏导是 (-1)
* 对 (s_l) 的偏导是 (+v_l/b)
* 对 (s_r) 的偏导是 (-v_r/b)

---

## 9.6 绝对航向量测

你有 heading 量测时，建议显式估计安装偏差 (\psi_{off})。

量测值：

[
z_\psi = \psi_m
]

量测函数：

[
h_\psi(x)=\psi+\psi_{off}
]

残差必须做角度归一：

[
r_\psi=\mathrm{wrap}!\left(z_\psi-h_\psi(x)\right)
]

雅可比：

[
H_\psi=
\begin{bmatrix}
0&0&0&0&1&0&0&0&0&0&1
\end{bmatrix}
]

绝对航向可来自：

* 双天线 GNSS 航向
* 校准后的磁航向
* 高速时 GPS course

但它们的噪声 (R_\psi) 不能一样大，磁航向通常要更谨慎。航向/外参类参数的在线校准效果也明显依赖运动激励。

---

# 10. EKF 更新公式

对于任意观测：

[
r = z - h(x^-)
]

[
S = H P^- H^T + R
]

[
K = P^- H^T S^{-1}
]

[
\delta \hat x = K r
]

推荐用 Joseph 形式更新协方差：

[
P^+ = (I-KH)P^-(I-KH)^T + K R K^T
]

---

# 11. 误差注入

得到误差状态估计后，注入名义状态：

[
p_x \leftarrow p_x + \delta p_x
]

[
p_y \leftarrow p_y + \delta p_y
]

[
v_f \leftarrow v_f + \delta v_f
]

[
v_{lat} \leftarrow v_{lat} + \delta v_{lat}
]

[
\psi \leftarrow \mathrm{wrap}(\psi+\delta\psi)
]

[
b_g \leftarrow b_g + \delta b_g
]

[
b_{ax} \leftarrow b_{ax} + \delta b_{ax}
]

[
b_{ay} \leftarrow b_{ay} + \delta b_{ay}
]

[
s_l \leftarrow s_l + \delta s_l
]

[
s_r \leftarrow s_r + \delta s_r
]

[
\psi_{off} \leftarrow \mathrm{wrap}(\psi_{off}+\delta\psi_{off})
]

然后把误差均值重置为 0。

对于这类二维小误差模型，误差重置 Jacobian 直接取单位阵即可。

---

# 12. 运行顺序

## 12.1 IMU 高频循环

每次 IMU 数据到来：

1. 读 (a_{xm},a_{ym},\omega_m)
2. 名义状态 propagation
3. 计算 (F,G,\Phi,Q_d)
4. 传播 (P)

---

## 12.2 轮速 10 ms 更新

每次轮速到来建议按这个顺序：

1. **前向速度更新**
   [
   (z_w,h_w,H_w)
   ]

2. **若 `slip_flag == 0`，做 NHC**
   [
   (z_{nhc},h_{nhc},H_{nhc})
   ]

3. **若不打滑且车速不太低，做轮速差分角速度更新**
   [
   (z_\omega,h_\omega,H_\omega)
   ]

---

## 12.3 GPS 100 ms 更新

若 `state == 1`：

1. GPS 位置更新
2. 若 GPS 速度可信，再做 GPS 速度更新
3. 若 heading 可信，再做绝对航向更新

---

# 13. 这套模型的关键噪声怎么设

最重要的是这三类：

### 13.1 轮尺度过程噪声

[
\sigma_{sl}^2,\ \sigma_{sr}^2
]
要很小。它们是慢变量，不应该快速变化。

### 13.2 航向安装偏差过程噪声

[
\sigma_{\psi off}^2
]
比轮尺度还应更小，近似常值。

### 13.3 打滑时动态调噪声

若 `slip_flag == 1`：

* 放大 (R_w)
* 关闭或放大 (R_{nhc})
* 放大 (R_\omega)

否则滤波器很容易把打滑误解释成：

* 轮子尺度变了
* gyro bias 变了

---

# 14. 你写代码时最常抄的几组式子

传播：

[
\dot p_x = c v_f - s v_{lat}
]

[
\dot p_y = s v_f + c v_{lat}
]

[
\dot v_f = (a_{xm}-b_{ax}) + (\omega_m-b_g)v_{lat}
]

[
\dot v_{lat} = (a_{ym}-b_{ay}) - (\omega_m-b_g)v_f
]

[
\dot\psi = \omega_m - b_g
]

前向轮速：

[
h_w=v_f-\frac12(s_l v_l+s_r v_r)
]

[
H_w=
\begin{bmatrix}
0&0&1&0&0&0&0&0&-\frac{v_l}{2}&-\frac{v_r}{2}&0
\end{bmatrix}
]

NHC：

[
h_{nhc}=v_{lat}
]

[
H_{nhc}=
\begin{bmatrix}
0&0&0&1&0&0&0&0&0&0&0
\end{bmatrix}
]

轮速差分角速度：

[
h_\omega=(\omega_m-b_g)-\frac{s_r v_r-s_l v_l}{b}
]

[
H_\omega=
\begin{bmatrix}
0&0&0&0&0&-1&0&0&\frac{v_l}{b}&-\frac{v_r}{b}&0
\end{bmatrix}
]

绝对航向：

[
h_\psi=\psi+\psi_{off}
]

[
H_\psi=
\begin{bmatrix}
0&0&0&0&1&0&0&0&0&0&1
\end{bmatrix}
]

GPS 位置：

[
H_p=
\begin{bmatrix}
1&0&0&0&0&0&0&0&0&0&0\
0&1&0&0&0&0&0&0&0&0&0
\end{bmatrix}
]

GPS 速度：

[
h_v=
\begin{bmatrix}
c v_f-s v_{lat}\
s v_f+c v_{lat}
\end{bmatrix}
]

[
H_v=
\begin{bmatrix}
0&0&c&-s&-s v_f-c v_{lat}&0&0&0&0&0&0\
0&0&s&c&c v_f-s v_{lat}&0&0&0&0&0&0
\end{bmatrix}
]


