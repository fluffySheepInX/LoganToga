#
# Tools/check_shaders.ps1
#   3D_Pi 内の全 HLSL ピクセルシェーダを fxc.exe で構文チェックする。
#   実行例:
#       powershell -ExecutionPolicy Bypass -File Tools\check_shaders.ps1
#

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$shaderDir = Join-Path $repoRoot "3D_Pi\App\example\shader\hlsl"

# fxc.exe の場所を解決 (Windows SDK にインストールされる)
$fxc = Get-ChildItem "C:\Program Files (x86)\Windows Kits\10\bin" -Recurse -Filter fxc.exe -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -match "x64\\fxc\.exe$" } |
    Sort-Object FullName -Descending | Select-Object -First 1

if (-not $fxc) {
    Write-Error "fxc.exe not found under Windows Kits. Install Windows 10/11 SDK."
}

Write-Host "Using $($fxc.FullName)"
Write-Host "Scanning $shaderDir"
Write-Host ""

$shaders = Get-ChildItem $shaderDir -Filter *.hlsl
$hadError = $false

foreach ($s in $shaders) {
    Write-Host "==> $($s.Name)" -ForegroundColor Cyan
    & $fxc.FullName /nologo /T ps_5_0 /E PS $s.FullName | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "    FAILED (running again with full output):" -ForegroundColor Red
        & $fxc.FullName /nologo /T ps_5_0 /E PS $s.FullName
        $hadError = $true
    }
    else {
        Write-Host "    OK" -ForegroundColor Green
    }
}

if ($hadError) { exit 1 } else { Write-Host "All shaders compiled." -ForegroundColor Green }
