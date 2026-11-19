# Runs queued siege lab requests on this PC. Task Scheduler starts it every few minutes (Register-LabWatcher.ps1).
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File ai-lab\runs\Watch-LabQueue.ps1            # one pass over the queue
#   powershell -NoProfile -ExecutionPolicy Bypass -File ai-lab\runs\Watch-LabQueue.ps1 -DryRun    # show what it would run
#
# Each pass: stop at once if paused (%LOCALAPPDATA%\cp-ai-lab\PAUSE exists), if another pass is running, or if
# Civ V is open (you are playing). Otherwise pull ai/v2 into the lab checkout, take every request in
# ai-lab\runs\queue\*.json that has no marker in ai-lab\runs\done\, oldest name first, run its launches with
# ai-lab\siege\Run-SiegeLab.ps1, check the pack's stock DLL is back, write ai-lab\runs\done\<id>.json and push the
# results and the marker. Request and marker formats: README.md next to this script.
#
# A request whose DLL build isn't published yet is left for a later pass, and marked failed after -ReleaseWaitMinutes.
# Nothing here changes the pack except through Run-SiegeLab.ps1, which always removes the lab, restores the stock DLL
# and closes Civ V. Works in Windows PowerShell 5.1.
param(
    [string]$Repo = "C:\Users\itstr\cp-ai-lab",
    [string]$Branch = "ai/v2",
    [int]$ReleaseWaitMinutes = 90,
    [string]$CivDir = "C:\Program Files (x86)\Steam\steamapps\common\Sid Meier's Civilization V",
    [string]$GitHubRepo = "treezeegaga/Community-Patch-DLL",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"
$WatcherVersion = 1
$state = Join-Path $env:LOCALAPPDATA "cp-ai-lab"
New-Item -ItemType Directory -Force $state | Out-Null
$logFile = Join-Path $state "watcher.log"
$lockFile = Join-Path $state "watcher.lock"
$pauseFile = Join-Path $state "PAUSE"

function Write-Log([string]$text) {
    $line = "{0:yyyy-MM-dd HH:mm:ss}  {1}" -f (Get-Date), $text
    if ((Test-Path $logFile) -and (Get-Item $logFile).Length -gt 2MB) { Move-Item $logFile "$logFile.old" -Force }
    Add-Content -Path $logFile -Value $line
    Write-Host $line
}
function Test-CivRunning { return [bool](Get-Process CivilizationV, CivilizationV_DX11, CivilizationV_Tablet -ErrorAction SilentlyContinue) }
function Invoke-Git([string[]]$GitArgs) {
    $out = & git -C $Repo @GitArgs 2>&1
    if ($LASTEXITCODE -ne 0) { throw "git $($GitArgs -join ' ') failed: $($out -join ' / ')" }
    return $out
}
function Test-Release([string]$tag) {
    [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
    try {
        Invoke-WebRequest -Method Head -UseBasicParsing -Uri "https://github.com/$GitHubRepo/releases/download/$tag/CvGameCore_Expansion2.dll" | Out-Null
        return $true
    } catch { return $false }
}
function Get-StockDllCheck {
    # Compares the pack's DLL(s) with the stock copy Set-LabDll.ps1 saved (no saved copy yet = nothing to compare).
    $manifest = Join-Path $state "stock-dll\manifest.csv"
    if (-not (Test-Path $manifest)) { return "no stock copy saved yet" }
    $pack = Join-Path $CivDir "Assets\DLC\MP_CPLeaderheads"
    foreach ($s in @(Import-Csv $manifest)) {
        $path = Join-Path $pack $s.RelativePath
        if (-not (Test-Path $path)) { return "missing: $path" }
        if ((Get-FileHash $path -Algorithm SHA256).Hash -ne $s.Sha256) { return "differs: $path" }
    }
    return "ok"
}
function Push-Results([string]$message) {
    Invoke-Git @("add", "-f", "ai-lab/siege/results", "ai-lab/runs/done") | Out-Null
    $staged = & git -C $Repo diff --cached --name-only
    if ($staged) { Invoke-Git @("commit", "-q", "-m", $message) | Out-Null }
    for ($i = 1; $i -le 4; $i++) {
        try { Invoke-Git @("pull", "-q", "--rebase", "origin", $Branch) | Out-Null; Invoke-Git @("push", "-q", "origin", "HEAD:$Branch") | Out-Null; return $true }
        catch { Write-Log "  push attempt $i failed: $($_.Exception.Message)"; Start-Sleep (15 * $i) }
    }
    return $false
}

# ---------- One pass ----------
if (Test-Path $pauseFile) { exit 0 }
if (Test-Path $lockFile) {
    $otherPid = (Get-Content $lockFile -ErrorAction SilentlyContinue | Select-Object -First 1) -as [int]
    if ($otherPid -and (Get-Process -Id $otherPid -ErrorAction SilentlyContinue)) { exit 0 }
}
if (Test-CivRunning) { exit 0 }   # you are playing; try again next pass
Set-Content $lockFile $PID

try {
    # Pull. Rebase keeps any results commit a failed push left behind; a conflict needs a person.
    Invoke-Git @("fetch", "-q", "origin", $Branch) | Out-Null
    try { Invoke-Git @("pull", "-q", "--rebase", "origin", $Branch) | Out-Null }
    catch {
        & git -C $Repo rebase --abort 2>$null | Out-Null
        Write-Log "PULL FAILED, nothing run: $($_.Exception.Message)"
        exit 1
    }
    # Push a results commit an earlier pass couldn't push.
    $ahead = (& git -C $Repo rev-list --count "origin/$Branch..HEAD") -as [int]
    if ($ahead -gt 0) { if (-not (Push-Results "Lab results (retrying an earlier push)")) { Write-Log "PUSH STILL FAILING"; exit 1 } }

    $queueDir = Join-Path $Repo "ai-lab\runs\queue"
    $doneDir = Join-Path $Repo "ai-lab\runs\done"
    New-Item -ItemType Directory -Force $doneDir | Out-Null
    $pending = @(Get-ChildItem $queueDir -Filter "*.json" -File -ErrorAction SilentlyContinue | Sort-Object Name |
        Where-Object { -not (Test-Path (Join-Path $doneDir $_.Name)) })
    if ($pending.Count -eq 0) { exit 0 }

    foreach ($file in $pending) {
        $id = $file.BaseName
        $marker = [ordered]@{ id = $id; status = "failed"; message = ""; watcherVersion = $WatcherVersion; host = $env:COMPUTERNAME
            started = (Get-Date).ToString("s"); finished = $null; requestCommit = ""; stockDll = ""; launches = @() }
        try {
            $marker.requestCommit = (& git -C $Repo log -1 --format=%h -- "ai-lab/runs/queue/$($file.Name)") -join ""
            $req = Get-Content $file.FullName -Raw | ConvertFrom-Json
            if ($req.id -and $req.id -ne $id) { throw "the id inside ($($req.id)) differs from the file name ($id)" }
            $launches = @($req.launches)
            if ($launches.Count -eq 0) { throw "no launches" }
            foreach ($l in $launches) {
                if ($l.label -notmatch '^[A-Za-z0-9_.-]+$') { throw "bad label '$($l.label)'" }
                if ($l.release -and $l.release -notmatch '^ab-[A-Za-z0-9.-]+$') { throw "bad release '$($l.release)'" }
                if ($l.newAI -and @("none", "attacker", "defender", "both") -notcontains $l.newAI) { throw "bad newAI '$($l.newAI)'" }
                foreach ($n in @($l.only)) { if ($n -and $n -notmatch '^[A-Za-z0-9_]+$') { throw "bad scenario name '$n'" } }
            }

            # Every build the request names must be published; otherwise wait for a later pass.
            $missing = @($launches | Where-Object { $_.release } | ForEach-Object { $_.release } | Sort-Object -Unique | Where-Object { -not (Test-Release $_) })
            if ($missing.Count -gt 0) {
                $queuedAt = (& git -C $Repo log -1 --format=%ct -- "ai-lab/runs/queue/$($file.Name)") -as [long]
                $ageMin = if ($queuedAt) { ([DateTimeOffset]::UtcNow.ToUnixTimeSeconds() - $queuedAt) / 60 } else { 0 }
                if ($ageMin -lt $ReleaseWaitMinutes) { Write-Log "$id waits for build(s) $($missing -join ', ') (queued $([int]$ageMin) min ago)"; continue }
                throw "build(s) $($missing -join ', ') not published after $ReleaseWaitMinutes minutes"
            }
            if ($DryRun) { Write-Log "DRY RUN: would run $id ($($launches.Count) launch(es))"; continue }

            Write-Log "RUN $id ($($launches.Count) launch(es))"
            $runner = Join-Path $Repo "ai-lab\siege\Run-SiegeLab.ps1"
            $resultsDir = Join-Path $Repo "ai-lab\siege\results"
            $allOk = $true
            foreach ($l in $launches) {
                $entry = [ordered]@{ label = $l.label; exitCode = $null; attempts = 0; runFolder = ""; summary = @(); assertions = 0; labErrors = 0 }
                for ($attempt = 1; $attempt -le 2; $attempt++) {
                    if (Test-CivRunning) { throw "Civ V was opened while the request was running; stopped before launch $($l.label)" }
                    $entry.attempts = $attempt
                    $preStart = $false
                    $began = Get-Date
                    $a = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $runner, "-Label", $l.label)
                    if ($l.release) { $a += @("-Release", $l.release) }
                    if ($l.set) { $a += @("-Set", $l.set) }
                    if (@($l.only).Count -gt 0 -and @($l.only)[0]) { $a += @("-Only", (@($l.only) -join ",")) }
                    if ($l.repeats) { $a += @("-Repeats", [string]$l.repeats) }
                    if ($l.script) { $a += "-Script" }
                    if ($l.newAI) { $a += @("-NewAI", $l.newAI) }
                    Write-Log "  launch $($l.label), attempt $attempt"
                    & powershell.exe @a | Out-Null
                    $entry.exitCode = $LASTEXITCODE
                    $run = Get-ChildItem $resultsDir -Directory -Filter "$($l.label)-*-run" -ErrorAction SilentlyContinue |
                        Where-Object { $_.CreationTime -ge $began.AddSeconds(-5) } | Sort-Object CreationTime | Select-Object -Last 1
                    if ($run) {
                        $entry.runFolder = "ai-lab/siege/results/$($run.Name)"
                        $runLog = Join-Path $run.FullName "Run-SiegeLab.log"
                        if (Test-Path $runLog) {
                            $lines = Get-Content $runLog
                            $entry.summary = @($lines | Where-Object { $_ -match 'results: |FAILED|Run saved|stock DLL' } | ForEach-Object { $_.Substring([Math]::Min(21, $_.Length)) })
                            $entry.assertions = @($lines | Where-Object { $_ -match 'ASSERTION \d+ :' }).Count
                            $preStart = @($lines | Where-Object { $_ -match "Could not get a new game started|no '\[SiegeLab\] RUN' line" }).Count -gt 0
                        }
                        $lua = Join-Path $run.FullName "Lua.log"
                        if (Test-Path $lua) { $entry.labErrors = @(Select-String -Path $lua -SimpleMatch "[SiegeLab] ERROR").Count }
                    }
                    if ($entry.exitCode -eq 0 -or -not $preStart) { break }   # only a failure before the game started is retried
                }
                if ($entry.exitCode -ne 0) { $allOk = $false }
                $marker.launches += [pscustomobject]$entry
            }

            # Big AI log folders go in as one zip each.
            Get-ChildItem $resultsDir -Directory -Filter "*-logs" | ForEach-Object {
                $mb = (Get-ChildItem $_.FullName -Recurse -File | Measure-Object Length -Sum).Sum / 1MB
                if ($mb -gt 50) { Compress-Archive -Path "$($_.FullName)\*" -DestinationPath "$($_.FullName).zip" -Force; Remove-Item $_.FullName -Recurse -Force }
            }
            $marker.status = if ($allOk) { "done" } else { "failed" }
            if (-not $allOk) { $marker.message = "a launch ended with a non-zero exit code; see its run folder" }
        }
        catch { $marker.message = $_.Exception.Message; Write-Log "  $id FAILED: $($marker.message)" }

        if ($DryRun) { continue }
        # Whatever happened, the pack must have its stock DLL.
        $check = Get-StockDllCheck
        if ($check -ne "ok" -and $check -ne "no stock copy saved yet") {
            try { & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Repo "ai-lab\siege\Set-LabDll.ps1") -Restore | Out-Null; $check = "restored by the watcher ($check); now " + (Get-StockDllCheck) }
            catch { $check = "NOT RESTORED ($check): $($_.Exception.Message)" }
        }
        $marker.stockDll = $check
        $marker.finished = (Get-Date).ToString("s")
        $json = [pscustomobject]$marker | ConvertTo-Json -Depth 5
        [IO.File]::WriteAllText((Join-Path $doneDir $file.Name), $json, (New-Object Text.UTF8Encoding($false)))
        if (Push-Results "Lab run ${id}: $($marker.status)") { Write-Log "PUSHED $id ($($marker.status)); stock DLL: $check" }
        else { Write-Log "COULD NOT PUSH $id; the next pass retries" }
        if (Test-CivRunning) { break }
    }
}
catch { Write-Log "WATCHER ERROR: $($_.Exception.Message)"; exit 1 }
finally { Remove-Item $lockFile -ErrorAction SilentlyContinue }
exit 0
