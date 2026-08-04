#!/bin/sh
set +e

project=/home/oc/.openclaw/workspace-doClawCoder/projects/the-house-at-the-end
log="$project/reviews/2026-08-04-01-fire-tiers/logs/cloud-install.log"
status="$project/reviews/2026-08-04-01-fire-tiers/logs/cloud-install.status"

cd "$project" || exit 125
/home/oc/.local/bin/pebble install --cloudpebble >"$log" 2>&1
result=$?
printf '%s\n' "$result" >"$status"
exit "$result"
