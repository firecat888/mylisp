#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include "parsing.h"

int main(int argc, char** argv) {
   
  /* Print Version and Exit Information */
  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");
   

  create_parser();

  /* In a never ending loop */
  while (1) {
    
    /* Output our prompt and get input */
    char* input = readline("lispy> ");
    
    /* Add input to history */
    add_history(input);
    
    parse_string(input);

    /* Free retrived input */
    free(input);
    
  }

  clean_parser();
  
  return 0;
}


