param(
    [string]$Output = "",
    [switch]$NoTiming,
    [switch]$NoCsv
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path (Join-Path $ScriptDir "..\..\..\..")

if ([string]::IsNullOrWhiteSpace($Output)) {
    $OutDir = Join-Path $ProjectRoot "data\line_gray_red_c_run"
    $Output = Join-Path $OutDir "line_video_cli.exe"
}
else {
    $OutDir = Split-Path -Parent $Output
    if ([string]::IsNullOrWhiteSpace($OutDir)) { $OutDir = "." }
}
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$DetectorC = Join-Path $ScriptDir "line_detector.c"
$CliC = Join-Path $ScriptDir "line_video_cli.c"

$Defines = @()
if ($NoTiming) { $Defines += "LINE_PC_ENABLE_TIMING=0" } else { $Defines += "LINE_PC_ENABLE_TIMING=1" }
if ($NoCsv) { $Defines += "LINE_PC_WRITE_CSV=0" } else { $Defines += "LINE_PC_WRITE_CSV=1" }

function Get-CommandPathOrNull($Name) {
    $Cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if ($null -eq $Cmd) { return $null }
    return $Cmd.Source
}

function Get-VsDevCmdOrNull {
    $VsWhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path -LiteralPath $VsWhere)) { return $null }
    $InstallPath = & $VsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ([string]::IsNullOrWhiteSpace($InstallPath)) { return $null }
    $DevCmd = Join-Path $InstallPath "Common7\Tools\VsDevCmd.bat"
    if (Test-Path -LiteralPath $DevCmd) { return $DevCmd }
    return $null
}

$Gcc = Get-CommandPathOrNull "gcc"
$Clang = Get-CommandPathOrNull "clang"
$Cl = Get-CommandPathOrNull "cl"

if ($Gcc) {
    $DefineArgs = $Defines | ForEach-Object { "-D$_" }
    & $Gcc -std=c99 -O2 -Wall -Wextra @DefineArgs $DetectorC $CliC -lm -o $Output
    Write-Host "built with gcc: $Output"
    exit 0
}

if ($Clang) {
    $DefineArgs = $Defines | ForEach-Object { "-D$_" }
    & $Clang -std=c99 -O2 -Wall -Wextra @DefineArgs $DetectorC $CliC -lm -o $Output
    Write-Host "built with clang: $Output"
    exit 0
}

if ($Cl) {
    $DefineArgs = $Defines | ForEach-Object { "/D$_" }
    & $Cl /nologo /O2 /TC @DefineArgs /Fo:$OutDir\ $DetectorC $CliC /Fe:$Output
    Write-Host "built with cl: $Output"
    exit 0
}

$VsDevCmd = Get-VsDevCmdOrNull
if ($VsDevCmd) {
    $DefineArgs = ($Defines | ForEach-Object { "/D$_" }) -join " "
    $Cmd = "call `"$VsDevCmd`" -arch=x64 -host_arch=x64 && cl /nologo /O2 /TC $DefineArgs /Fo:$OutDir\ $DetectorC $CliC /Fe:$Output"
    & cmd.exe /d /s /c $Cmd
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    Write-Host "built with Visual Studio cl via VsDevCmd: $Output"
    exit 0
}

Write-Error "No C compiler found in PATH. Install gcc/clang, or run this script from a Visual Studio Developer PowerShell where cl.exe is available."
