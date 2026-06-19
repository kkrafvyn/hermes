# Tag HERMES v0.1.0-alpha release
param([switch]$Push)

$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

if (-not (Test-Path ".git")) {
    Write-Host "Not a git repository. Initialize first: git init"
    exit 1
}

$tag = "v0.1.0-alpha"
git tag -a $tag -m "HERMES v0.1.0 Alpha — distributed MCU runtime platform"
Write-Host "Tagged $tag"
git tag -l "v0.1*"

if ($Push) {
    git push origin $tag
    Write-Host "Pushed tag to origin"
} else {
    Write-Host "Run with -Push to push tag to remote"
}
