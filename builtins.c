#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "builtins.h"


char *builtin_str[] = {
	"cd",
	"pinfo",
	"exit",
	"pwd"
};

int builtin_echo(char *arg)
{
	printf("%s\n", arg);
	return 0;
}

int builtin_cd(char **arg, int argc)
{
	const char *dest_dir;

	if (argc < 2 || arg[1] == NULL || strcmp(arg[1], "~") == 0) {
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

int builtin_exit(char **arg, int argc)
{
	fprintf(stderr, "Exit os-shell!\n");
	_exit(0);
	exit(0);
}

int builtin_pinfo(char **arg, int argc)
{
	pid_t proc_id;
	int i;

	if (arg == NULL)
		return -1;

	if (argc < 2 || arg[1] == NULL) {
		proc_id = getpid();
		arg[1] = malloc(sizeof(char) * 15);
		itoa(proc_id, arg[1]);
	}

	char path[100] = "/proc/";
	strcat(path, arg[1]);
	strcat(path, "/stat");
	FILE *fp = fopen(path, "r");
	if (fp == NULL) {
		fprintf(stderr, "process with pid: %s not identified!\n", arg[1]);
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

int builtin_pwd(char **arg, int argc)
{
	const int size = 1024;
	char *cwd = calloc(size, sizeof(char));

	if (cwd == NULL) {
		fprintf(stderr, "Error! %s\n", strerror(errno));
		return -1;
	}
	getcwd(cwd, size);
	printf("%s\n", cwd);
	free(cwd);
	return 0;
}

int (*builtin_call[]) (char**, int) = {
	&builtin_cd,
	&builtin_pinfo,
	&builtin_exit,
	&builtin_pwd
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
				(builtin_call[i])(st, len);
			}
		}
	}
	return 0;
}

#endif
