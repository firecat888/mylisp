#if !defined(__PARSING_H__)
#define __PARSING_H__

/* Create MPC parser */
int create_parser(void);

/* Cleanup MPC parser */
int clean_parser(void);

/* Parse string with MPC parser */
int parse_string(char* input);

#endif
