#ifndef _BACKGROUND_
#define _BACKGROUND_ 1
#include <sys/types.h>

#define NAME_LEN 256
extern char *STATE[];
extern int MAP_STATE[];

typedef struct CHILD_ {
	pid_t pid;
	char name[256];
	struct CHILD_ *next;
	struct CHILD_ *prev;
} child_process;

int child_insert(child_process**, pid_t, char*);
int child_remove(child_process**, child_process*);
void print_children(child_process**);
child_process* search_index(int, child_process*);
#endif
