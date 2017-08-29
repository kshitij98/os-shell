#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "parser.h"

#define MEM_ERR_MSG "Memory not allocated"
#define MEM_ERR -1
#define OS_SHELL "os-shell : "



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

		if (ch == '\n' || ch == EOF) {
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


unsigned int number_of_elements(const char *s, char *sep, char esc)
{
    unsigned int q;
	unsigned int e;
    const char *p;
	int sep_len = strlen(sep);
	int flag = 0;
	int j = 0;

    for (e = 0, q = 1, p = s; *p; ++p) {
		flag = 0;
		for (j = 0; j < sep_len; j++) {
			if (sep[j] == *p)
				flag = 1;
		}

        if (*p == esc)
            e = !e;
        else if (flag)
            q += !e;
        else
			e = 0;
    }

    return q;
}

Str *string_tokenizer(char *s, char *sep, char esc, unsigned int *q)
{
    Str *list = NULL;
	int sep_len = strlen(sep);
	int j = 0;
	int flag = 0;
	int shift = 0;

    *q = number_of_elements(s, sep, esc);
    list = malloc((*q + 1) * sizeof(Str));

    if(list != NULL) {
        unsigned int e;
		unsigned int i = 0;
        char *p;
        list[i++] = s;

        for (e = 0, p = s; *p; ++p) {
			flag = 0;
			for (j = 0; j < sep_len; j++) {
				if (sep[j] == *p)
					flag = 1;
			}

            if (*p == esc) {
                e = !e;
            } else if (flag && !e) {
                list[i++] = p + 1;
                *p = '\0';
            } else {
                e = 0;
            }
        }
    }
	list[*q] = NULL;
	j = 0;
	while (list[j] != NULL) {
		if (strlen(list[j]) == 0)
			++shift;
		else
			list[j - shift] = list[j];
		++j;
	}
	*q = j - shift;
	list[j - shift] = NULL;

    return list;
}

Str esc_stripper(Str word, char esc)
{
	int i = 0;
	int shift = 0;

	if (word == NULL)
		return NULL;

	while (word[i] != '\0') {
		if (word[i] == esc && word[i + 1] != '\0' &&
			!isalpha(word[i+1])) {
			++shift;
		} else {
			word[i - shift] = word[i];
		}
		++i;
	}
	word[i - shift] = '\0';

	return word;
}

char *echo_parser(char *line)
{
	int i = 4;
	int j = 0;
	int flag = 0;
	int shift = 0;
	char *sep = SEP_LIST;
	int sep_len = strlen(SEP_LIST);
	while (line[i] != '\0') {
		flag = 0;
		for (j = 0; j < sep_len; j++) {
			if (line[i] == sep[j])
				flag = 1;
		}
		if (flag == 0)
			break;
		++i;
	}
	j = i;
	while (line[j] != '\0') {
		if (line[j] == ESC) {
			++shift;
		} else {
			line[j - shift] = line[j];
		}
		++j;
	}
	line[j] = '\0';
	return line + i;
}

#ifdef _LOCAL_TESTING

int main()
{
	char *line;

	line = line_read();
	unsigned int len = 0;
	char **tk = string_tokenizer(line, SEP_LIST, ESC, &len);
	int i = 0;
	while (i < len) {
		printf("%s-\n", esc_stripper(tk[i], ESC));
		i++;
	}

	return 0;
}

#endif
