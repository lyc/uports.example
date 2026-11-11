"""Command execution and clean host environments for toolchain exports."""

import os
import subprocess


def run(command, env, timeout=60, cwd=None):
    result = subprocess.run(
        [str(arg) for arg in command], env=env, cwd=cwd, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout,
    )
    if result.returncode:
        raise RuntimeError(f"{' '.join(map(str, command))}\n{result.stdout}")
    return result.stdout


def run_live(command, environment, log, timeout=360):
    # Stream tutorial commands while retaining the complete validation output.
    bounded = ["timeout", "--kill-after=5", str(timeout), *map(str, command)]
    with log.open("w") as output, subprocess.Popen(
        bounded, env=environment, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, bufsize=1,
    ) as process:
        try:
            for line in process.stdout:
                print(line, end="", flush=True)
                output.write(line)
                output.flush()
            status = process.wait()
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
    if status:
        raise RuntimeError(f"Validation failed (exit {status}); see {log}")


def clean_environment():
    environment = os.environ.copy()
    for name in list(environment):
        if name.startswith(("LD_", "UPORTS_", "MAKE")) or name in {
            "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX", "CPATH",
            "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "PYTHONPATH", "PYTHONHOME",
            "CONFIG_SITE", "GCCPREFIX", "QEMU", "CROSS_COMPILE", "TERMINFO",
            "TERMINFO_DIRS",
        }:
            environment.pop(name, None)
    environment["PATH"] = "/usr/bin:/bin"
    environment["LC_ALL"] = "C"
    return environment


def prepare_source(project, source, repository, commit, environment):
    # Use the framework's bounded, locked, atomic checkout implementation.
    # Existing source roots are checked by the profile and never reset here.
    if source.exists() or source.is_symlink():
        return source
    checkout = environment.copy()
    checkout.update({
        "SCM_REPO_URL": repository, "SCM_DETACH": commit,
        "SCM_WORKTREE": str(source), "SCM_CACHE": "no",
        "SCM_TIMEOUT": "120", "SCM_LOCK_TIMEOUT": "30",
    })
    helper = project / "make/uports/Mk/Scripts/do-scm.sh"
    print(f"  SOURCE  preparing validation checkout: {source}", flush=True)
    output = run(["sh", helper, "checkout"], checkout, timeout=180)
    print(output, end="", flush=True)
    return source


def check_source(source, commit, required_files, environment):
    # Construction input integrity before shipping validation source.
    for relative in required_files:
        if not (source / relative).is_file():
            raise RuntimeError(f"Missing source input: {source / relative}")
    actual = run(["git", "-C", source, "rev-parse", "HEAD"], environment)
    if actual.strip() != commit:
        raise RuntimeError(f"Source must select commit {commit}: {source}")
    status = run(["git", "-C", source, "status", "--porcelain",
                  "--untracked-files=no"], environment)
    if status.strip():
        raise RuntimeError(f"Source has modified tracked files: {source}")


def copy_git_source(source, target, repository, commit, environment):
    # Mirror all refs into independent objects, including from a Git worktree.
    # Convert the result to a normal checkout with a usable upstream remote.
    target.mkdir()
    run(["git", "clone", "--mirror", "--no-local", source, target / ".git"],
        environment, timeout=120)
    git = ["git", "-C", target]
    run([*git, "config", "core.bare", "false"], environment)
    run([*git, "config", "--unset", "remote.origin.mirror"], environment)
    run([*git, "config", "--replace-all", "remote.origin.fetch",
         "+refs/heads/*:refs/remotes/origin/*"], environment)
    run([*git, "remote", "set-url", "origin", repository], environment)
    run([*git, "checkout", "--detach", commit], environment)
