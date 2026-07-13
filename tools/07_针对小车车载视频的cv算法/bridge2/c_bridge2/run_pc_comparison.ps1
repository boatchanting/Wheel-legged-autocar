param(
    [string]$Frames = "",
    [string]$PythonSummary = "",
    [string]$RunDir = "",
    [int]$MaxFrames = 0,
    [int]$DebugEvery = 100
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path (Join-Path $ScriptDir "..\..\..\..")
$DatasetRoot = Get-ChildItem -LiteralPath (Join-Path $ProjectRoot "data") -Directory -Recurse | `
    Where-Object { $_.Name -eq "2026_07_06_16_54_45_Video_v30" } | `
    Select-Object -First 1 -ExpandProperty FullName
if ([string]::IsNullOrWhiteSpace($DatasetRoot)) { throw "Dataset 2026_07_06_16_54_45_Video_v30 was not found under data." }
if ([string]::IsNullOrWhiteSpace($Frames)) { $Frames = Join-Path $DatasetRoot "originals" }
if ([string]::IsNullOrWhiteSpace($PythonSummary)) { $PythonSummary = Join-Path $DatasetRoot "summary.csv" }
if ([string]::IsNullOrWhiteSpace($RunDir)) { $RunDir = Join-Path $DatasetRoot "c_bridge2_run" }

$PgmDir = Join-Path $RunDir "pgm"
$Exe = Join-Path $RunDir "bridge_detection_pc.exe"
$Csv = Join-Path $RunDir "c_summary.csv"
$Timing = Join-Path $RunDir "timing.json"
$Report = Join-Path $RunDir "compare_report.md"
New-Item -ItemType Directory -Force -Path $RunDir | Out-Null

$Python = Join-Path $ProjectRoot ".venv\Scripts\python.exe"
if (-not (Test-Path -LiteralPath $Python)) { $Python = "python" }

$PrepareArgs = @((Join-Path $ScriptDir "prepare_pgm_frames.py"), "--frames", $Frames, "--output", $PgmDir)
if ($MaxFrames -gt 0) { $PrepareArgs += @("--max-frames", "$MaxFrames") }
& $Python @PrepareArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& (Join-Path $ScriptDir "build_bridge_detection.ps1") -Output $Exe
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$CliArgs = @("--pgm-dir", $PgmDir, "--output-csv", $Csv, "--timing-json", $Timing, "--debug-every", "$DebugEvery")
if ($MaxFrames -gt 0) { $CliArgs += @("--max-frames", "$MaxFrames") }
& $Exe @CliArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $Python (Join-Path $ScriptDir "compare_with_python_summary.py") `
    --python-summary $PythonSummary `
    --c-summary $Csv `
    --timing $Timing `
    --output $Report
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "C summary: $Csv"
Write-Host "Timing: $Timing"
Write-Host "Comparison: $Report"
