#include <stdio.h>
#include "prompt.h"
#include "parser.h"
#include "builtins.h"

int main() {
	char *line;
	char **args;
	while (1) {
		// Get command
		print_prompt();
		line = line_read();

		// Parse command
		unsigned int len;
		args = string_tokenizer(line, SEP_LIST, ESC, &len);

		// Execute command
		int i = 0;
		while (i < len) {
			printf("%s\n", args[i]);
			i++;
		}
	}

	return 0;
}
