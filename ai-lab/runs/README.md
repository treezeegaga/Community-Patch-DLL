# Lab run queue

Lab runs are requested by committing a file to `ai-lab/runs/queue/` on `ai/v2`. A watcher on the PC
(`Watch-LabQueue.ps1`, started by Task Scheduler every 5 minutes) pulls `ai/v2`, runs every request that has no
marker in `ai-lab/runs/done/` yet, pushes the results to `ai-lab/siege/results/` and a done marker to
`ai-lab/runs/done/<same name>.json`. This replaces sending run requests to the PC runner session by message.

## Requesting a run (the implementing session)

1. Write `ai-lab/runs/queue/<id>.json`. The id is the file name without `.json`. Names sort in the order they run,
   so start them with the date and a counter: `2026-11-19-01-m1d3.json`.
2. `git add -f` it, commit, push to `ai/v2`. If the run needs a new DLL build, push the code first: the watcher
   waits up to 90 minutes for the release `ab-<commit>` to be published, then marks the request failed.
3. Later, `git pull` and read `ai-lab/runs/done/<id>.json` and the results it names. Never edit or reuse a queued
   file: queue a new one.

```json
{
  "id": "2026-11-19-01-m1d3",
  "why": "M1 iteration 3: check the heal change on the four last-hit-point scenarios",
  "launches": [
    { "label": "m1d3", "release": "ab-1234abc", "only": ["large_vs_strong", "siege_train_vs_strong"], "repeats": 2 },
    { "label": "m1d3s", "release": "ab-1234abc", "script": true, "newAI": "none", "repeats": 2 }
  ]
}
```

Launch fields, all but `label` optional; each maps to a `Run-SiegeLab.ps1` option:

| Field | Meaning | Default |
|---|---|---|
| `label` | results file prefix (letters, digits, `_ . -`) | required |
| `release` | DLL build `ab-<commit>`; the stock DLL is restored afterwards | the pack's own DLL |
| `set` | scenario list in `SiegeLabScenarios.lua`: `siege` or `march` | `siege` |
| `only` | scenario names | the whole set |
| `repeats` | battles per scenario in this launch | 2 |
| `script` | `true`: the scripted attacker plays the attacker | `false` |
| `newAI` | `none`, `attacker`, `defender`, `both` | `none` |

Launches run one after another. A launch that fails before the game starts (menus) is retried once.

## The done marker

`ai-lab/runs/done/<id>.json`: `status` (`done` or `failed`), `message` (why it failed), `started`, `finished`,
`host`, `requestCommit`, `stockDll` (`ok`, or what the watcher found and did), and per launch: `label`,
`exitCode`, `attempts`, `runFolder`, `summary` (the result lines of its run log), `assertions`, `labErrors`.

## What the watcher does on each pass

- Exits at once if `%LOCALAPPDATA%\cp-ai-lab\PAUSE` exists, if another pass is running, or if Civ V is open
  (you are playing).
- `git pull --rebase` in the lab checkout; first pushes any results an earlier pass couldn't push.
- For each pending request, oldest first: checks the file, waits for its builds, runs its launches with
  `Run-SiegeLab.ps1` (which installs the lab, plays, saves results, removes the lab, restores the stock DLL and
  closes Civ V), zips any AI log folder over 50 MB, checks the pack's DLL against the saved stock copy (restores it
  if it differs), writes the marker and pushes. Stops between requests if you open Civ V.
- Writes `%LOCALAPPDATA%\cp-ai-lab\watcher.log`.

## One-time setup on the PC (the user)

The lab checkout is `C:\Users\itstr\cp-ai-lab` (sparse, `ai-lab` only, on `ai/v2`), which the PC runner session
already uses, with git able to push to GitHub.

1. Close Civ V. In PowerShell:
   ```
   git -C C:\Users\itstr\cp-ai-lab fetch origin ai/v2
   git -C C:\Users\itstr\cp-ai-lab checkout ai/v2
   git -C C:\Users\itstr\cp-ai-lab pull --rebase origin ai/v2
   ```
2. Dry run (runs nothing, shows what it would run, and checks that the script parses on your PC):
   ```
   powershell -NoProfile -ExecutionPolicy Bypass -File C:\Users\itstr\cp-ai-lab\ai-lab\runs\Watch-LabQueue.ps1 -DryRun
   ```
   An empty queue prints nothing; then look at `%LOCALAPPDATA%\cp-ai-lab\watcher.log` (it may not exist yet).
3. Register the task:
   ```
   powershell -ExecutionPolicy Bypass -File C:\Users\itstr\cp-ai-lab\ai-lab\runs\Register-LabWatcher.ps1
   ```
4. Keep Windows from locking or sleeping while runs are queued: the runner clicks through Civ V's menus, which only
   works on an unlocked desktop. (Settings > Accounts > Sign-in options, and Power > Screen and sleep.)

Day to day:

- **Playing Civ V yourself:** just start it. The watcher doesn't start anything while Civ V is open. To be safe
  during a long session, pause it: `New-Item $env:LOCALAPPDATA\cp-ai-lab\PAUSE` (resume: delete that file).
- **Something looks wrong:** `Get-Content $env:LOCALAPPDATA\cp-ai-lab\watcher.log -Tail 30`.
- **After an interrupted run** (power cut, killed process): `ai-lab\siege\Set-LabDll.ps1 -Restore` and
  `ai-lab\siege\Install-SiegeLab.ps1 -Remove` put the pack back.
- **Remove the watcher:** `Register-LabWatcher.ps1 -Remove`.
