#!/usr/bin/env bash
# Wait for lab results (the implementing session's standard way to wait for runs; PLAN.md section 7).
# Polls origin/ai/v2 every 2 minutes and exits as soon as a new done marker or launch result lands, or after
# MAX_MINUTES (default 110). Run it as a background command; when it exits, read what arrived, record it, queue
# what's next, and start it again. Prints the new files (paths on origin/ai/v2), or "timeout".
# Usage: ai-lab/runs/wait-for-results.sh [MAX_MINUTES]
set -u
cd "$(git rev-parse --show-toplevel)" || exit 2
MAX_MINUTES="${1:-110}"
list() {
	git ls-tree -r --name-only origin/ai/v2 -- ai-lab/runs/done ai-lab/runs/launches 2>/dev/null \
		| grep -E '\.json$' | grep -v '\.claim\.json$' | sort
}
git fetch -q origin ai/v2 2>/dev/null
before="$(list)"
deadline=$(( $(date +%s) + MAX_MINUTES * 60 ))
while [ "$(date +%s)" -lt "$deadline" ]; do
	sleep 120
	git fetch -q origin ai/v2 2>/dev/null || continue
	new="$(comm -13 <(printf '%s\n' "$before") <(list))"
	if [ -n "$new" ]; then
		echo "new results at $(TZ=America/New_York date '+%Y-%m-%d %H:%M ET'):"
		printf '%s\n' "$new"
		exit 0
	fi
done
echo "timeout after ${MAX_MINUTES} minutes ($(TZ=America/New_York date '+%H:%M ET')), nothing new"
exit 0
