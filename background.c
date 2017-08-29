#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "background.h"

int child_insert(child_process *list, pid_t proc_id)
{
	child_process *temp = malloc(sizeof(child_process));

	if (temp == NULL) {
		fprintf(stderr, "Could not create background process. %s\n", strerror(errno));
		return -1;
	}
	temp->pid = proc_id;
	temp->next = list;
	list = temp;
	return 1;
}

int child_remove(child_process *list, child_process *ptr)
{
	if (ptr == list) {
		list = list->next;
	} else {
		ptr->prev->next = ptr->next;
		if (ptr->next != NULL) {
			ptr->next->prev = ptr->prev;
		}
	}
	free(ptr);
	return 0;
}
