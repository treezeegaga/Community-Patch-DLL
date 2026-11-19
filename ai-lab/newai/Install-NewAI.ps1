# Puts the new AI into the installed pack for your own games (single player or multiplayer), or takes it out again.
#
#   powershell -ExecutionPolicy Bypass -File ai-lab\newai\Install-NewAI.ps1 -Release ab-1234567
#   powershell -ExecutionPolicy Bypass -File ai-lab\newai\Install-NewAI.ps1 -Remove
#
# Installing does three things:
#   1. puts the test build's game DLL (GitHub release ab-<commit>) in the pack, with Set-LabDll.ps1 (the stock DLL is
#      kept and checked by its hash, as for lab runs);
#   2. adds a "New AI (test build)" checkbox to the game setup's options (Advanced Setup in single player, the game
#      options in a multiplayer lobby): two small files copied into the pack and two lines added to its .Civ5Pkg;
#   3. pauses this PC's lab watcher (%LOCALAPPDATA%\cp-ai-lab\PAUSE), so a lab run can't swap the DLL under you.
# With the box ticked, every AI civ in that game uses the new AI; unticked, the game plays as stock.
# -Remove undoes all three. Run it before going back to normal play.
# In multiplayer every player's PC needs the same release installed (the script prints the DLL's hash to compare).
# Civ V must be closed. Works in Windows PowerShell 5.1. Steps for the multiplayer test: MP-SMOKE-TEST.md here.
param(
    [string]$Release = "",
    [switch]$Remove,
    [string]$CivDir = "C:\Program Files (x86)\Steam\steamapps\common\Sid Meier's Civilization V",
    [string]$MyGames = "$([Environment]::GetFolderPath('MyDocuments'))\My Games\Sid Meier's Civilization 5",
    [string]$PackName = "MP_CPLeaderheads"
)

$ErrorActionPreference = "Stop"
if (-not $Remove -and $Release -notmatch '^ab-[A-Za-z0-9.-]+$') { throw "-Release ab-<commit>, or -Remove" }
if (Get-Process -Name CivilizationV, CivilizationV_DX11, CivilizationV_Tablet -ErrorAction SilentlyContinue) {
    throw "Civ V is running. Close it first."
}
$pack = Join-Path $CivDir "Assets\DLC\$PackName"
if (-not (Test-Path $pack)) { throw "The pack isn't installed at $pack (use -CivDir if Civ V is elsewhere)" }
$pkgs = @(Get-ChildItem $pack -Filter "*.Civ5Pkg" -File)
if ($pkgs.Count -ne 1) { throw "Expected one .Civ5Pkg file in $pack, found $($pkgs.Count)" }
$pkg = $pkgs[0].FullName
$files = @("AILab_NewAIOption.xml", "AILab_NewAIOption_Text.xml")
$lines = @("    <GameData>AILab_NewAIOption.xml</GameData>", "    <TextData>AILab_NewAIOption_Text.xml</TextData>")
$state = Join-Path $env:LOCALAPPDATA "cp-ai-lab"
$pause = Join-Path $state "PAUSE"
$pausedByUs = Join-Path $state "PAUSE.by-Install-NewAI"
New-Item -ItemType Directory -Force $state | Out-Null

# The .Civ5Pkg is edited as text, keeping its encoding (byte order mark or not) and line endings.
$bytes = [IO.File]::ReadAllBytes($pkg)
$hasBom = $bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF
$text = [IO.File]::ReadAllText($pkg)
$nl = if ($text.Contains("`r`n")) { "`r`n" } else { "`n" }
$enc = New-Object Text.UTF8Encoding($hasBom)
# Take our lines out, wherever they are; installing puts them back.
$clean = $text
foreach ($l in $lines) { $clean = [regex]::Replace($clean, "[ \t]*" + [regex]::Escape($l.Trim()) + "[ \t]*\r?\n", "") }

# Civ V keeps a copy of the database in its cache folder; clearing it makes the game read the pack again.
function Clear-CivCache {
    $cache = Join-Path $MyGames "cache"
    if (Test-Path $cache) { Get-ChildItem $cache -Force | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue }
}

if ($Remove) {
    if ($clean -ne $text) { [IO.File]::WriteAllText($pkg, $clean, $enc) }
    foreach ($f in $files) { Remove-Item (Join-Path $pack $f) -Force -ErrorAction SilentlyContinue }
    & (Join-Path $PSScriptRoot "..\siege\Set-LabDll.ps1") -Restore -CivDir $CivDir -PackName $PackName
    Clear-CivCache
    if (Test-Path $pausedByUs) {
        Remove-Item $pause, $pausedByUs -Force -ErrorAction SilentlyContinue
        Write-Output "Lab watcher un-paused."
    }
    Write-Output "New AI removed: stock DLL back, no New AI checkbox."
    return
}

# Before anything is changed, a copy of the .Civ5Pkg as the pack had it (kept once, never overwritten).
$backup = Join-Path $state "pkg-backup"
New-Item -ItemType Directory -Force $backup | Out-Null
$backupFile = Join-Path $backup (Split-Path $pkg -Leaf)
if (-not (Test-Path $backupFile)) { [IO.File]::WriteAllText($backupFile, $clean, $enc) }

if (-not (Test-Path $pause)) {
    New-Item -ItemType File $pause | Out-Null
    New-Item -ItemType File $pausedByUs | Out-Null
    Write-Output "Lab watcher paused while the new AI is installed."
}

$i = $clean.IndexOf("</Gameplay>")
if ($i -lt 0) { throw "No </Gameplay> section in $pkg; the checkbox can't be added this way. Nothing was changed but the pause." }
& (Join-Path $PSScriptRoot "..\siege\Set-LabDll.ps1") -Release $Release -CivDir $CivDir -PackName $PackName
foreach ($f in $files) { Copy-Item (Join-Path $PSScriptRoot $f) (Join-Path $pack $f) -Force }
# Insert at the start of the </Gameplay> line, so the indentation of that line is kept.
$lineStart = $clean.LastIndexOf("`n", $i) + 1
$new = $clean.Substring(0, $lineStart) + ($lines -join $nl) + $nl + $clean.Substring($lineStart)
[IO.File]::WriteAllText($pkg, $new, $enc)
Clear-CivCache

$dll = Join-Path $env:LOCALAPPDATA "cp-ai-lab\builds\$Release\CvGameCore_Expansion2.dll"
$hash = (Get-FileHash $dll -Algorithm SHA256).Hash.Substring(0, 16)
Write-Output ""
Write-Output "New AI $Release installed. DLL hash $hash (in multiplayer, every PC must show the same)."
Write-Output "Start Civ V; in game setup tick 'New AI (test build)'. Afterwards: Install-NewAI.ps1 -Remove"
