#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "parser.h"
#include "prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include "builtins.h"
#include "non-blocking-input.h"
#include "utilities.h"
#include "background.h"
#include "os-shell.h"


char *builtin_str[] = {
	"cd",
	"pinfo",
	"exit",
	"pwd",
	"ls",
	"nightswatch",
	"jobs",
	"kjob"
};

int builtin_echo(char *arg)
{
	printf("%s\n", arg);
	return 0;
}

int builtin_cd(char **arg, int argc)
{
	char *dest_dir;

	if (argc < 2 || arg[1] == NULL || strcmp(arg[1], "~") == 0) {
		dest_dir = getpwuid(getuid()) -> pw_dir;
		if (dest_dir == NULL) {
			perror("Could not get home directory.");
			return 1;
		}
	} else {
		dest_dir = getpwuid(getuid()) -> pw_dir;
		if (arg[1][0] == '~')
			strcat(dest_dir, arg[1] + 1);
		else
			dest_dir = arg[1];
	}

	if (chdir(dest_dir) != 0) {
		fprintf(stderr, "'cd' failed! %s : %s\n", strerror(errno), dest_dir);
		return 1;
	}

	return 0;
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

int builtin_ls__print(struct dirent *file, char *flags, char *path, int isFile) {
	struct stat mystat;
	char buf[512];
	char *fileName = isFile ? path : file -> d_name;
	if (fileName[0] != '.' || flags['a']) {
		if (!isFile) {
			sprintf(buf, "%s/%s", path, fileName);
			stat(buf, &mystat);
		}
		else {
			stat(fileName, &mystat);
		}
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
		if (S_ISDIR(mystat.st_mode)) printf("\033[1m\033[34m%s\033[0m/\n", fileName);
		else if (mystat.st_mode & S_IXUSR) printf("\033[1m\033[32m%s\033[0m*\n", fileName);
		else printf("%s\n", fileName);
	}
}

int builtin_ls(char **arg, int argc)
{
	char flags[300];
	get_flags(arg, argc, flags);

	const int size = 1024;
	char *cwd = calloc(size, sizeof(char));
	char *initwd = calloc(size, sizeof(char));

	if (cwd == NULL || initwd == NULL) {
		fprintf(stderr, "Error! %s\n", strerror(errno));
		return -1;
	}
	getcwd(cwd, size);
	getcwd(initwd, size);

	struct dirent *myfile;
	DIR *mydir;
	int i, tried = 0;
	struct stat buf;
	for (i=0 ; i<argc ; ++i) {
		chdir(initwd);
		strcpy(cwd, getpwuid(getuid()) -> pw_dir);
		if (arg[i][0] == '~')
			strcat(cwd, arg[i] + 1);
		else
			strcpy(cwd, arg[i]);

		if (cwd[0] != '/') {
			char temp[1000];
			strcpy(temp, initwd);
			strcat(temp, "/");
			strcat(temp, cwd);
			strcpy(cwd, temp);
			// fprintf(stderr, "<%s>\n", cwd);
		}

		int invalid = chdir(cwd);
		// if (invalid)
			// fprintf(stderr, "<%s>", cwd);
		if (i > 0 && arg[i][0] != '-')
			tried = 1;
		if (!invalid || (!tried && i == argc-1)) {
			if (argc > 1 && !invalid)
				printf("%s:\n", arg[i]);
			tried = 1;
			getcwd(cwd, size);
			mydir = opendir(cwd);
			while(mydir && (myfile = readdir(mydir)) != NULL)
				builtin_ls__print(myfile, flags, cwd, 0);

			printf("\n");
		}
		else if (stat(cwd, &buf) >= 0) {
			if (!S_ISDIR(buf.st_mode)) {
				builtin_ls__print(NULL, flags, cwd, 1);
				printf("\n");
			}
		}
		else if (i > 0 && arg[i][0] != '-')
			fprintf(stderr, "ls: cannot access '%s': No such file or directory\n\n", arg[i]);
		chdir(initwd);
	}
	free(cwd);
	return 0;
}

int builtin_nightswatch(char **arg, int argc) {
  char usage[100] = "Usage:\n\r watch [options] <command>\n\r";
  if (argc != 4) {
		fprintf(stderr, "%s", usage);
  	return 1;
  }

  set_conio_terminal_mode();
  time_t start = time(NULL);
	int curr = -1, n = 0;
	char intFileName[100] = "/proc/interrupts";
	char memFileName[100] = "/proc/meminfo";
	char line[1000], ch;
	int dirty=0, interrupt=0;
	FILE *file;
	if (strcmp("interrupt", arg[3]) == 0) interrupt = 1;
	else if (strcmp("dirty", arg[3]) == 0) dirty = 1;

	if (strcmp(arg[1], "-n")) {
		fprintf(stderr, "%s", usage);
		return 1;
	}

	int i=0;
	while (arg[2][i] != '\0') {
		if ('0' <= arg[2][i] && arg[2][i] <= '9')
			n = (n*10) + arg[2][i++] - '0';
		else {
			fprintf(stderr, "%s", usage);
			reset_terminal_mode();
			return 1;
		}
	}

	if (interrupt || dirty) {
		while (1) {
			if (isKeyDown() == 'q')
				break;

			if ((time(NULL) - start) / n > curr) {
				++curr;

				if (dirty) {
					file = fopen(memFileName, "r");

					for (i=0 ; i<17 ; ++i)
						fscanf(file, "%[^\n]%c", line, &ch);
					printf("%s\n\r", line);
				}
				else {
					file = fopen(intFileName, "r");

					fscanf(file, "%[^\n]%c", line, &ch);
					printf("%s\n\r", line);
					fscanf(file, "%[^\n]%c", line, &ch);
					fscanf(file, "%[^\n]%c", line, &ch);
					printf("%s\n\r", line);
				}
				fclose(file);
			}
		}
	}
	else {
		fprintf(stderr, "%s", usage);
		reset_terminal_mode();
		return 1;
	}
	reset_terminal_mode();
	return 0;
}

int builtin_jobs(char **arg, int argc)
{
	print_children(&children);
	return 0;
}

int builtin_kjob(char **arg, int argc)
{
	if (argc < 2) {
		fprintf(stderr, "kjobs <job number> <signal number>\n");
		return -1;
	}
	pid_t proc_id = atoint(arg[1]);
	int sig = atoint(arg[2]);
	int ret = kill(proc_id, sig);

	if (ret)
		fprintf(stderr, "os-shell: %s\n", strerror(errno));

	return ret;
}


int (*builtin_call[]) (char**, int) = {
	&builtin_cd,
	&builtin_pinfo,
	&builtin_exit,
	&builtin_pwd,
	&builtin_ls,
	&builtin_nightswatch,
	&builtin_jobs,
	&builtin_kjob
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
