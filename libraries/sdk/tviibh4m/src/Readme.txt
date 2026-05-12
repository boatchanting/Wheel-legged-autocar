1. CYT3BB devices support only one CM7 core (CM7_0)
2. CYT4BB devices support two CM7 cores (CM7_0 and CM7_1)

For CYT3BB devices, in the main_cm0plus.c source file,
comment the below statement "Cy_SysEnableApplCore(CORE_CM7_1, CY_CORTEX_M7_1_APPL_ADDR);"
as these devices do not support the 2nd CM7 (CM7_1).

Do not use main_cm7_1.c for CYT3BB devices!

From the workspaces for CYT3BB devices,
IAR: Do not use the below workspaces
   tviibh4m_flash_cm0plus_cm7_0_cm7_1_template.eww
   tviibh4m_flash_cm7_1_mc_template.eww
   tviibh4m_sram_cm0plus_cm7_0_cm7_1_template.eww
   tviibh4m_sram_cm7_1_mc_template.eww
