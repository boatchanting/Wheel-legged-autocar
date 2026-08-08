<#!
    从 code/config/sys_options.h 生成按功能拆分的内部配置头。

    sys_options.h 是唯一的人工编辑入口；本脚本不会修改它。输出文件仅在
    内容发生变化时才写回，因此不会无意义地触发 IAR 的增量编译。
#>
[CmdletBinding()]
param()

$projectRoot = Split-Path -Parent $PSScriptRoot
$sourcePath = Join-Path $projectRoot 'code\config\sys_options.h'
$outputDirectory = Join-Path $projectRoot 'code\config\generated'

if (-not (Test-Path -LiteralPath $sourcePath)) {
    throw "Cannot find configuration source: $sourcePath"
}

$values = @{}
foreach ($line in Get-Content -LiteralPath $sourcePath) {
    if ($line -match '^\s*#define\s+([A-Za-z_]\w*)\s+([^\s/]+)') {
        $values[$matches[1]] = $matches[2]
    }
}

$groups = [ordered]@{
    'sys_options_display.h' = @('DEBUG_DISPLAY', 'DEBUG_DISPLAY_CORE_SELECT')
    'sys_options_camera_menu.h' = @('CAMERA_MENU_REFRESH_DIV', 'CAMERA_MENU_DEBUG_LOG_ENABLE', 'CAMERA_MENU_DEBUG_LOG_DIV')
    'sys_options_wifi.h' = @('WIFI_USE', 'WIFI_CORE_SELECT', 'WIFI_PROTOCOL_SELECT')
    'sys_options_motion.h' = @('G_MOTOR_ENABLE_INIT', 'REMOTE_CONTROL', 'ROLL_BALANCE_ENABLE_INIT')
    'sys_options_debug.h' = @('DEBUG_LOG_ENABLE')
    'sys_options_imu.h' = @('IMU_CATEGORY', 'IMU_REFRESH_TEST_ENABLE')
    'sys_options_accel.h' = @('ACCEL_FF_ENABLE', 'ACCEL_FF_MODE', 'ACCEL_FF_BUZZER_ENABLE')
    'sys_options_navigation.h' = @('GNSS_NAV', 'CURRENT_NAV_PLAN', 'PLAN1_FAST_UTURN_ENABLE')
    'sys_options_sbus.h' = @('SUBS_CATEGORY', 'SBUS_ACTIVE_POINT')
    'sys_options_jump.h' = @('JUMP_ENABLE_LANDING_BUFFER')
    'sys_options_misc.h' = @('SLIP_DETECTION_ENABLE')
}

New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)

foreach ($entry in $groups.GetEnumerator()) {
    $guard = ('CODE_CONFIG_GENERATED_' + $entry.Key.ToUpper().Replace('.', '_'))
    $lines = @("#ifndef $guard", "#define $guard", '')

    foreach ($name in $entry.Value) {
        if (-not $values.ContainsKey($name)) {
            throw "Required option $name is missing from $sourcePath"
        }
        $lines += "#define $name $($values[$name])"
    }

    if ($entry.Key -eq 'sys_options_display.h') {
        $lines += '', '#if (DEBUG_DISPLAY && (DEBUG_DISPLAY_CORE_SELECT != 0) && (DEBUG_DISPLAY_CORE_SELECT != 1))', '#error "DEBUG display config error: invalid DEBUG_DISPLAY_CORE_SELECT."', '#endif', '', '#define DEBUG_DISPLAY_CORE0 (DEBUG_DISPLAY && (DEBUG_DISPLAY_CORE_SELECT == 0))', '#define DEBUG_DISPLAY_CORE1 (DEBUG_DISPLAY && (DEBUG_DISPLAY_CORE_SELECT == 1))'
    }
    elseif ($entry.Key -eq 'sys_options_camera_menu.h') {
        $lines += '', '#if (CAMERA_MENU_REFRESH_DIV == 0U)', '#error "DEBUG display config error: CAMERA_MENU_REFRESH_DIV must be greater than 0."', '#endif', '', '#if (CAMERA_MENU_DEBUG_LOG_DIV == 0U)', '#error "DEBUG display config error: CAMERA_MENU_DEBUG_LOG_DIV must be greater than 0."', '#endif'
    }
    elseif ($entry.Key -eq 'sys_options_jump.h') {
        $lines += '', '#if (JUMP_ENABLE_LANDING_BUFFER != 0U) && (JUMP_ENABLE_LANDING_BUFFER != 1U)', '#error "JUMP config error: JUMP_ENABLE_LANDING_BUFFER must be 0 or 1."', '#endif'
    }

    $lines += '', "#endif /* $guard */", ''
    $content = $lines -join "`r`n"
    $targetPath = Join-Path $outputDirectory $entry.Key
    $oldContent = if (Test-Path -LiteralPath $targetPath) { [IO.File]::ReadAllText($targetPath) } else { $null }
    if ($oldContent -cne $content) {
        [IO.File]::WriteAllText($targetPath, $content, $utf8NoBom)
        Write-Host "Updated $($entry.Key)"
    }
}
