# MyShell

A working shell prototype written in C, built as a homework assignment for an operating systems class.

Only "low-level" functions are used:
- System calls: `write()`, `fork()`, `exec()`, `waitpid()`, `getcwd()`, `chdir()`, `mkdir()`, `rmdir()`, `open()`, `read()`, `link()`, `symlink()`, `readlink()`, `unlink()`, `stat()`, `getpid()`, `getppid()`, `getuid()`, `geteuid()`, `getgid()`, `getegid()`, `uname()`, `kill()`...
- Memory allocation: `calloc()`, `malloc()`, `free()`...
- String manipulation: `strlen()`, `strcmp()`, `strcpy()`, `strerror()`...
- Type conversion: `atoi()`
- Basic I/O: `printf()`, `fgets()`, `fopen()`, `fclose()`, `fscanf()`
- Directory traversal: `opendir()`, `readdir()`, `closedir()`

---

## Architecture

The shell runs as a REPL (Read-Evaluate-Print Loop). Each iteration reads one line from stdin, tokenizes it, and parses the result.

```
stdin
  |
  v
fgets()             -- reads one line at a time (up to 1000 chars)
  |
  v
tokenize()          -- splits the line into tokens in-place
  |
  v
parse()
  |-- check_redirect()    -- detects redirect / background operators
  |-- check_builtin()     -- looks up command in builtin table
  |-- execute_builtin()   -- dispatches to the matching builtin function
  |-- execute_external()  -- forks and execs the external command
```

---

## Tokenizer

`tokenize(char *buff)` splits the input line in-place by replacing delimiters with `\0`, turning the single buffer into a sequence of adjacent C strings. Returns the token count.

Special cases handled:

| Case | Behaviour |
|---|---|
| Leading spaces | Skipped — no empty token produced |
| Consecutive spaces | Treated as a single separator |
| Trailing spaces | Do not produce a phantom extra token |
| Quoted strings `"..."` | Spaces inside quotes are preserved; the content becomes one token |
| Comments `#` | A `#` at the start of a token (preceded by whitespace) causes the rest of the line to be ignored; `#` mid-word is treated as a regular character |
| Empty lines | Detected in the main loop and skipped entirely |

---

## Built-in Commands

Built-ins are stored as a table of `{ name, function_pointer }` structs. Adding a new built-in requires only a new function and one entry in the table.

### Shell control

| Command | Arguments | Description |
|---|---|---|
| `debug` | _(none)_ | Print current debug level |
| `debug N` | `N` — integer | Set debug level to N. Level > 0 enables token printing |
| `prompt` | _(none)_ | Print current prompt string |
| `prompt STR` | `STR` — max 8 chars | Set prompt to STR |
| `status` | _(none)_ | Print exit status of the last command |
| `exit` | _(none)_ | Exit the shell with the last status code |
| `exit N` | `N` — integer | Exit the shell with status N |

### Text utilities

| Command | Arguments | Description |
|---|---|---|
| `print ARGS...` | any tokens | Print arguments separated by spaces, no trailing newline |
| `echo ARGS...` | any tokens | Print arguments separated by spaces with a trailing newline |
| `len ARGS...` | any tokens | Print total character count of all arguments (excluding spaces) |
| `sum N...` | integers | Print the sum of all integer arguments |
| `calc N OP M` | two integers and an operator | Evaluate `N OP M`; supports `+`, `-`, `*`, `/`, `%` |

### Path utilities

| Command | Arguments | Description |
|---|---|---|
| `basename PATH` | path string | Print the last component of PATH |
| `dirname PATH` | path string | Print PATH with the last component removed |

### Directory commands

| Command | Arguments | Description |
|---|---|---|
| `dirch PATH` | path string | Change working directory to PATH; no argument changes to `/` |
| `dirwd` | _(none)_ | Print basename of current working directory |
| `dirwd f` | `f` | Print full path of current working directory |
| `dirmk DIR` | directory name | Create directory DIR with permissions `0755` |
| `dirrm DIR` | directory name | Remove empty directory DIR |
| `dirls` | _(none)_ | List contents of current directory |
| `dirls PATH` | path string | List contents of PATH |

### File operations

| Command | Arguments | Description |
|---|---|---|
| `rename OLD NEW` | two names | Rename file or directory OLD to NEW |
| `remove NAME` | file or directory | Remove file or empty directory (calls `remove()`) |
| `unlink NAME` | file or symlink | Remove a file or symbolic link (calls `unlink()`) |
| `cpcat` | _(none)_ | Copy stdin to stdout |
| `cpcat SRC` | source file | Copy SRC to stdout |
| `cpcat SRC DST` | source, destination | Copy SRC to DST (appends if DST exists) |

### Links

| Command | Arguments | Description |
|---|---|---|
| `linkhard GOAL NAME` | target, link name | Create a hard link NAME pointing to GOAL |
| `linksoft GOAL NAME` | target, link name | Create a symbolic link NAME pointing to GOAL |
| `linkread NAME` | symlink name | Print the target path of symbolic link NAME |
| `linklist NAME` | file name | Print all hard links to NAME found in the current directory |

### Process information

| Command | Arguments | Description |
|---|---|---|
| `pid` | _(none)_ | Print the shell's PID |
| `ppid` | _(none)_ | Print the shell's parent PID |
| `uid` | _(none)_ | Print the real user ID |
| `euid` | _(none)_ | Print the effective user ID |
| `gid` | _(none)_ | Print the real group ID |
| `egid` | _(none)_ | Print the effective group ID |
| `sysinfo` | _(none)_ | Print system info: sysname, nodename, release, version, machine |
| `proc` | _(none)_ | Print current proc filesystem path (default `/proc`) |
| `proc PATH` | path string | Set proc filesystem path to PATH |
| `pids` | _(none)_ | Print all running PIDs in ascending order |
| `pinfo` | _(none)_ | Print PID, PPID, state, and name of all running processes |

### Background and waiting

| Command | Arguments | Description |
|---|---|---|
| `waitone` | _(none)_ | Wait for any one background child; sets STATUS to its exit code |
| `waitone PID` | process ID | Wait for the specific background child PID; sets STATUS to its exit code |
| `waitall` | _(none)_ | Wait for all background children; STATUS is set to the last one's exit code |

---

## Background Execution

Appending `&` as the last token runs the command in the background:

```
dirmk mydir &
hostname &
```

- **External commands**: the parent forks and does not call `waitpid`, returning immediately.
- **Built-in commands**: the parent forks a child that runs the built-in and exits with its return value, so the result can be collected later with `waitone` or `waitall`.
- Background children have stdin redirected to `/dev/null` to prevent them from accidentally reading shell input.
- A `SIGCHLD` handler automatically reaps children that finish on their own, preventing zombies.
- `waitone` and `waitall` block `SIGCHLD` around their `waitpid` call to avoid a race condition where the handler reaps the child before the explicit wait can collect its status.

---

## External Command Execution

`execute_external()` uses `fork()` + `execvp()` + `waitpid()`:

1. `fflush(stdin)` to avoid duplicate output when stdout is piped.
2. `fork()` creates a child process.
3. In the child: build a `NULL`-terminated `argv` array from the token buffer and call `execvp()`. If `execvp` fails, print an error and `exit(127)`.
4. In the parent: if foreground, call `waitpid()` and store `WEXITSTATUS()` in `STATUS`. If background, return immediately.

---

## Debug Mode

Controlled by the `debug N` command. When `N > 0`, each parsed line produces additional output before execution:

```
Input line: 'cmd arg1 arg2'
Token 0: 'cmd'
Token 1: 'arg1'
Token 2: 'arg2'
Input redirect: 'file'       (if present)
Output redirect: 'file'      (if present)
Background: 1                (if present)
Executing builtin 'cmd' in foreground
```

For external commands the last line is instead:
```
External command 'cmd arg1 arg2'
```
The reconstructed external command string excludes redirect and background tokens.

---

## Redirect & Background Detection

`check_redirect()` scans the **last up to 3 tokens** for special operators (redirects must appear at the end of the command):

- `<filename` — input redirect (operator and filename as one token, no space)
- `>filename` — output redirect (operator and filename as one token, no space)
- `&` — background execution (must be a standalone single-character token)

Detected option tokens are subtracted from `token_num` before the command is dispatched, so built-in and external functions never see them.

---

## Global State

| Variable | Type | Description |
|---|---|---|
| `PROMPT` | `char[9]` | Current prompt string, max 8 chars, default `"mysh"` |
| `DEBUG_LVL` | `int` | Debug verbosity level, default `0` |
| `STATUS` | `int` | Exit status of the last executed command |
| `EXIT` | `int` | Set to `1` by the `exit` builtin to break the main loop |
| `IS_BUILTIN` | `int` | `0` if current command is a builtin, `1` if external |
| `BACKGROUND` | `int` | `1` if current command should run in background |
| `PROC_PATH` | `char[1024]` | Path to the proc filesystem, default `"/proc"` |

`IS_BUILTIN` and `BACKGROUND` are reset to `0` at the start of each loop iteration.

---

## Main Loop

1. Register `SIGCHLD` handler to auto-reap finished background children.
2. Allocate a zero-initialized 1000-byte buffer with `calloc`.
3. Read one line with `fgets`.
4. On EOF — break out of the loop and return the last `STATUS` code.
5. Skip empty lines (length 1, only `\n`).
6. Strip the trailing newline.
7. Tokenize → parse → execute.
8. Free the buffer, reset per-iteration state, repeat.
