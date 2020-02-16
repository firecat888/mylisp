#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lval.h"
#include "mpc.h"

typedef struct {
  mpc_parser_t* Number;
  mpc_parser_t *Symbol;
  mpc_parser_t *List;
  mpc_parser_t *Sexpr;
  mpc_parser_t *Qexpr;
  mpc_parser_t *Expr;
  mpc_parser_t *Lispy;
} lispy_lang_t;


static lispy_lang_t lispy_lang;

int create_parser(void)
{
  /* Create Some Parsers */
  lispy_lang.Number   = mpc_new("number");
  lispy_lang.Symbol   = mpc_new("symbol");
  lispy_lang.List     = mpc_new("list");
  lispy_lang.Sexpr    = mpc_new("sexpr");
  lispy_lang.Qexpr    = mpc_new("qexpr");
  lispy_lang.Expr     = mpc_new("expr");
  lispy_lang.Lispy    = mpc_new("lispy");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+/ ;                             \
      symbol   : \"list\" | \"head\" | \"tail\"          \
               |  \"eval\" | \"join\"                     \
               |  '+' | '-' | '*' | '/' ;                  \
      list     : '(' <sexpr>* ')' ;                        \
      sexpr    : <number> | <symbol> | <list> ;           \
      qexpr    : '''<sexpr> ;                   \
      expr     : <sexpr> | <qexpr> ;           \
      lispy    : /^/ <expr>* /$/ ;             \
    ",
    lispy_lang.Number, lispy_lang.Symbol, lispy_lang.List, lispy_lang.Sexpr, 
    lispy_lang.Qexpr, lispy_lang.Expr, lispy_lang.Lispy);

  return 0;
}

/* Read type num lval from AST */
lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno == 0?
    lval_num(x) : lval_err(LERR_BAD_NUM, "Bad number");
}

/* Create internal structure from AST for evalution */
lval *lval_read(mpc_ast_t* t) {
  
  /* If Symbol or Number return conversion to that type */
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
  
  lval* x = NULL;
  /* If root (>) or list then create empty list */
  if ((strcmp(t->tag, ">") == 0) || (strstr(t->tag, "list")))
  { 
    x = lval_list(); 
    /* Fill this list with any valid expression contained within */
    for (int i = 0; i < t->children_num; i++) {
      if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
      if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
      if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
      x = lval_list_add(x, lval_read(t->children[i]));
    }
  }
  else if (strstr(t->tag, "qexpr")) {

    mpc_ast_t* child = t->children[1];  /*Get 2nd children */

    x = lval_qexpr();
    lval* c = lval_read(child);
    x = lval_qexpr_add(x, c);
  }
  else {
    perror("Parsing fail: unpxprected tag");
    exit(1);
  }

  return x;
}

extern lval* lval_eval(lval* v);

int parse_string(char *input)
{
  static mpc_result_t r;

  if (mpc_parse("<stdin>", input, lispy_lang.Lispy, &r)) {

    mpc_ast_print(r.output);

    /* Parse AST into List */
    lval *llist = NULL;
    llist = lval_read(r.output);
    printf("Parsing result: ");lval_println(llist);
    //lval_del(llist);
#if 1
    /* Evaluate List */
    lval *result;
    result = lval_eval(llist);
    printf("Evaluating result: ");lval_println(result);
    lval_del(result);
#endif
    mpc_ast_delete(r.output);

  } else {

    mpc_err_print(r.error);
    mpc_err_delete(r.error);

  }

  return 0;
}

//todo 2/7
//
void clean_parser(void)
{
  mpc_cleanup(4, lispy_lang.Number, lispy_lang.Symbol, 
      lispy_lang.List, lispy_lang.Sexpr, lispy_lang.Lispy);
}


