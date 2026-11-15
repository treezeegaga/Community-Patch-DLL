# Runs the siege lab unattended: installs it, starts Civ V, clicks through the setup screens to a new single-player
# game, waits for "[SiegeLab] DONE" in Logs\Lua.log, closes Civ V, saves the results, and removes the lab again.
#
#   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Run-SiegeLab.ps1 -Label stock548
#   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Run-SiegeLab.ps1 -Label test -Only small_vs_weak,large_vs_medium -Repeats 1
#
# -Set, -Only and -Repeats go straight to Install-SiegeLab.ps1. The lab is removed at the end whatever happens
# (done, failed, an error), and Civ V is never left running. Exit code 0 = the run finished and was saved.
# (Killing this script's process by force skips the clean-up: then run Install-SiegeLab.ps1 -Remove yourself.)
#
# The setup clicks are positions inside Civ V's window, taken from the window's own corner. They fit this PC: a
# 3840x2160 screen at 150%, GraphicsSettingsDX9.ini FullScreen = 0, WindowResX = 1914, WindowResY = 2034 (the window
# opens at 963,63). Screens are recognised by colour against pictures saved in Run-SiegeLab.screens.json next to this
# script (copied from the earlier runner, tools\ab\CivScreens.json). On another PC or window size, edit the positions
# at the top of the script and run once with -Calibrate (it saves the pictures; look at the calibrate-*.png files).
# Civ remembers most setup choices from the last game (civs, map, King, Quick pace); the two Quick boxes are ticked
# every time by this script. For the first run on a PC, set those by hand once and start one game.
#
# Engine assertion dialogs ("Assertion Failed", OK/Cancel) are noticed: the script saves a screenshot and the text to
# <results>\<Label>-<time>-run\assertions\, presses OK and carries on. Every one is listed at the end.
#
# Works in Windows PowerShell 5.1. See README.md next to this script.
param(
    [string]$Label = "siege",
    [string]$Set = "siege",
    [string[]]$Only = @(),
    [int]$Repeats = 2,
    [int]$StartMinutes = 6,          # after the game starts, how long to wait for the lab's "RUN" line
    [int]$StallMinutes = 15,         # Lua.log not growing for this long = stuck
    [int]$MaxMinutes = 180,          # hard limit for the whole wait
    [switch]$Calibrate,              # save the current setup screens as pictures (first time on a PC)
    [string]$CivDir = "C:\Program Files (x86)\Steam\steamapps\common\Sid Meier's Civilization V",
    [string]$MyGames = "$([Environment]::GetFolderPath('MyDocuments'))\My Games\Sid Meier's Civilization 5",
    [string]$OutDir = (Join-Path $PSScriptRoot "results")
)

$ErrorActionPreference = "Stop"
if ($Label -notmatch '^[A-Za-z0-9_.-]+$') { throw "-Label is letters, digits, dots, dashes and underscores" }

Add-Type -AssemblyName System.Drawing
Add-Type -ReferencedAssemblies System.Drawing @"
using System; using System.Text; using System.Collections.Generic; using System.Runtime.InteropServices;
public class SlInput {
    public delegate bool EnumProc(IntPtr h, IntPtr l);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern int GetSystemMetrics(int i);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, UIntPtr e);
    [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint f, UIntPtr e);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr h, EnumProc p, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc p, IntPtr l);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern IntPtr SendMessage(IntPtr h, uint m, IntPtr w, StringBuilder l);
    [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
    public static List<IntPtr> TopWindowsOf(uint pid) {
        var found = new List<IntPtr>();
        EnumWindows(delegate(IntPtr h, IntPtr l) { uint p; GetWindowThreadProcessId(h, out p); if (p == pid && IsWindowVisible(h)) found.Add(h); return true; }, IntPtr.Zero);
        return found;
    }
    public static string Title(IntPtr h) { var s = new StringBuilder(512); GetWindowText(h, s, 512); return s.ToString(); }
    public static string Class(IntPtr h) { var s = new StringBuilder(256); GetClassName(h, s, 256); return s.ToString(); }
    // Id, class and text of every child control of a dialog, e.g. "1234|Static|Assertion text" and "5678|Button|OK"
    public static List<string> Children(IntPtr dlg) {
        var found = new List<string>();
        EnumChildWindows(dlg, delegate(IntPtr h, IntPtr l) {
            var s = new StringBuilder(8192); SendMessage(h, 0x000D, (IntPtr)8192, s);   // WM_GETTEXT
            found.Add(h.ToInt64() + "|" + Class(h) + "|" + s.ToString().Replace("\r", " ").Replace("\n", " / "));
            return true; }, IntPtr.Zero);
        return found;
    }
    public static void Click(IntPtr h) { SendMessage(h, 0x00F5, IntPtr.Zero, IntPtr.Zero); }   // BM_CLICK
}
"@
[SlInput]::SetProcessDPIAware() | Out-Null

$stamp = Get-Date -Format "yyyyMMdd-HHmm"
$runDir = Join-Path $OutDir "$Label-$stamp-run"
New-Item -ItemType Directory -Force $runDir | Out-Null
$runLog = Join-Path $runDir "Run-SiegeLab.log"
$luaLog = Join-Path $MyGames "Logs\Lua.log"
$screensFile = Join-Path $PSScriptRoot "Run-SiegeLab.screens.json"
$installer = Join-Path $PSScriptRoot "Install-SiegeLab.ps1"
$reader = Join-Path $PSScriptRoot "Get-SiegeLabResults.ps1"
$script:Assertions = @()

function Write-Log([string]$text) {
    $line = "{0:yyyy-MM-dd HH:mm:ss}  {1}" -f (Get-Date), $text
    Add-Content -Path $runLog -Value $line
    Write-Host $line
}

# ---------- Civ V window and mouse ----------
# Click positions inside the window, measured from its top-left corner. They are the ones the earlier test runner
# (Run-Games.ps1) used on this PC, minus the window's corner (963,63).
$P = @{
    Continue = @(957, 1248); SinglePlayer = @(957, 910); SetUpGame = @(965, 971); AdvancedSetup = @(1036, 1335)
    QuickCombat = @(957, 1493); QuickMovement = @(957, 1519); StartGame = @(1268, 1991); BeginJourney = @(957, 1390)
    Neutral = @(957, 237)    # black band at the top of the window, nothing to click there
}
$MenuPanel = @(777, 782, 1147, 1309)     # menu buttons area, window-relative, for telling the main and Single Player menus apart
$BeginButton = @(832, 1378, 1052, 1398)  # "Begin Your Journey" button, window-relative
$AreaRel = @{ X = 0; Y = 43; W = 1914; H = 1990 }   # the picture used to recognise a screen: the window below its title bar

function Get-CivProcess { Get-Process CivilizationV -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1 }
function Get-Origin {
    $w = Get-CivProcess
    if (-not $w) { return $null }
    $r = New-Object SlInput+RECT
    [SlInput]::GetWindowRect($w.MainWindowHandle, [ref]$r) | Out-Null
    return @($r.L, $r.T, ($r.R - $r.L), ($r.B - $r.T))
}
function Stop-Civ {
    Get-Process CivilizationV, CivilizationV_DX11, CivilizationV_Tablet -ErrorAction SilentlyContinue | Stop-Process -Force
    Start-Sleep 3
}
function Set-CivFront {
    $w = Get-CivProcess
    if (-not $w) { return }
    [SlInput]::keybd_event(0x12, 0, 0, [UIntPtr]::Zero); [SlInput]::keybd_event(0x12, 0, 2, [UIntPtr]::Zero)   # a key press lets us take the foreground
    [SlInput]::SetForegroundWindow($w.MainWindowHandle) | Out-Null
    Start-Sleep -Milliseconds 300
}
function Get-Abs([string]$name) { $o = Get-Origin; return @(($o[0] + $P[$name][0]), ($o[1] + $P[$name][1])) }
function Click([string]$name, [int]$waitMs = 1500) {
    Set-CivFront
    $n = Get-Abs Neutral
    # a click on an inactive window only activates it, so click the empty band at the top first
    [SlInput]::SetCursorPos($n[0], $n[1]) | Out-Null; Start-Sleep -Milliseconds 100
    [SlInput]::mouse_event(0x02, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60
    [SlInput]::mouse_event(0x04, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 300
    $xy = Get-Abs $name
    [SlInput]::SetCursorPos($xy[0] - 6, $xy[1] - 6) | Out-Null; Start-Sleep -Milliseconds 120
    [SlInput]::SetCursorPos($xy[0], $xy[1]) | Out-Null; Start-Sleep -Milliseconds 150
    [SlInput]::mouse_event(0x02, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60
    [SlInput]::mouse_event(0x04, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 100
    [SlInput]::SetCursorPos($n[0], $n[1]) | Out-Null   # move away so no hover tooltip covers what we check next
    Start-Sleep -Milliseconds $waitMs
}
function Save-Shot([string]$path) {
    $bmp = New-Object System.Drawing.Bitmap ([SlInput]::GetSystemMetrics(0)), ([SlInput]::GetSystemMetrics(1))   # the whole screen
    $g = [System.Drawing.Graphics]::FromImage($bmp); $g.CopyFromScreen(0, 0, 0, 0, $bmp.Size); $g.Dispose()
    $bmp.Save($path); $bmp.Dispose()
}

# A ticked box has a bright green check mark; an empty one is dark teal
function Test-Ticked([string]$name) {
    $xy = Get-Abs $name
    $bmp = New-Object System.Drawing.Bitmap 20, 20
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($xy[0] - 10, $xy[1] - 10, 0, 0, $bmp.Size)
    $green = 0
    for ($x = 0; $x -lt 20; $x++) { for ($y = 0; $y -lt 20; $y++) {
        $c = $bmp.GetPixel($x, $y)
        if ($c.G -gt 120 -and $c.G -gt $c.R + 30 -and $c.G -gt $c.B + 30) { $green++ }
    } }
    $g.Dispose(); $bmp.Dispose()
    return ($green -ge 8)
}

# ---------- Recognising screens ----------
# 8x8 grid of average colours over the window, plus single pixels over the menu panel (the main menu and the
# Single Player menu differ only in their button text)
function Get-ScreenSignature {
    $o = Get-Origin
    $ax = $o[0] + $AreaRel.X; $ay = $o[1] + $AreaRel.Y; $aw = $AreaRel.W; $ah = $AreaRel.H
    $bmp = New-Object System.Drawing.Bitmap $aw, $ah
    $g = [System.Drawing.Graphics]::FromImage($bmp); $g.CopyFromScreen($ax, $ay, 0, 0, $bmp.Size); $g.Dispose()
    $sig = New-Object 'System.Collections.Generic.List[int]'
    $cw = [int]($aw / 8); $ch = [int]($ah / 8)
    for ($gy = 0; $gy -lt 8; $gy++) { for ($gx = 0; $gx -lt 8; $gx++) {
        $r = 0; $gg = 0; $b = 0; $n = 0
        for ($y = $gy * $ch + 4; $y -lt ($gy + 1) * $ch - 4; $y += 12) { for ($x = $gx * $cw + 4; $x -lt ($gx + 1) * $cw - 4; $x += 12) {
            $c = $bmp.GetPixel($x, $y); $r += $c.R; $gg += $c.G; $b += $c.B; $n++
        } }
        $sig.Add([int]($r / $n)); $sig.Add([int]($gg / $n)); $sig.Add([int]($b / $n))
    } }
    for ($y = $MenuPanel[1] - $AreaRel.Y; $y -lt $MenuPanel[3] - $AreaRel.Y; $y += 11) { for ($x = $MenuPanel[0] - $AreaRel.X; $x -lt $MenuPanel[2] - $AreaRel.X; $x += 9) {
        $c = $bmp.GetPixel($x, $y); $sig.Add([int](($c.R + $c.G + $c.B) / 3))
    } }
    $bmp.Dispose()
    return ,$sig.ToArray()
}
function Get-BeginSignature {
    $o = Get-Origin
    $w = $BeginButton[2] - $BeginButton[0]; $h = $BeginButton[3] - $BeginButton[1]
    $bmp = New-Object System.Drawing.Bitmap $w, $h
    $g = [System.Drawing.Graphics]::FromImage($bmp); $g.CopyFromScreen($o[0] + $BeginButton[0], $o[1] + $BeginButton[1], 0, 0, $bmp.Size); $g.Dispose()
    $sig = New-Object 'System.Collections.Generic.List[int]'
    for ($y = 0; $y -lt $h; $y += 3) { for ($x = 0; $x -lt $w; $x += 4) { $c = $bmp.GetPixel($x, $y); $sig.Add([int](($c.R + $c.G + $c.B) / 3)) } }
    $bmp.Dispose()
    return ,$sig.ToArray()
}
function Read-Screens {
    $h = @{}
    if (Test-Path $screensFile) { $o = Get-Content $screensFile -Raw | ConvertFrom-Json; foreach ($p in $o.PSObject.Properties) { $h[$p.Name] = @($p.Value) } }
    return $h
}
function Save-ScreenPicture([string]$name, $sig) {
    $h = Read-Screens
    $h[$name] = @($sig)
    ($h | ConvertTo-Json -Depth 3 -Compress) | Set-Content $screensFile -Encoding ASCII
    Write-Log "  calibration: saved screen '$name'"
}
function Get-Distance($sig, $ref) {
    if (-not $ref -or $ref.Count -ne $sig.Count) { return 999.0 }
    $d = 0; for ($i = 0; $i -lt $sig.Count; $i++) { $d += [Math]::Abs($sig[$i] - $ref[$i]) }
    return $d / $sig.Count
}
$script:Screens = Read-Screens
# The setup screens show the chosen leader, map and options, which change between games, so they match less closely
# than the menus; the menus differ from each other only in their button text, so they need a close match. (The saved
# pictures, from an 8-civ Pangaea setup, measured 16.7 against a Tiny Songhai setup; it must still be the nearest.)
$script:Tolerance = @{ menu = 2.5; single = 2.5; setup = 25; advanced = 25 }
# The saved screen nearest to what is shown now
function Get-ClosestScreen {
    $sig = Get-ScreenSignature
    $best = $null; $bestD = [double]::MaxValue
    foreach ($k in $script:Screens.Keys) {
        if ($k -like '_*') { continue }
        $d = Get-Distance $sig $script:Screens[$k]
        if ($d -lt $bestD) { $bestD = $d; $best = $k }
    }
    return [pscustomobject]@{ Name = $best; Distance = $bestD }
}
# Waits until the screen matches the saved picture of $name. With -Calibrate there are no pictures yet: wait a fixed
# time, then save what is on screen (and a screenshot to look at) under that name.
function Wait-Screen([string]$name, [int]$timeoutSec = 40) {
    if ($Calibrate) {
        Start-Sleep 6
        Save-ScreenPicture $name (Get-ScreenSignature)
        Save-Shot (Join-Path $runDir "calibrate-$name.png")
        return $true
    }
    if (-not $script:Screens[$name]) { throw "No saved picture of the '$name' screen in $screensFile. Run once with -Calibrate (and look at the calibrate-*.png it saves)." }
    $until = (Get-Date).AddSeconds($timeoutSec)
    $last = $null
    while ((Get-Date) -lt $until) {
        Test-Assertion | Out-Null
        $last = Get-ClosestScreen
        if ($last.Name -eq $name -and $last.Distance -lt $script:Tolerance[$name]) { Write-Log ("  screen '{0}' (distance {1:N1})" -f $name, $last.Distance); return $true }
        Start-Sleep 1
    }
    Write-Log ("  waiting for '{0}', the closest saved screen was '{1}' at distance {2:N1}" -f $name, $last.Name, $last.Distance)
    return $false
}
function Test-MenuShown { if ($Calibrate) { return $false }; $s = Get-ClosestScreen; return ($s.Name -eq "menu" -and $s.Distance -lt 2.5) }

# ---------- Assertion dialogs ----------
# True if an "Assertion Failed" dialog was found (and closed with OK)
function Test-Assertion {
    $civ = Get-Process CivilizationV -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $civ) { return $false }
    foreach ($h in [SlInput]::TopWindowsOf([uint32]$civ.Id)) {
        if ([SlInput]::Title($h) -notlike "*Assertion Failed*") { continue }
        $n = $script:Assertions.Count + 1
        $dir = Join-Path $runDir "assertions"
        New-Item -ItemType Directory -Force $dir | Out-Null
        $children = @([SlInput]::Children($h))
        $text = ($children | Where-Object { $_ -notmatch '\|Button\|' } | ForEach-Object { ($_ -split '\|', 3)[2] } | Where-Object { $_ }) -join " // "
        $png = Join-Path $dir ("assertion-{0:D2}.png" -f $n)
        try { Save-Shot $png } catch { $png = "(screenshot failed)" }
        Set-Content (Join-Path $dir ("assertion-{0:D2}.txt" -f $n)) ("{0}`r`n{1}" -f (Get-Date -Format s), ($children -join "`r`n"))
        Write-Log "ASSERTION $n : $text   (screenshot $png)"
        $script:Assertions += $text
        $ok = $children | Where-Object { $_ -match '\|Button\|&?OK$' } | Select-Object -First 1
        if ($ok) { [SlInput]::Click([IntPtr][int64](($ok -split '\|')[0])) } else { Write-Log "  no OK button found in the dialog: $($children -join ' ; ')"; return $true }
        Start-Sleep -Milliseconds 800
        return $true
    }
    return $false
}

# ---------- Starting the game ----------
function Start-LabGame {
    Stop-Civ
    Start-Process (Join-Path $CivDir "CivilizationV.exe") -WorkingDirectory $CivDir
    for ($i = 0; $i -lt 60 -and -not (Get-CivProcess); $i++) { Start-Sleep 2 }
    if (-not (Get-CivProcess)) { Write-Log "FAILED: no Civ V window"; return $false }
    $o = Get-Origin
    Write-Log ("Civ V window at {0},{1} size {2}x{3}" -f $o[0], $o[1], $o[2], $o[3])
    if ($o[0] -ne 963 -or $o[1] -ne 63 -or $o[2] -ne 1914 -or $o[3] -ne 2034) { Write-Log "  warning: the click positions and screen pictures were made for a 1914x2034 window at 963,63" }

    # Intro movie, loading, then the legal screen. Click its Continue button until the main menu shows; never click
    # there once the menu is up, because the menu's EXIT button is at nearly the same spot.
    $until = (Get-Date).AddSeconds(240)
    if ($Calibrate) { Start-Sleep 60; Click Continue 4000; Click Continue 4000; Wait-Screen menu | Out-Null }
    else {
        while (-not (Test-MenuShown)) {
            if ((Get-Date) -gt $until) { Write-Log "FAILED: main menu never showed"; Save-Shot (Join-Path $runDir "fail-menu.png"); return $false }
            Test-Assertion | Out-Null
            Click Continue 4000
        }
    }
    Click SinglePlayer
    if (-not (Wait-Screen single)) { Write-Log "FAILED: Single Player menu never showed"; Save-Shot (Join-Path $runDir "fail-single.png"); return $false }
    Click SetUpGame
    if (-not (Wait-Screen setup)) { Write-Log "FAILED: Set Up Game never showed"; Save-Shot (Join-Path $runDir "fail-setup.png"); return $false }
    Click AdvancedSetup
    if (-not (Wait-Screen advanced)) { Write-Log "FAILED: Advanced Setup never showed"; Save-Shot (Join-Path $runDir "fail-advanced.png"); return $false }
    foreach ($box in "QuickCombat", "QuickMovement") {
        for ($try = 0; $try -lt 3 -and -not (Test-Ticked $box); $try++) { Click $box 800 }
        if (-not (Test-Ticked $box)) { Write-Log "FAILED: couldn't tick $box"; Save-Shot (Join-Path $runDir "fail-$box.png"); return $false }
    }
    Save-Shot (Join-Path $runDir "setup-before-start.png")
    Click StartGame 5000

    # Loading the map; then the "Begin Your Journey" screen
    $beginRef = $script:Screens["_begin"]
    $until = (Get-Date).AddSeconds(300)
    $seen = $false
    while ((Get-Date) -lt $until) {
        Test-Assertion | Out-Null
        if (-not (Get-CivProcess)) { Write-Log "FAILED: Civ V exited while the game loaded"; return $false }
        if ($Calibrate) { Start-Sleep 40; Save-ScreenPicture "_begin" (Get-BeginSignature); Save-Shot (Join-Path $runDir "calibrate-begin.png"); $seen = $true; break }
        if ((Get-Distance (Get-BeginSignature) $beginRef) -lt 8) { $seen = $true; break }
        Start-Sleep 3
    }
    if (-not $seen) { Write-Log "  Begin Your Journey not recognised after 5 minutes; clicking it anyway"; Save-Shot (Join-Path $runDir "begin-not-recognised.png") }
    for ($try = 0; $try -lt 3; $try++) {
        Click BeginJourney 3000
        if ($Calibrate -or (Get-Distance (Get-BeginSignature) $beginRef) -ge 8) { break }
    }
    return $true
}

# ---------- Waiting for the run ----------
function Read-LuaLog {
    $fs = [IO.File]::Open($luaLog, [IO.FileMode]::Open, [IO.FileAccess]::Read, ([IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete))
    try { $sr = New-Object IO.StreamReader($fs); return ($sr.ReadToEnd() -split "`r?`n") } finally { $fs.Dispose() }
}
# Literal text match (-like would read the [ ] in "[SiegeLab]" as a wildcard)
function Get-Matching($lines, [string]$text) { return @($lines | Where-Object { $_.Contains($text) }) }

# Returns "done" or a message saying what went wrong
function Wait-Run([datetime]$launched, [datetime]$gameStarted) {
    $lastSize = -1; $lastGrowth = Get-Date; $sawRun = $false; $lastBattle = 0
    $began = Get-Date
    while ($true) {
        Start-Sleep 5
        Test-Assertion | Out-Null
        if (-not (Get-Process CivilizationV -ErrorAction SilentlyContinue)) {
            # one last look: it may have exited right after writing DONE
            if (Test-Path $luaLog) { if ((Get-Matching (Read-LuaLog) "[SiegeLab] DONE").Count -gt 0) { return "done" } }
            return "Civ V exited before the run finished (crash?)"
        }
        $lines = @()
        $fresh = (Test-Path $luaLog) -and ((Get-Item $luaLog).LastWriteTime -ge $launched)   # an old Lua.log is not ours
        if ($fresh) {
            $size = (Get-Item $luaLog).Length
            if ($size -ne $lastSize) { $lastSize = $size; $lastGrowth = Get-Date }
            $lines = @(Read-LuaLog)
        }
        if ((Get-Matching $lines "[SiegeLab] DONE").Count -gt 0) { return "done" }
        $errs = @($lines | Where-Object { $_ -match 'SiegeLab' -and $_ -match 'Error' })
        if ($errs.Count -gt 0) { return "Lua error in Lua.log: $($errs[0])" }
        $runLines = @(Get-Matching $lines "[SiegeLab] RUN")
        if (-not $sawRun -and $runLines.Count -gt 0) {
            $sawRun = $true
            $run = $runLines[0]
            Write-Log "Lab started: $run"
            $v = ($run.Substring($run.IndexOf("[SiegeLab] RUN") + 15) -split ",")
            if ($v.Count -ge 8 -and ($v[5] -ne "HANDICAP_KING" -or $v[6] -ne "GAMESPEED_QUICK")) { Write-Log "  warning: the game is $($v[5]) / $($v[6]), not King / Quick" }
        }
        if (-not $sawRun -and ((Get-Date) - $gameStarted).TotalMinutes -ge $StartMinutes) { return "no '[SiegeLab] RUN' line within $StartMinutes minutes of starting the game (was the game new, single-player, and the lab installed?)" }
        $battles = (Get-Matching $lines "[SiegeLab] RESULT").Count
        if ($battles -ne $lastBattle) { $lastBattle = $battles; Write-Log "  $battles battle(s) finished" }
        if ($sawRun -and ((Get-Date) - $lastGrowth).TotalMinutes -ge $StallMinutes) { return "Lua.log has not grown for $StallMinutes minutes (game stuck?)" }
        if (((Get-Date) - $began).TotalMinutes -ge $MaxMinutes) { return "still not done after $MaxMinutes minutes" }
    }
}

# ---------- The run ----------
$exit = 1
try {
    if (Get-Process -Name CivilizationV, CivilizationV_DX11, CivilizationV_Tablet -ErrorAction SilentlyContinue) {
        throw "Civ V is already running. Close it first (this script starts its own)."
    }
    Write-Log "Run '$Label': set $Set, only [$($Only -join ',')], repeats $Repeats"
    & $installer -Set $Set -Only $Only -Repeats $Repeats -EnableLogging | ForEach-Object { Write-Log "  installer: $_" }

    $launched = Get-Date
    if (-not (Start-LabGame)) { throw "Could not get a new game started (see the screenshots in $runDir)" }
    $gameStarted = Get-Date
    Write-Log "Game started; waiting for the lab"
    $status = Wait-Run $launched $gameStarted
    if ($status -ne "done") {
        Save-Shot (Join-Path $runDir "fail-end.png")
        throw $status
    }
    Write-Log "Lab finished ([SiegeLab] DONE)"
    Start-Sleep 3
    Copy-Item $luaLog (Join-Path $runDir "Lua.log") -Force   # before anything else can touch it
    Stop-Civ
    & $reader -Label $Label -MyGames $MyGames -OutDir $OutDir | Out-String -Stream | Where-Object { $_.Trim() } | ForEach-Object { Write-Log "  results: $_" }
    $exit = 0
}
catch {
    Write-Log "FAILED: $($_.Exception.Message)"
    try { if (Test-Path $luaLog) { Copy-Item $luaLog (Join-Path $runDir "Lua.log") -Force } } catch {}
}
finally {
    Stop-Civ
    try { & $installer -Remove | ForEach-Object { Write-Log "  installer: $_" } } catch { Write-Log "WARNING: could not remove the lab: $($_.Exception.Message). Run Install-SiegeLab.ps1 -Remove." }
    if ($script:Assertions.Count -gt 0) {
        Write-Log "ASSERTIONS ($($script:Assertions.Count)), screenshots in $runDir\assertions :"
        $script:Assertions | ForEach-Object { Write-Log "  - $_" }
    }
    else { Write-Log "No assertion dialogs." }
    Write-Log $(if ($exit -eq 0) { "Run saved. Civ V closed, lab removed." } else { "Run FAILED. Civ V closed, lab removed. Log: $runLog" })
}
exit $exit
