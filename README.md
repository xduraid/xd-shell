# 🐚 XD-Shell

A full-featured Linux shell written in C, Flex, and Bison.

---

## 🎯 Why XD-Shell?

This project is a personal learning effort and a challenge I took on to dive
deeper into systems programming and to understand how a real shell works.  
It is inspired by Bash and aims to feel familiar in its features and behavior,
while being designed and implemented completely from scratch.

---

## 🌟 Features

- **Full command language**: Supports commands, pipelines, I/O redirections,
  background execution, quoting, escape sequences, and comments.
- **Shell and environment variables**: Supports shell and environment variables,
  with builtins to define, update, and remove them.
- **Aliases**: Allows defining and removing command aliases through builtins.
- **Shell expansions**: Supports tilde expansion, parameter expansion, command
  substitution, and filename expansion (globbing).
- **Interactive mode**: Provides a full interactive shell experience with a
  dynamic prompt, advanced line editing, persistent command history, and
  context-aware tab completion.
- **Job control**: Supports foreground and background job management, including
  listing, suspending, resuming, and signaling jobs.
- **Script execution**: Supports non-interactive script execution and sourcing
  scripts into the current shell environment.
- **Startup files and initialization**: Implements login and non-login shell 
  initialization by setting up the default environment and running startup scripts.

---

## 🧭 Table of Contents

- [🚀 1 Getting Started](#getting-started)
    - [1.1 Prerequisites](#prerequisites)
    - [1.2 Building from Source](#building-from-source)
    - [1.3 Installation](#installation)
    - [1.4 Usage](#usage)
- [🔤 2 Shell Language](#shell-language)
    - [2.1 Commands](#commands)
    - [2.2 Pipelines](#pipelines)
    - [2.3 Quoting and Escaping](#quoting-and-escaping)
        - [2.3.1 Single Quotes](#single-quotes)
        - [2.3.2 Double Quotes](#double-quotes)
        - [2.3.3 Escape Sequences](#escape-sequences)
        - [2.3.4 Line Continuation](#line-continuation)
    - [2.4 Redirections](#redirections)
    - [2.5 Comments](#comments)
- [⚙️ 3 Command Execution](#command-execution)
- [🗝️ 4 Variables and Environment](#variables-and-environment)
    - [4.1 Shell Variables](#shell-variables)
        - [4.1.1 The `set` Builtin](#the-set-builtin)
        - [4.1.2 The `unset` Builtin](#the-unset-builtin)
    - [4.2 Environment Variables](#environment-variables)
        - [4.2.1 The `export` Builtin](#the-export-builtin)
        - [4.2.2 The `unexport` Builtin](#the-unexport-builtin)
    - [4.3 Variable Naming](#variable-naming)
- [🏷️ 5 Aliases](#aliases)
    - [5.1 Alias Naming](#alias-naming)
    - [5.2 Alias Expansion](#alias-expansion)
    - [5.3 The `alias` Builtin](#the-alias-builtin)
    - [5.4 The `unalias` Builtin](#the-unalias-builtin)
- [🪄 6 Shell Expansions](#shell-expansions)
    - [6.1 Tilde Expansion](#tilde-expansion)
    - [6.2 Parameter Expansion](#parameter-expansion)
        - [6.2.1 Variable Expansion](#variable-expansion)
        - [6.2.2 Special Parameters](#special-parameters)
    - [6.3 Command Substitution](#command-substitution)
    - [6.4 Word Splitting](#word-splitting)
    - [6.5 Filename Expansion](#filename-expansion)
    - [6.6 Quote Removal](#quote-removal)
- [🖥️ 7 Interactive Shell Mode](#interactive-shell-mode)
    - [7.1 Input Prompt](#input-prompt)
    - [7.2 Readline Features](#readline-features)
        - [7.2.1 Line Editing and Cursor Movement](#line-editing)
        - [7.2.2 History Navigation](#history-navigation)
        - [7.2.3 History Search](#history-search)
        - [7.2.4 Tab Completion](#tab-completion)
    - [7.3 Command History](#command-history)
        - [7.3.1 The `history` Builtin](#the-history-builtin)
    - [7.4 Tab Completion Generation](#tab-completion-generation)
- [🚦 8 Job Control](#job-control)
    - [8.1 Foreground and Background Jobs](#foreground-and-background-jobs)
    - [8.2 Job States](#job-states)
    - [8.3 Job Specifiers](#job-specifiers)
    - [8.4 The `jobs` Builtin](#the-jobs-builtin)
    - [8.5 The `fg` Builtin](#the-fg-builtin)
    - [8.6 The `bg` Builtin](#the-bg-builtin)
    - [8.7 The `kill` Builtin](#the-kill-builtin)
- [📄 9 Running Scripts](#running-scripts)
- [🔧 10 Startup Files and Initialization](#startup-files-and-initialization)
    - [10.1 Default Environment](#default-environment)
    - [10.2 Login Shell Detection](#login-shell-detection)
    - [10.3 Startup Files](#startup-files)
- [🧰 11 Other Builtins](#other-builtins)
    - [11.1 The `cd` Builtin](#the-cd-builtin)
    - [11.2 The `pwd` Builtin](#the-pwd-builtin)
    - [11.3 The `echo` Builtin](#the-echo-builtin)
    - [11.4 The `source` Builtin](#the-source-builtin)
    - [11.5 The `exit` Builtin](#the-exit-builtin)
    - [11.6 The `logout` Builtin](#the-logout-builtin)
- [✅ 12 Testing](#testing)
- [🤝 13 Contributing](#contributing)
- [📜 14 License](#license)
- [🔗 15 Related Projects](#related-projects)

---

## 🚀 1 Getting Started <a name="getting-started"></a> 

This section covers everything you need to build, install, and run `xd-shell`
from the command line.

### 1.1 Prerequisites <a name="prerequisites"></a>

To use `xd-shell`, you need a Linux environment with the following tools 
installed:

- **Git** (to clone the repository)
- **Flex** (lexer generator)
- **Bison** (parser generator)
- **GCC** (C compiler)
- **Make** (build system)

Below are example installation commands for Debian/Ubuntu-based systems:

#### Git

```sh
sudo apt install git
```

#### Flex

```sh
sudo apt install flex
```

#### Bison

```sh
sudo apt install bison
```

#### GCC

```sh
sudo apt install gcc
```

#### Make

```sh
sudo apt install make
```

--- 

### 1.2 Building from Source <a name="building-from-source"></a>

Clone the repository and build the project as follows:

```sh
git clone https://github.com/xduraid/xd-shell.git
cd xd-shell
make
```

This produces the `xd_shell` executable in `./bin/`. You can run it
directly without installing it with `./bin/xd_shell`.

--- 

### 1.3 Installation <a name="installation"></a>

To install the shell system-wide, run:

```sh
sudo make install
```

By default, this installs the `xd_shell` executable into `/usr/local/bin/`.

> ℹ️ **Note:** If you want to uninstall it later, run: 
> `sudo make uninstall`. 

---

### 1.4 Usage <a name="usage"></a>

The shell can be invoked as follows:

```sh
xd_shell [-l] [-c string | script]
```

When started without arguments, the shell reads commands from standard input.
If both standard input `stdin` and standard output `stdout` are connected to
a terminal, the shell runs in interactive mode (see
[Interactive Shell Mode](#interactive-shell-mode)).

A script file may be specified as an argument to read commands from that file, or
the `-c` option may be used to read commands from a string argument (see
[Running Scripts](#running-scripts)).

When invoked with the `-l` option, the shell runs as a login shell and performs
login-specific initialization before processing any commands (see
[Login Shell Detection](#login-shell-detection) and
[Startup Files](#startup-files)).

When the shell exits, it exits with the status of the last command executed.

---

## 🔤 2 Shell Language <a name="shell-language"></a>

`xd-shell` provides a command language supporting commands, pipelines,
I/O redirections, background execution, quoting, escape sequences, and comments.
This section describes the syntax and interpretation rules of the shell language.

---

### 2.1 Commands <a name="commands"></a>

A command consists of a command name (which may refer to a builtin or an external
program), zero or more arguments, and optional I/O redirections (see
[Redirections](#redirections)).

**General form:**

```text
command [arg ...] [redirection ...]
```

---

### 2.2 Pipelines <a name="pipelines"></a>

A pipeline is a sequence of one or more commands separated by the pipe character
(`|`). In a pipeline with multiple commands, the standard output of each command
is connected to the standard input of the next command.

A pipeline may optionally end with `&`, indicating that it will be executed
in the background.

**General form:**

```text
cmd [| cmd ...] [&]
```

---

### 2.3 Quoting and Escaping <a name="quoting-and-escaping"></a>

Quoting and escaping determine how the shell treats special characters. `xd-shell`
supports single quotes, double quotes, backslash escaping, and line continuation.

---

#### 2.3.1 Single Quotes <a name="single-quotes"></a>

Single quotes preserve the literal value of every character inside them. A single
quote cannot appear inside a single-quoted string, even when preceded by a
backslash.

---

#### 2.3.2 Double Quotes <a name="double-quotes"></a>

Double quotes preserve the literal value of all characters except `$`, `"`, and
`\`. Inside double quotes, a backslash can be used to escape `$`, `"`, or `\`.
When a backslash appears before one of these characters, the backslash is
removed and the following character is preserved literally. A backslash before
any other character inside double quotes is left unchanged.

---

#### 2.3.3 Escape Sequences <a name="escape-sequences"></a>

Outside of quotes, a backslash (`\`) preserves the literal value of the next
character by removing any special meaning it would otherwise have. The backslash
is removed, and the following character is left unchanged.

---

#### 2.3.4 Line Continuation <a name="line-continuation"></a>

A backslash (`\`) immediately followed by a newline (`\n`) is treated as a line
continuation. Both characters are removed, allowing a command or pipeline to
span multiple physical lines.

---

### 2.4 Redirections <a name="redirections"></a>

Redirections change the source of standard input `stdin` and the destinations of
standard output `stdout` and standard error `stderr`.

The shell supports the following redirections:

| Redirection   | Description                                              |
|---------------|----------------------------------------------------------|
| `< file`      | Redirect `stdin` from `file`                             |
| `> file`      | Redirect `stdout` to `file` (truncate)                   |
| `>> file`     | Redirect `stdout` to `file` (append)                     |
| `2> file`     | Redirect `stderr` to `file` (truncate)                   |
| `2>> file`    | Redirect `stderr` to `file` (append)                     |
| `>& file`     | Redirect both `stdout` and `stderr` to `file` (truncate) |
| `>>& file`    | Redirect both `stdout` and `stderr` to `file` (append)   |

Redirections are processed from left to right. When multiple redirections modify
the same stream, the last one takes effect.

> ℹ️ **Note:** Only filenames or words that expand to filenames may be used as
> redirection targets (`file`).

---

### 2.5 Comments <a name="comments"></a>

A comment is text that is ignored by the shell, it begins with the `#`
character and continues to the end of the line. Comments are recognized only
when the `#` appears outside single quotes (`'...'`) or double quotes (`"..."`).

---

## ⚙️ 3 Command Execution <a name="command-execution"></a>

After reading the input line, the shell processes it in three main phases:
scanning, parsing, and execution.

During the scanning phase, the input line is converted into a stream of tokens
by the lexer. As part of this phase, alias expansion is applied to the command
word of each command (see [Alias Expansion](#alias-expansion)). The token stream
is then passed to the parser.

During the parsing phase, the token stream is analyzed according to the shell
grammar and transformed into internal command and pipeline structures, as
described in [Commands](#commands) and [Pipelines](#pipelines). As part of this
process, shell expansions are applied to each word token (see
[Shell Expansions](#shell-expansions)), producing the final command names,
arguments, and redirection targets. Once a complete pipeline has been parsed,
it is passed to the execution phase.

During the execution phase, each command in the pipeline is prepared and run.
Any redirections associated with a command are applied first, establishing the
appropriate standard input, output, and error streams. The shell then determines
whether the command is a builtin or an external command.

For external commands, if the command name contains a `/`, it is treated as an
explicit path and used as-is. Otherwise, the shell searches the directories
listed in the `$PATH` environment variable to locate the executable. If the
executable is not found, the shell checks for an executable with that name in
the current working directory.

Commands that are part of the same pipeline normally run concurrently, each in
its own process, with pipes connecting their standard input and output streams.
Pipelines consisting of a single builtin command are executed directly within
the shell process so that their effects (such as variable assignments or
directory changes) persist in the current shell environment.

By default, the shell waits for the pipeline to complete before reading the next
command. If the pipeline is terminated with `&`, it is executed in the
background, and the shell continues reading input without waiting for it.

When running in interactive mode, each executed pipeline is registered with the
job control system. This enables foreground and background execution, terminal
control, and signal handling, as described in [Job Control](#job-control).

The exit status of a pipeline is the exit status of the last command executed
in the pipeline.

---

## 🗝️ 4 Variables and Environment <a name="variables-and-environment"></a>

`xd-shell` provides a variable system for storing named values within the shell.
These variables may be referenced in commands and are expanded during parameter
expansion (see [Parameter Expansion](#parameter-expansion)). Some variables also
affect the behavior of the shell and the execution environment of commands.

The shell distinguishes between two kinds of variables: **shell variables** and
**environment variables**. Shell variables exist only within the current shell
instance, while environment variables are additionally included in the
environment of external commands executed by the shell.

This section describes how variables are defined, updated, exported, and removed,
as well as the rules governing valid variable names.

---

### 4.1 Shell Variables <a name="shell-variables"></a>

Shell variables are name-value pairs that exist within the current shell
instance.

They are local to the shell and are not automatically included in the
environment of external commands.

Shell variables may be defined, updated, and removed using the `set` and
`unset` builtins.

---

#### 4.1.1 The `set` Builtin <a name="the-set-builtin"></a>

The `set` builtin is used to define or update shell variables, or to display
existing ones.

**Usage:**

```sh
set [name[=value] ...]
```

**Options:**

| Option       | Description            |
|--------------|------------------------|
| `--help`     | Show help information  |

**Behavior:**

- **Without arguments:**  
  Prints all defined variables in the following shell-reusable form:
  
  ```sh
  set name1='value1'
  set name2='value2'
  ...
  set nameN='valueN'
  ```

- **With arguments:**  
  Each argument must be in one of the following forms:
  - `name` — prints the variable `name` in the shell-reusable form.
  - `name=value` — defines or updates the variable `name` to `value`,
    preserving its exported status (see
  [Environment Variables](#environment-variables)).

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

#### 4.1.2 The `unset` Builtin <a name="the-unset-builtin"></a>

The `unset` builtin is used to remove variables.

**Usage:**

```sh
unset name [name ...]
```

**Options:**

| Option       | Description            |
|--------------|------------------------|
| `--help`     | Show help information  |

**Behavior:**

For each argument `name`, the corresponding variable is removed if it exists.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

### 4.2 Environment Variables <a name="environment-variables"></a>

Environment variables are shell variables that have been marked for export and
are included in the environment of external commands executed by the shell.

Environment variables may be defined, exported, and removed using the `export`
and `unexport` builtins.

---

#### 4.2.1 The `export` Builtin <a name="the-export-builtin"></a>

The `export` builtin is used to mark variables for export so that they are
included in the environment of subsequently executed external commands.

**Usage:**

```sh
export [name[=value] ...]
```

**Options:**

| Option   | Description                                         |
|----------|-----------------------------------------------------|
| `--help` | Show help information                               |

**Behavior:**

- **Without arguments:**  
  Prints all exported variables in the following shell-reusable form:
  
  ```sh
  export name1='value1'
  export name2='value2'
  ...
  export nameN='valueN'
  ```

- **With arguments:**  
  Each argument must be in one of the following forms:
  - `name` — marks the variable `name` as exported.
  - `name=value` — defines or updates the variable `name` to `value`,
    and marks it for export.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

#### 4.2.2 The `unexport` Builtin <a name="the-unexport-builtin"></a>

The `unexport` builtin is used to remove the export attribute from variables so
that they are no longer included in the environment of subsequently executed
external commands.

**Usage:**

```sh
unexport name [name ...]
```

**Options:**

| Option   | Description           |
|----------|-----------------------|
| `--help` | Show help information |


**Behavior:**

For each argument `name`, the export attribute of the corresponding
variable is removed.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

### 4.3 Variable Naming <a name="variable-naming"></a>

Variable names may contain letters, digits, and underscores, but must not begin
with a digit.

> ℹ️ **Note:** Variable names are case-sensitive.

---

## 🏷️ 5 Aliases <a name="aliases"></a>

`xd-shell` provides a mechanism for defining command aliases. Aliases allow
commands or command pipelines to be referenced by shorter or more convenient
names. They may also be used to override existing commands with alternative
behavior.

An alias associates a name with a replacement text. When a command is read, the
shell checks whether the command word matches a defined alias and performs
alias expansion accordingly.

Aliases are maintained within the current shell instance and are managed using
the `alias` and `unalias` builtins.

This section describes how aliases are defined, expanded, and removed, as well
as the rules governing valid alias names.

---

### 5.1 Alias Naming <a name="alias-naming"></a>

Alias names follow the same naming rules as shell variable names. They may
contain letters, digits, and underscores, but must not begin with a digit.

> ℹ️ **Note:** Alias names are case-sensitive.

---

### 5.2 Alias Expansion <a name="alias-expansion"></a>

Alias expansion is performed during the scanning phase of command processing,
before parsing and execution (see [Command Execution](#command-execution)).

For each command, the shell examines its command word. If it matches the name
of a defined alias, it is replaced by the corresponding alias text.

The result of alias expansion is rescanned for further alias expansion. This
process continues until no further expansions are possible or an alias expands
to itself.

If an alias expands to multiple words, the resulting text is processed as if it
had been entered directly by the user.

> ℹ️ **Note:** Alias expansion applies only to *unquoted command words*. Words
> that appear in other positions, such as arguments or redirection targets, are
> not subject to alias expansion.

---

### 5.3 The `alias` Builtin <a name="the-alias-builtin"></a>

The `alias` builtin is used to define or update aliases, or display existing
ones.

**Usage:**

```sh
alias [name[=value] ...]
```

**Options:**

| Option   | Description           |
|----------|-----------------------|
| `--help` | Show help information |

**Behavior:**

- **Without arguments:**  
  Prints all defined aliases in the following shell-reusable form:

  ```sh
  alias name1='value1'
  alias name2='value2'
  ...
  alias nameN='valueN'
  ```

- **With arguments:**  
  Each argument must be in one of the following forms:
  - `name` — prints the alias `name` in the shell-reusable form.
  - `name=value` — defines or updates the alias `name` to the replacement text
    `value`.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

### 5.4 The `unalias` Builtin <a name="the-unalias-builtin"></a>

The `unalias` builtin is used to remove aliases.

**Usage:**

```sh
unalias name [name ...]
```

**Options:**

| Option   | Description           |
|----------|-----------------------|
| `--help` | Show help information |

**Behavior:**

For each argument `name`, the corresponding alias is removed if it exists.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

## 🪄 6 Shell Expansions <a name="shell-expansions"></a>

`xd-shell` provides a set of expansion mechanisms that transform words in a
command into their final form before execution. These expansions allow the shell
to resolve variable references, substitute special patterns, incorporate the
output of commands into the command, and produce multiple words from a single
input word.

The shell supports the following expansions, which are performed during the
parsing phase of command processing (see [Command Execution](#command-execution)),
in the following order:

1. [Tilde Expansion](#tilde-expansion)
2. [Parameter Expansion](#parameter-expansion)
3. [Command Substitution](#command-substitution)
4. [Word Splitting](#word-splitting)
5. [Filename Expansion](#filename-expansion)
6. [Quote Removal](#quote-removal)

This section describes each type of expansion and the rules governing its behavior.

---

### 6.1 Tilde Expansion <a name="tilde-expansion"></a>

Tilde expansion provides a convenient way to refer to the home directory of the
current user or other users using `~` at the beginning of a word, instead of
specifying an absolute path.

The shell supports the following forms of tilde expansion:

- `~` and `~/path`  
  Expands `~` to the current user's home directory.  
  If `$HOME` is set, its value is used. Otherwise, the home directory is
  obtained from the system's user database.

- `~user` and `~user/path`  
  Expands `~user` to the home directory of the user `user`.  
  If no such user exists, the word is left unchanged.

- `~+` and `~+/path`  
  Expands `~+` to the current working directory `$PWD`.  
  If `$PWD` is unset, the word is left unchanged.

- `~-` and `~-/path`  
  Expands `~-` to the previous working directory `$OLDPWD`.  
  If `$OLDPWD` is unset, the word is left unchanged.

Tilde expansion is applied only when `~` appears at the beginning of an
unquoted word, and only within the first path component (that is, before the
first `/`). A `~` appearing in any other position, or within single or double
quotes, is treated as a literal character.

> ℹ️ **Note:** `$PWD` and `$OLDPWD` are maintained by 
[The `cd` builtin](#the-cd-builtin).

---

### 6.2 Parameter Expansion <a name="parameter-expansion"></a>

Parameter expansion replaces references to parameters with their corresponding
values. Parameters include shell variables, environment variables, and special
parameters maintained by the shell.

The shell supports the following forms of parameter expansion:

- `$name`
- `${name}`

In these forms, `name` refers to a shell variable, an environment variable, or
a special parameter.

A `$` begins parameter expansion only when it appears in unquoted or
double-quoted text and is followed by a valid parameter name or expansion form.
Otherwise, the `$` character is treated as a literal and is not expanded.

---

#### 6.2.1 Variable Expansion <a name="variable-expansion"></a>

Variable expansion replaces references to shell variables and environment
variables with their corresponding values.

When using the `$name` form, all consecutive characters that form a valid
variable name are considered part of the variable reference. To avoid ambiguity
with adjacent characters, the `${name}` form may be used to explicitly delimit
the variable name.

If a referenced variable is not defined, it is replaced by an empty string.

---

#### 6.2.2 Special Parameters <a name="special-parameters"></a>

Special parameters are predefined, read-only parameters maintained by the shell.
They provide information about the state of the shell and recently executed
commands.

`xd-shell` supports the following special parameters:

| Parameter | Description                                                            |
|-----------|------------------------------------------------------------------------|
| `?`       | Expands to the exit status of the most recently executed command       |
| `$`       | Expands to the process ID of the current shell                         |
| `!`       | Expands to the process ID of the most recently executed background job |

Special parameters are expanded using the forms described in
[Parameter Expansion](#parameter-expansion). Both `$name` and `${name}` forms
are valid. For example, both `$?` and `${?}` expand to the exit status of the
most recently executed command.

---

### 6.3 Command Substitution <a name="command-substitution"></a>

Command substitution allows the output of a command to be used as part of another
command using the form `$(command)`.

The text inside `$(...)` is parsed as normal shell input and may contain
commands, pipelines, redirections, and nested command substitutions.
It is executed in a subshell (a child shell process), and the entire `$(...)`
construct is replaced by its standard output, with any trailing newline
characters removed.

Only the standard output `stdout` of the command is captured. The standard
error `stderr` is not affected and is passed through unchanged.

A `$(...)` construct is recognized as command substitution only when it
appears in unquoted or double-quoted text. Otherwise, it is treated as
literal text.

If the command produces no output, the `$(...)` construct expands to an
empty string.

> ℹ️ **Note:** Because command substitution is executed in a subshell,
> any side effects such as modifying variables or changing the working
> directory do not affect the parent shell.

---

### 6.4 Word Splitting <a name="word-splitting"></a>

Word splitting divides the text produced by expansions into separate words.

The resulting text is split using the following whitespace characters as
delimiters: space (` `), tab (`\t`), and newline (`\n`). Consecutive
whitespace characters are treated as a single delimiter, and leading and
trailing whitespaces are ignored.

Word splitting is applied only to the results of expansions that occur outside
double quotes.

If an expansion results in empty text, no words are produced, since there is no
content to split. However, when such an expansion occurs within double quotes,
it is treated as a single empty word rather than being discarded.

---

### 6.5 Filename Expansion <a name="filename-expansion"></a>

Filename expansion matches patterns in words against filenames and replaces
them with the names of matching files or directories.

After word splitting, each word is examined. If it contains unquoted
wildcard characters, bracket expressions, or brace patterns, the word is
treated as a filename pattern and is matched against filenames.

The shell supports the following constructs in filename expansion:

| Syntax      | Description                                                                  |
|-------------|------------------------------------------------------------------------------|
| `*`         | Matches any sequence of characters, including an empty sequence              |
| `?`         | Matches exactly one character                                                |
| `[abc]`     | Matches a single character from those listed inside the brackets             |
| `[a-z]`     | Matches a single character from the specified character range                |
| `[!abc]`    | Matches a single character not listed inside the brackets                    |
| `[!a-z]`    | Matches a single character outside the specified character range             |
| `{a,b,c}`   | Brace pattern: specifies multiple alternative patterns                       |

Brace patterns are expanded into multiple patterns during matching.
This expansion is purely textual and does not by itself produce any output.
Each resulting pattern is then matched independently.

Patterns are applied independently to each path component separated by `/`.
The `/` character is not matched by `*`, `?`, or bracket expressions and must
appear explicitly in the pattern.

If a pattern matches one or more filenames, it is replaced by the list of
matching names, in lexicographical order. Each match becomes a separate word.

Filenames beginning with `.` are matched only when the pattern explicitly
begins with a `.`.

If a pattern does not match any filenames, no expansion is performed and the
word is left unchanged.

> ℹ️ **Note:** Brace patterns may be nested.

---

### 6.6 Quote Removal <a name="quote-removal"></a>

Quote removal removes the quote characters and backslashes that were used
to preserve the literal value of text.

After all expansions have been performed, any remaining quote characters
and backslashes that did not result from the preceding expansions are
removed from each word. This includes single quotes (`'`), double quotes
(`"`), and backslashes (`\`).

A backslash is removed when it appears outside quotes, and within double
quotes when it precedes `$`, `"`, `\`, or a newline, as described in
[Quoting and Escaping](#quoting-and-escaping).

---

## 🖥️ 7 Interactive Shell Mode <a name="interactive-shell-mode"></a>

`xd-shell` provides an interactive shell environment when both standard input
`stdin` and standard output `stdout` are connected to a terminal.

In interactive mode, the shell presents a dynamic prompt and accepts input
through an interactive line editor. It maintains command history across
sessions, provides context-aware tab completion, and integrates with the job
control system (see [Job Control](#job-control)).

This section describes the behavior and features available when the shell is
running interactively.

---

### 7.1 Input Prompt <a name="input-prompt"></a>

In interactive mode, the shell displays a dynamic, colorized prompt that
includes the current user, host, and working directory.

The prompt has the general form:

```sh
user@host:cwd$
```

The prompt ends with `$` for normal users and `#` for the superuser.

When the current working directory is within the user's home directory, the
`$HOME` prefix is replaced with `~`.

The prompt is updated before each input line is read to reflect changes such as
the current working directory.

---

### 7.2 Readline Features <a name="readline-features"></a>

The shell uses [xd-readline](https://github.com/xduraid/xd-readline), a custom
line-editing library implemented specifically for this project.

In interactive mode, input is read through this line editor rather than directly
from standard input. This provides an editing experience similar to that of
modern interactive shells, allowing the user to modify the current line before
it is submitted for execution.

The line editor supports cursor movement, in-line text editing, history
navigation and search, and tab-triggered completion.

---

#### 7.2.1 Line Editing and Cursor Movement <a name="line-editing"></a>

The line editor allows modification of the current input line before it is
submitted for execution.

The editor supports cursor movement by character and by word, navigation to the
beginning and end of the line, and deletion of characters or words.

The following cursor movement and editing actions are supported:

| Key Combination          | Action                                                                   |
|--------------------------|--------------------------------------------------------------------------|
| `←` or `Ctrl+B`          | Move the cursor one character to the left                                |
| `→` or `Ctrl+F`          | Move the cursor one character to the right                               |
| `Ctrl+←` or `Alt+B`      | Move the cursor one word to the left                                     |
| `Ctrl+→` or `Alt+F`      | Move the cursor one word to the right                                    |
| `Home` or `Ctrl+A`       | Move the cursor to the beginning of the line                             |
| `End` or `Ctrl+E`        | Move the cursor to the end of the line                                   |
| `Backspace` or `Ctrl+H`  | Delete the character before the cursor                                   |
| `Delete`                 | Delete the character at the cursor                                       |
| `Ctrl+D`                 | Delete the character at the cursor, or signal `EOF` if the line is empty |
| `Alt+Backspace`          | Delete the word before the cursor                                        |
| `Alt+D` or `Ctrl+Delete` | Delete the word after the cursor                                         |
| `Ctrl+U`                 | Delete all text before the cursor                                        |
| `Ctrl+K`                 | Delete all text from the cursor to the end of the line                   |
| `Ctrl+L`                 | Clear screen                                                             |
| `Enter` or `Ctrl+J`      | Submit the current input line                                            |

> ℹ️ **Note:** A word is a sequence of letters and digits.
> Word-based operations use this definition when moving the cursor or deleting text.

---

#### 7.2.2 History Navigation <a name="history-navigation"></a>

The line editor allows previously executed commands to be recalled from the
command history by navigating through the history list sequentially
(see [Command History](#command-history)).

By moving backward and forward through the history list, the user may recall
or modify commands in the order they were executed, without retyping them from
scratch. When a history entry is recalled, it becomes the current editable line
and may be modified before execution.

The following history-navigation actions are supported:

| Key Combination              | Action                                       |
|------------------------------|----------------------------------------------|
| `↑` or `Page Up`             | Move to the previous history entry           |
| `↓` or `Page Down`           | Move to the next history entry               |
| `Ctrl+↑` or `Ctrl+Page Up`   | Jump to the first (oldest) history entry     |
| `Ctrl+↓` or `Ctrl+Page Down` | Jump to the last (most recent) history entry |

---

#### 7.2.3 History Search <a name="history-search"></a>

The line editor allows previously executed commands to be recalled from the
command history by matching text within them (see [Command History](#command-history)).
Instead of traversing entries sequentially, the user can jump directly to
commands that contain a given search string.

The search is incremental: as the user types the search text, the current match
is updated and highlighted in real time. The user may search in either the
reverse (older entries) or forward (more recent entries) direction, and may
accept, skip, or cancel matches.

When a match is accepted, it becomes the current editable line and may be
modified before execution.

The following history-search actions are supported:

| Key Combination | Action                                                |
|-----------------|-------------------------------------------------------|
| `Ctrl+R`        | Start reverse search or jump to the previous match    |
| `Ctrl+S`        | Start forward search or jump to the next match        |
| `Ctrl+G`        | Cancel the search and restore the original input line |
| `Esc Esc`       | Accept the current match and exit search mode         |

---

#### 7.2.4 Tab Completion <a name="tab-completion"></a>

The line editor supports tab-triggered completion to assist in writing
commands.

When the `Tab` key is pressed, the line editor requests completions for the
current word at the cursor. The shell generates and returns a set of
completions based on the word and its position within the command line (see
[Tab Completion Generation](#tab-completion-generation)).

After receiving the completions, the line editor:

- Completes the word fully if there is a single completion.
- Inserts the longest common prefix if there are multiple completions.
- Displays all completions on subsequent `Tab` presses.

---

### 7.3 Command History <a name="command-history"></a>

The shell maintains command history across interactive sessions, allowing
previously executed commands to be recalled, edited, and executed again.

During an interactive session, executed commands are appended to an in-memory
history list. Consecutive identical commands are not duplicated, and commands
that begin with leading whitespace are not added to the history list. The history
list holds up to 1000 entries. When this limit is reached, the oldest entries are
discarded to make room for newer commands.

Command history can also be persisted through a history file. When the shell
starts in interactive mode, it determines the history file from `$HISTFILE`. If
`$HISTFILE` is unset, the shell sets it to `.xdsh_history` inside the user's
home directory. If the home directory cannot be resolved, `$HISTFILE` is set to
`.xdsh_history` instead.

The history file is loaded on startup and written back on exit, allowing
commands from previous sessions to be reused in later interactive sessions.

The current history list is used by [History Navigation](#history-navigation)
and [History Search](#history-search), and can be displayed and managed using
the [`history` builtin](#the-history-builtin).

---

#### 7.3.1 The `history` Builtin <a name="the-history-builtin"></a>

The `history` builtin is used to display and manage the current command history.

**Usage:**

```sh
history [-c]
history -a [file]
history -r [file]
history -w [file]
```

**Options:**

| Option      | Description                                                                    |
|-------------|--------------------------------------------------------------------------------|
| `-c`        | Clear the in-memory history list                                               |
| `-a [file]` | Append history entries from the current session to `file`                      |
| `-r [file]` | Read history entries from `file` and append them to the in-memory history list |
| `-w [file]` | Write the entire in-memory history list to `file`                              |
| `--help`    | Show help information                                                          |

**Behavior:**

The `history` builtin is available only in interactive shell mode. Without
options, it prints the current in-memory history list with line numbers. For
`-a`, `-r`, and `-w`, if `file` is not specified, the file named by `$HISTFILE`
is used. If `-c` is combined with `-a`, `-r`, or `-w`, the history list is
cleared before the file operation is performed. When more than one of `-a`,
`-r`, or `-w` is given, the last one takes effect.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

### 7.4 Tab Completion Generation <a name="tab-completion-generation"></a>

When the `Tab` key is pressed, the line editor requests completions for the
word at the cursor (see [Tab Completion](#tab-completion)). The shell then
generates context-aware completions based on the word’s position within the
command line.

Completions are generated as follows:

- When completing a command word, completions are generated from executable
  paths such as `./program`, external command names found in the directories
  listed in `$PATH`, shell builtins, and aliases.

- When completing an argument or redirection target, completions are generated
  from file and directory paths. Directory names are returned with a trailing
  `/` to indicate that further path components may follow.

- When the word begins with `$` or `${`, completions are generated from variable
  names.

- When the word begins with `~`, completions are generated from user home
  directories.

---

## 🚦 8 Job Control <a name="job-control"></a>

When running interactively in a terminal, `xd-shell` treats each executed
pipeline as a job for the purpose of job control. In this mode, all processes
belonging to the same job share a process group ID (PGID), and each active job 
is assigned a job ID that the shell uses to identify and manage it through
job-control builtins.

`xd-shell` provides job control only in interactive mode, allowing users to run
jobs in the foreground or background and to suspend or resume them as needed.

---

### 8.1 Foreground and Background Jobs <a name="foreground-and-background-jobs"></a>

In interactive mode, `xd-shell` manages terminal ownership by deciding which
process group controls the terminal. At any moment, either the shell or the job
running in the foreground holds the terminal, ensuring that input and
terminal-generated signals are delivered correctly.

By default, a job runs in the foreground. The shell gives it control of the
terminal and waits for it to finish or become stopped before reading the next
command. Foreground jobs receive terminal-generated signals such as *SIGINT*
(`Ctrl+C`) and *SIGTSTP* (`Ctrl+Z`).

A job can also be started in the background by appending `&` to the command
line. Background jobs do not take control of the terminal; the shell
immediately returns to the prompt so other commands can be entered while the job
continues running. Background jobs do not receive terminal-generated signals,
and if they attempt to read from or write to the terminal, they are stopped by
the operating system through *SIGTTIN* or *SIGTTOU*.

> ℹ️ **Note:** In interactive mode, `xd-shell` ignores several signals that are 
intended for foreground jobs rather than for the shell itself, including *SIGTERM*, 
*SIGINT*, *SIGQUIT*, *SIGTSTP*, *SIGTTIN*, and *SIGTTOU*. This prevents the shell 
from being interrupted or suspended.

---

### 8.2 Job States <a name="job-states"></a>

`xd-shell` tracks the state of each active job so it can report changes and
manage jobs correctly during execution.

A job may be in one of the following states:

- **Running** — the job is currently executing.
- **Stopped** — the job has been suspended.
- **Done** — the job finished with an exit status of `0`.
- **Exited** — the job finished with a non-zero exit status.
- **Signaled** — the job was terminated by a signal.

---

### 8.3 Job Specifiers <a name="job-specifiers"></a>

`xd-shell` allows referring to jobs using job specifiers, which are short
notations beginning with `%`. These are used by job-control builtins to identify
which job should be acted on.

Supported job specifiers:

- `%n` — job with ID `n`
- `%+` or `%%` — the *current job*
- `%-` — the *previous job*

After each command completes or a job changes state, `xd-shell` refreshes the job
table and updates the current and previous jobs. The *current job* is the most
recently active job, with stopped jobs preferred over running ones. The *previous
job* is simply the next most recently active job.

---

### 8.4 The `jobs` Builtin <a name="the-jobs-builtin"></a>

The `jobs` builtin is used to display the status of all active jobs.

**Usage:**

```sh
jobs [-lp]
```

**Options:**

| Option   | Description                                      |
|----------|--------------------------------------------------|
| `-l`     | Show detailed status for each process in the job |
| `-p`     | Show the process ID(s) associated with each job  |
| `--help` | Show help information                            |

**Behavior:**

The `jobs` builtin does not accept any non-option arguments. It prints the
current job table, and the `-l` and `-p` options modify which additional
information is included.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

### 8.5 The `fg` Builtin <a name="the-fg-builtin"></a>

The `fg` builtin is used to move a job into the foreground, resuming it if it is 
stopped.

**Usage:**

```sh
fg [job_spec]
```

**Options:**

| Option   | Description            |
|----------|------------------------|
| `--help` | Show help information  |


**Behavior:**

The `fg` builtin only works when job control is enabled in interactive shell mode. 
It accepts at most one optional `job_spec` operand. If `job_spec`
is omitted, the *current job* (as described in [Job Specifiers](#job-specifiers)) is 
used. The specified job is brought into the foreground, if it was stopped, it is 
resumed and runs in the foreground while `xd-shell` waits for it to finish or stop 
again.

**Exit status:**

Returns the exit status of the command placed in the foreground unless an error occurs.

---

### 8.6 The `bg` Builtin <a name="the-bg-builtin"></a>

The `bg` builtin is used to move one or more jobs to the background, resuming them 
if they are stopped.

**Usage:**

```sh
bg [job_spec ...]
```

**Options:**

| Option   | Description           |
|----------|-----------------------|
| `--help` | Show help information |

**Behavior:**

The `bg` builtin only works when job control is enabled in interactive shell
mode. It accepts zero or more `job_spec` operands. If no `job_spec` is given,
the *current job* (as described in [Job Specifiers](#job-specifiers)) is 
used. Each specified job is moved to the background, if it was stopped, it is 
resumed and continues running in the background as if it had been started with `&`.

**Exit status:**

Returns `0` unless job control is not enabled or an error occurs.

---

### 8.7 The `kill` Builtin <a name="the-kill-builtin"></a>

The `kill` builtin is used to send a signal to one or more processes or jobs.

**Usage:**

```sh
kill [-s sigspec | -n signum] pid | jobspec ...
kill -l
```

**Options:**

| Option   | Description                                           |
|----------|-------------------------------------------------------|
| `-s sig` | Use the signal named by `sig`                         |
| `-n num` | Use the signal numbered `num`                         |
| `-l`     | List the available signal names and their numbers     |
| `--help` | Show help information                                 |

**Behavior:**

By default, `kill` sends the `SIGTERM` signal. When `-s` or `-n` is provided, the
specified signal name or number determines which signal is sent. Each argument is
either a process ID or a job specifier beginning with `%` (as described in
[Job Specifiers](#job-specifiers)). When a job specifier is used, the signal is
sent to the job's process group. With `-l`, `kill` prints the list of available
signals and their numbers instead of sending a signal.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

## 📄 9 Running Scripts <a name="running-scripts"></a>

A shell script is a file containing commands written in the `xd-shell` language.
Scripts can be executed in two ways: by invoking the shell with a script file as
an argument, or by running an executable script as a normal command.

When the shell is invoked with a script file argument, it reads and executes the
commands in that file sequentially and then exits. Invoking the shell with the
`-c` option follows the same execution model, reading commands from the provided
string argument and exiting after they are executed.

When an executable script is run as a normal command, the shell locates the
script file using the same rules as for other external commands, as described
in [Command Execution](#command-execution). Once the script file is located,
the shell executes it by starting a new `xd-shell` instance with the script path
passed as an argument. If the script begins with a `#!` interpreter line, the
specified interpreter may be invoked instead.

> ℹ️ **Note:** Scripts executed this way do not affect the current shell
> environment. To execute a script in the current shell environment, use the
> [`source`](#the-source-builtin) builtin.

---

## 🔧 10 Startup Files and Initialization <a name="startup-files-and-initialization"></a>

When `xd-shell` starts, it initializes a small set of common environment
variables and may run commands from a startup file.  
A startup file is a normal shell script that is executed in the current shell
process, so variable assignments, exported variables, aliases, and other 
changes take effect in the current shell session.

---

### 10.1 Default Environment <a name="default-environment"></a>

During startup, `xd-shell` ensures that a basic set of environment variables is
available. These variables provide information commonly expected by shell users
and external programs.

If a variable is already defined, the shell leaves it unchanged. Otherwise, it
assigns a default value.

| Variable   | Description                               | Default value (if unset)                                  |
|------------|-------------------------------------------|----------------------------------------------------------|
| `HOME`     | The current user's home directory         | The current user's home directory as defined in the system database |
| `USER`     | The current user's name                   | The current user's login name as defined in the system database  |
| `LOGNAME`  | The current user's login name                     | The current user's login name as defined in the system database  |
| `PATH`     | Directories searched for external commands | `/usr/local/bin:/usr/local/sbin:`<br>`/usr/bin:/usr/sbin:/bin:/sbin` |
| `SHLVL`    | Shell nesting level                       | `1` for login shells; otherwise incremented by `1`       |
| `SHELL`    | Path of the running `xd-shell` executable | The full path to the `xd-shell` executable               |
| `HISTFILE` | Persistent command history file           | `$HOME/.xdsh_history`                                    |


---

### 10.2 Login Shell Detection <a name="login-shell-detection"></a>

`xd-shell` is considered a login shell when it is invoked with `-l`, or when
`argv[0]` begins with a leading `-` (when started by `login`).

---

### 10.3 Startup Files <a name="startup-files"></a>

After completing initialization, `xd-shell` may run commands from a startup file
to perform user-specified initialization.

If the shell is a login shell, `~/.xdsh_profile` is executed. If the shell is an
interactive but non-login shell, `~/.xdshrc` is executed.

If the startup file does not exist or cannot be read, it is silently skipped.

---

## 🧰 11 Other Builtins <a name="other-builtins"></a>

`xd-shell` provides builtins for common shell tasks such as changing the working
directory, printing information, executing commands from a file, and exiting the
shell.

---


### 11.1 The `cd` Builtin <a name="the-cd-builtin"></a>

The `cd` builtin is used to change the shell's current working directory.

**Usage:**

```sh
cd [dir]
```

**Options:**

| Option       | Description            |
|--------------|------------------------|
| `--help`     | Show help information  |

**Behavior:**

- **Without arguments:**  
  Changes the current directory to `$HOME`.

- **With an argument:**  
  If the argument is `-`, it is treated as `$OLDPWD`. Otherwise, it is treated
  as the target directory.

The `cd` builtin maintains the environment variables `$PWD` and `$OLDPWD` 
to track the current and previous working directories.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

### 11.2 The `pwd` Builtin <a name="the-pwd-builtin"></a>

The `pwd` builtin is used to print the path of the current working directory.

**Usage:**

```sh
pwd
```

**Options:**

| Option       | Description            |
|--------------|------------------------|
| `--help`     | Show help information  |

**Behavior:**

The `pwd` builtin does not take any arguments. It prints the absolute path of the
current working directory followed by a newline.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

### 11.3 The `echo` Builtin <a name="the-echo-builtin"></a>

The `echo` builtin is used to write arguments to the standard output.

**Usage:**

```sh
echo [-ne] [arg ...]
```

**Options:**

| Option   | Description                                |
|----------|--------------------------------------------|
| `-n`     | Do not output the trailing newline         |
| `-e`     | Enable interpretation of backslash escapes |
| `--help` | Show help information                      |

**Behavior:**

The `echo` builtin prints its arguments separated by a single space. By default,
a trailing newline is printed. When `-n` is given, the trailing newline is suppressed.

When `-e` is given, the following backslash escapes are recognized:

| Escape | Meaning                 |
|--------|-------------------------|
| `\a`   | Alert (BEL)             |
| `\b`   | Backspace               |
| `\c`   | Suppress further output |
| `\e`   | Escape character        |
| `\E`   | Escape character        |
| `\f`   | Form feed               |
| `\n`   | New line                |
| `\r`   | Carriage return         |
| `\t`   | Horizontal tab          |
| `\v`   | Vertical tab            |
| `\\`   | Backslash               |

If `\c` is encountered while `-e` is enabled, `echo` stops producing output and
does not print a trailing newline.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

### 11.4 The `source` Builtin <a name="the-source-builtin"></a>

The `source` builtin is used to execute commands from a file in the current shell
environment.

**Usage:**

```sh
source file
```

**Options:**

| Option   | Description           |
| -------- | --------------------- |
| `--help` | Show help information |

**Behavior:**

The `source` builtin reads commands from the given file and executes them in 
the current shell process. The commands are executed in the same mode as the 
current shell: if `source` is used while running a script, the file is executed 
as part of the script, and if `source` is used in interactive mode, the file is
executed as if its commands were entered by the user.

**Exit status:**

Returns `0` unless an invalid option is given or an error occurs.

---

### 11.5 The `exit` Builtin <a name="the-exit-builtin"></a>

The `exit` builtin is used to exit the shell.

**Usage:**

```sh
exit [n]
```

**Options:**

| Option   | Description           |
|----------|-----------------------|
| `--help` | Show help information |

**Behavior:**

Exits the shell with a status of `n`. If `n` is omitted, the exit status is that
of the last command executed.

---

### 11.6 The `logout` Builtin <a name="the-logout-builtin"></a>

The `logout` builtin is used to exit a login shell.

**Usage:**

```sh
logout [n]
```

**Options:**

| Option   | Description           |
|----------|-----------------------|
| `--help` | Show help information |

**Behavior:**

Exits the login shell with a status of `n`. If `n` is omitted, the exit status is
that of the last command executed. If the shell is not a login shell, it prints
an error and fails.

---

## ✅ 12 Testing <a name="testing"></a>

---

## 🤝 13 Contributing <a name="contributing"></a>

Contributions are welcome.  
Feel free to open an issue for bug reports or suggestions, or submit a pull request
with improvements.

---

## 📜 14 License <a name="license"></a>

This project is released under the MIT License. See the
[LICENSE](./LICENSE) file for details.

---

## 🔗 15 Related Projects <a name="related-projects"></a>

Check out [xd-readline](https://github.com/xduraid/xd-readline), a custom 
command-line editor developed specifically for this project.

---
