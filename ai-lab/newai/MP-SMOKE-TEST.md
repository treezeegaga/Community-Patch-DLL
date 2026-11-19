# New AI: 30-turn LAN multiplayer smoke test (for the user, on two PCs)

What it checks: a multiplayer game with the new AI on runs 30 turns without desyncs, crashes or engine assertion
dialogs, and turns don't get noticeably slower. It does not judge how well the AI plays. About 1 to 1.5 hours.

The release to use is named in LOG.md when the test is due (an `ab-<commit>` build of `ai/v2`). Below it is
`ab-XXXXXXX`.

## Before: on both PCs (BOX and TREEDESKTOP)

1. Close Civ V. In PowerShell, in the lab checkout (`cd $env:USERPROFILE\cp-ai-lab`, or BOX's own lab folder):
   ```
   git pull
   powershell -ExecutionPolicy Bypass -File ai-lab\newai\Install-NewAI.ps1 -Release ab-XXXXXXX
   ```
   It puts the test DLL in the pack, adds the "New AI (test build)" checkbox to game setup, and pauses that PC's lab
   watcher. Write down the "DLL hash" it prints: **both PCs must print the same one.**
2. Logging on, so the game writes its logs (the lab PCs already have this; no harm doing it again):
   ```
   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Install-SiegeLab.ps1 -EnableLogging
   powershell -ExecutionPolicy Bypass -File ai-lab\siege\Install-SiegeLab.ps1 -Remove
   ```
   (The first line also installs the siege lab, which would take over single-player games; the second takes it out
   again and leaves the logging on. The lab never touches multiplayer games.)
3. Note the time on each PC (the logs are matched to the test by time).

## The game

- Host on BOX: Multiplayer > Local Network > Host Game. TREEDESKTOP joins.
- Settings: map Continents (or the map type you usually play), size Small, speed Quick, difficulty King.
  Two human players and 4 AI civs, at least two of them aggressive (for example Mongolia, Zulu, Assyria, Aztec).
  Turn mode as you usually play. No turn timer.
- In the game options tick **New AI (test build)**. If the box is missing on either PC, stop: step 1 didn't take.
- Play 30 turns. Normal early-game play is enough: found cities, build, explore, end turns. At about turn 10, one of
  you declares war on the nearest AI civ (so the AI's war code runs), and keep it going to turn 30.

While playing, note:
- **Desyncs**: any "out of sync" message, a player dropping and rejoining, or the two screens showing different
  things (a unit in different places, different scores). Write the turn number.
- **Assertion dialogs** ("Assertion Failed", OK/Cancel/Ignore): take a picture of it (Win+Shift+S), write the turn,
  then press **OK**. Not Cancel.
- **Crashes**: which PC, which turn, what was happening.
- **Turn time**: at turns 10, 20 and 30, the seconds from the last player ending the turn to both of you being able
  to move again. A phone stopwatch is fine.

## After: on both PCs

1. Quit Civ V.
2. Take the new AI out again (puts the stock DLL back, removes the checkbox, un-pauses the watcher):
   ```
   powershell -ExecutionPolicy Bypass -File ai-lab\newai\Install-NewAI.ps1 -Remove
   ```
3. Hand in the logs: copy the game's Logs folder into the lab checkout and push it:
   ```
   $t = Get-Date -Format "yyyyMMdd-HHmm"
   $dest = "ai-lab\runs\mp\$t-$env:COMPUTERNAME"
   New-Item -ItemType Directory -Force $dest | Out-Null
   Copy-Item "$([Environment]::GetFolderPath('MyDocuments'))\My Games\Sid Meier's Civilization 5\Logs\*" $dest -Recurse
   git add -f $dest
   git commit -m "MP smoke test logs from $env:COMPUTERNAME"
   git push
   ```
4. Tell Claude: the notes above (desyncs, assertions, crashes, the three turn times) and that the logs are pushed.

What Claude checks in the logs: `AILab_NewAI.csv` lists every AI civ as "new AI on" at turn 0 (humans are not listed),
`net_message_debug.log` for sync errors, and the AI logs for the new behaviour's lines.
