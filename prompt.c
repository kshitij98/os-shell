#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include "prompt.h"


// Check if the path inside the home directory
int inside_home(char *path, char *home_dir) {
	int home_len = strlen(home_dir);
	for (int i=0 ; i<home_len ; ++i)
		if (path[i] != home_dir[i])
			return 0;
	return 1;
}


// <username@system_name:curr_dir>
char *print_prompt() {
	char *username = getpwuid(geteuid()) -> pw_name;
	char *home_dir = getpwuid(geteuid()) -> pw_dir;
	char system_name[1023];
	gethostname(system_name, 1023);
	
	char curr_dir[1023];
	if (getcwd(curr_dir, 1023) == NULL)
		perror("Couldn't fetch current working directory!\ngetcurr_dir() error");

	if (inside_home(curr_dir, home_dir))
		printf("%s%s<%s@%s:%s~%s>%s", BOLD_TEXT, GREEN_TEXT, username, system_name, BLUE_TEXT, curr_dir + strlen(home_dir), RESET_TEXT);
	else
		printf("%s%s<%s@%s:%s%s>%s", BOLD_TEXT, GREEN_TEXT, username, system_name, BLUE_TEXT, curr_dir, RESET_TEXT);

	return "asd";
}


#ifdef _LOCAL_TESTING

int main()
{
	print_prompt();
	printf("\n");

	return 0;
}

#endif
