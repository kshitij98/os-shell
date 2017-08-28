#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUILTIN(str) builtin_##str
#define N 75
#define M 52
#define BUILTIN_LEN 3

char *builtin_str[] = {
	"cd",
	"pinfo",
	"exit"
};

int builtin_cd(char **arg)
{
	const char *dest_dir;

	if (arg[1] == NULL) {
		dest_dir = getpwuid(getuid()) -> pw_dir;
		if (dest_dir == NULL) {
			perror("Could not get home directory.");
			return 1;
		}
	} else {
		dest_dir = arg[1];
	}

	if (chdir(dest_dir) != 0) {
		fprintf(stderr, "'cd' failed! %s : %s\n", strerror(errno), dest_dir);
		return 1;
	}

	return 0;
}


void itoa(long long num, char *snum)
{
	int i = 0;
	snum[i] = '\0';
	while (num > 0) {
		snum[i] = num % 10;
		num /= 10;
	}
	snum[i] = '\0';
	return;
}

int builtin_exit(char **arg)
{
	fprintf(stderr, "Exit os-shell!");
	exit(0);
}

int builtin_pinfo(char **arg)
{
	pid_t proc_id;
	int i;
	fprintf(stderr, "checkpoint 1\n");
	if (arg == NULL)
		return -1;
	fprintf(stderr, "checkpoint 2\n");
	if (arg[1] == NULL)
		proc_id = getpid();
	fprintf(stderr, "checkpoint 3\n");
	//char *sproc_id = malloc(sizeof(char) * 64);
	//itoa(proc_id, sproc_id);
	char path[100] = "/proc/";
	strcat(path, arg[1]);
	strcat(path, "/stat");
	FILE *fp = fopen(path, "r");
	fprintf(stderr, "checkpoint 4\n");
	if (fp == NULL) {
		perror("process not identified!");
		return -1;
	}

	char str[M][N];

	for (i = 0; i < M; i++) {
		fscanf(fp, "%s", str[i]);
	}
	printf("%s\t%s\t%s\t%s\n", str[0], str[1], str[2], str[22]);
	fclose(fp);
	return 0;
}


int (*builtin_call[]) (char**) = {
	&builtin_cd,
	&builtin_pinfo,
	&builtin_exit
};


#ifdef _LOCAL_TESTING

int main()
{
	char *str;
	unsigned int len;
	int i;
	while (1) {
		printf(">");
		str = line_read();
		char **st = string_tokenizer(str, SEP_LIST, ESC, &len);
		st[1] = esc_stripper(st[1], ESC);
		for (i = 0; i < BUILTIN_LEN; i++) {
			if (strcmp(st[0], builtin_str[i]) == 0) {
				(builtin_call[i])(st);
			}
		}
	}
	return 0;
}

#endif
