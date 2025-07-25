# Advanced Git Rebase and Porting Tool

## Overview

This Python script is a powerful, specialized tool designed to automate and manage the complex process of rebasing a long series of commits from one branch to another. It is particularly useful for porting changes between major versions of a codebase where a standard `git rebase` would be impractical due to numerous and repeated conflicts.

The core philosophy of the tool is to make a long-running rebase **resumable, repeatable, and collaborative**. It achieves this by snapshotting conflict resolutions and providing intelligent automation for common and complex scenarios.

## Key Features

- **Linearized History Creation**: Creates a clean, linear branch from a complex history, replacing merge commits with their resulting file trees.
- **Automated Rebase Planning**: Generates a JSON-based "rebase plan" from the linearized history, which serves as the source for the rebase operation.
- **Intelligent Conflict Resolution**:
    - Automatically saves the resolution for any conflict it encounters.
    - On subsequent runs, it automatically reuses previously saved resolutions, even for complex structural conflicts.
    - Provides multiple automated strategies for handling new, unseen conflicts.
- **Specialized Conflict Handling**:
    - Automatically detects and fixes problematic "subtree import" commits that often cause file/directory conflicts.
    - Gracefully handles empty commits that result from the rebase.
- **Collaborative Workflow**: Includes a mode to commit conflicts as-is, allowing a team to parallelize the work of resolving a large number of conflicts.
- **Test Mode**: Provides a `--test-commit` flag to debug the script's logic on a single, troublesome commit without running the entire rebase.
- **Summary Reporting**: Generates a detailed report at the end of a rebase, summarizing what happened.

## Standard Workflow

The primary workflow consists of two main steps: creating a plan and executing it.

### Step 1: Create a Rebase Plan

First, you use the `create-rebase-plan` command. This command reads the history of your source branch, linearizes it, and creates a new, clean branch. It then extracts the commit information from this new branch into a `commits.json` file.

```bash
python3 main.py create-rebase-plan \
    --repo-path /path/to/your/repo \
    --source-branch main \
    --start-commit-ref <start-commit-sha> \
    --end-commit-ref <end-commit-sha> \
    --new-branch-name linear-main-plan \
    --output-file commits.json
```

This step ensures the rebase will operate on a clean, linear history, which is crucial for its success.

### Step 2: Execute the Rebase

Next, you use the `rebase` command. This command reads the `commits.json` file and applies each commit, one by one, to your target branch.

```bash
python3 main.py rebase \
    --repo-path /path/to/your/repo \
    --rebase-branch om120 \
    --commits-file commits.json \
    --conflicts-dir m114_to_m120_conflicts
```

When a conflict occurs, the script will stop and wait for you to resolve it. Once you have fixed the conflict and staged the changes, you can press Enter to continue. The script will automatically save your resolution in the `--conflicts-dir` for future use.

## Conflict Resolution Strategies

The `rebase` command has powerful flags to control how it handles new, unseen conflicts.

### Normal Mode (Default)

- **Usage**: Run the `rebase` command with no special flags.
- **Behavior**: When a new conflict occurs, the script stops and prompts you to resolve it manually. After you fix the conflict and stage the changes, the script saves your resolution and continues. This is the recommended mode for creating high-quality, reusable resolutions.

### YOLO Mode (`--yolo`)

- **Usage**: `python3 main.py rebase --yolo ...`
- **Behavior**: When a new conflict occurs, the script does not stop. It automatically resolves the conflict using a "theirs" strategy (`git checkout --theirs` or `git rm`). It then saves this automated resolution and continues.
- **Best For**: Fast, fully automated runs where you trust the "theirs" strategy or plan to review the results later. **Note:** This mode will still prioritize using a previously saved manual resolution if one exists.

### Commit Conflicts Mode (`--commit-conflicts`)

- **Usage**: `python3 main.py rebase --commit-conflicts ...`
- **Behavior**: When a conflict occurs, the script does not try to resolve it. Instead, it commits the conflicted state as-is, with conflict markers (`<<<<<<<`), into a new commit with a special `[CONFLICT]` message.
- **Best For**: Collaborative workflows. This allows a long rebase to run to completion, creating a "to-do list" of conflict commits that can be divided among team members for resolution later using `git rebase -i`.

## Commands and Options

### `create-rebase-plan`

Creates a linearized branch and a JSON file for rebasing.

- `--repo-path`: Path to the local repository.
- `--source-branch`: The branch to linearize (e.g., `main`).
- `--start-commit-ref`: The starting commit of the range.
- `--end-commit-ref`: The ending commit of the range.
- `--new-branch-name`: The name of the new, clean branch to create.
- `--output-file`: The path to write the `commits.json` plan file.

### `rebase`

Rebases commits from a plan file onto a new branch.

- `--repo-path`: Path to the local repository.
- `--rebase-branch`: The branch to apply the commits to.
- `--commits-file`: The `commits.json` file to use as the plan.
- `--conflicts-dir`: The directory to store and retrieve conflict resolutions.
- `--last-successful-commit-ref`: (Optional) A commit SHA to resume from.
- `--yolo`: (Optional) Use the automated "theirs" strategy for new conflicts.
- `--commit-conflicts`: (Optional) Commit conflicts as-is for later resolution.
- `--test-commit`: (Optional) Run the rebase logic on only a single commit for debugging.

### `import-subtree`

A utility to correctly import a dependency from `DEPS` as a squashed subtree. This is useful for creating clean commits that can be handled smoothly by the rebase process.

- `--repo-path`: Path to the local repository.
- `--dependency-path`: The path of the dependency in the `DEPS` file (e.g., `third_party/boringssl/src`).
- `--branch`: The branch to create the new import commit on.

