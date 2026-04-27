param(
    [string]$Frames = "",
    [string]$PythonSummary = "",
    [int]$MaxFrames = 0,
    [int]$DebugEvery = 500,
    [switch]$NoTiming,
    [switch]$NoCsv
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path (Join-Path $ScriptDir "..\..\..\..")

if ([string]::IsNullOrWhiteSpace($Frames)) {
    $Frames = Join-Path $ProjectRoot "data\frames\2026_04_17_21_18_39_Video"
}

if ([string]::IsNullOrWhiteSpace($PythonSummary)) {
    $PythonSummary = Join-Path $ProjectRoot "data\bridge_white_pvc_detection_video_2026_04_17_21_18_39\video_summary.json"
}

$PgmDir = Join-Path $ProjectRoot "data\bridge_white_pvc_c_pgm_frames"
$RunDir = Join-Path $ProjectRoot "data\bridge_white_pvc_c_run"
$Exe = Join-Path $RunDir "pvc_video_cli.exe"
$CJson = Join-Path $RunDir "pvc_c_summary.json"
$Csv = Join-Path $RunDir "pvc_c_summary.csv"
$Report = Join-Path $RunDir "compare_report.md"

New-Item -ItemType Directory -Force -Path $RunDir | Out-Null

$PrepareArgs = @(
    (Join-Path $ScriptDir "prepare_pgm_frames.py"),
    "--frames", $Frames,
    "--output", $PgmDir
)
if ($MaxFrames -gt 0) {
    $PrepareArgs += @("--max-frames", "$MaxFrames")
}

python @PrepareArgs

$BuildScript = Join-Path $ScriptDir "build_pvc_detector.ps1"
if ($NoTiming -and $NoCsv) {
    & $BuildScript -Output $Exe -NoTiming -NoCsv
}
elseif ($NoTiming) {
    & $BuildScript -Output $Exe -NoTiming
}
elseif ($NoCsv) {
    & $BuildScript -Output $Exe -NoCsv
}
else {
    & $BuildScript -Output $Exe
}

$CliArgs = @(
    "--pgm-dir", $PgmDir,
    "--output-json", $CJson,
    "--debug-every", "$DebugEvery"
)
if (-not $NoCsv) {
    $CliArgs += @("--output-csv", $Csv)
}
if ($MaxFrames -gt 0) {
    $CliArgs += @("--max-frames", "$MaxFrames")
}

& $Exe @CliArgs

python (Join-Path $ScriptDir "compare_with_python_summary.py") `
    --python-summary $PythonSummary `
    --c-summary $CJson `
    --output $Report

Write-Host "C summary: $CJson"
Write-Host "Compare report: $Report"
