#include <stdio.h>
#include "prompt.h"
#include "parser.h"

int main() {
	char *line;
	char **args;
	while (1) {
		print_prompt();
		line = line_read();
		args = string_tokenizer(line);
		
		int i = 0;
		while (args[i] != NULL) {
			printf("%s\n", args[i]);
			i++;
		}
	}

	return 0;
}