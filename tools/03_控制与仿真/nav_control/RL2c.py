import torch
import numpy as np
from stable_baselines3 import PPO

# 1. 加载模型
# 注意：如果你的模型是在 GPU 上训练的，load 默认会加载到 GPU
# 如果你在没有 GPU 的机器上运行，或者想强制使用 CPU，可以添加 device='cpu'
model = PPO.load("ppo_multicar_rotation.zip")

# 2. 获取 Actor 网络的参数
# Stable-baselines3 中，policy 包含提取器和多层感知机
actor_net = model.policy.mlp_extractor.policy_net
action_net = model.policy.action_net

# 我们将所有层存下来
layers = []
for name, param in actor_net.named_parameters():
    # 修改点：先调用 .cpu() 将张量移动到 CPU，再调用 .detach() 和 .numpy()
    layers.append((name, param.detach().cpu().numpy()))
for name, param in action_net.named_parameters():
    # 修改点：先调用 .cpu() 将张量移动到 CPU，再调用 .detach() 和 .numpy()
    layers.append((name, param.detach().cpu().numpy()))

# 3. 自动生成 C 语言头文件 (weights.h)
with open("weights.h", "w") as f:
    f.write("#ifndef WEIGHTS_H\n#define WEIGHTS_H\n\n")
    for name, data in layers:
        # 将 PyTorch 变量名转为 C 变量名
        c_name = name.replace('.', '_')
        shape_str = "".join([f"[{dim}]" for dim in data.shape])
        f.write(f"const float {c_name}{shape_str} = {{\n")
        
        # 展平并写入浮点数据
        flat_data = data.flatten()
        for i, val in enumerate(flat_data):
            f.write(f"{val:.6f}f, ")
            if (i+1) % 8 == 0: f.write("\n")
            
        f.write("};\n\n")
    f.write("#endif\n")

print("成功导出 weights.h！")
