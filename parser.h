char **string_tokenizer(char*);
char *line_read(void);

#define TOKEN_BUFFSIZE 128
#define TOKEN_DELIMITER " \t\r\n\0"
#define LINE_BUFFSIZE 2048
