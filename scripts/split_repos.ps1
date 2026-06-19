# Split HERMES monorepo into Phase 10 open-source repos (dry-run by default).
param(
    [string]$OutputDir = "..\hermes-split",
    [switch]$Execute
)

$Root = Split-Path -Parent $PSScriptRoot
$Repos = @{
    "hermes-runtime" = @("hardware", "device-os", "runtime", "platform", "security", "tests")
    "hermes-sdk"     = @("sdk")
    "hermes-dashboard" = @("dashboard", "tools\control-plane")
    "hermes-examples"  = @("examples")
}

Write-Host "HERMES repo split plan"
Write-Host "===================="
foreach ($name in $Repos.Keys) {
    $dest = Join-Path $OutputDir $name
    Write-Host "`n$name -> $dest"
    foreach ($dir in $Repos[$name]) {
        $src = Join-Path $Root $dir
        if (Test-Path $src) {
            Write-Host "  + $dir"
            if ($Execute) {
                New-Item -ItemType Directory -Force -Path $dest | Out-Null
                Copy-Item -Recurse -Force $src (Join-Path $dest (Split-Path $dir -Leaf))
            }
        }
    }
    if ($Execute) {
        Copy-Item (Join-Path $Root "LICENSE") $dest -ErrorAction SilentlyContinue
        Copy-Item (Join-Path $Root "README.md") $dest -ErrorAction SilentlyContinue
    }
}

if (-not $Execute) {
    Write-Host "`nDry run. Re-run with -Execute to copy files."
} else {
    Write-Host "`nSplit complete under $OutputDir"
}
