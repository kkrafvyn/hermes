# Connect STM32 on COM6 to HERMES cluster
param(
    [string]$Port = "COM6",
    [int]$NodeId = 0,
    [switch]$Flash,
    [switch]$SkipServer
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
. (Join-Path $PSScriptRoot "pio_env.ps1")
$Pio = "C:\Users\Donfrass\AppData\Local\Programs\Python\Python313\Scripts\pio.exe"
if (-not (Test-Path $Pio)) { $Pio = "pio" }

Write-Host "STM32 HERMES bring-up on $Port (node $NodeId)"

# Check port exists
$ports = [System.IO.Ports.SerialPort]::GetPortNames()
python -c "import serial.tools.list_ports; print([p.device for p in __import__('serial').tools.list_ports.comports()])"
if ($ports -notcontains $Port) {
    Write-Host "WARNING: $Port not in Windows port list yet."
    Write-Host "  Install Silicon Labs CP210x driver if using UART adapter."
    Write-Host "  Or install ST-Link driver for Nucleo boards."
}

if ($Flash) {
    Write-Host "Building + flashing stm32h7 firmware..."
    Push-Location (Join-Path $Root "hardware\stm32h7-pio")
    & $Pio run -e stm32h7 -t upload --upload-port $Port
    Pop-Location
    Start-Sleep -Seconds 3
}

if (-not $SkipServer) {
    $existing = Get-NetTCPConnection -LocalPort 8787 -ErrorAction SilentlyContinue
    if (-not $existing) {
        Write-Host "Starting control plane..."
        Start-Process python -ArgumentList "$Root\tools\control-plane\server.py" -WorkingDirectory $Root -WindowStyle Hidden
        Start-Sleep -Seconds 3
    }
}

Write-Host "Starting serial bridge -> http://localhost:8787"
python "$Root\tools\serial-bridge\bridge.py" $Port --register --node-id $NodeId --name STM32H7 --node-type stm32
