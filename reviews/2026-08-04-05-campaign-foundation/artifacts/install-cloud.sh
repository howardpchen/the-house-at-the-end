#!/bin/sh
set +e

project=/home/oc/.openclaw/workspace-doClawCoder/projects/the-house-at-the-end
artifact="$project/reviews/2026-08-04-05-campaign-foundation/artifacts/the-house-at-the-end-0.2.0.pbw"
log="$project/reviews/2026-08-04-05-campaign-foundation/logs/cloud-install.log"
status="$project/reviews/2026-08-04-05-campaign-foundation/logs/cloud-install.status"

cd "$project" || exit 125
/home/oc/.local/bin/pebble install --cloudpebble "$artifact" >"$log" 2>&1
result=$?
printf '%s\n' "$result" >"$status"
exit "$result"
