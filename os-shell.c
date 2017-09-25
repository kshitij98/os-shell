#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "prompt.h"
#include "parser.h"
#include "builtins.h"
#include "background.h"
#include "utilities.h"

#define SHELL_NAME "os-shell"
#define MAX_COMMANDS 1000

pid_t os_proc_gid;
pid_t os_proc_id;
child_process *children;
int fds[MAX_COMMANDS << 1];


char *line;
char **args;
char **cmds;
int flag;
char *dup_line;
char *dup_line_cmd;
char *format_line;
int pid;
int status;
char *process_name;
int exec_back = 0;


void init()
{
	MAP_STATE['R'] = 0;
	MAP_STATE['S'] = 1;
	MAP_STATE['D'] = 2;
	MAP_STATE['Z'] = 3;
	MAP_STATE['T'] = 4;
	MAP_STATE['t'] = 5;
	MAP_STATE['W'] = 6;
	MAP_STATE['X'] = 7;
	MAP_STATE['x'] = 8;
	MAP_STATE['K'] = 9;
	MAP_STATE['W'] = 10;
	MAP_STATE['P'] = 11;
	return;
}


void child_handler(int sig)
{
	pid_t proc_id;
	pid_t proc_gid;
	int st;
	int ret_stat = 0;
	int flag = 0;
	child_process *curr = children;
	fprintf(stderr, "XXX\n");
	while (curr != NULL) {
		fprintf(stderr, "YYY\n");
		proc_id = waitpid(curr->pid, &ret_stat, WNOHANG);
		if (proc_id > 0) {
			fprintf(stderr, "\nProcess with ID: %d\tNAME: %s has exited with code: %d\n", proc_id, curr->name, ret_stat);
			child_remove(&children, curr);
			print_prompt();
		}
		curr = curr->next;
	}
	return;
}
void interrupt_handler(int sig)
{
	fprintf(stderr, "Ctrl+C Handler..\n");
	return;
}

static void zparent_handler(int sig)
{
	fprintf(stderr, "Parent Handler..\n");
	pid_t ppid = tcgetpgrp(0);
	//	child_insert(&children, ppid, "Process");
	//	signal(SIGTTOU, SIG_IGN);
	//	signal(SIGTTIN, SIG_IGN);
	//	tcsetpgrp(0, ppid);
	fprintf(stderr, "1\n");
	//	tcsetpgrp(1, ppid);
	fprintf(stderr, "2\n");
	//	tcsetpgrp(2, ppid);
	//	kill(ppid, SIGTSTP);
	fprintf(stderr, "3\n");
	//	child_insert(&children, ppid, "Process");
	return;
}


static void zchild_handler(int sig)
{
	child_insert(&children, getpid(), "Process");
	fprintf(stderr, "Child Handler Z\n");
	raise(SIGTSTP);
	return;
}

int execute_builtins(Str cmd) {
	int old0 = dup(0);
	int old1 = dup(1);
	unsigned int len;
	if (setFileDescriptors(cmd) < 0)
		return 0;
	int flag = 0;

	strcpy(dup_line, cmd);
	args = string_tokenizer(cmd, SEP_LIST, ESC, &len);

	if (strcmp(args[0], "setenv") != 0 && strcmp(args[0], "unsetenv") != 0) {
		for (int t = 0; t < len; t++) {
			if (args[t][0] == '$') {
				dup_line = replace_str(dup_line, args[t], getenv(args[t] + 1));
				args[t] = getenv(args[t] + 1);
			}
		}
	}
	int i = 0;
	if (strcmp(args[0], "echo") == 0) {
		flag = 1;
		format_line = echo_parser(dup_line);
		builtin_echo(format_line);
		return 1;
	}
	for (i = 0; i < BUILTIN_LEN; i++) {
		if (strcmp(builtin_str[i], args[0]) == 0) {
			flag = 1;
			(builtin_call[i])(args, len);
		}
	}

	dup2(old0, 0);
	dup2(old1, 1);

	return flag;
}

void register_handlers()
{
	signal(SIGINT, interrupt_handler);
	struct sigaction sa;
	sa.sa_handler = zparent_handler;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &sa, NULL);
	signal(SIGCHLD, child_handler);
}

void unregister_handlers()
{
	signal(SIGINT, SIG_DFL);
	struct sigaction sa;
	sa.sa_handler = zchild_handler;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &sa, NULL);
	//signal(SIGTSTP, zchild_handler);
	signal(SIGCHLD, SIG_DFL);
	return;
}

int execute_command(Str cmd) {
	int i;
	int old0 = dup(0);
	int old1 = dup(1);
	unsigned int len;

	if (setFileDescriptors(cmd) < 0)
		return -1;
	args = string_tokenizer(cmd, SEP_LIST, ESC, &len);
	if (strcmp(args[len - 1], "&") == 0) {
		exec_back = 1;
		args[len - 1] = NULL;
		--len;
	}

	pid = fork();
	if (pid == 0) {
		i = 0;
		unregister_handlers();
		if (exec_back == 1)
			setpgid(0, 0);
		execvp(args[0], args);
		fprintf(stderr, "os-shell: command %s not found!\n", args[0]);
		exit(1);
	} else if (pid < 0) {
		// Error in fork()
		perror("os-shell");
	} else {
		if (exec_back == 0)
			waitpid(-1, NULL, WUNTRACED);
		else
			child_insert(&children, pid, args[0]);
	}

	dup2(old0, 0);
	dup2(old1, 1);
}

int main(int argc, char *argv[])
{
	init();
	os_proc_gid = getgid();
	os_proc_id = getpid();
	children = NULL;
	dup_line = calloc(MAX_LINE_LEN, sizeof(char));
	dup_line_cmd = calloc(MAX_LINE_LEN, sizeof(char));
	process_name = SHELL_NAME"\0";

	memcpy((void *)argv[0], process_name, sizeof(process_name));
	prctl(PR_SET_NAME, SHELL_NAME);
	register_handlers();
	while (1) {
		print_prompt();
		// Get command

		line = line_read();

		if (line[0] == '\n')
			continue;
		if (line[0] == '\0') {
			printf("\n");
			continue;
		}
		unsigned int cmd_len;
		cmds = string_tokenizer(line, ";", ESC, &cmd_len);

		// Execute command
		for (int i=0 ; i<cmd_len ; ++i) {
			int old0 = dup(0);
			int old1 = dup(1);
			exec_back = 0;
			unsigned int pipe_len;
			Str *piped_cmds = string_tokenizer(cmds[i], "|", '\\', &pipe_len);
			int pipe_counter = pipe_len - 1;
			strcpy(dup_line_cmd, piped_cmds[0]);
			int k = 0;
			if (pipe_len > 1) {
				for (int j=0 ; j<pipe_len ; ++j)
					pipe(fds + (j << 1));
				for (int j=0 ; j<pipe_len ; ++j) {
					if((pid = fork()) < 0) exit(1);
					else if (pid == 0) {
						unregister_handlers();
						if(j < pipe_counter && dup2(fds[k + 1], 1) < 0)
							exit(1);
						if(k != 0 && dup2(fds[k-2], 0) < 0)
							exit(1);
						for(int c=0 ; c<2*pipe_counter ; ++c)
							close(fds[c]);
						if (execute_builtins(piped_cmds[j]) == 1);
						else execute_command(piped_cmds[j]);
						exit(1);
					}
					k+=2;
				}
				for(int j=0 ; j<2*pipe_counter ; ++j)
					close(fds[j]);
				for(int j=0 ; j<pipe_len ; ++j)
					wait(&status);
			} else {
				if (execute_builtins(piped_cmds[0]) == 1);
				else execute_command(dup_line_cmd);
			}

			dup2(old0, 0);
			dup2(old1, 1);
		}
	}

	return 0;
}
