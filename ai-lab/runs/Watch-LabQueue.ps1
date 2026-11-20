# Runs queued siege lab requests on this PC. Task Scheduler starts it every few minutes (Register-LabWatcher.ps1).
# Several lab PCs can run the same queue at once: each launch is claimed before it runs, so no two PCs run it.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File ai-lab\runs\Watch-LabQueue.ps1            # one pass over the queue
#   powershell -NoProfile -ExecutionPolicy Bypass -File ai-lab\runs\Watch-LabQueue.ps1 -DryRun    # show what it would run
#
# Each pass: stop at once if paused (%LOCALAPPDATA%\cp-ai-lab\PAUSE exists), if another pass is running on this PC, or
# if Civ V is open (you are playing). Otherwise, one launch at a time:
#   1. pull ai/v2 into the lab checkout (if that brought a new version of this script, stop: the next pass runs it);
#   2. write done markers for requests whose launches have all finished (ai-lab\runs\done\<id>.json);
#   3. take the first launch, oldest request first, that has no result yet, may run on this PC (a launch with a
#      "host" field runs only on that PC) and is not claimed by another PC (a claim older than -ClaimTimeoutMinutes
#      with no result is released; a claim by this PC with no result is from a run that died, and is taken again);
#   4. claim it: commit ai-lab\runs\launches\<id>\<label>.claim.json and push. If the push is refused because another
#      PC pushed first, drop the claim and start again at 1. Git lets only one of two competing pushes in;
#   5. run it with ai-lab\siege\Run-SiegeLab.ps1, check the pack's stock DLL is back, write the launch's result
#      (ai-lab\runs\launches\<id>\<label>.json, with this PC's name) and push it with the results.
# Request and marker formats: README.md next to this script.
#
# A request whose DLL build isn't published yet is left for a later pass, and marked failed after -ReleaseWaitMinutes.
# Nothing here changes the pack except through Run-SiegeLab.ps1, which always removes the lab, restores the stock DLL
# and closes Civ V. Works in Windows PowerShell 5.1.
param(
    [string]$Repo = (Join-Path $env:USERPROFILE "cp-ai-lab"),
    [string]$Branch = "ai/v2",
    [int]$ReleaseWaitMinutes = 90,
    [int]$ClaimTimeoutMinutes = 240,  # a claim this old with no result is released (a launch takes 3-20 minutes)
    [string]$CivDir = "",             # default: civDir in ai-lab\siege\hosts\<COMPUTERNAME>.json, else the usual Steam folder
    [string]$GitHubRepo = "treezeegaga/Community-Patch-DLL",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"
$WatcherVersion = 3
$Me = $env:COMPUTERNAME
$state = Join-Path $env:LOCALAPPDATA "cp-ai-lab"
New-Item -ItemType Directory -Force $state | Out-Null
$logFile = Join-Path $state "watcher.log"
$lockFile = Join-Path $state "watcher.lock"
$pauseFile = Join-Path $state "PAUSE"
$queueDir = Join-Path $Repo "ai-lab\runs\queue"
$doneDir = Join-Path $Repo "ai-lab\runs\done"
$launchRoot = Join-Path $Repo "ai-lab\runs\launches"
$resultsDir = Join-Path $Repo "ai-lab\siege\results"
$utf8 = New-Object Text.UTF8Encoding($false)
$statusFile = Join-Path $Repo "ai-lab\runs\status\$Me.json"
$lastStatusFile = Join-Path $state "last-status.json"

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
function Get-Ahead { return ((& git -C $Repo rev-list --count "origin/$Branch..HEAD") -as [int]) }
function Write-Json([string]$path, $obj) {
    New-Item -ItemType Directory -Force (Split-Path $path) | Out-Null
    [IO.File]::WriteAllText($path, ([pscustomobject]$obj | ConvertTo-Json -Depth 6), $utf8)
}
function Read-Json([string]$path) {
    if (-not (Test-Path $path)) { return $null }
    try { return (Get-Content $path -Raw | ConvertFrom-Json) } catch { return $null }
}
function Get-UnixNow { return [DateTimeOffset]::UtcNow.ToUnixTimeSeconds() }
function Get-RepoPath([string]$full) { return $full.Substring($Repo.Length).TrimStart('\').Replace('\', '/') }

# The Civ V folder: -CivDir, else this PC's lab calibration file, else the usual Steam folder.
if (-not $CivDir) {
    $hostCfg = Read-Json (Join-Path $Repo "ai-lab\siege\hosts\$Me.json")
    if ($hostCfg -and $hostCfg.civDir) { $CivDir = $hostCfg.civDir }
    else { $CivDir = "C:\Program Files (x86)\Steam\steamapps\common\Sid Meier's Civilization V" }
}
$packDir = Join-Path $CivDir "Assets\DLC\MP_CPLeaderheads"

$script:ReleaseCache = @{}
$script:WaitLogged = @()
function Test-Release([string]$tag) {
    if ($script:ReleaseCache.ContainsKey($tag)) { return $script:ReleaseCache[$tag] }
    [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
    $ok = $true
    try { Invoke-WebRequest -Method Head -UseBasicParsing -Uri "https://github.com/$GitHubRepo/releases/download/$tag/CvGameCore_Expansion2.dll" | Out-Null }
    catch { $ok = $false }
    if ($ok) { $script:ReleaseCache[$tag] = $true }   # a missing one is asked again next time
    return $ok
}
function Get-StockDllCheck {
    # Compares the pack's DLL(s) with the stock copy Set-LabDll.ps1 saved (no saved copy yet = nothing to compare).
    $manifest = Join-Path $state "stock-dll\manifest.csv"
    if (-not (Test-Path $manifest)) { return "no stock copy saved yet" }
    foreach ($s in @(Import-Csv $manifest)) {
        $path = Join-Path $packDir $s.RelativePath
        if (-not (Test-Path $path)) { return "missing: $path" }
        if ((Get-FileHash $path -Algorithm SHA256).Hash -ne $s.Sha256) { return "differs: $path" }
    }
    return "ok"
}
# The first 16 hex digits of the pack's DLL, to see that every lab PC has the same stock pack.
function Get-PackDllHash {
    $dll = Get-ChildItem $packDir -Recurse -Filter "CvGameCore_Expansion2.dll" -File -ErrorAction SilentlyContinue | Sort-Object FullName | Select-Object -First 1
    if (-not $dll) { return "" }
    return (Get-FileHash $dll.FullName -Algorithm SHA256).Hash.Substring(0, 16)
}

# Pushes results (paths no other PC writes). Rebase keeps other PCs' new commits.
function Push-Results([string]$message) {
    foreach ($dir in @("ai-lab/siege/results", "ai-lab/runs/launches")) {
        if (Test-Path (Join-Path $Repo $dir)) { Invoke-Git @("add", "-f", $dir) | Out-Null }
    }
    $staged = & git -C $Repo diff --cached --name-only
    if ($staged) { Invoke-Git @("commit", "-q", "-m", $message) | Out-Null }
    for ($i = 1; $i -le 4; $i++) {
        try { Invoke-Git @("pull", "-q", "--rebase", "origin", $Branch) | Out-Null; Invoke-Git @("push", "-q", "origin", "HEAD:$Branch") | Out-Null; return $true }
        catch {
            try { & git -C $Repo rebase --abort 2>$null | Out-Null } catch {}
            Write-Log "  push attempt $i failed: $($_.Exception.Message)"; Start-Sleep (15 * $i)
        }
    }
    return $false
}
# Commits one file another PC may also write (a claim, a done marker) and pushes it on its own. If the push is refused
# (another PC got there first, or the network failed), the commit is dropped and $false returned; the caller looks
# again after the next pull. Needs nothing else unpushed, so dropping the commit loses nothing.
function Push-Contested([string]$path, [string]$message) {
    if ((Get-Ahead) -gt 0) { Remove-Item $path -ErrorAction SilentlyContinue; return $false }
    Invoke-Git @("add", "-f", (Get-RepoPath $path)) | Out-Null
    Invoke-Git @("commit", "-q", "-m", $message) | Out-Null
    try { Invoke-Git @("push", "-q", "origin", "HEAD:$Branch") | Out-Null; return $true }
    catch {
        Write-Log "  push of '$message' refused; dropping it ($($_.Exception.Message))"
        try { Invoke-Git @("fetch", "-q", "origin", "+refs/heads/${Branch}:refs/remotes/origin/${Branch}") | Out-Null } catch {}
        Invoke-Git @("reset", "-q", "--hard", "origin/$Branch") | Out-Null
        return $false
    }
}
function Sync-Repo {
    # Explicit refspec: a clone made with another branch only would otherwise leave origin/<branch> stale, and the
    # unpushed-commit count (Get-Ahead) then blocks every claim (BOX, 2026-11-19 5:09-8:19 PM).
    Invoke-Git @("fetch", "-q", "origin", "+refs/heads/${Branch}:refs/remotes/origin/${Branch}") | Out-Null
    try { Invoke-Git @("pull", "-q", "--rebase", "origin", $Branch) | Out-Null }
    catch {
        $pullError = $_.Exception.Message
        # "no rebase in progress" is an error of its own and must not hide why the pull failed.
        try { & git -C $Repo rebase --abort 2>$null | Out-Null } catch {}
        # Watcher v3: heal it when that loses nothing. With no local commits, everything this PC made is either pushed or
        # an untracked result file (which a reset keeps), so the checkout is put back to the branch and pulled again.
        if ((Get-Ahead) -eq 0) {
            Write-Log "HEAL: pull failed ($pullError); resetting the lab checkout to origin/$Branch (no local commits to lose)"
            try { Invoke-Git @("reset", "-q", "--hard", "origin/$Branch") | Out-Null; Invoke-Git @("pull", "-q", "--rebase", "origin", $Branch) | Out-Null; return }
            catch { $pullError = "$pullError; after the reset: $($_.Exception.Message)" }
        }
        throw "pull failed, nothing run: $pullError"
    }
}
# Watcher v3: what this PC's watcher is doing, in ai-lab/runs/status/<COMPUTERNAME>.json, so a stall can be read from the
# repository. Pushed when the state changes, and hourly while it is anything but idle. Only this PC writes the file.
function Publish-Status([string]$st, [string]$detail) {
    if ($DryRun) { return }
    $now = Get-UnixNow
    $prev = $null
    if (Test-Path $lastStatusFile) { try { $prev = Read-Json $lastStatusFile } catch {} }
    if ($prev -and $prev.state -eq $st -and $prev.detail -eq $detail -and ($st -eq "idle" -or ($now - [long]$prev.updatedUnix) -lt 3600)) { return }
    $obj = [ordered]@{ host = $Me; watcherVersion = $WatcherVersion; state = $st; detail = $detail; updated = (Get-Date).ToString("s")
        updatedUnix = $now; paused = (Test-Path $pauseFile); civRunning = (Test-CivRunning) }
    try {
        New-Item -ItemType Directory -Force (Split-Path $statusFile) | Out-Null
        Write-Json $statusFile $obj
        Invoke-Git @("add", "-f", (Get-RepoPath $statusFile)) | Out-Null
        $staged = & git -C $Repo diff --cached --name-only
        if ($staged) { Invoke-Git @("commit", "-q", "-m", "Lab PC $Me watcher: $st") | Out-Null }
        Invoke-Git @("pull", "-q", "--rebase", "origin", $Branch) | Out-Null
        Invoke-Git @("push", "-q", "origin", "HEAD:$Branch") | Out-Null
        Write-Json $lastStatusFile $obj
    }
    catch {
        try { & git -C $Repo rebase --abort 2>$null | Out-Null } catch {}
        Write-Log "  status '$st' not pushed: $($_.Exception.Message)"
    }
}

# ---------- Requests ----------
function Get-LaunchPaths([string]$id, [string]$label) {
    $dir = Join-Path $launchRoot $id
    return [pscustomobject]@{ Claim = (Join-Path $dir "$label.claim.json"); Result = (Join-Path $dir "$label.json") }
}
function Read-Request($file) {
    $id = $file.BaseName
    $req = Get-Content $file.FullName -Raw | ConvertFrom-Json
    if ($req.id -and $req.id -ne $id) { throw "the id inside ($($req.id)) differs from the file name ($id)" }
    $launches = @($req.launches)
    if ($launches.Count -eq 0) { throw "no launches" }
    $labels = @{}
    foreach ($l in $launches) {
        if ($l.label -notmatch '^[A-Za-z0-9_.-]+$') { throw "bad label '$($l.label)'" }
        if ($labels.ContainsKey($l.label)) { throw "label '$($l.label)' appears twice" }
        $labels[$l.label] = $true
        if ($l.release -and $l.release -notmatch '^ab-[A-Za-z0-9.-]+$') { throw "bad release '$($l.release)'" }
        if ($l.newAI -and @("none", "attacker", "defender", "both") -notcontains $l.newAI) { throw "bad newAI '$($l.newAI)'" }
        if ($l.host -and $l.host -notmatch '^[A-Za-z0-9_-]+$') { throw "bad host '$($l.host)'" }
        foreach ($n in @($l.only)) { if ($n -and $n -notmatch '^[A-Za-z0-9_]+$') { throw "bad scenario name '$n'" } }
    }
    $queuedAt = (& git -C $Repo log -1 --format=%ct -- "ai-lab/runs/queue/$($file.Name)") -as [long]
    $commit = (& git -C $Repo log -1 --format=%h -- "ai-lab/runs/queue/$($file.Name)") -join ""
    return [pscustomobject]@{ Id = $id; File = $file; Launches = $launches; QueuedAt = $queuedAt; Commit = $commit }
}
function Write-DoneMarker($r, [string]$status, [string]$message, $entries) {
    $hosts = @($entries | ForEach-Object { $_.host } | Where-Object { $_ } | Sort-Object -Unique)
    $started = @($entries | ForEach-Object { $_.started } | Where-Object { $_ } | Sort-Object)
    $finished = @($entries | ForEach-Object { $_.finished } | Where-Object { $_ } | Sort-Object)
    $marker = [ordered]@{
        id = $r.Id; status = $status; message = $message; watcherVersion = $WatcherVersion
        host = ($hosts -join ", "); hosts = $hosts; markedBy = $Me
        started = $(if ($started.Count) { $started[0] } else { (Get-Date).ToString("s") })
        finished = $(if ($finished.Count) { $finished[-1] } else { (Get-Date).ToString("s") })
        requestCommit = $r.Commit
        stockDll = ((@($entries | ForEach-Object { "$($_.host): $($_.stockDll)" } | Sort-Object -Unique)) -join "; ")
        launches = @($entries)
    }
    $path = Join-Path $doneDir "$($r.Id).json"
    Write-Json $path $marker
    if (Push-Contested $path "Lab run $($r.Id): $status") { Write-Log "PUSHED done marker $($r.Id) ($status)" }
}
# Finishes what can be finished and returns the requests that still have launches to run, oldest first.
function Get-OpenRequests {
    $open = @()
    $files = @(Get-ChildItem $queueDir -Filter "*.json" -File -ErrorAction SilentlyContinue | Sort-Object Name |
        Where-Object { -not (Test-Path (Join-Path $doneDir $_.Name)) })
    foreach ($file in $files) {
        $r = $null
        try { $r = Read-Request $file }
        catch {
            if (-not $DryRun) { Write-DoneMarker ([pscustomobject]@{ Id = $file.BaseName; Commit = "" }) "failed" $_.Exception.Message @() }
            Write-Log "$($file.BaseName) is not a valid request: $($_.Exception.Message)"
            continue
        }
        $entries = @(); $allDone = $true; $anyStarted = $false
        foreach ($l in $r.Launches) {
            $p = Get-LaunchPaths $r.Id $l.label
            $res = Read-Json $p.Result
            if ($res) { $entries += $res; $anyStarted = $true } else { $allDone = $false; if (Test-Path $p.Claim) { $anyStarted = $true } }
        }
        if ($allDone) {
            if (-not $DryRun) {
                $failed = @($entries | Where-Object { $_.exitCode -ne 0 })
                if ($failed.Count -gt 0) { Write-DoneMarker $r "failed" "a launch ended with a non-zero exit code; see its run folder" $entries }
                else { Write-DoneMarker $r "done" "" $entries }
            }
            continue
        }
        # Every build the request names must be published.
        $missing = @($r.Launches | Where-Object { $_.release } | ForEach-Object { $_.release } | Sort-Object -Unique | Where-Object { -not (Test-Release $_) })
        if ($missing.Count -gt 0) {
            $ageMin = if ($r.QueuedAt) { ((Get-UnixNow) - $r.QueuedAt) / 60 } else { 0 }
            if ($ageMin -lt $ReleaseWaitMinutes -or $anyStarted) {
                # Said once per request and build set, not on every look (a pass can look many times a minute).
                $waitKey = "$($r.Id) $($missing -join ',')"
                if ($script:WaitLogged -notcontains $waitKey) {
                    $script:WaitLogged += $waitKey
                    Write-Log "$($r.Id) waits for build(s) $($missing -join ', ') (queued $([int]$ageMin) min ago); further looks are not logged"
                }
                continue
            }
            if (-not $DryRun) { Write-DoneMarker $r "failed" "build(s) $($missing -join ', ') not published after $ReleaseWaitMinutes minutes" $entries }
            continue
        }
        $open += $r
    }
    return ,$open
}
# The next launch this PC may run, or $null.
function Find-NextLaunch($open) {
    $trustedFile = Join-Path $Repo "ai-lab\runs\trusted-hosts.txt"
    $trusted = (Test-Path $trustedFile) -and (@(Get-Content $trustedFile | ForEach-Object { $_.Trim() }) -contains $Me)
    foreach ($r in $open) {
        foreach ($l in $r.Launches) {
            if ($l.host -and $l.host -ne $Me) { continue }
            # Launches that name no host go only to PCs listed in ai-lab/runs/trusted-hosts.txt (a PC joins the list once it
            # has passed the two-PC check); any other PC runs only launches pinned to it.
            if (-not $l.host -and -not $trusted) { continue }
            $p = Get-LaunchPaths $r.Id $l.label
            if (Test-Path $p.Result) { continue }
            $claim = Read-Json $p.Claim
            $note = ""
            if (Test-Path $p.Claim) {
                $claimHost = if ($claim) { $claim.host } else { "?" }
                $claimedAt = if ($claim -and $claim.claimedAtUnix) { [long]$claim.claimedAtUnix } else { (& git -C $Repo log -1 --format=%ct -- (Get-RepoPath $p.Claim)) -as [long] }
                $ageMin = if ($claimedAt) { ((Get-UnixNow) - $claimedAt) / 60 } else { 0 }
                if ($claimHost -eq $Me) { $note = "taking back this PC's own claim (its run ended without a result)" }
                elseif ($ageMin -ge $ClaimTimeoutMinutes) { $note = "releasing $claimHost's claim from $([int]$ageMin) minutes ago (no result)" }
                else { continue }
            }
            return [pscustomobject]@{ Request = $r; Launch = $l; Paths = $p; Note = $note }
        }
    }
    return $null
}
function Invoke-Launch($l) {
    $runner = Join-Path $Repo "ai-lab\siege\Run-SiegeLab.ps1"
    $entry = [ordered]@{ label = $l.label; host = $Me; exitCode = $null; attempts = 0; runFolder = ""; summary = @(); assertions = 0; labErrors = 0
        started = (Get-Date).ToString("s"); finished = $null; stockDll = ""; packDll = ""; watcherVersion = $WatcherVersion }
    for ($attempt = 1; $attempt -le 2; $attempt++) {
        if (Test-CivRunning) { throw "Civ V was opened; stopped before launch $($l.label)" }
        $entry.attempts = $attempt
        $preStart = $false
        $began = Get-Date
        $a = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $runner, "-Label", $l.label, "-CivDir", $CivDir)
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
    return $entry
}

# ---------- One pass ----------
if (Test-Path $pauseFile) { Publish-Status "paused" "PAUSE file present"; exit 0 }
if (Test-Path $lockFile) {
    $otherPid = (Get-Content $lockFile -ErrorAction SilentlyContinue | Select-Object -First 1) -as [int]
    if ($otherPid -and (Get-Process -Id $otherPid -ErrorAction SilentlyContinue)) { exit 0 }
}
if (Test-CivRunning) { Publish-Status "civ-open" "Civ V is running; launches wait"; exit 0 }   # you are playing; try again next pass
Set-Content $lockFile $PID
$selfHash = (Get-FileHash $PSCommandPath -Algorithm SHA256).Hash

try {
    for ($round = 1; $round -le 100; $round++) {
        try { Sync-Repo } catch { Write-Log "PULL FAILED: $($_.Exception.Message)"; Publish-Status "pull-failed" $_.Exception.Message; exit 1 }
        if ((Get-FileHash $PSCommandPath -Algorithm SHA256).Hash -ne $selfHash) { Write-Log "The watcher script was updated; the next pass runs the new version."; break }
        if ((Get-Ahead) -gt 0) {
            if ($DryRun) { Write-Log "DRY RUN: $(Get-Ahead) local commit(s) not pushed yet" }
            elseif (-not (Push-Results "Lab results (retrying an earlier push)")) { Write-Log "PUSH STILL FAILING"; Publish-Status "push-failed" "results committed here but not pushed"; exit 1 }
        }
        New-Item -ItemType Directory -Force $doneDir | Out-Null

        $open = Get-OpenRequests
        if ($DryRun) {
            foreach ($r in $open) { foreach ($l in $r.Launches) {
                $p = Get-LaunchPaths $r.Id $l.label
                $what = "would run here"
                if (Test-Path $p.Claim) { $what = "is claimed by $((Read-Json $p.Claim).host)" }
                if ($l.host -and $l.host -ne $Me) { $what = "is for $($l.host)" }
                if (Test-Path $p.Result) { $what = "has a result" }
                Write-Log "DRY RUN: $($r.Id) / $($l.label) $what"
            } }
            $next = Find-NextLaunch $open
            if ($next) { Write-Log "DRY RUN: this PC ($Me) would take $($next.Request.Id) / $($next.Launch.label) first $($next.Note)" }
            else { Write-Log "DRY RUN: nothing for this PC ($Me) to run" }
            break
        }
        $next = Find-NextLaunch $open
        if (-not $next) { Publish-Status "idle" ""; break }
        if (Test-CivRunning) { break }

        $id = $next.Request.Id; $label = $next.Launch.label
        if ($next.Note) { Write-Log "$id / ${label}: $($next.Note)" }
        $claim = [ordered]@{ id = $id; label = $label; host = $Me; claimedAt = (Get-Date).ToString("s"); claimedAtUnix = (Get-UnixNow)
            watcherVersion = $WatcherVersion; note = $next.Note }
        Write-Json $next.Paths.Claim $claim
        if (-not (Push-Contested $next.Paths.Claim "Claim $id / $label on $Me")) {   # someone else was first: look again
            Write-Log "  claim of $id / $label not taken; looking again"
            Start-Sleep 20
            continue
        }

        Write-Log "RUN $id / $label"
        $entry = $null
        try { $entry = Invoke-Launch $next.Launch }
        catch {
            Write-Log "  $id / $label FAILED: $($_.Exception.Message)"
            $entry = [ordered]@{ label = $label; host = $Me; exitCode = -1; attempts = 0; runFolder = ""; summary = @($_.Exception.Message)
                assertions = 0; labErrors = 0; started = $claim.claimedAt; finished = $null; stockDll = ""; packDll = ""; watcherVersion = $WatcherVersion }
        }

        # Big AI log folders go in as one zip each.
        Get-ChildItem $resultsDir -Directory -Filter "*-logs" | ForEach-Object {
            $mb = (Get-ChildItem $_.FullName -Recurse -File | Measure-Object Length -Sum).Sum / 1MB
            if ($mb -gt 50) { Compress-Archive -Path "$($_.FullName)\*" -DestinationPath "$($_.FullName).zip" -Force; Remove-Item $_.FullName -Recurse -Force }
        }
        # Whatever happened, the pack must have its stock DLL.
        $check = Get-StockDllCheck
        if ($check -ne "ok" -and $check -ne "no stock copy saved yet") {
            try { & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Repo "ai-lab\siege\Set-LabDll.ps1") -Restore -CivDir $CivDir | Out-Null; $check = "restored by the watcher ($check); now " + (Get-StockDllCheck) }
            catch { $check = "NOT RESTORED ($check): $($_.Exception.Message)" }
        }
        $entry.stockDll = $check
        $entry.packDll = Get-PackDllHash
        $entry.finished = (Get-Date).ToString("s")
        Write-Json $next.Paths.Result $entry
        if (Push-Results "Lab run $id / ${label} on ${Me}: exit $($entry.exitCode)") { Write-Log "PUSHED $id / $label (exit $($entry.exitCode)); stock DLL: $check" }
        else { Write-Log "COULD NOT PUSH $id / $label; the next pass retries"; break }
        if (Test-CivRunning) { break }
    }
}
catch { Write-Log "WATCHER ERROR: $($_.Exception.Message)"; Publish-Status "error" $_.Exception.Message; exit 1 }
finally { Remove-Item $lockFile -ErrorAction SilentlyContinue }
exit 0
