#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "parser.h"
#include <stdio.h>

int builtin_cd(char **arg)
{
	const char *dest_dir;

	if (arg[1] == NULL) {
		dest_dir = getpwuid(getuid()) -> pw_dir;
		if (dest_dir == NULL) {
			perror("Could not get home directory.");
			return 1;
		}
	} else {
		dest_dir = arg[1];
	}

	if (chdir(dest_dir) != 0) {
		perror("'cd' Failed!");
		return 1;
	}

	return 0;
}


//#ifdef _LOCAL_TESTING

int main()
{
	char *str;
	while (1) {
		printf(">");
		str = line_read();
		builtin_cd(string_tokenizer(str));
	}
	return 0;
}

//#endif
