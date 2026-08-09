# 编译 bridge_host_file.exe —— PC 宿主验证 (有效检测门控移植回归)
# 依赖: 当前工程 code1/vision/bridge_detect.c (MCU 源码直接编译)
#        + bridge_conv_ref_b2.c (b2_ 前缀卷积 C 参考, 替代汇编)
# 用法: 在 tools/bridge_valid_host/ 下执行  .\build.ps1
$ErrorActionPreference = "Stop"
$root = "D:\WORKS\2026LunTui\project"
$scriptDir = $PSScriptRoot

gcc -O2 -I "$root\code1\vision" -I "$root\code" -o "$scriptDir\bridge_host_file.exe" `
    "$scriptDir\bridge_host_file.c" `
    "$scriptDir\bridge_conv_ref_b2.c" `
    "$scriptDir\bridge_mlp_ref.c" `
    "$root\code1\vision\bridge_detect.c"

if ($LASTEXITCODE -eq 0) {
    Write-Host "OK: $scriptDir\bridge_host_file.exe"
} else {
    Write-Host "BUILD FAILED (exit $LASTEXITCODE)"
    exit $LASTEXITCODE
}
