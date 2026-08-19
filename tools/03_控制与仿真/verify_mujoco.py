import mujoco
import numpy as np

print("mujoco", mujoco.__version__)

# 最小物理冒烟测试：方块在平面上受重力 + 摩擦接触
xml = """
<mujoco model="smoke">
  <option timestep="0.001" />
  <worldbody>
    <geom name="ground" type="plane" size="2 2 0.1" friction="0.8 0.05 0.0001"/>
    <body name="box" pos="0 0 0.5">
      <joint name="free" type="free"/>
      <geom name="bx" type="box" size="0.1 0.1 0.1" mass="2" friction="0.8 0.05 0.0001"/>
    </body>
  </worldbody>
</mujoco>
"""
m = mujoco.MjModel.from_xml_string(xml)
d = mujoco.MjData(m)
for i in range(500):
    mujoco.mj_step(m, d)
print("box z after 0.5s:", d.geom_xpos[1][2], "(应≈0.1=贴地)")
print("contact pairs:", d.ncon)
print("OK: mujoco 物理引擎可用")
