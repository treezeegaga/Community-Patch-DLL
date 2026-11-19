# One-time setup: registers the lab watcher (Watch-LabQueue.ps1) in Windows Task Scheduler, or removes it.
#
#   powershell -ExecutionPolicy Bypass -File ai-lab\runs\Register-LabWatcher.ps1            # register (or update)
#   powershell -ExecutionPolicy Bypass -File ai-lab\runs\Register-LabWatcher.ps1 -Remove    # unregister
#
# The task runs as you, only while you are logged on (Civ V needs your desktop), every -EveryMinutes minutes,
# hidden, one copy at a time. See README.md next to this script. Works in Windows PowerShell 5.1.
param(
    [string]$Repo = "C:\Users\itstr\cp-ai-lab",
    [int]$EveryMinutes = 5,
    [string]$TaskName = "Civ V AI lab watcher",
    [switch]$Remove
)
$ErrorActionPreference = "Stop"

if ($Remove) {
    Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false -ErrorAction SilentlyContinue
    Write-Output "Removed the scheduled task '$TaskName' (if it existed)."
    return
}

$watcher = Join-Path $Repo "ai-lab\runs\Watch-LabQueue.ps1"
if (-not (Test-Path $watcher)) { throw "Not found: $watcher. Is the lab checkout at $Repo on ai/v2?" }

$action = New-ScheduledTaskAction -Execute "powershell.exe" `
    -Argument "-NoProfile -NonInteractive -WindowStyle Hidden -ExecutionPolicy Bypass -File `"$watcher`" -Repo `"$Repo`"" `
    -WorkingDirectory $Repo
$trigger = New-ScheduledTaskTrigger -Once -At (Get-Date).AddMinutes(1) -RepetitionInterval (New-TimeSpan -Minutes $EveryMinutes)
$settings = New-ScheduledTaskSettingsSet -MultipleInstances IgnoreNew -StartWhenAvailable -DontStopIfGoingOnBatteries `
    -AllowStartIfOnBatteries -ExecutionTimeLimit (New-TimeSpan -Hours 6)
$principal = New-ScheduledTaskPrincipal -UserId "$env:USERDOMAIN\$env:USERNAME" -LogonType Interactive -RunLevel Limited

Register-ScheduledTask -TaskName $TaskName -Action $action -Trigger $trigger -Settings $settings -Principal $principal `
    -Description "Runs queued Civ V AI lab requests from ai-lab\runs\queue (repo ai/v2). Pause: create $env:LOCALAPPDATA\cp-ai-lab\PAUSE." -Force | Out-Null
Write-Output "Registered '$TaskName': every $EveryMinutes minutes while you are logged on."
Write-Output "Log: $env:LOCALAPPDATA\cp-ai-lab\watcher.log    Pause: create $env:LOCALAPPDATA\cp-ai-lab\PAUSE    Remove: -Remove"
