#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "prompt.h"
#include "parser.h"
#include "builtins.h"
#include "background.h"

int main() {
	char *line;
	char **args;
	char **cmds;
	int flag;
	char *dup_line = calloc(MAX_LINE_LEN, sizeof(char));
	char *format_line;
	int pid;
	int status;

	while (1) {

		// Get command
		print_prompt();

		line = line_read();

		if (line[0] == '\n')
			continue;

		if (line[0] == '\0') {
			(builtin_call[2])(args, 0);
		}

		unsigned int len;
		unsigned int cmd_len;
		cmds = string_tokenizer(line, ";", ESC, &cmd_len);

		// Execute command
		for (int j = 0; j < cmd_len; j++) {
			flag = 0;
			strcpy(dup_line, cmds[j]);
			args = string_tokenizer(cmds[j], SEP_LIST, ESC, &len);

			int i = 0;
			if (strcmp(args[0], "echo") == 0) {
				format_line = echo_parser(dup_line);
				builtin_echo(format_line);
				flag = 1;
				continue;
			}

			for (i = 0; i < BUILTIN_LEN; i++) {
				if (strcmp(builtin_str[i], args[0]) == 0) {
					flag = 1;
					(builtin_call[i])(args, len);
				}
			}

			if (flag == 0) {
				// forking the process
				pid = fork();

				if (pid == 0) {
					i = 0;
					execvp(args[0], args);
					fprintf(stderr, "os-shell: command %s not found!\n", args[0]);
					exit(1);
				} else if (pid < 0) {
					// Error in fork()
					perror("os-shell");
				} else {
					wait(NULL);
				}
			}
		}
	}

	return 0;
}
