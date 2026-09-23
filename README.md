# context-forge

> [!TIP]
> Static, dependency-free documentation [TsukiNi22/context-forge](https://tsukini22.github.io/context-forge) (v1.0.0).

A C++20 wrapper to forge the output of a command into what you really want: `context-forge exec -c <command>` runs the command, sends its output to a local server (systemd user daemon) which applies your **rules** (`.cfg` + plugins) and optionally an **ollama** model, then prints the forged result.
If the server is not available, the command is executed as it is (fallback), nothing is lost.

### Table of Contents
 - [Dependencies](#dependencies)
 - [Packages](#packages)
 - [Quick Setup 1 (All)](#quick-setup---1-all)
 - [Quick Setup 2 (Limited)](#quick-setup---2-limited)
 - [Auto-completion](#auto-completion)
 - [Usage](#usage)
 - [Rules](#rules)
 - [Unit tests](#unit-tests)
 - [Workflows/Release](#workflowsrelease)

## Dependencies

> [!CAUTION]
> This project's license does not apply to the content of the dependencies used by `context-forge`.

| Name + Link | Status | Last Update |
| ----------- | ------ | ----------- |
| [libutils](https://github.com/TsukiNi22/libutils) | ![CD - Dispatch](https://github.com/TsukiNi22/libutils/actions/workflows/dispatch.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/libutils) |

Build requirements (only needed for the [Quick Setup 1](#quick-setup---1-all)):

| Name | Version | Fedora (`dnf`) | Debian/Ubuntu (`apt`) |
| ---- | ------- | -------------- | --------------------- |
| `clang++` / `cmake` | C++20 / `>= 3.20` | `clang cmake` | `clang cmake` |
| `libutils` | `>= 2.13.22` | `libutils` (see [libutils](https://github.com/TsukiNi22/libutils#quick-setup---2-limited)) | `libutils` |
| `libconfig++` | `>= 1.7` | `libconfig-devel` | `libconfig++-dev` |
| `cpp-httplib` | cmake config required | `cpp-httplib-devel` | `libcpp-httplib-dev` |
| `nlohmann/json` | - | `json-devel` | `nlohmann-json3-dev` |
| `python3` | - | `python3` | `python3` |
| `googletest` (tests only) | - | `gtest-devel` | `libgtest-dev` |

> [!WARNING]
> Some distributions ship older versions: `libconfig++ 1.5` (Ubuntu 24.04) doesn't have `Setting::isString()`, and `libcpp-httplib-dev` (Debian/Ubuntu) doesn't provide the cmake config (`find_package(httplib)`).
> In that case, build them from the sources ([libconfig](https://github.com/hyperrealm/libconfig), [cpp-httplib](https://github.com/yhirose/cpp-httplib)).

> [!NOTE]
> `ollama` is optional, without it (or without a system-prompt) only the rules part of the formating is applied.
> It can be installed with `sudo context-forge install-ollama`.

## Packages

> [!NOTE]
> The package have a pre-release/unstable version named `context-forge-pre`.
> The `-pre` package is marked as obsolete by any release/stable package (without `-pre`) of the same version or higher.

| File Name | Content |
| --------- | ------- |
| `context-forge` | Stable binary + shell completions (zsh & bash) |
| `context-forge-pre` | Pre-release/unstable binary + shell completions (zsh & bash) |

> [!WARNING]
> The packages only contain the binary and the completions, the default plugins (`plugins/**/*.so`) are only built from the sources (see [Quick Setup 1](#quick-setup---1-all)).

## Quick Setup - 1 (all)
> Setup into `/usr/local`

### Clone the repository
```bash
git clone https://github.com/TsukiNi22/context-forge.git
cd context-forge
```

### Build & install
```bash
export BUILD_DIR=build
cmake -S . -B $BUILD_DIR
cmake --build $BUILD_DIR --parallel $(nproc) # build the binary & the plugins (./plugins)
sudo cmake --install $BUILD_DIR              # install the binary & the completions
```

> [!TIP]
> `make` (or `make re`) is a shortcut for the configuration/build part, the result is in `./context-forge` and `./plugins/`.

## Quick Setup - 2 (Limited)
> Setup into `/usr`

> [!WARNING]
> If during the installation using tools like `dnf`, `apt`... you see an `invalid hash` error, reset the cache of the tool or relaunch the `setup.sh` script
> 
> If the error still persists, it might be due to an error in the repository

> [!WARNING]
> Restriction: `fedora-based (rpm)`, `debian-based (deb)`

> [!NOTE]
> The usage of `sudo` in the script can be remove using `--no-sudo` argument

Run the setup script directly, without cloning the repository manually. It:
1. Setup the mirror (`/etc/yum.repos.d/context-forge.repo` or `/etc/apt/sources.list.d/context-forge.list`)
2. Import the GPG key used to sign the packages/metadata
3. Install the package (`context-forge`, or `context-forge-pre` with `--pre`)
4. Add a block in your `~/.zshrc` & `~/.bashrc` to load the auto-completion from the install paths (`/usr` & `/usr/local`)

```bash
wget -qO- https://raw.githubusercontent.com/TsukiNi22/context-forge/main/setup.sh | bash -s
```

or with `curl`:

```bash
curl -fsSL https://raw.githubusercontent.com/TsukiNi22/context-forge/main/setup.sh | bash -s
```

Arguments are given after `bash -s --`, ex: `... | bash -s -- --pre --no-rc`

| Argument | Effect |
| -------- | ------ |
| `--no-sudo` | Run without sudo (requires the script to be run as root already) |
| `--pre` | Install the pre-release/unstable package (`context-forge-pre`) |
| `--no-install` | Only setup the mirror & the GPG key |
| `--no-rc` | Don't edit `~/.zshrc` & `~/.bashrc` |

## Auto-completion

The completions are installed by cmake/the packages into:

| Shell | Package (`/usr`) | Sources (`/usr/local`) |
| ----- | ---------------- | ---------------------- |
| zsh | `/usr/share/zsh/site-functions/_context-forge` | `/usr/local/share/zsh/site-functions/_context-forge` |
| bash | `/usr/share/bash-completion/completions/context-forge` | `/usr/local/share/bash-completion/completions/context-forge` |

Most of the time they are loaded automatically (zsh `fpath` / `bash-completion`), otherwise the `setup.sh` add this block (can be run again without duplication, remove the block to disable it):

```bash
# >>> context-forge completion >>>
...
# <<< context-forge completion <<<
```

To only setup the rc files on an already installed binary: `curl -fsSL https://raw.githubusercontent.com/TsukiNi22/context-forge/main/setup.sh | bash -s -- --no-install`

## Usage

```bash
context-forge -h                                    # help (also: context-forge <mode> -h)

# ollama (optional)
sudo context-forge install-ollama                   # install ollama (official script)
context-forge pull qwen2.5-coder:1.5b               # pull a model

# server (systemd user daemon)
context-forge setup --copy -P ./plugins -r ./rules -R -s ./system-prompt
context-forge status                                # server & ollama status
context-forge start | stop | restart
context-forge remove                                # remove the daemon & the copied files

# check the loading of the rules/plugins/ollama without the daemon
context-forge check local -P ./plugins -r ./rules -R -s ./system-prompt
context-forge check dist

# client
context-forge exec -c ls -la                        # everything after -c is the command
context-forge exec -v debug -d 1 -n -c make re
```

| Mode | Flags |
| ---- | ----- |
| `exec` | `-c\|--command <bin> [args]*` (required, last), `-d\|--redirect <fd>`, `-n\|--no-nl`, `-v\|--verbose <level>` |
| `server`, `check local` | `-P\|--plugins <dir>`, `-r\|--rules <dir>`, `-R\|--recursive`, `-a\|--ip <ip>`, `-p\|--port <port>`, `-m\|--model <model>`, `-s\|--system-prompt <file>`, `-v\|--verbose <level>` |
| `setup` | same as `server` + `-C\|--copy` (copy plugins/rules/system-prompt into `~/.config/context-forge/`) |
| `check dist` | `-a\|--ip`, `-p\|--port`, `-m\|--model`, `-v\|--verbose` |
| `status` | `-a\|--ip`, `-p\|--port` |
| `start`, `stop`, `restart`, `remove`, `install-ollama` | `-v\|--verbose` |
| `pull` | `<model>` |

| Default | Value | Environment |
| ------- | ----- | ----------- |
| ollama ip | `localhost` | `CONTEXT_FORGE_HOST_IPV4` |
| ollama port | `11434` | `CONTEXT_FORGE_HOST_PORT` |
| ollama model | `qwen2.5-coder:1.5b` | `CONTEXT_FORGE_MODEL` |
| system-prompt | none (llm disabled) | `CONTEXT_FORGE_SYSTEM_PROMPT` |
| verbose | `basic` (`none\|basic\|advanced\|debug`) | `VERBOSE` |

## Rules

Each `.cfg` file of the rules directory is a rule. Every rule whose trigger match is applied (one after the other), then the llm (if enabled).
Inside a rule: `pre-rules -> rules` on each block, in the order of the file.
Each key (except `enable` & `block`, reserved) is the name of a plugin (`<plugins>/**/*.so`), an unknown plugin or an invalid value makes the whole file ignored (see `-v debug`).

```cfg
enable = true; // optional, allow the rule to be ignored

// separate by block the formating (default: one block / all)
block = {
    ln = <int>;         // number of line per block | int > 0 (only one of ln/char)
    char = <int>;       // number of char per block | int > 0
    show = <int>;       // number of block displayed
    sep = "<string>";   // separator written between each block
};
```

| Plugin | Type | Config | Effect |
| ------ | ---- | ------ | ------ |
| `trigger` | trigger | `trigger = {bin = ["<bin>", ...]; contains = "<regex>"; match = "<regex>";};` | Apply the rule if the binary is in `bin`, or the output contains/match the regex (nothing given: always) |
| `ansi` | pre-rule | `ansi = <boolean>;` | Keep (`true`, default) or remove (`false`) the ansi sequences |
| `ln` | rule | `ln = {head = <int>; tail = <int>;};` | pos: keep n lines, neg: remove n lines (head applied before tail) |
| `drop` | rule | `drop = ["<regex>", ...];` | Remove any line matching one of the regex |
| `dup` | rule | `dup = {match = ["<regex>", ...]; eq = ["<string>", ...]; keep = <int>; invert = <boolean>;};` | Keep only the `keep` (default: 1) first (or last with `invert`) lines of each group |
| `replace` | rule | `replace = {match = ["<regex>", ...]; eq = ["<string>", ...]; by = "<string>";};` | Replace every match by `by` (`<INSERT>` is replaced by the match), regex first then exact strings |
| `insert` | rule | `insert = {before = "<string>"; after = "<string>";};` | Insert text before/after the content |

Example (`ls` without the `total` line):
```cfg
trigger = {
    bin = ["ls"];
};

drop = ["^total \\d+"];
```

## Unit tests

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build --parallel $(nproc)
./unit_tests                                   # or: cd build && ctest --output-on-failure
```

| Directory | Content |
| --------- | ------- |
| `tests/` | `Forge` (arguments parsing), `Ollama` (mocked http server) |
| `tests/rules/` | `AInstruction`, `Rules` (block/trigger/apply), `Pipeline` (full `.cfg` using the real plugins classes) |
| `tests/rules/{triggers,pre-rules,rules}/` | Each default plugin class (`load` validation + `format`/`trigger` results) |
| `tests/plugins/` | Shared objects built by the `plugins` target (`extern "C"` interface: `type`, `name`, `factory`) |
| `tests/tools/` | Mocks (`MockInstruction`, `MockOllama`) & helpers (`Config`) |

## Workflows/Release
### Workflows
- The workflow `Dispatch (CI/CD)` runs on every push (branch `main` or tag `v*`) and decides what to trigger based on the ref/commit message
- The workflow `Unit Tests - Libraries (CI)` always runs first, nothing else is triggered if it fails
- The workflow `Build - Packages (CI/CD)` builds the packages (RPM/DEB), signs them and syncs them into the mirror (`gh-pages`)
- The workflow `Build - Executables (CI)` only builds the binary and checks the compilation, without producing or releasing packages

> [!NOTE]
> Only when `Dispatch (CI/CD)` decides **not** to build packages, the `Build - Executables (CI)` workflow runs instead, to validate that the code still compiles

> [!NOTE]
> A commit containing the string `[ignore]`, preferably in the description, skips every workflow

### Pre-Release (unstable)
The pre-release of the package (`context-forge-pre`) can be triggered by 2 events:
- Pushing a tag that matches the regex `vx.x.x-*` (`x` stands for the version number: `major`, `minor`, `fix`, ex: `v1.0.0-pre`)
- Or pushing a commit containing the string `[build]`, preferably in the description

### Release (stable)
The release of the package (`context-forge`) can only be triggered by pushing a tag that matches the regex `vx.x.x` (`x` stands for the version number: `major`, `minor`, `fix`)
