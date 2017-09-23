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

// os_proc_gid = getgid();
// os_proc_id = getpid();
char *line;
char **args;
char **cmds;
int flag;
char *dup_line;
char *format_line;
int pid;
int status;
char *process_name;
int exec_back = 0;


void child_handler(int sig)
{
	pid_t proc_id;
	pid_t proc_gid;
	int st;
	int ret_stat = 0;
	int flag = 0;
	proc_id = wait(&ret_stat);
	child_process *curr = children;
	while (curr != NULL) {
		if (curr->pid == proc_id) {
			flag = 1;
			break;
		}
		curr = curr->next;
	}

	if (flag == 0)
		return;

	fprintf(stderr, "\nProcess with ID: %d\tNAME: %s has exited with code: %d\n", proc_id, curr->name, ret_stat);
	print_prompt();
	return;
}

void interrupt_handler(int sig)
{
	return;
}

int execute_command(Str cmd) {
	// fprintf(stderr, "Executing <%s>\n", cmd);
	
	int old0 = dup(0);
	int old1 = dup(1);
	unsigned int len;
	if (setFileDescriptors(cmd) < 0)
		return -1;
	flag = 0;
	exec_back = 0;
	strcpy(dup_line, cmd);
	args = string_tokenizer(cmd, SEP_LIST, ESC, &len);
	if (strcmp(args[len - 1], "&") == 0) {
		exec_back = 1;
		args[len - 1] = NULL;
		--len;
	}
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
		format_line = echo_parser(dup_line);
		builtin_echo(format_line);
		flag = 1;
		return 0;
	}
	for (i = 0; i < BUILTIN_LEN; i++) {
		if (strcmp(builtin_str[i], args[0]) == 0) {
			flag = 1;
			(builtin_call[i])(args, len);
		}
	}
	if (flag == 0) {
		// forking the process
		pid = fork();
		if (pid == 0) {
			i = 0;
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
				wait(NULL);
			else
				child_insert(&children, pid, args[0]);
		}
	}
	dup2(old0, 0);
	dup2(old1, 1);
}

int main(int argc, char *argv[])
{
	os_proc_gid = getgid();
	os_proc_id = getpid();
	children = NULL;
	dup_line = calloc(MAX_LINE_LEN, sizeof(char));
	process_name = SHELL_NAME"\0";

	memcpy((void *)argv[0], process_name, sizeof(process_name));
	prctl(PR_SET_NAME, SHELL_NAME);
	signal(SIGINT, interrupt_handler);
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &sa, NULL);

	while (1) {
		print_prompt();
		signal(SIGCHLD, child_handler);
		// Get command

		line = line_read();

		if (line[0] == '\n')
			continue;

		if (line[0] == '\0') {
			(builtin_call[2])(args, 0);
		}

		unsigned int cmd_len;
		cmds = string_tokenizer(line, ";", ESC, &cmd_len);

		// Execute command
		for (int i=0 ; i<cmd_len ; ++i) {
			int old0 = dup(0);
			int old1 = dup(1);

			unsigned int pipe_len;
			Str *piped_cmds = string_tokenizer(cmds[i], "|", '\\', &pipe_len);
			int pipe_counter = pipe_len - 1;

			// for (int j=0 ; j<pipe_len<<1 ; ++j)
				// fds[j] = 0;

			for (int j=0 ; j<pipe_len ; ++j)
				pipe(fds + (j << 1));

			int k = 0;
			for (int j=0 ; j<pipe_len ; ++j) {
	      if((pid = fork()) < 0) exit(1);
	      else if (pid == 0) {
	        if(j < pipe_counter && dup2(fds[k + 1], 1) < 0)
	        	exit(1);
	        if(k != 0 && dup2(fds[k-2], 0) < 0)
	        	exit(1);
	        for(int c=0 ; c<2*pipe_counter ; ++c)
	        	close(fds[c]);
	        execute_command(piped_cmds[j]);
	        exit(1);
	      }
	      k+=2;
			}
	    for(int j=0 ; j<2*pipe_counter ; ++j)
	    	close(fds[j]);
	    for(int j=0 ; j<pipe_len ; ++j)
	    	wait(&status);

			dup2(old0, 0);
			dup2(old1, 1);
		}
	}

	return 0;
}
