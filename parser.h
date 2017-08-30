#ifndef _PARSER_
#define _PARSER_ 1
#define TOKEN_BUFFSIZE 128
#define TOKEN_DELIMITER " \t\r\n\0"
#define LINE_BUFFSIZE 2048


#define SEP_LIST " \t\n\r"
#define CMD_SEP ';'
#define ESC '\\'
#define MAX_LINE_LEN 1024
typedef char* Str;


unsigned int number_of_elements(const char *s, char *sep, char esc);
Str *string_tokenizer(char *s, char *sep, char esc, unsigned int *q);
Str esc_stripper(Str s, char esc);
char *line_read(void);
char *echo_parser(char*);
char *get_flags(char **args, char agrc, char *flags);

#endif
