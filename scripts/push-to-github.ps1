# Crea el repo privado en GitHub y sube main (requiere: gh auth login)
param(
    [string]$RepoName = "dcn-firmware",
    [string]$Visibility = "private"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

gh auth status | Out-Null

$remote = git remote get-url origin 2>$null
if ($LASTEXITCODE -eq 0) {
    Write-Host "Remote origin ya configurado: $remote"
    git push -u origin main
    exit 0
}

gh repo create $RepoName --$Visibility `
    --description "Dual-Cell-Net (DCN) — firmware Meshtastic/MeshCore para T-Deck Plus" `
    --source . --remote origin --push

Write-Host "Repositorio: https://github.com/$(gh api user -q .login)/$RepoName" -ForegroundColor Green