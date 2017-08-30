#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include "builtins.h"


char *builtin_str[] = {
	"cd",
	"pinfo",
	"exit",
	"pwd",
	"ls"
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


void itoa(long long num, char **snum)
{
	int i = 0;
	int j = 0;
	int temp = 0;
	snum[0][i] = '\0';
	while (num > 0) {
		snum[0][i] = num % 10 + '0';
		num /= 10;
		i++;
	}

	while (j < i / 2) {
		temp = snum[0][j];
		snum[0][j] = snum[0][i - j - 1];
		snum[0][i - j - 1] = temp;
		j++;
	}
	snum[0][i] = '\0';
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
	char path_stat[100] = "/proc/";
	char path_exe[100] = "/proc/";
	char dest[100] = {0};

	if (arg == NULL)
		return -1;

	if (argc < 2 || arg[1] == NULL) {
		proc_id = getpid();
		arg[1] = malloc(sizeof(char) * 15);
		itoa(proc_id, &arg[1]);
	}
	strcat(path_stat, arg[1]);
	strcat(path_exe, arg[1]);
	strcat(path_stat, "/stat");
	strcat(path_exe, "/exe");
	FILE *fps = fopen(path_stat, "r");
	if (fps == NULL) {
		fprintf(stderr, "process with pid: %s not identified!\n", arg[1]);
		return -1;
	}

	if (readlink(path_exe, dest, 100) == -1) {
		perror("readlink");
		return -1;
	}
	char str[M][N];

	for (i = 0; i < M; i++) {
		fscanf(fps, "%s", str[i]);
	}
	printf("%s\t%s\t%s\t%s\t%s\n", str[0], str[1], str[2], str[22], dest);
	fclose(fps);
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

int builtin_ls(char **arg, int argc)
{
	char flags[300];
	get_flags(arg, argc, flags);

	const int size = 1024;
	char *cwd = calloc(size, sizeof(char));

	if (cwd == NULL) {
		fprintf(stderr, "Error! %s\n", strerror(errno));
		return -1;
	}
	getcwd(cwd, size);

	DIR *mydir;
	struct dirent *myfile;
	struct stat mystat;
	char buf[512];
	mydir = opendir(cwd);
	while((myfile = readdir(mydir)) != NULL) {
		if (myfile -> d_name[0] != '.' || flags['a']) {
			sprintf(buf, "%s/%s", cwd, myfile->d_name);
			stat(buf, &mystat);
			if (flags['l']) {
				printf( (S_ISDIR(mystat.st_mode)) ? "d" : "-");
				printf( (mystat.st_mode & S_IRUSR) ? "r" : "-");
				printf( (mystat.st_mode & S_IWUSR) ? "w" : "-");
				printf( (mystat.st_mode & S_IXUSR) ? "x" : "-");
				printf( (mystat.st_mode & S_IRGRP) ? "r" : "-");
				printf( (mystat.st_mode & S_IWGRP) ? "w" : "-");
				printf( (mystat.st_mode & S_IXGRP) ? "x" : "-");
				printf( (mystat.st_mode & S_IROTH) ? "r" : "-");
				printf( (mystat.st_mode & S_IWOTH) ? "w" : "-");
				printf( (mystat.st_mode & S_IXOTH) ? "x" : "-");

				printf("\t%s\t%s", getpwuid(mystat.st_uid)->pw_name,
					   getgrgid(mystat.st_gid)->gr_name);
			    printf("\t%zu\t",mystat.st_size);
		    }
			if (S_ISDIR(mystat.st_mode)) printf("\033[1m\033[34m%s\033[0m/\n", myfile->d_name);
			else if (mystat.st_mode & S_IXUSR) printf("\033[1m\033[32m%s\033[0m*\n", myfile->d_name);
		    else printf("%s\n", myfile->d_name);
		}
	}
	closedir(mydir);
	free(cwd);
	return 0;
}

int (*builtin_call[]) (char**, int) = {
	&builtin_cd,
	&builtin_pinfo,
	&builtin_exit,
	&builtin_pwd,
	&builtin_ls
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
