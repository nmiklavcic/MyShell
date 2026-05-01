**Repo for creating a version of the shell**
 - Homework assignment for the operating systems class
 - Goal: create a working prototype of a shell in C
 - Only "low-level" functions are allowed:
   - System calls                  -> write(), fork(), exit()...
   - Memory allocation functions   -> calloc(), malloc(), free()...
   - String manipulation functions -> strlen(), strcmp(), memcpy()...
   - Type conversion functions     -> atoi()
   - Basic I/O functions           -> printf(), fgets()

---

## Architecture

The shell runs as a REPL (Read-Evaluate-Print Loop). Each iteration reads one line from stdin, tokenizes it, and parses the result.

```
stdin
  |
  v
fgets()          -- reads one line at a time (up to MAX_CHARS = 1000)
  |
  v
tokenize()       -- splits the line into tokens
  |
  v
parse()
  |-- check_redirect()   -- detects redirect/background options
  |-- print_tokens()     -- prints tokens and interprets options
```

---

## Functions

### `tokenize(char *buff) -> int`

Splits the input line in-place by replacing space characters with `\0`, turning the single buffer into a sequence of adjacent C strings. Returns the number of tokens found.

Special cases handled:
- **Leading spaces** — skipped, do not produce empty tokens
- **Consecutive spaces** — treated as a single separator
- **Trailing spaces** — do not produce a phantom extra token
- **Quoted strings** (`"..."`) — spaces inside quotes are preserved; the quoted content becomes a single token
- **Comments** (`#`) — everything from a standalone `#` onwards is ignored

### `check_redirect(char *buff, int token_num) -> int`

Scans the last up to 3 tokens for redirect or background execution operators. Returns how many such options were found.

Recognised operators:
- `<filename` — input redirect
- `>filename` — output redirect
- `&` — background execution (single `&` token)

### `print_tokens(char *buff, int token_num, int options_num)`

Walks the tokenized buffer and prints each token. If `options_num > 0`, the trailing option tokens are printed separately with their meaning:
- `Input redirect: 'filename'`
- `Output redirect: 'filename'`
- `Background: 1`

### `parse(char *buff, int token_num)`

Orchestrates `check_redirect` and `print_tokens`.

---

## Main loop

1. Allocate a zero-initialized buffer with `calloc`
2. Read one line with `fgets`
3. Exit cleanly on EOF; report error on I/O failure
4. Strip the trailing newline
5. Call `tokenize` → `parse`
6. Free the buffer and repeat
