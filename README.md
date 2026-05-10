# MyShell

A working shell prototype written in C, built as a homework assignment for an operating systems class.

Only "low-level" functions are used:
- System calls: `write()`, `fork()`, `exit()`...
- Memory allocation: `calloc()`, `malloc()`, `free()`...
- String manipulation: `strlen()`, `strcmp()`, `strcpy()`...
- Type conversion: `atoi()`
- Basic I/O: `printf()`, `fgets()`

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
  |-- execute_external()  -- not yet implemented
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

---

## Built-in Commands

Built-ins are stored as a table of `{ name, function_pointer }` structs. Adding a new built-in requires only a new function and one entry in the table.

| Command | Arguments | Description |
|---|---|---|
| `debug` | _(none)_ | Print current debug level |
| `debug N` | `N` — integer | Set debug level to N. Level > 0 enables token printing |
| `prompt` | _(none)_ | Print current prompt string |
| `prompt STR` | `STR` — max 8 chars | Set prompt to STR (supports quoted strings with spaces) |
| `status` | _(none)_ | Print exit status of the last command |
| `exit` | _(none)_ | Exit the shell with the last status code |
| `exit N` | `N` — integer | Exit the shell with status N |

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

`IS_BUILTIN` and `BACKGROUND` are reset to `0` at the start of each loop iteration.

---

## Main Loop

1. Allocate a zero-initialized 1000-byte buffer with `calloc`
2. Read one line with `fgets`
3. On EOF — break out of the loop and return the last `STATUS` code
4. Strip the trailing newline
5. Tokenize → parse → execute
6. Free the buffer, reset per-iteration state, repeat
