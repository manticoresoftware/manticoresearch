# Manticore One-Line Installer Specification

## Purpose

The installer provides a zero-touch way to install, upgrade, start, and remove Manticore Search from a single shell pipeline:

```sh
curl https://manticoresearch.com | sh
```

The installer must keep the common path short: detect the host package family, configure the official Manticore repository when needed, install the `manticore` package, start the service, and print a clear result. It should avoid duplicating logic already owned by the `manticore-repo.noarch.deb` and `manticore-repo.noarch.rpm` packages.

## Design Principles

- Default install is fully automatic and starts Manticore by default.
- Interactive prompts are used only for destructive or ambiguous cases.
- Non-interactive mode must remain readable and must not depend on colors or TTY behavior.
- Prompt input must use the controlling terminal (`/dev/tty`) when available, not pipeline stdin, so interactive `curl|sh`, `wget|sh`, `curl|bash`, and `wget|bash` runs can still ask confirmations. If no controlling terminal is available, prompts must not block; they should take the safe non-interactive path.
- Critical privileged commands use `sudo` only at the command boundary. The whole script should not require being run through `sudo`.
- Scripts may live in `/tmp`, including on `noexec` filesystems, so downloaded modules are invoked as `bash module.sh` rather than executed directly.
- Package managers remain the source of truth for package contents, systemd units, config files, data directories, GPG keys, and repository definitions.
- Unsupported platforms must fail clearly and point to `https://manticoresearch.com/install/`.
- Public documentation should prefer the shortest useful command. Avoid extra flags such as `curl -sSL` unless they solve a specific problem in that context.


## Entrypoints

### `bootstrap-standalone.sh`

Primary public command:

```sh
curl https://manticoresearch.com | sh
```

Command examples use `sh -s command args` without an inserted `--`:

```sh
curl https://manticoresearch.com | sh -s help
curl https://manticoresearch.com | sh -s version 25.0.0
curl https://manticoresearch.com | sh -s dev
curl https://manticoresearch.com | sh -s upgrade
```

`bootstrap-standalone.sh` is the primary public execution path. Its outer wrapper is POSIX `sh` compatible, so it supports both `curl|sh` and `curl|bash`. The wrapper requires `bash` on the target host, writes the embedded Bash payload to a temporary file, and invokes it with `bash`; if `bash` is missing, it fails clearly before installer work. The embedded payload contains the install, upgrade, uninstall, detection, logging, and UI logic in one file. Because the script is self-contained, it does not need to know the URL it was downloaded from and does not download sibling modules. The final public release should expose this script through `https://manticoresearch.com`. Longer repository-hosted URLs are acceptable as temporary publishing bypasses or staging/debugging endpoints, but should not be treated as the long-term public interface. The standalone script still downloads Manticore repository packages from the canonical package URLs:

```sh
https://repo.manticoresearch.com/manticore-repo.noarch.deb
https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

Runtime rule: `bootstrap-standalone.sh` must validate `--help`, `-h`, `-?`, unknown options, and malformed option values before setting up installer logging, OS detection, repository setup, downloads, or package operations.

### `bootstrap.sh`

`bootstrap.sh` is the secondary modular entrypoint, primarily useful for staging and testing module downloads. Example:

```sh
export MANTICORE_INSTALLER_REPO_URL="https://repo.manticoresearch.com/repository/install"
curl -sSL "$MANTICORE_INSTALLER_REPO_URL/bootstrap.sh" | bash
```

`MANTICORE_INSTALLER_REPO_URL` is optional. If unset, the default source is:

```sh
https://repo.manticoresearch.com/repository/install
```

The variable exists primarily for tests and staging.

Responsibilities:

1. Create a temporary module directory with `mktemp -d`.
2. Configure logging.
3. Check that either `curl` or `wget` exists.
4. Download `constants.sh`, `detect.sh`, `ui.sh`, `install.sh`, `upgrade.sh`, `main.sh`, and `uninstall.sh` from the installer source.
5. Run `bash ./main.sh "$@"` from the temporary directory.
6. Remove the temporary directory on exit.


Maintenance rule: the modular installer is the source of truth. `installer/build.sh` builds `bootstrap-standalone.sh` from `constants.sh`, `ui.sh`, `detect.sh`, `install.sh`, `upgrade.sh`, `uninstall.sh`, and `main.sh` by concatenating them with a small standalone logging prologue. The generator may strip only module-local prologue boilerplate: shebangs, `set -euo pipefail`, the initial `SCRIPT_DIR` assignment block, and top-level `source "$SCRIPT_DIR/..."` lines. It must not use broad filters such as removing every line containing `SCRIPT_DIR`, because valid function bodies may need to reference module paths in the modular flow. It syntax-checks the embedded Bash payload, then wraps it in a POSIX `sh` launcher. The generated Bash payload sets `MANTICORE_STANDALONE=1`; module self-test/direct-execution guards must check that flag so they do not run during standalone execution. Whenever modular behavior changes, run `installer/build.sh`, then verify `bootstrap-standalone.sh` with `sh -n`, a piped `sh -s help` smoke test, and at least a lightweight `list-versions` smoke test. Avoid manual edits to `bootstrap-standalone.sh`; move changes into modules and regenerate it.

## Internal Modules

`install.sh`, `upgrade.sh`, and `uninstall.sh` are internal flow modules. They must not contain downloader, bootstrap redirection, or public curl|bash entrypoint logic and are not documented as public curl|bash targets. Public install, upgrade, uninstall, purge, and version-selection requests go through `bootstrap-standalone.sh` or the secondary modular `bootstrap.sh`.

`main.sh` dispatches to these modules after argument parsing and environment setup. In `bootstrap-standalone.sh`, the same module functions are included inline and called directly.


## Supported Package Families

### Debian Family

Detected when `/etc/os-release` indicates Debian, Ubuntu, Linux Mint, or Debian-like systems.

Install repository bootstrap package:

```sh
dpkg -i manticore-repo.noarch.deb
```

When the installer downloads `manticore-repo.noarch.deb`, it must inspect the downloaded package metadata with `dpkg-deb -f`. If the installed `manticore-repo` package has the same version, skip `dpkg -i` during the initial healthy path; otherwise install the downloaded package. The installer should stay agnostic about repository-package internals. If Debian metadata refresh or package installation reports apt authentication/signature problems, including `NO_PUBKEY` warnings that may still leave `apt-get update` with exit code 0 and stale indexes, force reinstall the same repository package version and retry the failed apt operation once. This avoids unnecessary repository-package reinstall work during healthy refresh/upgrade flows while still allowing the repository package to repair its own source/keyring state.

Refresh metadata:

```sh
apt-get update
```

Install Manticore:

```sh
apt-get install -y manticore
```

Specific versions use the exact package version after resolver expansion and allow explicit downgrades. Debian package shape must be detected from the resolved `manticore` package metadata:

- Thin/meta versions must build the exact-version package set from the resolved `manticore` package metadata. Start with `manticore=<resolved-package-version>`, then follow transitive `Depends:` entries for `manticore-* (= <resolved-package-version>)` packages and request that exact same-version set together. The installer must also inspect bounded same-repository `Depends:` entries from those selected packages, such as `manticore-buddy`, `manticore-tzdata`, or similar `manticore-*` dependencies. If the currently installed dependency package is absent or does not satisfy the target constraints, add the highest available version satisfying those constraints to the same apt transaction. After the transaction, mark split and derived dependency packages as automatically installed with `apt-mark auto`, leaving only the `manticore` meta package as the user-requested manual package.
- Fat versions whose `manticore` package `Provides`/`Breaks` split packages must request only `manticore=<resolved-package-version>`. Apt must be allowed to remove older real split packages that are broken/replaced by the fat package.

Version metadata parsing must remain compatible with old supported distributions such as Ubuntu 18.04. Prefer `apt-cache show --no-all-versions "${package}=${version}"` when inspecting a specific package version, use portable awk/sed expressions, and avoid ambiguous awk exits such as `exit expr ? 0 : 1`. A concrete regression to preserve is `version 17 no-start` on Ubuntu 18.04: `manticore=17.5.1-...` is a thin/meta package and must be installed together with its same-version split dependencies, otherwise apt reports unmet dependencies.

Distribution-specific repository details are delegated to the `manticore-repo.noarch.deb` package, including Linux Mint handling.

### RPM Family

Detected when `/etc/os-release` indicates RHEL, CentOS, Fedora, Rocky, AlmaLinux, Amazon Linux, or related systems.

Install repository bootstrap package:

```sh
yum install -y https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

or with `dnf` when available.

Refresh metadata:

```sh
dnf makecache
```

or:

```sh
yum makecache
```

Install Manticore:

```sh
dnf install -y manticore
```

or:

```sh
yum install -y manticore
```

Specific versions use package-manager version selection after resolver expansion. When RPM repository query metadata is available, the installer starts with `manticore-<resolved-package-version>` and follows transitive `manticore-* = <resolved-package-version>` requirements to build the exact same-version package set. If repoquery metadata is unavailable, it falls back to the known split package names. DNF/YUM remains responsible for resolving bounded same-repository dependencies such as `manticore-buddy` or `manticore-tzdata`, including downgrades when required by the selected version. Before an explicit RPM version switch, installed split packages are removed with `rpm -e --noscripts --nodeps` so historical broken split-package scriptlets cannot leave a partial transaction behind. If the installed package is a fat `manticore` package and the target version is split, the fat package is removed the same way before installing the split package set, avoiding file conflicts with packages such as `manticore-buddy`.

Distribution-specific repository and GPG-key details are delegated to the `manticore-repo.noarch.rpm` package.

### Homebrew

If `brew` is available and `apt-get`, `yum`, and `dnf` are not available, the installer uses Homebrew.

Homebrew detection is intentionally package-manager based, not OS-name based. Homebrew can exist on Linux, and macOS can exist without Homebrew installed. Native apt/rpm package managers take precedence when present; Homebrew is the fallback only when `brew` is available and no supported native package manager is detected.

Install Manticore:

```sh
brew install manticoresoftware/tap/manticore
```

Start Manticore:

```sh
brew services start manticore
```

There is no repository bootstrap package in the Homebrew flow. `version` is not supported for Homebrew installs or upgrades unless Homebrew support for this is explicitly designed later.

### Unsupported Platforms

If no supported package family is detected, the installer must fail with a direct message, for example:

```text
macOS is not supported by this installer. See https://manticoresearch.com/install/ for supported installation methods.
```

When Homebrew is available and no apt/yum/dnf package manager is present, macOS can use the Homebrew path instead of this unsupported message.

Unsupported platforms with Docker available should print Docker guidance or keep the generic install-guide URL.
Guidance is referenced from https://manticoresearch.com/install/ and located at `https://github.com/manticoresoftware/docker#production-use`

## Debian repositories

'Release' repository, set by installing repository package
```sh
https://repo.manticoresearch.com/manticore-repo.noarch.deb
```

'Dev' repository, set by installing repository package
```sh
https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb
```

All repository package files provide the package named 'manticore-repo', so when need to uninstall the package, you can always remove 'manticore-repo', and it will remove any existing, despite whether it is release or dev.

All repository packages are mutually exclusive, installing one over another will overwrite files from another one. So, when need to switch from one to another, first remove 'manticore-repo', then install target one.

All repository packages emit file `/etc/apt/sources.list.d/manticoresearch.list`. So, if the file present, it is one of the signs that 'manticore-repo' is installed; if not, it might be set up manually by user and so, might be warned.

'Release' repository creates file with contain like this:
```txt
deb http://repo.manticoresearch.com/repository/manticoresearch_DISTR DISTR main
```

'Dev' repository creates file with contain like this:
```txt
deb http://repo.manticoresearch.com/repository/manticoresearch_DISTR_dev DISTR main
```

the 'DISTR' in the file is not literal word, but is name of distribution, like 'buster', 'focal', etc.
Notice different suffixes in the repo url: 'Dev' repo has '_dev' at the end, and 'Release' has no suffix.

So, if you see 'manticore-repo' is installed, and see, file `/etc/apt/sources.list.d/manticoresearch.list` is exist and non empty, you can look to the suffix of the 2-nd word (which is the url), and if you see '_dev', it seems, that 'Dev' repository is now active, and if you se no '_dev', - it sigh, you deal with default 'Release' repository.

If user wants explicitly switch into concrete repository, you may detect, which one is already installed, and behave from this detection (say, if target is already the one requested, may be no need to download/reinstall repository package then).

## RPM repositories

You need to install this package:
```sh
https://repo.manticoresearch.com/manticore-repo.noarch.rpm
```

It will install three repositories:
```txt
manticore
manticore-dev
manticore-release-candidate
```

Repository 'manticore' is enabled by default, that is 'Release' repository.

Repository 'manticore-dev' is disabled by default, that is 'Dev' repository.

Repository 'manticore-release-candidate' is disabled by default, and mentioned here just because it exists. We're not going to expose it as an installer channel or activate it implicitly.

For RPM, the repository package enables the release repository by default, so a clean release install uses the package defaults. If the user explicitly selects `dev` or `release`, persistently switch the enabled repository with package-manager configuration tools rather than manually editing the package-owned repository file. Use `dnf config-manager --set-enabled/--set-disabled` when available, or `yum-config-manager --enable/--disable` on yum systems. If those tools are missing for an explicit channel switch, install the standard plugin package (`dnf-plugins-core` or `yum-utils`) and retry.

`dev` must enable `manticore-dev` and disable `manticore`. `release` must enable `manticore` and disable `manticore-dev`. Do not enable or disable `manticore-release-candidate`.

## Choose repository

On clean system where no manticore repository package installed, if user doesn't ask for specific repository, assume, he wants to deal with 'Release'. Behave according to it.

However, when a repo is already installed/choosen, assume user doesn't want to unexpectedly change it. For example, if 'Dev' repository was chosen and set up, and user wants to upgrade, or list available versions, it should not be changed.

If user explicitly provides a supported repository channel, behave according to it. That may imply reinstalling the correct repository package for Debian, or persistently switching the enabled RPM repository with config-manager tools. Without an explicit RPM channel, preserve the repository package defaults/current repository state.

## Command Syntax

Supported commands/options:

The documented public syntax is command-style: `upgrade`, `version 25.0.0`, `silent`, `purge-all`, `dev`, and similar commands without double-dash long option names. `--help` is the only documented double-dash long option. Other historical double-dash aliases may be accepted temporarily for compatibility, but new examples, tests, and user-facing documentation should use command-style syntax. Once compatibility aliases are removed from code, unknown double-dash options must fail during pre-validation.

Public pipe examples should pass commands as `sh -s command args`, not `sh -s -- command args`. The latter may work as shell syntax, but it is not the documented installer UX.

- `help`, `--help`: print a concise usage summary and exit successfully before OS detection, repository setup, or package operations. `--help` is the only supported double-dash long option.
- `upgrade`: upgrade an existing installation. When routed through `bootstrap.sh`, a host without Manticore may fall back to the installation flow; when `upgrade.sh` runs as the internal upgrade module, it requires an existing installation.
- `version <version>`: install or switch to the requested version when the package family supports version selection. This command is explicit user intent and does not ask for an additional confirmation when changing or downgrading an existing installation. For Debian/RPM, a short product version such as `25.0.0` must resolve to an available package version with the same prefix, such as `25.0.0-...` or `25.0.0_...`, before calling the package manager. If multiple matching package versions are available, use the highest one according to version sort. Versioned Debian/RPM installs must verify that the required exact-version package set is available. Debian must choose that set from the resolved `manticore` package metadata: transitive same-version `manticore-*` dependencies for thin/meta versions, plus any bounded same-repo `Depends:` packages whose installed versions do not satisfy the target constraints, followed by `apt-mark auto` for split/dependency packages; but only `manticore=<version>` for fat versions that provide and break the split packages. RPM should likewise derive the same-version `manticore-*` requirement closure from repoquery metadata when available, with a known-name fallback for older/minimal systems. RPM version switches must prepare package-shape changes by removing installed split packages, or an installed fat package when switching to split, using no-script/no-dependency rpm erasure before invoking DNF/YUM for the requested target set.
- `list-versions`: ensure the repository bootstrap package is installed when applicable, refresh package metadata, print available Manticore package versions one per line, and exit without installing, upgrading, or removing Manticore itself. The printed version list must be built from package-manager version fields, with blank lines removed, and sorted uniquely from oldest to newest so Debian and RPM output order is uniform. Debian epoch prefixes such as `1:` must not cause versions to be filtered out.
- `list-versions-file <path>`, `list-versions-file=<path>`: perform the same action as `list-versions` but write the refined version list to the given file. Normal installer progress may still be printed to stdout/stderr, but the file must contain only available version strings, one per line, with no log prefixes or status messages.
- `silent`, `yes`: non-interactive mode. Assume defaults and do not prompt.
- `no-start`: install or upgrade packages but do not start Manticore.
- `backup-data`: include the data directory in upgrade backup.
- `no-backup-data`: skip data directory backup. This is the default.
- `backup-dir <path>`: override the default backup directory.
- `uninstall`: stop and remove Manticore, preserving repository configuration and data/config state.
- `purge`: stop and remove Manticore, then remove the repository bootstrap package when applicable.
- `purge-all`: same as `purge`, then remove configured Manticore config and data directories after explicit confirmation unless `silent` is set.
- `dev`: explicitly switch to 'Dev' repository
- `release`: explicitly switch to 'Release' repository

If different `dev`, `release` exist in one command, it should be treated as error. (if user selects one and
same repo, i.e. `dev` more than once - that is not error).

Unknown options/commands or malformed values must abort before OS detection, repository setup, or package operations. The installer should print an error plus a `--help` hint and exit with status `2`.

## Installation Flow

1. Parse arguments in `main.sh`.
2. Detect OS family and architecture.
3. Fail clearly if unsupported.
4. Warn if an existing `searchd` binary appears to be outside the package-managed Manticore install.
5. Install repository bootstrap package unless the package family is Homebrew.
6. Refresh package metadata.
7. Install `manticore` or the requested version.
8. If `no-start` is not set:
   - Check whether the service is already active.
   - Check that ports `9306`, `9308`, and `9312` are not already occupied before starting a stopped service.
   - Start the service.
9. Verify that `searchd` is present.
10. Report whether Manticore is running.

The default happy path should end with Manticore installed and running on ports `9306`, `9308`, `9312`. This port list comes from the default `listen` directives in the repository root `manticore.conf.in` template.

## Filesystem Paths

The installer should use the package-managed filesystem layout:

- Config directory: `/etc/manticoresearch`.
- Data directory: `/var/lib/manticore`.
- Log directory: `/var/log/manticore`.
- Default installer/upgrade backup root: `/var/backups/manticore`.

The installer may create temporary files in `${TMPDIR:-/tmp}` or user-writable locations without privilege escalation. Root privileges should be requested only for critical package-manager, service, `/etc`, and `/var` operations.

## Upgrade Flow

1. Detect OS family and architecture.
2. Fail clearly if unsupported.
3. Require an existing Manticore installation.
4. Create a backup directory under `${MANTICORE_BACKUP_DIR:-/var/backups/manticore}/manticore_backup_<installed-version>`.
5. Always back up configuration when the config directory exists.
6. Back up the data directory only when `backup-data` is set. In this case manticore must be stopped, otherwise backup may be damaged.
7. Refresh or reinstall the repository bootstrap package unless the package family is Homebrew.
8. Refresh package metadata.
9. Stop Manticore if it is running. Upgrade should not stop on all platforms, but rely on package-manager hooks and restart afterward.
10. Upgrade the package:
    - Debian: for plain `upgrade`, resolve the latest available package version and use the metadata-derived exact-version package set with `--allow-downgrades`; this handles split-only installed states where the real `manticore` package is absent. If the latest version cannot be resolved, fall back to `apt-get install -y --only-upgrade manticore`.
    - RPM: `dnf upgrade -y manticore` / `yum update -y manticore` or requested exact-version package set.
    - Homebrew: `brew upgrade manticore || brew install manticoresoftware/tap/manticore`.
11. If `no-start` is not set, start Manticore.
12. Verify that `searchd` exists and report whether the service is running.

Rollback is not part of the current implementation because package-level rollback is more complex than restoring a single binary. This needs a separate design.

## Uninstall and Purge Flow

`uninstall`:

1. Detect OS family.
2. Stop Manticore if active.
3. Remove the `manticore` package.
4. Preserve repository package, config, and data.

`purge`:

1. Perform uninstall.
2. Remove the repository bootstrap package when applicable.
3. Preserve config and data.

`purge-all`:

1. Require typed `DELETE` confirmation in interactive mode.
2. In non-interactive mode, require `silent` to confirm destructive cleanup.
3. Perform purge.
4. Remove Manticore config and data directories.

Repository configuration and signing-key cleanup are encapsulated in the repository bootstrap packages. The installer removes the `manticore-repo` package for `purge` and `purge-all`; it must not duplicate package-internal cleanup paths. If a bootstrap package leaves repository files or keys behind after package removal, fix that package instead of adding installer-side cleanup.

The default behavior must preserve user data.

## Service Handling

Service state is checked in package-family-specific ways:

- Homebrew: `brew services list`.
- systemd hosts: `systemctl is-active --quiet manticore`.
- non-systemd containers or minimal environments: `ps` check for `searchd`.

Service start:

- Homebrew: `brew services start manticore`.
- systemd: `systemctl start manticore` via `sudo_exec`.
- non-systemd fallback: create `/var/run/manticore` and run `searchd` via `sudo_exec`.

Service stop:

- Homebrew: `brew services stop manticore`.
- systemd: `systemctl stop manticore` via `sudo_exec`.
- non-systemd fallback: `searchd --stopwait` via `sudo_exec`.

## Logging and Output

The installer must mirror output to a log file.

Preferred log path:

```sh
/var/log/manticore_install.log
```

If that path is not writable, fallback path:

```sh
${TMPDIR:-/tmp}/manticore_installer.log
```

Interactive TTY output may use color and symbols. Color eligibility must be detected before stdout and stderr are redirected into logging pipelines, because after redirection `-t 1` may no longer describe the original terminal. The result should be preserved in an internal flag such as `MANTICORE_COLOR_TTY` and used by UI helpers. Non-TTY output must be plain text. Log output should not include color escape sequences.

The installer must honor `NO_COLOR` by disabling ANSI color. `MANTICORE_FORCE_COLOR=1` or `CLICOLOR_FORCE=1` may force colored UI output for diagnostics or tests. These force flags should affect only user-facing output; the log file must still be written without ANSI escape sequences.

Usage output should be generated from a single formatter rather than separate colored and non-colored copies. Non-colored output is produced by leaving style variables empty. Usage headings may use cyan/bold. Command labels and example lines use bold/bright styling only. Argument placeholders in `[]` and `<>` are cyan in both usage lines and example lines. Keep these rules in the specification so future changes do not accidentally fork multiple help texts.

Prompts must read from and write to `/dev/tty` when available. Normal stdout/stderr may be mirrored through the logging pipeline, but prompt input must not consume piped installer script bytes from `curl|sh` or `wget|sh`.

## Privilege Escalation

The scripts can be run as root or as a normal user.

Rules:

- Do not require the entire script to run under `sudo`.
- Use `sudo_exec` for commands that modify system package state, service state, `/var`, `/etc`, or other privileged paths.
- Do not use `sudo` for user-level Homebrew commands.

## Idempotency and Partial Installs

The installer should be safe to rerun.

Required checks:

- If repository bootstrap package is already installed, skip reinstall in normal install/list flows. During upgrade/refresh, Debian should still download the repo bootstrap `.deb`, but skip `dpkg -i` when the downloaded package version matches the installed `manticore-repo` version.
- If requested Manticore version is already installed, report success and exit. Installation detection must handle both the `manticore` meta package and split-package installs such as `manticore-server-core`, because older Debian package transactions may leave the meta package absent after split packages replace its files.
- If Manticore is already installed and no upgrade is requested, compare with the latest available version when possible.
- If the service is already running, do not start it again.
- If a previous package install partially completed, rely on the package manager to resume or repair package state.

## Safety Checks

Before starting Manticore, the installer checks whether ports `9306`, `9308`, `9312` are already in use. These are the default listeners from `manticore.conf.in`: Sphinx/API `9312`, MySQL `9306`, and HTTP `9308`. If any of them are occupied and Manticore is not already active, the installer fails and asks the user to stop the conflicting service or adjust Manticore configuration.

If an unmanaged `searchd` binary is found before package installation, the installer warns that multiple Manticore installations may exist.

## Verification

Minimum verification after install or upgrade:

- `searchd` exists in `PATH`.
- Service is active, unless `no-start` was requested.

Target verification for future improvement:

- Run a SQL health check such as `SELECT 1` through port `9306`.

## Testing Requirements

The installer must have CLT coverage for:

- Debian/RPM release install through existing repository package flow.
- Debian/RPM `curl | sh` and `curl | bash` bootstrap flow.
- Debian/RPM `list-versions` through the bootstrap flow on a clean host, including repository bootstrap setup without installing Manticore.
- Debian/RPM `list-versions-file <path>` through the bootstrap flow, verifying that the file contains only refined version lines in uniform oldest-to-newest order and no installer logs.
- RPM `list-versions` output without awk escape warnings.
- Debian explicit `version` switch/downgrade through the bootstrap flow without an extra prompt, including split-package detection after downgrade.
- Debian explicit `version 17 no-start` through the bootstrap flow on Ubuntu 18.04, proving that thin/meta package dependencies are expanded correctly.
- Debian `upgrade` from an older thin/split installed state to the latest version, including the case where the real `manticore` package is absent and only split packages are installed.
- RPM explicit `dev` and `release` channel switches, proving that config-manager tools persistently enable/disable only `manticore` and `manticore-dev`, while leaving `manticore-release-candidate` untouched.
- Public pipe command syntax using `sh -s help`, `sh -s list-versions`, and `sh -s version <version>` without a double-dash separator.
- Colored and non-colored help output, including `NO_COLOR=1` and forced color through `MANTICORE_FORCE_COLOR=1` or `CLICOLOR_FORCE=1`.
- Service start behavior in systemd and non-systemd environments where practical.
- Cleanup after install.

Focused script validation should include:

```sh
sh -n installer/bootstrap-standalone.sh
bash -n installer/bootstrap.sh installer/build.sh installer/main.sh installer/install.sh installer/upgrade.sh installer/uninstall.sh installer/detect.sh installer/ui.sh
```

Homebrew behavior should be tested on a host or image where Homebrew is available and apt/yum/dnf are absent or hidden from `PATH`.

`bootstrap-standalone.sh` should be tested separately because it duplicates the modular flow and does not download modules.

## Deferred or Out of Scope

These items need separate product or implementation design before being added:

- Docker or Docker Compose install mode.
- Kubernetes or cluster setup.
- Offline installer bundle.
- User-mode non-root install outside package managers.
- Full package rollback after failed upgrade.
- Version pinning that prevents package-manager upgrades.
- Nightly/stable channel selection beyond what repository packages already provide.
- Config/data migrations for breaking upgrades.
- Feedback prompts after install.
- Script checksum workflow for users who intentionally choose `curl | bash`.

## Open Questions

- How to implement reliable package-level rollback without making the one-line installer fragile or large.
- Whether Homebrew should support version selection, and what syntax should be used if it does.
