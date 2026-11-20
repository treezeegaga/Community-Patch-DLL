# Lab run queue

Lab runs are requested by committing a file to `ai-lab/runs/queue/` on `ai/v2`. A watcher on each lab PC
(`Watch-LabQueue.ps1`, started by Task Scheduler every 5 minutes) pulls `ai/v2` and runs launches that have no result
yet. Several PCs can share the queue: a PC **claims** a launch before running it, so no launch runs twice, and one
request's launches can be split across PCs. Results go to `ai-lab/siege/results/`, each launch's result to
`ai-lab/runs/launches/<id>/<label>.json`, and when every launch of a request has a result, a done marker to
`ai-lab/runs/done/<id>.json`. Lab PCs: **BOX** (since 2026-11-19) and **TREEDESKTOP** (being set up, see below).

## Requesting a run (the implementing session)

1. Write `ai-lab/runs/queue/<id>.json`. The id is the file name without `.json`. Names sort in the order they run,
   so start them with the date and a counter: `2026-11-19-01-m1d3.json`.
2. `git add -f` it, commit, push to `ai/v2`. If the run needs a new DLL build, push the code first: the watchers
   wait up to 90 minutes for the release `ab-<commit>` to be published, then mark the request failed.
3. Later, `git pull` and read `ai-lab/runs/done/<id>.json` and the results it names. Never edit or reuse a queued
   file: queue a new one.

```json
{
  "id": "2026-11-19-01-m1d3",
  "why": "M1 iteration 3: check the heal change on the four last-hit-point scenarios",
  "launches": [
    { "label": "m1d3", "release": "ab-1234abc", "only": ["large_vs_strong", "siege_train_vs_strong"], "repeats": 2 },
    { "label": "m1d3s", "release": "ab-1234abc", "script": true, "newAI": "none", "repeats": 2, "host": "BOX" }
  ]
}
```

Launch fields, all but `label` optional; each maps to a `Run-SiegeLab.ps1` option:

| Field | Meaning | Default |
|---|---|---|
| `label` | results file prefix (letters, digits, `_ . -`), unique within the request | required |
| `release` | DLL build `ab-<commit>`; the stock DLL is restored afterwards | the pack's own DLL |
| `set` | scenario list in `SiegeLabScenarios.lua`: `siege` or `march` | `siege` |
| `only` | scenario names | the whole set |
| `repeats` | battles per scenario in this launch | 2 |
| `script` | `true`: the scripted attacker plays the attacker | `false` |
| `newAI` | `none`, `attacker`, `defender`, `both` | `none` |
| `host` | only this PC (its Windows computer name, e.g. `BOX`) may run the launch | any lab PC |

Launches of a request are taken in order, but with two PCs they run two at a time, so don't rely on one launch
finishing before the next starts. A launch that fails before the game starts (menus) is retried once on the same PC.

## Claims, launch results and done markers

- **Claim** `ai-lab/runs/launches/<id>/<label>.claim.json`: `host`, `claimedAt` (local time), `claimedAtUnix`,
  `watcherVersion`, `note`. A PC commits it and pushes it on its own before starting the launch. If two PCs claim
  the same launch, git accepts only the first push; the other PC's push is refused, it drops its claim and looks for
  another launch. A claim older than 240 minutes (`-ClaimTimeoutMinutes`) with no result is released and any PC may
  take the launch again; a PC that finds its own claim without a result (its run died: power cut, reboot) takes it
  again at once.
- **Launch result** `ai-lab/runs/launches/<id>/<label>.json`: `label`, `host`, `exitCode`, `attempts`, `runFolder`,
  `summary` (the result lines of its run log), `assertions`, `labErrors`, `started`, `finished`, `stockDll` (`ok`,
  or what the watcher found and did), `packDll` (first 16 hex digits of the pack's stock DLL, to see that the PCs
  have the same pack), `watcherVersion`.
- **Done marker** `ai-lab/runs/done/<id>.json`, written by whichever PC sees the last result: `status` (`done` or
  `failed`), `message`, `host` (the PCs that ran it, comma-separated) and `hosts`, `markedBy`, `started`,
  `finished`, `requestCommit`, `stockDll` (per PC), and `launches`: every launch result as above, in request order.
  Requests finished before watcher version 2 (up to `2026-11-19-12-m2d1`) have no launch files and one `host`.
- Every results row (`*.results.csv`) has a `Host` column, and each run log (`*-run/Run-SiegeLab.log`) starts with
  the host and its calibration.

## What the watcher does on each pass

- Exits at once if `%LOCALAPPDATA%\cp-ai-lab\PAUSE` exists, if another pass is running on this PC, or if Civ V is
  open (you are playing).
- Then, one launch at a time: `git pull --rebase` (if that brought a new version of the watcher, it stops and the
  next pass runs the new one); push any results an earlier pass couldn't push; write done markers for finished
  requests; claim the next launch; run it with `Run-SiegeLab.ps1` (which installs the lab, plays, saves results,
  removes the lab, restores the stock DLL and closes Civ V); zip any AI log folder over 50 MB; check the pack's DLL
  against the saved stock copy (restore it if it differs); write the launch result and push. Stops between launches
  if you open Civ V.
- Writes `%LOCALAPPDATA%\cp-ai-lab\watcher.log`.
- **Status (watcher v3):** writes `ai-lab/runs/status/<COMPUTERNAME>.json` (`state`: `idle`, `paused`, `civ-open`,
  `pull-failed`, `push-failed` or `error`, with `detail`, `updated`, `watcherVersion`) and pushes it when the state
  changes, and hourly while it is not idle. A stall can be read from the repository without going to the PC.
- **Self-heal (watcher v3):** if `git pull` fails and this PC has no unpushed commits, it resets the lab checkout to
  `origin/ai/v2` and pulls again (untracked result files are kept). So: never keep uncommitted edits in a lab PC's
  checkout; commit and push them, or they may be reset.

## Day to day (any lab PC)

- **Playing Civ V yourself:** just start it. The watcher doesn't start anything while Civ V is open. To be safe
  during a long session, pause it: `New-Item $env:LOCALAPPDATA\cp-ai-lab\PAUSE` (resume: delete that file).
- **Something looks wrong:** `Get-Content $env:LOCALAPPDATA\cp-ai-lab\watcher.log -Tail 30`.
- **After an interrupted run** (power cut, killed process): `ai-lab\siege\Set-LabDll.ps1 -Restore` and
  `ai-lab\siege\Install-SiegeLab.ps1 -Remove` put the pack back (add `-CivDir "<your Civ V folder>"` to both if
  Civ V isn't in the usual Steam folder). The watcher takes the interrupted launch again on its next pass.
- **Remove the watcher:** `Register-LabWatcher.ps1 -Remove`.

## Setting up a lab PC

BOX was set up on 2026-11-19 with the lab checkout at `C:\Users\itstr\cp-ai-lab`. Its click positions and screen
pictures are built into `Run-SiegeLab.ps1` and `Run-SiegeLab.screens.json`. Every other PC gets its own
`ai-lab\siege\hosts\<COMPUTERNAME>.json` (click positions, window, Civ V folder) and
`ai-lab\siege\hosts\<COMPUTERNAME>.screens.json` (screen pictures), made by steps 5 and 6 below.

### TREEDESKTOP: one-time setup

Do these on TREEDESKTOP, signed in as the Windows user the lab will run as, with Civ V closed. Steps 1-4 take about
15 minutes, steps 5-6 about 15 more.

1. **Same game and pack as BOX.** Civ V from Steam, and the "CP with Animated Leaders" pack installed exactly as on
   BOX (the same files, from the same download or copied from BOX). It must end up in Civ V's
   `Assets\DLC\MP_CPLeaderheads` folder. Check: the pack's DLL must have the same hash on both PCs. In PowerShell
   on each PC (use your own Civ V folder if it differs):
   ```
   Get-ChildItem "C:\Program Files (x86)\Steam\steamapps\common\Sid Meier's Civilization V\Assets\DLC\MP_CPLeaderheads" -Recurse -Filter CvGameCore_Expansion2.dll | Get-FileHash
   ```
   If BOX has ever had a lab DLL left in by mistake, its watcher log says so; the lab restores it after every run.
2. **Git, with push access.** Install Git for Windows (git-scm.com, default options). Then in PowerShell:
   ```
   git config --global user.name "TREEDESKTOP lab"
   git config --global user.email "<your GitHub noreply address, or any address you use with GitHub>"
   ```
   Push access: the watcher pushes results as you. The simplest is Git Credential Manager (installed with Git for
   Windows): the first `git push` below opens a browser window where you sign in to GitHub once; Windows keeps the
   sign-in. (Or copy whatever BOX uses: if BOX signs in with a personal access token, make a new token for
   TREEDESKTOP with access to `treezeegaga/Community-Patch-DLL`, contents read and write.)
3. **The lab checkout on ai/v2** (sparse: only the `ai-lab` folder, as on BOX):
   ```
   cd $env:USERPROFILE
   git clone --filter=blob:none --sparse --branch ai/v2 https://github.com/treezeegaga/Community-Patch-DLL.git cp-ai-lab
   git -C cp-ai-lab sparse-checkout set ai-lab
   ```
   Test push access with an empty push (changes nothing): `git -C cp-ai-lab push origin ai/v2`. It should say
   "Everything up-to-date", after the sign-in window the first time. If it asks for a password in the console or
   says "permission denied", fix step 2 before going on.
4. **Logging and the window.** Civ V on TREEDESKTOP must run windowed, as on BOX (BOX: `FullScreen = 0` in
   `Documents\My Games\Sid Meier's Civilization 5\GraphicsSettingsDX9.ini`, window 1914x2034). Pick a window size
   that fits this screen with its whole height visible, set it in Civ V's own video options (windowed mode), and
   then leave it: the click positions are measured for that window. The lab sets `LoggingEnabled = 1` and
   `AILog = 1` in `config.ini` itself. Start Civ V once by hand and start one game set up like BOX's: you play
   **Askia (Songhai)**, **Greece** as an AI, **Tiny** map, **King**, **Quick** pace, the same map type as BOX's
   last game. Civ V remembers these for the lab's games. Quit.
5. **Measure the click positions.** Start Civ V (the DirectX 9 version the lab uses, as on BOX) and wait for the
   screen with the Continue button after the intro. Then, in PowerShell:
   ```
   powershell -ExecutionPolicy Bypass -File $env:USERPROFILE\cp-ai-lab\ai-lab\siege\Measure-SiegeLabClicks.ps1
   ```
   Add `-CivDir "<folder>"` if Civ V is not in `C:\Program Files (x86)\Steam\steamapps\common\Sid Meier's
   Civilization V`. For each spot it names, press Enter, put the mouse on that spot in Civ V's window within 6
   seconds and hold still. Between spots it tells you which menu button to click. It writes
   `ai-lab\siege\hosts\TREEDESKTOP.json`. Quit Civ V when it says so.
6. **Save the screen pictures and test one launch.** With Civ V closed:
   ```
   powershell -ExecutionPolicy Bypass -File $env:USERPROFILE\cp-ai-lab\ai-lab\siege\Run-SiegeLab.ps1 -Label calib -Only small_vs_weak -Repeats 1 -Calibrate
   ```
   Don't touch the mouse while it runs (about 5 minutes). It clicks through the menus with fixed waits, saves each
   screen's picture to `ai-lab\siege\hosts\TREEDESKTOP.screens.json`, plays one battle, removes the lab and closes
   Civ V. Look at the `calibrate-*.png` pictures in `ai-lab\siege\results\calib-*-run\`: each should show the
   screen it is named after (menu, single, setup, advanced, begin). Then run it again **without** `-Calibrate` and
   `-Label calib2`; it must end with "Run saved". If it stops at a menu, the run log in `calib2-*-run\` says which
   screen it was waiting for; redo step 5 or 6.
7. **Commit the calibration**:
   ```
   cd $env:USERPROFILE\cp-ai-lab
   git add -f ai-lab/siege/hosts/TREEDESKTOP.json ai-lab/siege/hosts/TREEDESKTOP.screens.json
   git commit -m "Lab PC TREEDESKTOP: click positions and screen pictures"
   git pull --rebase origin ai/v2
   git push origin ai/v2
   ```
   (Leave the `calib*` results folders out; delete them.)
8. **Dry run, then register the watcher**:
   ```
   powershell -NoProfile -ExecutionPolicy Bypass -File $env:USERPROFILE\cp-ai-lab\ai-lab\runs\Watch-LabQueue.ps1 -DryRun
   powershell -ExecutionPolicy Bypass -File $env:USERPROFILE\cp-ai-lab\ai-lab\runs\Register-LabWatcher.ps1
   ```
   The dry run lists the queued launches it would take ("would run here", "is for BOX", "is claimed by BOX").
9. **Keep it unlocked and awake while runs are queued** (Settings > Accounts > Sign-in options, and Power > Screen
   and sleep), as on BOX: the runner clicks through Civ V's menus, which only works on an unlocked desktop.

Then tell the implementing session. The first job for TREEDESKTOP is the two-PC check below; until it passes, lab
requests name `"host": "BOX"` for anything that counts.

### The two-PC check

Before TREEDESKTOP's results count, both PCs play the full siege set with the stock AI on the same build, three
launches of 2 repeats each (6 battles per scenario per PC). They agree if: the RUN lines match (civs, map size,
King, Quick); the pack DLL hash matches; and per scenario, the two PCs' capture counts differ by at most 1 in 6 in
at least 13 of the 15 scenarios, and by at most 2 in 6 in all of them. That is the noise allowed between launches
of the same build on one PC (the plan's decision rule, section 7). As a cross-check, BOX's half is also compared with
BOX's earlier stock set on the same build (m1s9a/b/c): the two PCs should differ no more than BOX differs from itself.
Request: `2026-11-19-13-pc2check` (labels `pc2box1-3` on BOX, `pc2tree1-3` on TREEDESKTOP).
