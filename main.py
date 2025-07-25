import argparse
import git
import glob
import json
import os
import shutil
import subprocess
import re
import tempfile


def find_deepest_common_dir(paths):
    if not paths:
        return '/'
    
    dir_paths = [os.path.dirname(p) for p in paths]
    split_paths = [p.split('/') for p in dir_paths]
    
    if split_paths and split_paths[0] and split_paths[0][0] == '':
        for p in split_paths:
            if not p or p[0] != '':
                return ''
        
        min_len = min(len(p) for p in split_paths)
        common_path = ['']
        for i in range(1, min_len):
            if all(split_paths[j][i] == split_paths[0][i] for j in range(len(split_paths))):
                common_path.append(split_paths[0][i])
            else:
                break
        
        return os.path.join(*common_path) if len(common_path) > 1 else '/'
    else:
        min_len = min(len(p) for p in split_paths)
        common_path = []
        for i in range(min_len):
            if all(split_paths[j][i] == split_paths[0][i] for j in range(len(split_paths))):
                common_path.append(split_paths[0][i])
            else:
                break
        return '/'.join(common_path) if common_path else ''


class RebaseLogger:
    """A simple class to track and report events during a rebase."""
    def __init__(self):
        self.success_count = 0
        self.skipped_empty_count = 0
        self.resolved_by_patch_count = 0
        self.resolved_by_yolo_count = 0
        self.resolved_manually_count = 0
        self.committed_as_conflict_count = 0
        self.subtree_fixup_count = 0
        self.total_commits = 0

    def set_total_commits(self, total):
        self.total_commits = total

    def log_success(self):
        self.success_count += 1

    def log_skip(self):
        self.skipped_empty_count += 1

    def log_conflict_resolved(self, method):
        if method == 'patch':
            self.resolved_by_patch_count += 1
        elif method == 'yolo':
            self.resolved_by_yolo_count += 1
        elif method == 'manual':
            self.resolved_manually_count += 1
    
    def log_committed_as_conflict(self):
        self.committed_as_conflict_count += 1

    def log_subtree_fixup(self):
        self.subtree_fixup_count += 1

    def print_summary(self):
        print("\n" + "="*50)
        print("🎉 Rebase Complete: Summary Report")
        print("="*50)
        print(f"Total Commits Processed: {self.total_commits}")
        print("-" * 20)
        print(f"✅ Applied Cleanly:          {self.success_count}")
        print(f"🛠️ Resolved by Patch:        {self.resolved_by_patch_count}")
        print(f"🤖 Resolved by YOLO:         {self.resolved_by_yolo_count}")
        print(f"👤 Resolved Manually:        {self.resolved_manually_count}")
        print(f"💥 Committed as Conflict:    {self.committed_as_conflict_count}")
        print(f"🌳 Replaced (Subtree Fixup): {self.subtree_fixup_count}")
        print(f"⏩ Skipped (Empty):          {self.skipped_empty_count}")
        print("="*50)


def create_rebase_plan(repo, args):
    start_commit = repo.commit(args.start_commit_ref)
    end_commit = repo.commit(args.end_commit_ref)

    print(f"Creating new linearized branch '{args.new_branch_name}'...")
    if args.new_branch_name in repo.heads:
        repo.delete_head(args.new_branch_name, force=True)
    new_branch = repo.create_head(args.new_branch_name, start_commit)
    repo.head.reference = new_branch
    repo.head.reset(index=True, working_tree=True)
    print(f'✅ Created new branch "{args.new_branch_name}" at {args.start_commit_ref}')

    ancestry_path_output = repo.git.log(
        '--first-parent', '--reverse', '--pretty=format:%H',
        f'{start_commit.hexsha}..{end_commit.hexsha}'
    )
    commits_to_replay = [repo.commit(sha) for sha in ancestry_path_output.strip().split('\n')] if ancestry_path_output.strip() else []
    print(f'Found {len(commits_to_replay)} commits to linearize.')

    for i, commit in enumerate(commits_to_replay, 1):
        print(f'Applying commit {i}/{len(commits_to_replay)}: {commit.hexsha[:8]} {commit.summary}')
        if len(commit.parents) > 1:
            tree = commit.tree
            repo.git.read_tree(tree.hexsha)
            new_tree_hexsha = repo.git.write_tree()
            git.objects.commit.Commit.create_from_tree(
                repo=repo, tree=new_tree_hexsha,
                message=commit.message + f'\noriginal-hexsha: {commit.hexsha}',
                parent_commits=[repo.head.commit], head=True,
                author=commit.author, committer=commit.committer
            )
            repo.git.reset('--hard')
        else:
            repo.git.cherry_pick('--no-commit', commit.hexsha)
            repo.git.commit(
                '--allow-empty', '--no-verify',
                '--author', f'{commit.author.name} <{commit.author.email}>',
                '--date', commit.authored_datetime.isoformat(),
                '--message', commit.message + f'\noriginal-hexsha: {commit.hexsha}',
                env={
                    'GIT_COMMITTER_NAME': commit.committer.name,
                    'GIT_COMMITTER_EMAIL': commit.committer.email,
                    'GIT_COMMITTER_DATE': commit.committed_datetime.isoformat(),
                }
            )
        repo.git.clean('-fdx')

    print('\n🔍 Verifying linearized branch content...')
    diff = repo.git.diff(args.new_branch_name, args.end_commit_ref)
    if diff:
        print('❌ ERROR: Linearized branch content does not match original end commit.')
        return
    else:
        print('✅ Linearized branch matches original end commit.')

    print(f"\n📄 Creating rebase plan file: {args.output_file}")
    repo.git.checkout(args.new_branch_name)
    
    linear_commits = list(repo.iter_commits(f"{start_commit.hexsha}..{args.new_branch_name}"))
    linear_commits.reverse()

    commit_data = []
    for commit in linear_commits:
        stats = commit.stats
        commit_data.append({
            'hexsha': commit.hexsha,
            'author': commit.author.name,
            'datetime': commit.authored_datetime.isoformat(),
            'summary': commit.summary,
            'message': commit.message,
            'stats': {
                'files': stats.total['files'],
                'insertions': stats.total['insertions'],
                'deletions': stats.total['deletions'],
            },
            'is_merge': False
        })
    
    with open(args.output_file, 'w', encoding='utf-8') as f:
        json.dump(commit_data, f, indent=2)
    print(f'✅ Extracted {len(commit_data)} commits to {args.output_file}')
    print('\n🎉 SUCCESS: Rebase plan created successfully!')


def get_conflicted_files_with_status(repo):
    conflicted_files = {}
    status_output = repo.git.status('--porcelain').splitlines()
    conflict_prefixes = ('UU ', 'AA ', 'DD ', 'UD ', 'DU ', 'AU ', 'UA ')
    for line in status_output:
        if line.startswith(conflict_prefixes):
            status = line[:2].strip()
            file_path = line[3:].strip()
            conflicted_files[file_path] = status
    return conflicted_files


def record_conflict(repo, conflicts_dir, logger, original_commit, yolo=False):
    resolution_method_used = None
    commit_id = original_commit['hexsha']
    commit_record_dir = os.path.join(conflicts_dir, commit_id[:7])
    conflict_dir = os.path.join(commit_record_dir, 'conflict')
    resolved_dir = os.path.join(commit_record_dir, 'resolved')
    os.makedirs(conflict_dir, exist_ok=True)
    os.makedirs(resolved_dir, exist_ok=True)

    print(f'\n💾 Recording conflict for {commit_id[:7]} to: {commit_record_dir}')
    
    conflicted_files_with_status = get_conflicted_files_with_status(repo)
    if not conflicted_files_with_status:
        print("   Cherry-pick failed but no file conflicts were found. This is likely an empty commit.")
        print("   Skipping this commit to continue the rebase...")
        repo.git.cherry_pick('--skip')
        print("   ✅ Commit skipped.")
        logger.log_skip()
        return False, None

    conflicted_files = list(conflicted_files_with_status.keys())
    print('   Snapshotting conflicted state...')
    for file_path in conflicted_files:
        source_path = os.path.join(repo.working_dir, file_path)
        if os.path.exists(source_path):
            dst_path = os.path.join(conflict_dir, file_path)
            os.makedirs(os.path.dirname(dst_path), exist_ok=True)
            if os.path.isdir(source_path):
                shutil.copytree(source_path, dst_path, dirs_exist_ok=True)
            else:
                shutil.copy2(source_path, dst_path)

    repo.git.cherry_pick('--abort')
    repo.git.reset('--hard', 'HEAD')

    if os.path.exists(resolved_dir) and os.listdir(resolved_dir):
        print(f"   Found existing resolution in {resolved_dir}. Applying it directly.")
        for item in os.listdir(resolved_dir):
            src_path = os.path.join(resolved_dir, item)
            dest_path = os.path.join(repo.working_dir, item)
            if os.path.isdir(src_path):
                shutil.copytree(src_path, dest_path, dirs_exist_ok=True)
            else:
                shutil.copy2(src_path, dest_path)
        
        repo.git.add(all=True, force=True)
        resolution_method_used = 'patch'
    
    else:
        if yolo:
            print("   ⚠️  No previous resolution found. Using YOLO mode 'theirs' strategy...")
            try:
                repo.git.cherry_pick(commit_id)
            except git.exc.GitCommandError:
                pass
            
            try:
                for file_path, status in get_conflicted_files_with_status(repo).items():
                    if status in ('UD', 'DD', 'AU'):
                        repo.git.rm(file_path)
                    else:
                        repo.git.checkout('--theirs', file_path)
                        repo.git.add(file_path, force=True)
                
                if not get_conflicted_files_with_status(repo):
                    print('   ✅ YOLO resolution successful.')
                    resolution_method_used = 'yolo'
                else:
                    print('   ❌ YOLO resolution failed to clear all conflicts.')
                    repo.git.cherry_pick('--abort')
                    raise git.exc.GitCommandError("YOLO failed to resolve complex conflict", 1)
            except git.exc.GitCommandError as yolo_e:
                print(f"   ❌ YOLO resolution failed during operation: {yolo_e}")
                repo.git.cherry_pick('--abort')
                raise yolo_e
        else:
            resolution_method_used = 'manual'
            print('   ⚠️  Manual conflict resolution required.')
            try:
                repo.git.cherry_pick(commit_id)
            except git.exc.GitCommandError:
                pass
            input('      Press Enter after resolving conflicts to continue...')
            repo.git.add(all=True, force=True)

        print('   Snapshotting new resolution...')
        for file_path in conflicted_files:
            source_path = os.path.join(repo.working_dir, file_path)
            if os.path.exists(source_path):
                resolved_file_path = os.path.join(resolved_dir, file_path)
                os.makedirs(os.path.dirname(resolved_file_path), exist_ok=True)
                if os.path.isdir(source_path):
                    shutil.copytree(source_path, resolved_file_path, dirs_exist_ok=True)
                else:
                    shutil.copy2(source_path, resolved_file_path)

    original_commit_obj = repo.commit(commit_id)
    author_date = original_commit_obj.authored_datetime.strftime("%Y-%m-%d %H:%M:%S %z")
    commit_date = original_commit_obj.committed_datetime.strftime("%Y-%m-%d %H:%M:%S %z")
    
    repo.index.commit(
        message=original_commit_obj.message,
        author=original_commit_obj.author,
        committer=original_commit_obj.committer,
        author_date=author_date,
        commit_date=commit_date
    )
    
    logger.log_conflict_resolved(resolution_method_used)
    new_commit_sha = repo.head.commit.hexsha
    print(f"   ✅ Conflict resolved. Created new commit: {new_commit_sha}")
    return True, new_commit_sha


def perform_subtree_import(repo, dependency_path, dep_url, dep_commit):
    with tempfile.TemporaryDirectory() as temp_dir:
        print(f"   Cloning {dep_url} into temporary directory...")
        dep_repo = git.Repo.clone_from(dep_url, temp_dir)
        print(f"   Checking out specific commit: {dep_commit}")
        dep_repo.git.checkout(dep_commit)
        shutil.rmtree(os.path.join(temp_dir, '.git'))
        dest_path = os.path.join(repo.working_dir, dependency_path)
        print(f"   Copying files to {dest_path}")
        if os.path.exists(dest_path):
            shutil.rmtree(dest_path)
        shutil.copytree(temp_dir, dest_path)
        repo.git.add(dependency_path, force=True)


def import_subtree(repo, args):
    print(f"Attempting to import '{args.dependency_path}' as a subtree on branch '{args.branch}'...")
    repo.git.checkout(args.branch)
    deps_path = os.path.join(repo.working_dir, 'DEPS')
    if not os.path.exists(deps_path):
        print(f"❌ Error: DEPS file not found at {deps_path}")
        return

    dep_url, dep_commit = None, None
    dependency_path_pattern = re.compile(rf"['\"]{re.escape(args.dependency_path)}['\"]")
    with open(deps_path, 'r', encoding='utf-8') as f:
        deps_content = f.read()
    for line in deps_content.splitlines():
        if dependency_path_pattern.search(line):
            match = re.search(r"['\"]([^']+)@([^']+)['\"]", line)
            if match:
                dep_url, dep_commit = match.groups()
                break
    if not dep_url or not dep_commit:
        print(f"❌ Error: Could not find dependency info for '{args.dependency_path}' in DEPS.")
        return

    perform_subtree_import(repo, args.dependency_path, dep_url, dep_commit)
    
    print("   Updating DEPS file...")
    new_deps_lines = [line for line in deps_content.splitlines() if not dependency_path_pattern.search(line)]
    with open(deps_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(new_deps_lines))
    repo.git.add(deps_path)
    
    commit_message = (f"Import {args.dependency_path} as a subtree\n\n"
                    f"Removes {args.dependency_path} from DEPS and imports from {dep_commit}.")
    repo.index.commit(commit_message)
    print(f"\n🎉 SUCCESS: Imported '{args.dependency_path}' as a subtree.")


def main():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='command')

    plan_parser = subparsers.add_parser('create-rebase-plan', help='Create a linearized branch and a JSON file for rebasing.')
    plan_parser.add_argument('--repo-path', type=str, required=True)
    plan_parser.add_argument('--source-branch', type=str, required=True)
    plan_parser.add_argument('--start-commit-ref', type=str, required=True)
    plan_parser.add_argument('--end-commit-ref', type=str, required=True)
    plan_parser.add_argument('--new-branch-name', type=str, required=True)
    plan_parser.add_argument('--output-file', type=str, default='commits.json')

    rebase_parser = subparsers.add_parser('rebase', help='Rebase commits onto a new branch.')
    rebase_parser.add_argument('--repo-path', type=str, required=True)
    rebase_parser.add_argument('--rebase-branch', type=str, required=True)
    rebase_parser.add_argument('--commits-file', type=str, default='commits.json')
    rebase_parser.add_argument('--conflicts-dir', type=str, default='conflicts')
    rebase_parser.add_argument('--last-successful-commit-ref', type=str, default=None)
    rebase_parser.add_argument('--yolo', action='store_true', help='Automatically resolve all conflicts with "theirs" strategy. Use with caution.')
    rebase_parser.add_argument('--test-commit', type=str, default=None, help='Test the rebase logic on a single commit.')
    rebase_parser.add_argument('--commit-conflicts', action='store_true', help='Commit conflicts as-is with a special message for later resolution.')

    import_parser = subparsers.add_parser('import-subtree', help='Import a dependency from DEPS as a squashed subtree.')
    import_parser.add_argument('--repo-path', type=str, required=True)
    import_parser.add_argument('--dependency-path', type=str, required=True)
    import_parser.add_argument('--branch', type=str, default='main')

    args = parser.parse_args()
    repo = git.Repo(args.repo_path)

    if args.command == 'rebase' and args.yolo and args.commit_conflicts:
        print("❌ ERROR: --yolo and --commit-conflicts are mutually exclusive options.")
        return

    if args.command == 'create-rebase-plan':
        create_rebase_plan(repo, args)
    elif args.command == 'rebase':
        # Startup check and cleanup
        if repo.git.status('--porcelain'):
            print("⚠️  Repository is in a conflicted state. Aborting previous cherry-pick to start clean.")
            try:
                repo.git.cherry_pick('--abort')
            except git.exc.GitCommandError:
                print("   Cherry-pick abort failed. Resetting to HEAD.")
                repo.git.reset('--hard', 'HEAD')
        
        repo.git.checkout(args.rebase_branch)
        
        commits = []
        if args.test_commit:
            print(f"🧪 Running in test mode for single commit: {args.test_commit}")
            try:
                commit_obj = repo.commit(args.test_commit)
                commits.append({
                    'hexsha': commit_obj.hexsha,
                    'summary': commit_obj.summary,
                    'message': commit_obj.message
                })
            except git.exc.GitCommandError:
                print(f"❌ Error: Could not find test commit {args.test_commit}")
                return
        else:
            with open(args.commits_file, 'r', encoding='utf-8') as f:
                commits = json.load(f)
        
        logger = RebaseLogger()
        logger.set_total_commits(len(commits))

        last_successful_commit = None
        resume = args.last_successful_commit_ref is not None
        for i, commit in enumerate(reversed(commits), 1):
            if resume:
                if commit['hexsha'] == args.last_successful_commit_ref:
                    resume = False
                print(f'💤 {i}/{len(commits)} Skipped: {commit['hexsha']}')
                continue

            if commit.get('is_merge', False):
                print(f"❌ ERROR: Merge commit found in rebase plan: {commit['hexsha']}")
                return
            
            try:
                repo.git.cherry_pick(commit['hexsha'])
                last_successful_commit = repo.head.commit.hexsha
                print(f"✅ {i}/{len(commits)} cherry-picked successfully: {commit['hexsha']}")
                logger.log_success()
            except git.exc.GitCommandError as e:
                if args.commit_conflicts:
                    print(f"⚠️  Committing conflict for {commit['hexsha']} as-is...")
                    conflicted_files = list(get_conflicted_files_with_status(repo).keys())
                    
                    # Abort the cherry-pick so we can create our own commit.
                    repo.git.cherry_pick('--abort')

                    # Re-apply the conflicted state to the working directory
                    try:
                        repo.git.cherry_pick('--no-commit', commit['hexsha'])
                    except git.exc.GitCommandError:
                        # This is expected as it will result in a conflict.
                        pass

                    repo.git.add(all=True, force=True)
                    
                    original_commit_obj = repo.commit(commit['hexsha'])
                    conflict_message = (
                        f"[CONFLICT] Original Commit: {commit['hexsha']}\n\n"
                        f"Original Message:\n{original_commit_obj.message}\n\n"
                        f"Files with conflicts:\n" +
                        "\n".join(f"- {f}" for f in conflicted_files)
                    )
                    
                    repo.index.commit(
                        conflict_message,
                        author=original_commit_obj.author,
                        committer=original_commit_obj.committer,
                        author_date=original_commit_obj.authored_datetime.isoformat(),
                        commit_date=original_commit_obj.committed_datetime.isoformat()
                    )
                    
                    last_successful_commit = repo.head.commit.hexsha
                    logger.log_committed_as_conflict()
                    print(f"✅ Conflict for {commit['hexsha']} committed. Continuing rebase.\n")
                    continue

                handled = False
                is_import_commit = "Import " in commit.get('message', commit['summary'])
                is_directory_conflict = 'CONFLICT (file/directory)' in e.stdout

                if is_import_commit and is_directory_conflict:
                    print(f"⚠️  Detected import commit with directory conflict: {commit['hexsha']}. Attempting auto-fixup...")
                    # (Auto-fixup logic here)
                    logger.log_subtree_fixup()
                    pass # Placeholder

                if not handled:
                    print(f"❌ Failed to cherry-pick or auto-fix: {commit['hexsha']}")
                    try:
                        was_successful, new_sha = record_conflict(repo, args.conflicts_dir, logger, commit, yolo=args.yolo)
                        if was_successful:
                            last_successful_commit = new_sha
                            print(f"✅ {i}/{len(commits)} cherry-picked successfully after conflict: {commit['hexsha']}\n")
                        else:
                            print(f"⏩ {i}/{len(commits)} was skipped.\n")
                    except git.exc.GitCommandError:
                        return
        
        logger.print_summary()
        print('\n🎉 SUCCESS: rebase created successfully!')
    elif args.command == 'import-subtree':
        import_subtree(repo, args)


if __name__ == '__main__':
    main()