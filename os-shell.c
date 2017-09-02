#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include "prompt.h"
#include "parser.h"
#include "builtins.h"
#include "background.h"

#define SHELL_NAME "os-shell"

pid_t os_proc_gid;
pid_t os_proc_id;
child_process *children;


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

int main(int argc, char *argv[])
{
	os_proc_gid = getgid();
	os_proc_id = getpid();
	char *line;
	char **args;
	char **cmds;
	int flag;
	char *dup_line = calloc(MAX_LINE_LEN, sizeof(char));
	char *format_line;
	int pid;
	int status;
	char *process_name = SHELL_NAME"\0";
	int exec_back = 0;
	children = NULL;

	memcpy((void *)argv[0], process_name, sizeof(process_name));
	prctl(PR_SET_NAME, SHELL_NAME);
	signal(SIGINT, interrupt_handler);
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

		unsigned int len;
		unsigned int cmd_len;
		cmds = string_tokenizer(line, ";", ESC, &cmd_len);

		// Execute command
		for (int j = 0; j < cmd_len; j++) {
			flag = 0;
			exec_back = 0;
			strcpy(dup_line, cmds[j]);
			args = string_tokenizer(cmds[j], SEP_LIST, ESC, &len);

			if (strcmp(args[len - 1], "&") == 0) {
				exec_back = 1;
				args[len - 1] = NULL;
				--len;
			}

			int i = 0;
			if (strcmp(args[0], "echo") == 0) {
				format_line = echo_parser(dup_line);
				builtin_echo(format_line);
				flag = 1;
				continue;
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
		}
	}

	return 0;
}
