#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

#define MEM_ERR_MSG "Memory not allocated"
#define MEM_ERR -1
#define OS_SHELL "os-shell : "

char **string_tokenizer(char *line)
{
	int buff_size = TOKEN_BUFFSIZE;
	int limit = 0;
	char **tokens = malloc(buff_size * sizeof(char*));
	char *token;

	if (tokens == NULL) {
		perror( OS_SHELL MEM_ERR_MSG );
		exit(MEM_ERR);
	}

	token = strtok(line, TOKEN_DELIMITER);
	while (token != NULL) {
		tokens[limit] = token;
		limit++;
		if (limit >= TOKEN_BUFFSIZE) {
			buff_size += TOKEN_BUFFSIZE;
			tokens = realloc(tokens, buff_size * sizeof(char*));

			if (tokens == NULL) {
				perror( OS_SHELL MEM_ERR_MSG );
				exit(MEM_ERR);
			}
		}

		token = strtok(NULL, TOKEN_DELIMITER);
	}

	tokens[limit] = NULL;
	return tokens;
}


char *line_read()
{
	int line_size = LINE_BUFFSIZE;
	char *buff = malloc(line_size * sizeof(char));
	int limit = 0;
	char ch = 0;

	if (buff == NULL) {
		perror( OS_SHELL MEM_ERR_MSG );
		exit(MEM_ERR);
	}

	while (1) {
		ch = getchar();

		if (ch == EOF) {
			return buff;
		} else if (ch == '\n') {
			buff[limit] = '\0';
			return buff;
		} else {
			buff[limit] = ch;
		}

		++limit;

		if (limit >= line_size) {
			line_size += LINE_BUFFSIZE;
			buff = realloc(buff, line_size * sizeof(char));

			if (buff == NULL) {
				perror( OS_SHELL MEM_ERR_MSG );
				exit(MEM_ERR);
			}
		}
	}
}

#ifdef _LOCAL_TESTING

int main()
{
	char *line;
	//	scanf("%[^\n]", line);
	line = line_read();
	char **tk = string_tokenizer(line);
	int i = 0;
	while (tk[i] != NULL) {
		printf("%s\n", tk[i]);
		i++;
	}

	return 0;
}

#endif
