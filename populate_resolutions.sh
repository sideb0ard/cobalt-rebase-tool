#!/bin/bash
set -e

# This script back-populates the conflicts directory from a finished,
# resolved branch. It finds every commit that was originally a conflict,
# recreates the conflicted state, and saves it along with the final resolution.

CONFLICTS_DIR=$1
RESOLVED_BRANCH=$2

if [ -z "$CONFLICTS_DIR" ] || [ -z "$RESOLVED_BRANCH" ]; then
  echo "Usage: $0 <path-to-conflicts-dir> <resolved-branch-name>"
  exit 1
fi

if [ ! -d "$CONFLICTS_DIR" ]; then
  echo "Error: Conflicts directory not found at '$CONFLICTS_DIR'"
  exit 1
fi

echo "🔍 Starting back-population process..."
echo "Source Branch: $RESOLVED_BRANCH"
echo "Conflicts Dir: $CONFLICTS_DIR"
echo "-------------------------------------------------"

# Get the list of original commit SHAs that had conflicts by reading the directory names.
ORIGINAL_CONFLICT_SHAS=$(find "$CONFLICTS_DIR" -mindepth 1 -maxdepth 1 -type d -exec basename {} \; | sort -u)

for short_sha in $ORIGINAL_CONFLICT_SHAS; do
  echo "Processing original commit SHA prefix: $short_sha"

  # Find the full original SHA from the commits.json plan
  original_sha_full=$(jq -r --arg prefix "$short_sha" '.[] | select(.hexsha | startswith($prefix)) | .hexsha' commits.json)
  if [ -z "$original_sha_full" ]; then
    echo "   ⚠️ Could not find full SHA for prefix $short_sha in commits.json. Skipping."
    continue
  fi
  echo "   Found full original SHA: $original_sha_full"

  # Find the corresponding commit on the resolved branch
  resolved_commit_sha=$(git log "$RESOLVED_BRANCH" --grep="original-hexsha: $original_sha_full" --format=%H)
  if [ -z "$resolved_commit_sha" ]; then
    echo "   ⚠️ Could not find the corresponding resolved commit on '$RESOLVED_BRANCH'. Skipping."
    continue
  fi
  echo "   Found resolved commit: $resolved_commit_sha"

  parent_of_resolved=$(git rev-parse "$resolved_commit_sha^")
  commit_record_dir="$CONFLICTS_DIR/$short_sha"
  conflict_snapshot_dir="$commit_record_dir/conflict"
  resolved_snapshot_dir="$commit_record_dir/resolved"

  # 1. Recreate and snapshot the original conflict state
  echo "   Recreating original conflict state..."
  mkdir -p "$conflict_snapshot_dir"
  git checkout -b temp-conflict-recreation "$parent_of_resolved" > /dev/null 2>&1
  
  # This cherry-pick is expected to fail and create conflicts
  git cherry-pick --no-commit "$original_sha_full" > /dev/null 2>&1 || true
  
  conflicted_files=$(git status --porcelain | grep -E "^(UU|AA|DD|AU|UA|UD|DU) " | awk '{print $2}')
  if [ -z "$conflicted_files" ]; then
      echo "   ⚠️ Cherry-pick did not result in a conflict. It may have been an empty commit. Skipping snapshot."
      git cherry-pick --abort > /dev/null 2>&1
      git checkout "$RESOLVED_BRANCH" > /dev/null 2>&1
      git branch -D temp-conflict-recreation > /dev/null 2>&1
      continue
  fi

  echo "   Snapshotting conflicted files to '$conflict_snapshot_dir'"
  for file in $conflicted_files; do
    # Ensure the destination directory exists
    mkdir -p "$(dirname "$conflict_snapshot_dir/$file")"
    # Copy the file, preserving its path
    cp "$file" "$conflict_snapshot_dir/$file"
  done

  git cherry-pick --abort > /dev/null 2>&1
  git checkout "$RESOLVED_BRANCH" > /dev/null 2>&1
  git branch -D temp-conflict-recreation > /dev/null 2>&1

  # 2. Snapshot the final, resolved state
  echo "   Snapshotting final resolution from $resolved_commit_sha..."
  mkdir -p "$resolved_snapshot_dir"
  # Use git show to extract the content of the files from the resolved commit
  for file in $conflicted_files; do
    # Ensure the destination directory exists
    mkdir -p "$(dirname "$resolved_snapshot_dir/$file")"
    # Extract the file content from the resolved commit and save it
    git show "$resolved_commit_sha:$file" > "$resolved_snapshot_dir/$file"
  done
  
  echo "   ✅ Successfully processed $short_sha"
  echo "-------------------------------------------------"
done

echo "🎉 All conflicts have been processed and resolutions have been saved."
