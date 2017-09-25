# OS-SHELL :star2:

## INTRODUCTION

Simple shell implemented in C.

## AUTHORS

* Kshitij Gupta (20161234)
* Pratik Jain (20161050)

## FEATURES

* Builtins:
	1. cd
	2. pinfo
	3. exit
	4. pwd
	5. ls
	6. nightswatch
	7. jobs
	8. kjob
	9. fg
	10. bg
	11. overkill
	12. setenv
	13. unsetenv

* Escape sequence support in file names
* Builtin basic version of `ls` with `-l` and `-a` options
* Basic color support
* Prompt with username and host (colored)
* Background job processing and exit status display
* Multiple commands in same line using ';' separator
* Run installed commands
* Support for well known characters like - `~` for HOME directory
* Ctrl-D signal for EOF and exiting the shell is also supported
* Multiple I/O redirections
* Multi-pipelining support

## FILE DESCRIPTIONS

### os-shell.c

This is the main file from where builtins as well as installed commands are executed. This also
creates background jobs and reports them if they are ended. It maintains a dynamically allocated list of
background jobs with pids which can be retrieved later if required. This list is used in
implementing `fg` builtin command.

### background.c

It contains the logic of inserting the background job into the linked list and its removal. It
sores the PID and Name of the process.

### builtins.c

Builtin commands like `echo`, `pinfo`, etc. are coded here. The commands parsed by functions of
`parser.c` called from `os-shell.c` are passed to functions in `builtins.c` where the respective
function is executed.

A function pointer is used to avoid dirty `if-else-if` statement for identifying the builtin to be
executed.

### parser.c

Reading and parsing of entered commands are handled through the functions of these files.

* `string_tokenizer` : breaks the string into tokens, recognising any escape sequences present.
* `line_read` : reads the input line returns a pointer to the read line.
* `echo_parser` : since echo needs to handle things differently than other commands we made a
	separate parser for it.
* `get_flags` : this function can be used to retrieve passed flags to the commands.
* `setDescriptor` : this function replaces the I/O file descriptors with the provided descriptors.
* `setFileDescriptors` : this function sets the file descriptors for a command with multiple I/O redirections and returns the command as a string.

### prompt.c

Displaying of prompt is managed through this file.

* `print_prompt` : Displays the prompt

### non-blocking-input.c

Utilities for supporting non blocking input used in implementing `nightswatch`.

### utilities.c

Displaying of prompt is managed through this file.

* `atoint` : Converts a number of string format to integer.
* `itoa` : Converts an integer to a string representing that number.
* `print_prompt` : Displays the prompt

## HOW TO RUN

```
./run
./os-shell
```
