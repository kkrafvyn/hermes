# Flash and run HERMES on ESP8266 (COM4 default)
param(
    [string]$Port = "COM4",
    [switch]$SkipFlash,
    [switch]$BridgeOnly
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
. (Join-Path $PSScriptRoot "pio_env.ps1")
$Project = Join-Path $Root "hardware\esp8266"

function Find-Pio {
    $pio = Get-Command pio -ErrorAction SilentlyContinue
    if ($pio) { return $pio.Source }
    $py313 = "C:\Users\Donfrass\AppData\Local\Programs\Python\Python313\Scripts\pio.exe"
    if (Test-Path $py313) { return $py313 }
    return $null
}

$Pio = Find-Pio
if (-not $Pio) {
    Write-Host "PlatformIO not found. Install: pip install platformio"
    exit 1
}

if (-not $BridgeOnly) {
    if (-not $SkipFlash) {
        Write-Host "Building and flashing ESP8266 on $Port ..."
        Push-Location $Project
        & $Pio run -e esp8266 -t upload --upload-port $Port
        if ($LASTEXITCODE -ne 0) { Pop-Location; exit $LASTEXITCODE }
        Pop-Location
        Write-Host "Flash OK. Waiting for reboot..."
        Start-Sleep -Seconds 3
    }
}

Write-Host "Starting control plane..."
$server = Start-Process -FilePath "python" -ArgumentList "$Root\tools\control-plane\server.py" `
    -WorkingDirectory $Root -PassThru -WindowStyle Hidden
Start-Sleep -Seconds 2

Write-Host "Starting serial bridge on $Port ..."
Write-Host "Dashboard: http://localhost:8787"
try {
    python "$Root\tools\serial-bridge\bridge.py" $Port --register --node-id 1 `
        --name ESP8266 --node-type esp8266
} finally {
    if ($server -and -not $server.HasExited) { Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue }
}
