#include <stdio.h>
#include <string.h>
#include "prompt.h"
#include "parser.h"
#include "builtins.h"

int main() {
	char *line;
	char **args;
	int flag;

	while (1) {
		flag = 0;

		// Get command
		print_prompt();
		line = line_read();

		// Parse command
		unsigned int len;
		args = string_tokenizer(line, SEP_LIST, ESC, &len);

		// Execute command
		int i = 0;
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
