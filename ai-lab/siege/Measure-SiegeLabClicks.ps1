# One-time setup on a new lab PC: measures where Run-SiegeLab.ps1 must click in Civ V's window on this PC, and saves
# it to hosts\<COMPUTERNAME>.json next to this script. Run-SiegeLab.ps1 reads that file on this PC only.
#
#   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Measure-SiegeLabClicks.ps1
#   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Measure-SiegeLabClicks.ps1 -CivDir "D:\SteamLibrary\steamapps\common\Sid Meier's Civilization V"
#
# Start Civ V yourself first (windowed, as the lab will run it) and leave it on the screen with the Continue button
# (after the intro). The script then asks for one spot at a time. For each: press Enter here, then within the countdown
# put the mouse pointer on that spot in Civ V's window and keep it still. You click through Civ V's menus yourself
# between the steps when it tells you to. Nothing is clicked or typed for you. At the end, quit Civ V.
# Works in Windows PowerShell 5.1. Setup steps: ai-lab\runs\README.md.
param(
    [string]$CivDir = "C:\Program Files (x86)\Steam\steamapps\common\Sid Meier's Civilization V",
    [int]$Seconds = 6
)
$ErrorActionPreference = "Stop"

Add-Type @"
using System; using System.Runtime.InteropServices;
public class SlMeasure {
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool GetCursorPos(out POINT p);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
}
"@
[SlMeasure]::SetProcessDPIAware() | Out-Null   # the same pixel units as Run-SiegeLab.ps1

function Get-CivWindow {
    $p = Get-Process CivilizationV -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
    if (-not $p) { throw "Civ V (CivilizationV.exe, the DirectX 9 version the lab uses) is not running. Start it, then run this again." }
    return $p.MainWindowHandle
}
function Get-WindowBox {
    $r = New-Object SlMeasure+RECT
    [SlMeasure]::GetWindowRect((Get-CivWindow), [ref]$r) | Out-Null
    return @($r.L, $r.T, ($r.R - $r.L), ($r.B - $r.T))
}
# Asks for one spot; returns it relative to the window's top-left corner.
function Read-Spot([string]$what) {
    Write-Host ""
    Write-Host "NEXT: $what"
    Read-Host "  Press Enter, then put the mouse pointer there within $Seconds seconds and hold it still" | Out-Null
    for ($i = $Seconds; $i -gt 0; $i--) { Write-Host -NoNewline "$i.. "; Start-Sleep 1 }
    $pt = New-Object SlMeasure+POINT
    [SlMeasure]::GetCursorPos([ref]$pt) | Out-Null
    $w = Get-WindowBox
    $x = $pt.X - $w[0]; $y = $pt.Y - $w[1]
    if ($x -lt 0 -or $y -lt 0 -or $x -ge $w[2] -or $y -ge $w[3]) {
        Write-Host ""
        Write-Host "  That spot ($($pt.X),$($pt.Y)) is outside Civ V's window. Try again."
        return Read-Spot $what
    }
    Write-Host " got $x,$y (inside the window)"
    return @($x, $y)
}
function Wait-You([string]$what) { Write-Host ""; Read-Host "NOW YOU: $what. Press Enter when that screen is showing" | Out-Null }

$box = Get-WindowBox
Write-Host ("Civ V window: at {0},{1}, size {2}x{3}. Keep it there for the whole lab: Civ V opens it the same way next time." -f $box[0], $box[1], $box[2], $box[3])

# The picture used to recognise screens: the window's inside (below the title bar, inside the frame).
$h = Get-CivWindow
$cr = New-Object SlMeasure+RECT
[SlMeasure]::GetClientRect($h, [ref]$cr) | Out-Null
$c0 = New-Object SlMeasure+POINT
[SlMeasure]::ClientToScreen($h, [ref]$c0) | Out-Null
$area = [ordered]@{ X = ($c0.X - $box[0]); Y = ($c0.Y - $box[1]); W = ($cr.R - $cr.L); H = ($cr.B - $cr.T) }

$clicks = [ordered]@{}
$clicks.Continue = Read-Spot "the CONTINUE button on the screen after the intro (the legal text)"
Wait-You "click Continue, so the MAIN MENU shows"
$clicks.Neutral = Read-Spot "an empty spot in the dark band along the top of the window, where nothing reacts to a click"
$panelA = Read-Spot "the TOP-LEFT corner of the column of main-menu buttons (just outside the top button's left end)"
$panelB = Read-Spot "the BOTTOM-RIGHT corner of that column (just outside the bottom button's right end)"
$clicks.SinglePlayer = Read-Spot "the SINGLE PLAYER button"
Wait-You "click Single Player"
$clicks.SetUpGame = Read-Spot "the SET UP GAME button"
Wait-You "click Set Up Game"
$clicks.AdvancedSetup = Read-Spot "the ADVANCED SETUP button"
Wait-You "click Advanced Setup"
$clicks.QuickCombat = Read-Spot "the middle of the QUICK COMBAT check box (the box itself, not its label)"
$clicks.QuickMovement = Read-Spot "the middle of the QUICK MOVEMENT check box"
$clicks.StartGame = Read-Spot "the START GAME button"
Wait-You "set the game up as on BOX (you: Askia of Songhai; Greece as an AI; Tiny map; King; Quick pace; same map type as BOX), click Start Game and wait for the BEGIN YOUR JOURNEY button"
$clicks.BeginJourney = Read-Spot "the middle of the BEGIN YOUR JOURNEY button"
$beginA = Read-Spot "the TOP-LEFT corner of the Begin Your Journey button's text"
$beginB = Read-Spot "the BOTTOM-RIGHT corner of the Begin Your Journey button's text"

$box2 = Get-WindowBox
if (($box2 -join ",") -ne ($box -join ",")) { throw "Civ V's window moved or changed size during the measuring ($($box -join ',') -> $($box2 -join ',')). Leave it in place and run this again." }

$cfg = [ordered]@{
    host = $env:COMPUTERNAME
    measured = (Get-Date).ToString("s")
    civDir = $CivDir
    window = $box
    area = $area
    clicks = $clicks
    menuPanel = @([Math]::Min($panelA[0], $panelB[0]), [Math]::Min($panelA[1], $panelB[1]), [Math]::Max($panelA[0], $panelB[0]), [Math]::Max($panelA[1], $panelB[1]))
    beginButton = @([Math]::Min($beginA[0], $beginB[0]), [Math]::Min($beginA[1], $beginB[1]), [Math]::Max($beginA[0], $beginB[0]), [Math]::Max($beginA[1], $beginB[1]))
}
if (($cfg.menuPanel[2] - $cfg.menuPanel[0]) -lt 40 -or ($cfg.menuPanel[3] - $cfg.menuPanel[1]) -lt 40) { throw "The menu button column measured too small; run this again." }
if (($cfg.beginButton[2] - $cfg.beginButton[0]) -lt 12 -or ($cfg.beginButton[3] - $cfg.beginButton[1]) -lt 6) { throw "The Begin Your Journey text measured too small; run this again." }
if (-not (Test-Path (Join-Path $CivDir "CivilizationV.exe"))) { Write-Warning "No CivilizationV.exe in $CivDir. Run this again with -CivDir set to your Civ V folder." }

$dir = Join-Path $PSScriptRoot "hosts"
New-Item -ItemType Directory -Force $dir | Out-Null
$out = Join-Path $dir "$($env:COMPUTERNAME).json"
[IO.File]::WriteAllText($out, ([pscustomobject]$cfg | ConvertTo-Json -Depth 4), (New-Object Text.UTF8Encoding($false)))
Write-Host ""
Write-Host "Saved $out. You can quit Civ V now (no need to play the turn)."
Write-Host "Next: Run-SiegeLab.ps1 -Calibrate (setup steps in ai-lab\runs\README.md)."
