#define TOKEN_BUFFSIZE 128
#define TOKEN_DELIMITER " \t\r\n\0"
#define LINE_BUFFSIZE 2048
#define BLUE_TEXT "\033[34m"
#define GREEN_TEXT "\033[32m"
#define BOLD_TEXT "\033[1m"
#define RESET_TEXT "\033[0m"

void print_prompt();
char *line_read();