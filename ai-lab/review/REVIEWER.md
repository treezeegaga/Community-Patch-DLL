# Project reviewer: handover notes

Written 2026-11-22 by the first "Project review" session (session_01G2NZj4f5xWWrXLRuDN5eyd) for whoever takes the
role next. This file lives on the branch `claude/bold-feynman-cgnhjs`. The project itself lives on `ai/v2`.

## What this role is

The user is making the Civilization V AI competent in their own mod (Community Patch 5.4.8 plus their leader-screen
patch), so they and a friend have a real opponent in multiplayer. An **implementer** session (Opus) does the work on
branch `ai/v2`. Two Windows PCs ("BOX" and "TREEDESKTOP") run test games from a queue in the repository.

The reviewer does not change the AI. It:

1. **Checks on the project when the user asks** ("check on the project", "check everything") and reports what is
   true: progress, what changed, what is stuck, what needs the user. Short, most important thing first.
2. **Reviews critically.** Reads the branch, the logbook and the results, recounts numbers where it matters, and says
   plainly when a result is weaker than it looks, including when the reviewer's own earlier reading was wrong.
3. **Relays the user's instructions** to the implementer, and passes on its own suggestions once the user agrees.
4. **Starts the next implementer** when the current one writes `HANDOVER READY` (see "Starting an implementer").

How the user likes it: plain words (say "step 3: the AI builds the army in time", not "M3"; "the mod", not "the pack");
times in US Eastern; no approvals or questions overnight; spend is not a concern; they often answer "do it" to a
suggestion, so make each suggestion specific enough to act on and say exactly what you will send.

## Where everything is

| Thing | Where |
|---|---|
| The plan, rules, milestones | `ai-lab/PLAN.md` on `ai/v2` (read it in full once) |
| Logbook: current state at the top, rulings, one entry per change | `ai-lab/LOG.md` on `ai/v2`; closed steps in `LOG-archive.md` |
| Code map (functions, gates, where the plan overrides the stock AI) | `ai-lab/CODEMAP.md` on `ai/v2` |
| Result tables | `ai-lab/BASELINE.md` on `ai/v2` |
| Run queue and watchers | `ai-lab/runs/` on `ai/v2` (`README.md` explains it; `queue/`, `done/`, `launches/`, `status/`) |
| Results (about 18 GB of logs) | `ai-lab/siege/results/` on `ai/v2`: don't check it out; read single files with `git show` |
| The lab scripts | `ai-lab/siege/` on `ai/v2` |
| This file and the comparison script | `ai-lab/review/` on `claude/bold-feynman-cgnhjs` |
| Dashboard the implementer keeps | https://claude.ai/artifact/VuEFfTGtehw613tQvRx9Gj |

The stock baseline is branch `mp-leaderheads`. `master` is newer than 5.4.8 and is not what the mod uses. The earlier,
failed experiment (branches `ai/war` and others; notebook in the repository `treezeegaga/Civ5-CP-AnimatedLeaders`) is
reference only.

## How to check on the project

From a clone of `treezeegaga/Community-Patch-DLL` (public):

```
git fetch origin '+refs/heads/ai/v2:refs/remotes/origin/ai/v2'
git show origin/ai/v2:ai-lab/LOG.md | head -60            # current state, open items, next steps
git log --format='%h %ad %an | %s' --date=format:'%m-%d %H:%M' -80 origin/ai/v2 \
  | grep -v 'Claim \|: exit 0\|watcher: \|Merge branch\|Lab PC '   # the commits that matter
git show origin/ai/v2:ai-lab/runs/status/BOX.json          # and TREEDESKTOP.json: what each PC is doing
```

Pending work for the PCs is every file in `ai-lab/runs/queue/` with no file of the same name in `ai-lab/runs/done/`;
finished launches of a request are the non-claim files under `ai-lab/runs/launches/<request>/`. The implementer's
commits are in UTC; the PCs' commits are in US Eastern. `list_sessions` shows which implementer is live; its
conversation usually can't be read from here, so the branch is the record.

Things worth looking at every time: when the last implementer commit was (a long gap means it has stopped); whether
the queue is empty with idle PCs; whether the logbook's current state is fresh; whether the newest results are on the
tuning maps or the untouched ones; anything listed as waiting for the user.

`ai-lab/review/compare_games.py` (this branch) compares groups of full games from the branch without a checkout:
captures, wars, army, and the civ's state at turn 100, plus a same-map comparison of two groups. Its docstring has
the command for the 48 measuring maps.

## Where the project stands (2026-11-22, morning)

Steps 1 and 2 are closed: an army at the walls takes the city (staged sieges: 74 of 90 against stock's 54), and an
army starting at home marches and takes it (42 of 42 against stock's 0 of 42). Step 3 is in progress: the AI builds
the army in time and uses it. An "early rush" plan for warmonger civs was built from the user's own game (one
settler, no buildings, Archers upgraded to Composite Bowmen the turn the research lands, then march).

The measuring set is 48 maps (seeds 1919 to 6666) that were never used to tune anything, on the build of iteration 22:

| Aggressive test civ (Songhai), 48 maps | New AI | Stock AI |
|---|---|---|
| Builds the full rush army | 42, typically by turn 56 | 1 |
| Goes to war with a major civ | 38, typically turn 67 | 10 |
| Takes a major civ's city by turn 80 / 100 | 9 / 13 | 0 / 0 |
| Takes a capital | 7 (turns 49 to 72) | 0 |
| Meets the bar (a capital by about turn 60) | 1, or 2 counting turn 61 | 0 |
| At turn 100: population / techs / science | 11.6 / 17.7 / 10.8 | 15.7 / 20.7 / 26.3 |

So: the new AI arms and fights where stock never does, wins about one war in three, almost never meets the bar, and
is left well behind stock when the rush takes nothing. Other warmonger civs with the plan built the army in 24 games
and took a city in 4. Greece is the neighbour on all 48 maps (the remembered game setup), so results lean on Greece.

Open directions, already relayed to the implementer: the stalled sieges (25 of 38 wars take nothing; copy real
stalled sieges into the battle lab and fix them there); a way out of the plan so the civ rebuilds its economy;
varying the neighbour; the late-declaration tail; army completion (upgrades trail 10 to 25 turns after the research).
Still ahead in the plan: step 4 (who to attack, when, when to stop), step 5 (the other styles), step 6 (diplomacy
toward humans, the user's original complaint), step 7 (whole games, then the user and their friend play), a
multiplayer smoke test on the two PCs, and installing the finished mod on both PCs.

## Update, 2026-11-23, 8:35 AM ET (Project review 2)

- **Live implementer: "Civ V AI upgrade: implementer 10", session_01G2NZj21h7F5vCgt58ZnUJc**, created by Project
  review 2 at 8:33 AM ET from implementer 9's HANDOVER READY state. Implementer 9 (session_01G2NZj26YonLmNaosfnLk2R)
  was auto-compacted overnight before it handed over; watch the context figure (`get_session`) and ask for the
  handover before 650,000, not after.
- **Accepted build `ab-b8c9ffb`**: iteration 26 (a bowman holds only where it can shoot the city; this was the
  project's own bug from the march work) and iteration 25 (city-states off limits until turn 90 if no city taken).
  Reverted since: 27 (no healing near a takeable city) and 28 (marching army walks its path; lab faster, real games
  lost captures).
- **Headline on the 48 measuring maps, that build** (recounted with `compare_games.py`, labels `m25N`): a major's
  city by turn 80 / 100 on 13 / 16 (was 9 / 13), 9 capitals (7), 2 meet the bar (1); army complete about turn 56,
  first war about turn 68; wars that take nothing 23 of 39; science at turn 100 11.4 against stock's 26.3.
- **The stalled-war split was recounted** (`recount_stalls.py` here): judge over all of a major's cities and check
  the city's owner; the implementer's first split misfiled three games.
- **Next list (provisional, the reviewer's suggestion under the user's "do it all")**: the late start of the war
  first, then the way out of the rush plan, then varying the neighbour; seed 505 and the other stalls last.
- **BOX is paused by the user** (since 2:45 PM Nov 22) until they say it is back; its watcher ran one request on the
  morning of Nov 23 all the same. TREEDESKTOP carries the runs.
- **Messages work both ways with implementers 9 and 10** (send_message), unlike with the first review session.

## PAUSED until Thursday 2026-11-26 (user, 8:15 PM ET Nov 23)

The user paused everything. Implementer 10 was told to withdraw the unstarted games of `2026-11-23-12-i31`, leave
iteration 31 (the plan buys the army unit it still lacks with spare gold) undecided, mark the logbook PAUSED and
stop without a wake-up. Nobody resumes on their own: wait for the user. On resuming, check that the logbook says
PAUSED and the queue is empty, then relay "resume" to implementer 10 (it runs in default permission mode and was
stuck on a permission prompt from 5:25 PM Nov 23; ask the user to switch it to Auto). Since Nov 23 afternoon,
full-game changes are decided on 36 tuning maps (the 12 plus 7070-9393) by the net result (user's ruling via
Review 2); the implementer's table of what each first attack set out with is in the LOG (Nov 23).

## Agreements with the user

- **The review session speaks for the user** to the implementer, within what the plan allows (PLAN section 4). It
  never widens access and stands in for a sign-off only when the user gave one. The clause names a session ID: since
  2026-11-22 (about 10:10 AM ET) it names "Project review 2", session_01G2NZizyLCf9i5g45DQDYLB; the first session's
  relays no longer count. A later reviewer has a new ID, so the clause must be updated again before the implementer
  will accept its relays. Implementer 9 can message this reviewer directly (send_message works both ways).
- **No approvals or questions between about 11 PM and 7:30 AM Eastern.** Rulings wait on a list; the implementer adopts
  its own proposal provisionally.
- **Keep the PCs busy overnight.** The implementer queues several hours of decision-free runs before it goes quiet.
- **Step 3's bar** (the user's ruling): war by about turn 41 to 50, a capital by about turn 60 on Quick, about 6 melee
  and 4 Composite Bowmen, at least 2 cities, gold not at zero before the war is decided.
- **Losses don't count against the AI; the capture does** (ruling of 2026-11-18), but units lost stay a reported column.
- **Decisions on six or more games on fixed maps, before and after;** untouched maps are never used to choose a change.
- **Screenshots capture only the Civ V window.** Full-desktop screenshots were purged from the branch history and the
  old build tags deleted on 2026-11-21 to 22. GitHub may keep the old commits reachable by direct link; only GitHub
  Support can purge those. The repository is public: results pushed there are public.
- **Handover of implementers** at a step's sign-off or past 650,000 tokens of context; the successor archives its
  predecessor.

## Starting an implementer

When the logbook's current-state heading says `HANDOVER READY`, create a session: model `claude-opus-5-5`, title
`Civ V AI upgrade: implementer <n+1>`, tag `civ5-ai-lab`, source this repository at `ai/v2`, outcome branch `ai/v2`,
and as prompt the block under "The successor's prompt, word for word" in PLAN section 12. Session chains have a depth
limit (implementer 8 was at 8 of 8 because each had created the next); one created by the reviewer starts a fresh
chain. It starts in the reviewer's permission mode; if that isn't Auto, ask the user to switch it in the app.

## What doesn't work (so don't spend time on it)

- **Scheduled tasks.** A scheduled message into this session or into an implementer is refused ("access denied"). A
  scheduled task that starts a fresh session runs, but that session has no tools to list, message or create
  sessions. Routines made in the Claude app's own page were not tried.
- **Staying awake overnight.** A timed background wait re-invoked the reviewer twice (45 minutes each), then the
  session was stopped for eight hours, as the implementers' were. Nothing started from a cloud session survives the
  night; only the queue on the PCs does.
- **Reading or archiving some sessions.** Sessions limited to the user's trusted devices refuse `list_events`,
  `send_message` and `archive_session` from here. The implementer could not message this session for the same reason.
- **Pushing.** This role's session could push only to `claude/bold-feynman-cgnhjs`: not to `ai/v2`, and it could not
  delete tags. Changes on `ai/v2` go through the implementer; things that need the user's own push access can go
  through a session on one of their PCs, with their say-so.

## Lessons from reviewing this project

- **Check the claim, not the summary.** Recount from the result files when a number carries a decision. The
  logbook has been honest, including about its own miscounts, but a headline can still rest on the wrong set of maps.
- **Lab results are not game results.** Three steps looked finished on staged battles while real games showed nothing;
  the real-game check and then the untouched maps each changed the picture. Ask for the real-game number early.
- **Small samples lie in both directions.** Three games on different maps decided several changes; six untouched
  maps looked much worse than 48 did. Prefer the same maps before and after, and enough of them.
- **Watch for the old failure.** The earlier experiment died of judging everything through whole games, changing many
  things at once, and layering overrides on the stock AI. The plan's rules exist against that; the list of overrides
  in CODEMAP is the thing to keep short.
- **Say what you could not see.** Much of the time the implementer's conversation is closed to the reviewer.
