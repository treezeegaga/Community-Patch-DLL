# Reads the siege lab's lines out of Lua.log, saves them as two CSV files and prints a summary per scenario.
#
#   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Get-SiegeLabResults.ps1 [-Label stock548] [-Log <path to Lua.log>]
#
# Run it after a lab run and before starting Civ V again: the game overwrites Lua.log on every launch.
# Output: <OutDir>\<Label>-<time>.results.csv (one row per battle) and .turns.csv (one row per battle turn).
# Works in Windows PowerShell 5.1. See README.md next to this script.
param(
    [string]$MyGames = "$([Environment]::GetFolderPath('MyDocuments'))\My Games\Sid Meier's Civilization 5",
    [string]$Log = "",
    [string]$OutDir = (Join-Path $PSScriptRoot "results"),
    [string]$Label = "siege",      # goes into the file names: say what was tested, e.g. the build
    [switch]$NoSave                # only print the summary
)

$ErrorActionPreference = "Stop"
if (-not $Log) { $Log = Join-Path $MyGames "Logs\Lua.log" }
if (-not (Test-Path $Log)) { throw "No log at $Log. Is LoggingEnabled = 1 in config.ini?" }
if ($Label -notmatch '^[A-Za-z0-9_.-]+$') { throw "-Label is letters, digits, dots, dashes and underscores" }

$resultColumns = "Scenario", "Rep", "Captured", "CaptureTurn", "TurnsRun", "CityMaxHP", "MaxDamage", "FinalDamage",
    "AttackersStart", "AttackersAlive", "AttackersKilledByEnemy", "DefendersStart", "DefendersAlive",
    "DefendersKilledByEnemy", "EngagedTurns", "IdleTurns", "MinMeanDistance", "FinalMeanDistance", "GameTurn",
    "MeleeStart", "MeleeAlive"
$turnColumns = "Scenario", "Rep", "Turn", "DamageBefore", "DamageAfter", "AttackersAlive", "AttackersWithin2",
    "MeanDistance", "AttackersFought", "DefendersAlive", "Captured", "Late", "MeleeAlive"
$startColumns = "Scenario", "Rep", "GameTurn", "CityStrength", "CityMaxHP", "Attackers", "Defenders", "AttackerGoldRate"

function ConvertTo-Row([string[]]$columns, [string[]]$values) {
    $row = New-Object PSObject
    for ($i = 0; $i -lt $columns.Count; $i++) {
        $v = ""
        if ($i -lt $values.Count) { $v = $values[$i] }
        $row | Add-Member -NotePropertyName $columns[$i] -NotePropertyValue $v
    }
    return $row
}

$run = $null; $done = $false
$results = @(); $turns = @(); $starts = @{}; $notes = @()
foreach ($line in [IO.File]::ReadAllLines($Log)) {
    $at = $line.IndexOf("[SiegeLab] ")
    if ($at -lt 0) {
        if ($line -match 'SiegeLab' -and $line -match 'Error') { $notes += "LUA ERROR: $line" }
        continue
    }
    $body = $line.Substring($at + 11)
    $kind, $rest = $body -split ",", 2
    $values = @()
    if ($rest) { $values = @($rest -split ",") }
    switch ($kind) {
        "RUN" { $run = $values }
        "START" { $s = ConvertTo-Row $startColumns $values; $starts["$($s.Scenario)|$($s.Rep)"] = $s }
        "TURN" { $turns += ConvertTo-Row $turnColumns $values }
        "RESULT" { $results += ConvertTo-Row $resultColumns $values }
        "DONE" { $done = $true }
        default { $notes += "${kind}: $rest" }
    }
}

if (-not $run) { throw "No siege lab run in $Log. Was the lab installed, and was the game a new single-player game?" }
Write-Output ("Run: lab v{0}, {1} attacking {2}, map {3}x{4}, {5}, {6}, {7}" -f $run[0], $run[1], $run[2], $run[3], $run[4],
    $run[5], $run[6], $(if ($run[7] -eq "1") { "a human played the attacker" } else { "the AI played the attacker" }))
if (-not $done) { Write-Warning "The run did not finish (no DONE line): $($results.Count) battles were scored." }
foreach ($n in $notes) { Write-Warning $n }
if ($results.Count -eq 0) { return }

# Add what the START line knew about each battle to its result row.
foreach ($r in $results) {
    $s = $starts["$($r.Scenario)|$($r.Rep)"]
    $strength = ""; $goldRate = ""
    if ($s) { $strength = $s.CityStrength; $goldRate = $s.AttackerGoldRate }
    $r | Add-Member -NotePropertyName CityStrength -NotePropertyValue $strength
    $r | Add-Member -NotePropertyName AttackerGoldRate -NotePropertyValue $goldRate
}

if (-not $NoSave) {
    New-Item -ItemType Directory -Force $OutDir | Out-Null
    $stamp = Get-Date -Format "yyyyMMdd-HHmm"
    $resultsPath = Join-Path $OutDir "$Label-$stamp.results.csv"
    $turnsPath = Join-Path $OutDir "$Label-$stamp.turns.csv"
    $results | Export-Csv $resultsPath -NoTypeInformation -Encoding ASCII
    $turns | Export-Csv $turnsPath -NoTypeInformation -Encoding ASCII
    Write-Output "Saved $resultsPath"
    Write-Output "Saved $turnsPath"
    # The game's own tactical AI logs say why it did or didn't attack; keep them with the results.
    $aiLogs = @(Get-ChildItem (Split-Path $Log) -Filter "*Tactical*" -File -ErrorAction SilentlyContinue)
    if ($aiLogs.Count -gt 0) {
        $logDir = Join-Path $OutDir "$Label-$stamp-logs"
        New-Item -ItemType Directory -Force $logDir | Out-Null
        $aiLogs | Copy-Item -Destination $logDir -Force
        Copy-Item $Log $logDir -Force
        Write-Output "Saved $($aiLogs.Count) tactical AI log(s) and Lua.log to $logDir"
    }
    else { Write-Output "No tactical AI logs found (they need AILog = 1 in config.ini)." }
}

Write-Output ""
$summary = foreach ($g in ($results | Group-Object Scenario)) {
    $rows = @($g.Group)
    $captures = @($rows | Where-Object { $_.Captured -eq "1" })
    $captureTurn = ""
    if ($captures.Count -gt 0) {
        $captureTurn = "{0:N1}" -f (($captures | ForEach-Object { [double]$_.CaptureTurn } | Measure-Object -Average).Average)
    }
    # Worst damage the city took, as a share of its hit points; a captured city counts as 100%.
    $damage = ($rows | ForEach-Object {
        if ($_.Captured -eq "1") { 100.0 }
        elseif ([double]$_.CityMaxHP -gt 0) { 100.0 * [double]$_.MaxDamage / [double]$_.CityMaxHP }
        else { 0.0 }
    } | Measure-Object -Average).Average
    $idle = ($rows | ForEach-Object { [double]$_.IdleTurns } | Measure-Object -Average).Average
    $lost = ($rows | ForEach-Object { [double]$_.AttackersStart - [double]$_.AttackersAlive } | Measure-Object -Average).Average
    $melee = ""
    if ($rows[0].MeleeStart -ne "") {
        $melee = "{0:N1} of {1}" -f (($rows | ForEach-Object { [double]$_.MeleeAlive } | Measure-Object -Average).Average), $rows[0].MeleeStart
    }
    New-Object PSObject -Property ([ordered]@{
        Scenario = $g.Name
        Battles = $rows.Count
        Captured = $captures.Count
        "Capture turn" = $captureTurn
        "City damage %" = "{0:N0}" -f $damage
        "Idle turns" = "{0:N1}" -f $idle
        "Attackers lost" = "{0:N1}" -f $lost
        "Melee left" = $melee
        "City strength" = $rows[0].CityStrength
    })
}
$summary | Format-Table -AutoSize
$total = $results.Count
$taken = @($results | Where-Object { $_.Captured -eq "1" }).Count
Write-Output ("{0} of {1} battles ended in a capture." -f $taken, $total)
