param(
    [string]$Output = "",
    [switch]$PhaseProfile
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
if ([string]::IsNullOrWhiteSpace($Output)) {
    $Output = Join-Path $ScriptDir "build\bridge_detection_pc.exe"
}
$OutDir = Split-Path -Parent $Output
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$Sources = @(
    (Join-Path $ScriptDir "bridge_detection.c"),
    (Join-Path $ScriptDir "bridge_detection_pc.c")
)
$ProfileDefine = if ($PhaseProfile) { "/DBRIDGE_DETECTION_PC_PROFILE" } else { "" }

function Find-VsDevCmd {
    $VsWhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path -LiteralPath $VsWhere)) { return $null }
    $Install = & $VsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ([string]::IsNullOrWhiteSpace($Install)) { return $null }
    $Candidate = Join-Path $Install "Common7\Tools\VsDevCmd.bat"
    if (Test-Path -LiteralPath $Candidate) { return $Candidate }
    return $null
}

$Cl = Get-Command cl -ErrorAction SilentlyContinue
if ($Cl) {
    & $Cl.Source /nologo /O2 /W4 /TC /utf-8 $ProfileDefine "/Fo:$OutDir\" $Sources "/Fe:$Output"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    Write-Host "built: $Output"
    exit 0
}

$VsDevCmd = Find-VsDevCmd
if ($VsDevCmd) {
    $Command = "call `"$VsDevCmd`" -arch=x64 -host_arch=x64 && cl /nologo /O2 /W4 /TC /utf-8 $ProfileDefine /Fo:$OutDir\ $($Sources[0]) $($Sources[1]) /Fe:$Output"
    & cmd.exe /d /s /c $Command
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    Write-Host "built with Visual Studio: $Output"
    exit 0
}

throw "Visual Studio C compiler was not found."
