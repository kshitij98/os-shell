#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "background.h"
#include "utilities.h"
#include <sys/types.h>
#include <signal.h>



char* STATE[] = {
	"Running",
	"Sleeping",
	"Waiting",
	"Zombie",
	"Stopped(signal)",
	"Tracing stop",
	"Paging",
	"Dead",
	"Dead",
	"Wakekill",
	"Waking",
	"Parked"
};

int MAP_STATE[256];



int child_insert(child_process **list, pid_t proc_id, char *name)
{
	child_process *temp = malloc(sizeof(child_process));

	if (temp == NULL) {
		fprintf(stderr, "Could not create background process. %s\n", strerror(errno));
		return -1;
	}

	temp->prev = NULL;
	temp->pid = proc_id;
	strcpy(temp->name, name);
	temp->next = *list;
	if (*list != NULL)
		(*list)->prev = temp;
	*list = temp;
	return 0;
}

int child_remove(child_process **list, child_process *ptr)
{
	if (ptr == NULL)
		return 0;
	if (ptr == *list) {
		*list = (*list)->next;
		if ((*list) != NULL)
			(*list)->prev = NULL;
	} else {
		ptr->prev->next = ptr->next;
		if (ptr->next != NULL) {
			ptr->next->prev = ptr->prev;
		}
	}
	free(ptr);
	return 0;
}

char* getStatus(char *str)
{
	return STATE[MAP_STATE[str[0]]];
}

void print_children(child_process **list)
{
	child_process *itr = *list;
	int count = 0;
	int proc_id;
	char *pid = calloc(10, sizeof(char));
	char status[10];
	while (itr != NULL && itr->next != NULL) {
		itr = itr->next;
	}
	while (itr != NULL) {
		proc_id = itr->pid;
		itoa(proc_id, &pid);
		char path[100] = "/proc/";
		strcat(path, pid);
		strcat(path, "/stat");
		FILE *filp = fopen(path, "r");
		if (filp == NULL) {
			child_process* temp = itr->prev;
			child_remove(list, itr);
			itr = temp;
			continue;
		}
		fscanf(filp, "%*s %*s %s", status);
		printf("[%d]\t[%s]\t%s [%d]\n", ++count, getStatus(status),itr->name, itr->pid);
		fclose(filp);
		itr = itr->prev;
	}

	free(pid);
	return;
}


child_process* search(pid_t proc_id, child_process *list)
{
	child_process *itr = list;
	while (itr != NULL) {
		if (itr->pid == proc_id)
			break;
	}
	return itr;
}


child_process* search_index(int index, child_process *list)
{
	child_process *itr = list;

	while (itr != NULL && itr->next != NULL) {
		itr = itr->next;
	}
	while (itr != NULL && index > 1) {
		--index;
		itr = itr->prev;
	}
	if (index != 1)
		return NULL;
	return itr;
}

int empty_child(child_process **list)
{
	int ret = 0;
	while(*list != NULL && ret == 0) {
		ret = child_remove(list, *list);
	}
	return ret;
}
