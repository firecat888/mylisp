#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

typedef struct {
  mpc_parser_t* Number;
  mpc_parser_t *Operator;
  mpc_parser_t *Expr;
  mpc_parser_t *Lispy;
} lispy_lang_t;

extern long eval(mpc_ast_t* t);

static lispy_lang_t lispy_lang;

int create_parser(void)
{
  /* Create Some Parsers */
  lispy_lang.Number   = mpc_new("number");
  lispy_lang.Operator = mpc_new("operator");
  lispy_lang.Expr     = mpc_new("expr");
  lispy_lang.Lispy    = mpc_new("lispy");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+/ ;                             \
      operator : '+' | '-' | '*' | '/' ;                  \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    lispy_lang.Number, lispy_lang.Operator, lispy_lang.Expr, lispy_lang.Lispy);

  return 0;
}

int parse_string(char *input)
{
  static mpc_result_t r;
  long result;

  if (mpc_parse("<stdin>", input, lispy_lang.Lispy, &r)) {
    mpc_ast_print(r.output);
    result = eval(r.output);
    printf("%li\n", result);
    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }

  return 0;
}

void clean_parser(void)
{
  mpc_cleanup(4, lispy_lang.Number, lispy_lang.Operator, 
      lispy_lang.Expr, lispy_lang.Lispy);
}


