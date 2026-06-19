# Resume MCU builds on F: drive (sequential — do not run multiple instances)
$ErrorActionPreference = "Stop"
. "$PSScriptRoot\pio_env.ps1"
$Log = Join-Path (Split-Path $PSScriptRoot -Parent) "build\mcu_build_resume.log"
Write-Host "Logging to $Log (append)"
python "$PSScriptRoot\build_mcu_check.py" --resume --timeout 3600 2>&1 | Tee-Object -FilePath $Log -Append
