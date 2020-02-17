#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include "lval.h"
#include "parsing.h"

/* Extern function */
extern void lenv_add_builtins(lenv* e);

int main(int argc, char** argv) {
   
  /* Print Version and Exit Information */
  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");
   

  create_parser();
  
  lenv* e = lenv_new();
  lenv_add_builtins(e);

  /* In a never ending loop */
  while (1) {
    
    /* Output our prompt and get input */
    char* input = readline("lispy> ");
    
    /* Add input to history */
    add_history(input);
    
    parse_string(e, input);

    /* Free retrived input */
    free(input);
    
  }

  lenv_del(e);
  clean_parser();
  
  return 0;
}


