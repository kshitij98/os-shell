#ifndef _BUILTINS_
#define _BUILTINS_ 1
#define BUILTIN(str) builtin_##str
#define N 75
#define M 52
#define BUILTIN_LEN 4


extern char *builtin_str[];
extern int (*builtin_call[]) (char**, int);
int builtin_echo(char*);

#endif
