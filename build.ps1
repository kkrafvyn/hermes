# HERMES Windows build script
param(
    [switch]$Sim,
    [switch]$Test
)

$ErrorActionPreference = "Stop"
$Root = $PSScriptRoot
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + `
            [System.Environment]::GetEnvironmentVariable("Path","User")

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "CMake not found. Install: winget install Kitware.CMake"
    exit 1
}

$gcc = Get-Command gcc -ErrorAction SilentlyContinue
if (-not $gcc) {
    # Common WinLibs path
    $winlibs = Get-ChildItem "C:\Program Files\WinLibs*" -ErrorAction SilentlyContinue |
               Get-ChildItem -Recurse -Filter "gcc.exe" -ErrorAction SilentlyContinue |
               Select-Object -First 1
    if ($winlibs) {
        $env:Path = "$($winlibs.DirectoryName);$env:Path"
    } else {
        Write-Host "GCC not found. Install: winget install BrechtSanders.WinLibs.POSIX.UCRT"
        Write-Host "Using Python simulator instead: python tools/cluster-sim/sim.py"
        exit 1
    }
}

Push-Location $Root
cmake -B build -DHERMES_BUILD_SIM=ON -DHERMES_BUILD_TESTS=ON -G "MinGW Makefiles"
cmake --build build

if ($Sim) {
    & "$Root\build\tools\cluster-sim\cluster-sim.exe"
}
if ($Test) {
    ctest --test-dir build --output-on-failure
}
Pop-Location
Write-Host "Build complete."
