# Puts a test build of the game core DLL into the installed pack, or puts the stock one back.
#
#   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Set-LabDll.ps1 -Release ab-03b00ce
#   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Set-LabDll.ps1 -Restore
#
# A build is a GitHub release of this fork named ab-<commit> (the build workflow makes one for every push to ai/**).
# Only the DLL file(s) named CvGameCore_Expansion2.dll inside the pack's folder are swapped; nothing else in the pack
# changes. The first time, every stock copy is saved to %LOCALAPPDATA%\cp-ai-lab\stock-dll with its SHA-256, and that
# saved copy is never overwritten, so -Restore always brings back the DLL the pack had before the lab first touched it.
# Run-SiegeLab.ps1 -Release <tag> calls this before the run and -Restore after it, whatever happens.
# Works in Windows PowerShell 5.1.
param(
    [string]$Release = "",
    [switch]$Restore,
    [string]$CivDir = "C:\Program Files (x86)\Steam\steamapps\common\Sid Meier's Civilization V",
    [string]$PackName = "MP_CPLeaderheads",
    [string]$Repo = "treezeegaga/Community-Patch-DLL"
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"
if (-not $Restore -and $Release -notmatch '^ab-[A-Za-z0-9.-]+$') { throw "-Release ab-<commit>, or -Restore" }
if (Get-Process -Name CivilizationV, CivilizationV_DX11, CivilizationV_Tablet -ErrorAction SilentlyContinue) {
    throw "Civ V is running. Close it first."
}
$pack = Join-Path $CivDir "Assets\DLC\$PackName"
if (-not (Test-Path $pack)) { throw "The pack isn't installed at $pack" }
$store = Join-Path $env:LOCALAPPDATA "cp-ai-lab\stock-dll"
$manifest = Join-Path $store "manifest.csv"   # RelativePath, Sha256 of each stock copy

function Get-Hash([string]$path) { return (Get-FileHash $path -Algorithm SHA256).Hash }
$copies = @(Get-ChildItem $pack -Recurse -Filter "CvGameCore_Expansion2.dll" -File)
if ($copies.Count -eq 0) { throw "No CvGameCore_Expansion2.dll under $pack" }

# Save the stock copies once.
if (-not (Test-Path $manifest)) {
    New-Item -ItemType Directory -Force $store | Out-Null
    $rows = foreach ($c in $copies) {
        $rel = $c.FullName.Substring($pack.Length).TrimStart('\')
        $dest = Join-Path $store $rel
        New-Item -ItemType Directory -Force (Split-Path $dest) | Out-Null
        Copy-Item $c.FullName $dest -Force
        [PSCustomObject]@{ RelativePath = $rel; Sha256 = (Get-Hash $c.FullName) }
    }
    $rows | Export-Csv $manifest -NoTypeInformation -Encoding ASCII
    Write-Output "Saved the stock DLL ($($rows.Count) cop$(if ($rows.Count -eq 1) {'y'} else {'ies'})) to $store"
}
$stock = @(Import-Csv $manifest)

if ($Restore) {
    foreach ($s in $stock) {
        $dest = Join-Path $pack $s.RelativePath
        Copy-Item (Join-Path $store $s.RelativePath) $dest -Force
        if ((Get-Hash $dest) -ne $s.Sha256) { throw "Restoring $dest failed: its hash doesn't match the saved stock copy" }
    }
    Write-Output "Stock DLL restored and checked ($($stock.Count) cop$(if ($stock.Count -eq 1) {'y'} else {'ies'}))."
    return
}

[Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
$dlDir = Join-Path $env:LOCALAPPDATA "cp-ai-lab\builds\$Release"
New-Item -ItemType Directory -Force $dlDir | Out-Null
$dll = Join-Path $dlDir "CvGameCore_Expansion2.dll"
if (-not (Test-Path $dll)) {
    $url = "https://github.com/$Repo/releases/download/$Release/CvGameCore_Expansion2.dll"
    try { Invoke-WebRequest -Uri $url -OutFile $dll -UseBasicParsing }
    catch { Remove-Item $dll -ErrorAction SilentlyContinue; throw "Couldn't download $url ($($_.Exception.Message)). The build may still be running." }
}
$want = Get-Hash $dll
foreach ($s in $stock) {
    $dest = Join-Path $pack $s.RelativePath
    Copy-Item $dll $dest -Force
    if ((Get-Hash $dest) -ne $want) { throw "Installing $Release to $dest failed: the hash doesn't match the download" }
}
Write-Output "Installed $Release ($((Get-Item $dll).Length) bytes, SHA-256 $($want.Substring(0,12))...) in $($stock.Count) place(s); stock copy kept in $store"
