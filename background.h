#include <sys/types.h>

typedef struct CHILD_ {
	pid_t pid;
	struct CHILD_ *next;
	struct CHILD_ *prev;
} child_process;

int child_insert(child_process*, pid_t);
int child_remove(child_process*, child_process*);
