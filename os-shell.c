#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

	while (1) {
		flag = 0;

		// Get command
		print_prompt();
		line = line_read();

		unsigned int len;
		unsigned int cmd_len;
		cmds = string_tokenizer(line, ";", ESC, &cmd_len);

		// Execute command
		for (int j = 0; j < cmd_len; j++) {
			strcpy(dup_line, cmds[j]);
			args = string_tokenizer(cmds[j], SEP_LIST, ESC, &len);

			int i = 0;
			if (strcmp(args[0], "echo") == 0) {
				format_line = echo_parser(dup_line);
				builtin_echo(format_line);
				continue;
			}

			for (i = 0; i < BUILTIN_LEN; i++) {
				if (strcmp(builtin_str[i], args[0]) == 0) {
					flag = 1;
					(builtin_call[i])(args, len);
				}
			}
			i = 0;
			if (flag == 0) {
				while (i < len) {
					printf("%s\n", args[i]);
					i++;
				}
			}
		}
	}

	return 0;
}
