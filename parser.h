//char **string_tokenizer(char*);
char *line_read(void);
#define TOKEN_BUFFSIZE 128
#define TOKEN_DELIMITER " \t\r\n\0"
#define LINE_BUFFSIZE 2048


#define SEP_LIST " \t\n\r"
#define CMD_SEP ';'
#define ESC '\\'
typedef char* Str;


unsigned int number_of_elements(const char *s, char *sep, char esc);
Str *string_tokenizer(char *s, char *sep, char esc, unsigned int *q);
Str esc_stripper(Str s, char esc);
