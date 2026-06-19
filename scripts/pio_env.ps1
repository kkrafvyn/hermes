# HERMES PlatformIO + build cache on F: drive (keeps C: free)
$HermesRoot = if ($PSScriptRoot) { Split-Path -Parent $PSScriptRoot } else { "F:\hermes" }
$PioHome = Join-Path $HermesRoot ".platformio"
$TmpDir  = Join-Path $HermesRoot "tmp"

New-Item -ItemType Directory -Force -Path $PioHome, $TmpDir | Out-Null

$env:PLATFORMIO_CORE_DIR = $PioHome
$env:PLATFORMIO_HOME_DIR = $PioHome
$env:TMP = $TmpDir
$env:TEMP = $TmpDir
$env:PIP_CACHE_DIR = Join-Path $HermesRoot ".pip-cache"

Write-Host "PlatformIO home: $PioHome"
Write-Host "Temp dir:        $TmpDir"
