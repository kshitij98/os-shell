#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
			if (limit == 0 && ch == '\n')
				buff[limit++] = '\n';
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
	int i = 0;
	int j = 0;
	int flag = 0;
	int shift = 0;
	char *sep = SEP_LIST;
	int sep_len = strlen(SEP_LIST);
	for (i = 0; line[i] != '\0'; i++) {
		flag = 0;
		for (j = 0; j < sep_len; j++) {
			if (line[i] == sep[j])
				flag = 1;
		}
		if (flag == 0)
			break;
	}
	i += 4;
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
	line[j - shift] = '\0';
	return line + i;
}


char *get_flags(char **arg, char argc, char *flags) {
	memset(flags, 0, 300);
	
	int i;
	for (i=0 ; i<argc ; ++i) {
		if (arg[i][0] == '-' && arg[i][1] && arg[i][1] != '-') {
			int curr = 1;
			while (arg[i][curr] != '\0')
				flags[arg[i][curr++]] = 1;
		}
	}
	return flags;
}

int setDescriptor(Str fileName, int dir) {
	unsigned int temp;
	char **ret = string_tokenizer(fileName, " ", '\\', &temp);
	fileName = ret[0];
	// fprintf(stderr, "setting descriptors for <%s> <%d>\n", fileName, dir);

	if (dir == -1)
		return 0;

	int file, fd = dir;
	if (dir == 0)
		file = open(fileName, O_RDONLY);
  else if (dir == 1)
	  file = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
	else if (dir == 2) {
	  file = open(fileName, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		fd = 1;
	}
	
	if (file < 0) {
		fprintf(stderr, OS_SHELL "%s: No such file or directory\n", fileName);
		return -1;		
	}

	if (dup2(file, fd) < 0) {
		close(file);
		fprintf(stderr, "file = %d, fd = %d,  Some Error occured\n", file, fd);
		return -1;
	}
	close(file);

	return 0;
}


// 0 <- input
// 1 <- output
// 2 <- append
int setFileDescriptors(Str cmd) {
	int i, len = strlen(cmd), flag = 0;

	int dir = -1, k=0;
	char curr[1000];
	for (i=0 ; i<len ; ++i) {
		curr[k] = cmd[i];
		if (cmd[i] == '<') {
			// Input file
			curr[k] = cmd[i] = '\0';
			if (setDescriptor(curr, dir) < 0)
				return -1;
			dir = 0;
			k = -1;
		}
		else if (cmd[i] == '>') {
			if (i < len-1 && cmd[i+1] == '>') {
				// Append
				curr[k] = cmd[i] = '\0';
				if (setDescriptor(curr, dir) < 0)
					return -1;
				dir = 2;
				++i;
			}
			else {
				//Truncate
				curr[k] = cmd[i] = '\0';
				if (setDescriptor(curr, dir) < 0)
					return -1;
				dir = 1;
			}
			k = -1;
		}
		++k;
	}
	curr[k] = '\0';
	if (setDescriptor(curr, dir) < 0)
		return -1;
	return 0;
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
