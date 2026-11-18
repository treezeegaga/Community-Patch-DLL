# Installs the siege lab into the installed "CP with Animated Leaders" pack, or removes it again.
#
#   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Install-SiegeLab.ps1 [-Set siege|march] [-Only a,b] [-Repeats 2] [-Human | -Script]
#   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Install-SiegeLab.ps1 -Remove
#
# Installing copies SiegeLab.lua and SiegeLabScenarios.lua into the pack, writes SiegeLabConfig.lua (which
# scenarios to run), and adds one line to the pack's UI\InGame.lua so the game loads the lab. Nothing in the
# DLL or the database changes. While it is installed, every new single-player game is taken over by the lab,
# so run -Remove before playing. The lab refuses to start in multiplayer, but remove it anyway.
#
# Works in Windows PowerShell 5.1. See README.md next to this script.
param(
    [string]$CivDir = "C:\Program Files (x86)\Steam\steamapps\common\Sid Meier's Civilization V",
    [string]$MyGames = "$([Environment]::GetFolderPath('MyDocuments'))\My Games\Sid Meier's Civilization 5",
    [string]$PackName = "MP_CPLeaderheads",
    [string]$Set = "siege",        # which list in SiegeLabScenarios.lua to run: siege or march
    [string[]]$Only = @(),         # run only these scenario names (comma-separated); default: the whole set
    [int]$Repeats = 2,             # how many times each scenario is played
    [switch]$Human,                # you play the attacker instead of the AI (the human reference)
    [switch]$Script,               # the lab's scripted attacker plays instead of the AI (the reference in place of a human)
    [switch]$EnableLogging,        # set LoggingEnabled = 1 (the lab's results) and AILog = 1 (the game's AI logs) in config.ini
    [switch]$Remove
)

$ErrorActionPreference = "Stop"
$utf8 = New-Object Text.UTF8Encoding($false)

if (Get-Process -Name CivilizationV_DX11, CivilizationV, CivilizationV_Tablet -ErrorAction SilentlyContinue) {
    throw "Civ V is running. Close it first."
}
$pack = Join-Path $CivDir "Assets\DLC\$PackName"
if (-not (Test-Path $pack)) { throw "The pack isn't installed at $pack" }
$inGamePath = Join-Path $pack "UI\InGame.lua"
if (-not (Test-Path $inGamePath)) { throw "No UI\InGame.lua in the pack at $pack" }
$labDir = Join-Path $pack "Mods\SiegeLab"
$marker = '-- Modpacks will add "g_uiAddins[#g_uiAddins + 1] = fileName" lines below'
$addinLine = 'g_uiAddins[#g_uiAddins + 1] = "SiegeLab"'

$inGame = [IO.File]::ReadAllText($inGamePath)
if (-not $inGame.Contains($marker)) { throw "The pack's InGame.lua has no add-in marker line; was it built by Build-CPModpack.ps1?" }
# Take the lab's line out, wherever it is; installing puts it back right after the marker.
$inGame = [regex]::Replace($inGame, "\r?\n" + [regex]::Escape($addinLine), "")

if ($Remove) {
    [IO.File]::WriteAllText($inGamePath, $inGame, $utf8)
    if (Test-Path $labDir) { Remove-Item -Recurse -Force $labDir }
    Write-Output "Siege lab removed from $pack"
    return
}

if ($Set -notmatch '^[A-Za-z_][A-Za-z0-9_]*$') { throw "-Set must be a plain name such as siege or march" }
if ($Repeats -lt 1) { throw "-Repeats must be 1 or more" }
$names = @($Only | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
foreach ($n in $names) {
    if ($n -notmatch '^[A-Za-z0-9_]+$') { throw "Scenario names are letters, digits and underscores: '$n'" }
}

New-Item -ItemType Directory -Force $labDir | Out-Null
Copy-Item (Join-Path $PSScriptRoot "SiegeLab.lua") $labDir -Force
Copy-Item (Join-Path $PSScriptRoot "SiegeLabScenarios.lua") $labDir -Force

$onlyLua = ($names | ForEach-Object { '"' + $_ + '"' }) -join ", "
if ($Human -and $Script) { throw "-Human and -Script can't be combined" }
$attackerLua = "ai"
if ($Script) { $attackerLua = "script" }
$humanLua = "false"
if ($Human) { $humanLua = "true" }
$config = @"
-- Written by Install-SiegeLab.ps1 on $(Get-Date -Format "yyyy-MM-dd HH:mm"). Delete the lab with Install-SiegeLab.ps1 -Remove.
SiegeLabConfig = {
	enabled = true,
	set = "$Set",
	only = { $onlyLua },
	repeats = $Repeats,
	human = $humanLua,
	attacker = "$attackerLua",
}
"@
[IO.File]::WriteAllText((Join-Path $labDir "SiegeLabConfig.lua"), $config.Replace("`r`n", "`n").Replace("`n", "`r`n"), $utf8)

$inGame = $inGame.Replace($marker, "$marker`r`n$addinLine")
[IO.File]::WriteAllText($inGamePath, $inGame, $utf8)

# The lab reports through Lua.log, which the game only writes with logging on.
$configIni = Join-Path $MyGames "config.ini"
$loggingOn = $false
if (Test-Path $configIni) {
    $ansi = [Text.Encoding]::Default
    $ini = [IO.File]::ReadAllText($configIni, $ansi)
    $loggingOn = $ini -match '(?m)^\s*LoggingEnabled\s*=\s*1\s*$'
    if (-not $loggingOn -and $EnableLogging) {
        if ($ini -match '(?m)^\s*LoggingEnabled\s*=') {
            $ini = [regex]::Replace($ini, '(?m)^(\s*LoggingEnabled\s*=\s*)\d+', '${1}1')
            [IO.File]::WriteAllText($configIni, $ini, $ansi)
            $loggingOn = $true
            Write-Output "LoggingEnabled set to 1 in $configIni"
        }
        else { Write-Warning "No LoggingEnabled line in $configIni; set it by hand" }
    }
    # The game's own AI logs (tactical, military, operations, diplomacy) say why the AI did what it did.
    if ($EnableLogging) {
        $ini = [IO.File]::ReadAllText($configIni, $ansi)
        if ($ini -match '(?m)^\s*AILog\s*=\s*1\s*$') { Write-Output "AILog is 1 in $configIni" }
        elseif ($ini -match '(?m)^\s*AILog\s*=') {
            $ini = [regex]::Replace($ini, '(?m)^(\s*AILog\s*=\s*)\d+', '${1}1')
            [IO.File]::WriteAllText($configIni, $ini, $ansi)
            Write-Output "AILog set to 1 in $configIni"
        }
        else { Write-Warning "No AILog line in $configIni; the game's AI logs won't be written. Add 'AILog = 1' by hand" }
    }
}
if (-not $loggingOn) {
    Write-Warning "Logging is off, so the lab's results won't be written. Set LoggingEnabled = 1 in $configIni, or run again with -EnableLogging."
}

$what = "the '$Set' set"
if ($names.Count -gt 0) { $what = "$($names -join ', ') from the '$Set' set" }
$who = "the AI plays the attacker"
if ($Human) { $who = "you play the attacker" }
if ($Script) { $who = "the scripted attacker plays the attacker" }
Write-Output "Siege lab installed in $pack"
Write-Output "  Runs $what, $Repeats time(s) each; $who."
Write-Output "  Start a new single-player game (2 or more civs, Quick Combat and Quick Movement on) and click Begin."
Write-Output "  Afterwards: Get-SiegeLabResults.ps1, then Install-SiegeLab.ps1 -Remove before playing."
