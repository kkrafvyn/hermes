# Manual GCC build (no CMake required)
$ErrorActionPreference = "Stop"
$Root = $PSScriptRoot
$Gcc = $null
foreach ($p in @("C:\msys64\ucrt64\bin\gcc.exe", "C:\MinGW\bin\gcc.exe")) {
    if (Test-Path $p) { $Gcc = $p; break }
}
if (-not $Gcc) { Write-Host "gcc not found"; exit 1 }

$Includes = @(
    "-I$Root/hardware/cluster/include",
    "-I$Root/device-os/include",
    "-I$Root/runtime/include",
    "-I$Root/platform/include",
    "-I$Root/security/include",
    "-I$Root/hardware/common"
)

$Out = "$Root/build/manual"
New-Item -ItemType Directory -Force -Path $Out | Out-Null

$Sources = @(
    "$Root/hardware/cluster/src/hermes_link.c",
    "$Root/hardware/cluster/src/hermes_power.c",
    "$Root/hardware/cluster/src/hermes_cluster_mgr.c",
    "$Root/hardware/cluster/src/hermes_router.c",
    "$Root/hardware/cluster/src/hermes_transport_host.c",
    "$Root/hardware/common/hermes_node_port.c",
    "$Root/runtime/src/hermes_checkpoint_io.c",
    "$Root/runtime/src/hermes_node_runtime.c",
    "$Root/security/src/hermes_hmac.c",
    "$Root/hardware/cluster/src/hermes_transport_ble.c",
    "$Root/device-os/src/hermes_freertos.c",
    "$Root/runtime/src/hermes_ota.c",
    "$Root/security/src/hermes_sha256.c",
    "$Root/device-os/src/hermes_task.c",
    "$Root/device-os/src/hermes_memory.c",
    "$Root/device-os/src/hermes_ipc.c",
    "$Root/runtime/src/hermes_runtime.c",
    "$Root/runtime/src/hermes_monitor.c",
    "$Root/runtime/src/hermes_scheduler.c",
    "$Root/runtime/src/hermes_checkpoint.c",
    "$Root/runtime/src/hermes_quota.c",
    "$Root/runtime/src/hermes_migration.c",
    "$Root/platform/src/hermes_app.c",
    "$Root/security/src/hermes_identity.c",
    "$Root/security/src/hermes_policy.c"
)

$Objs = @()
foreach ($s in $Sources) {
    $o = Join-Path $Out ("$([IO.Path]::GetFileNameWithoutExtension($s)).o")
    & $Gcc -std=c11 -Wall @Includes -c $s -o $o
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $Objs += $o
}

& $Gcc -std=c11 -Wall @Includes -o "$Out/cluster-sim.exe" $Objs "$Root/tools/cluster-sim/main.c"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $Gcc -std=c11 -Wall @Includes -o "$Out/test_platform.exe" $Objs "$Root/tests/test_platform.c"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Built: $Out/cluster-sim.exe"
Write-Host "Built: $Out/test_platform.exe"
& "$Out/cluster-sim.exe"
& "$Out/test_platform.exe"
Write-Host "test_platform exit: $LASTEXITCODE"
