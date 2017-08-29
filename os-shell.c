#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prompt.h"
#include "parser.h"
#include "builtins.h"

int main() {
	char *line;
	char **args;
	int flag;
	char *dup_line = calloc(MAX_LINE_LEN, sizeof(char));
	char *format_line;

	while (1) {
		flag = 0;

		// Get command
		print_prompt();
		line = line_read();
		strcpy(dup_line, line);

		// Parse command
		unsigned int len;
		args = string_tokenizer(line, SEP_LIST, ESC, &len);

		// Execute command
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

	return 0;
}
