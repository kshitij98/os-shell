#include <stdio.h>
#include "prompt.h"
#include "parser.h"

int main() {
	char *line;
	char **args;
	while (1) {
		// Get command
		print_prompt();
		line = line_read();

		// Parse command
		args = string_tokenizer(line);
		
		// Execute command
		int i = 0;
		while (args[i] != NULL) {
			printf("%s\n", args[i]);
			i++;
		}
	}

	return 0;
}