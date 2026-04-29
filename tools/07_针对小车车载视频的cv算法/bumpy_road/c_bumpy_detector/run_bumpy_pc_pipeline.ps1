param(
    [int]$MaxFrames = 0,
    [int]$DebugEvery = 0,
    [switch]$NoTiming,
    [switch]$NoCsv
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path (Join-Path $ScriptDir "..\..\..\..")

$PgmDir = Join-Path $ProjectRoot "data\bumpy_road_c_pgm_frames"
$RunDir = Join-Path $ProjectRoot "data\bumpy_road_c_run"
$Exe = Join-Path $RunDir "bumpy_video_cli.exe"
$CJson = Join-Path $RunDir "bumpy_c_summary.json"
$CCsv = Join-Path $RunDir "bumpy_c_summary.csv"
$PySummary = Join-Path $ProjectRoot "data\bumpy_road_line_detection_video\full_line_detection_timeline.json"
$CompareOut = Join-Path $RunDir "compare_report.md"

New-Item -ItemType Directory -Force -Path $PgmDir | Out-Null
New-Item -ItemType Directory -Force -Path $RunDir | Out-Null

$PrepareArgs = @(
    (Join-Path $ScriptDir "prepare_pgm_frames.py"),
    "--output", $PgmDir
)
if ($MaxFrames -gt 0) {
    $PrepareArgs += @("--max-frames", $MaxFrames)
}
& python @PrepareArgs
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$BuildArgs = @{
    Output = $Exe
}
if ($NoTiming) { $BuildArgs.NoTiming = $true }
if ($NoCsv) { $BuildArgs.NoCsv = $true }
& (Join-Path $ScriptDir "build_bumpy_detector.ps1") @BuildArgs
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$CliArgs = @(
    "--pgm-dir", $PgmDir,
    "--output-json", $CJson
)
if (-not $NoCsv) {
    $CliArgs += @("--output-csv", $CCsv)
}
if ($MaxFrames -gt 0) {
    $CliArgs += @("--max-frames", $MaxFrames)
}
if ($DebugEvery -gt 0) {
    $CliArgs += @("--debug-every", $DebugEvery)
}

& $Exe @CliArgs
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& python (Join-Path $ScriptDir "compare_with_python_timeline.py") `
    --python-summary $PySummary `
    --c-summary $CJson `
    --output $CompareOut
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "pgm_dir: $PgmDir"
Write-Host "c_summary: $CJson"
Write-Host "compare_report: $CompareOut"
