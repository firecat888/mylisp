
#include <string.h>
#include <stdarg.h>
#include "mpc.h"
#include "lval.h"

#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval* err = lval_err(LERR_ERR, fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

#define LASSERT_TYPE(func, args, index, expect) \
  LASSERT(args, args->value.cell[index]->type == expect, \
    "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.", \
    func, index, ltype_name(args->value.cell[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num) \
  LASSERT(args, args->count == num, \
    "Function '%s' passed incorrect number of arguments. Got %i, Expected %i.", \
    func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
  LASSERT(args, args->value.cell[index]->value.qexpr->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index)

/* Add builtin function  */

/* Convert list to q-expr
 * list -> q_expr 
 * (a b c) -> '(a b c)
 */
lval* builtin_list(lenv* e, lval* a) {
  return lval_sexpr_quote(a);
}

/* Get first element from q-expr 
* list(q-expr(list)) -> q-expr
* ('(a b c)) -> 'a 
*/
lval* builtin_head(lenv* e, lval* a) {
  LASSERT_NUM("head", a, 1);
  LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("head", a, 0);
  
  lval* q = lval_list_take(a, 0);  
  lval* v = lval_qexpr_unquote(q);
  lval* x = lval_list_take(v, 0);
  return lval_sexpr_quote(x);
}

/* Take tail element from List Q-expression 
*  list(qexpr(list)) -> qexpr(list) 
*  ('(a b c)) -> '(b c)
*/
lval* builtin_tail(lenv* e, lval* a) {
  LASSERT_NUM("tail", a, 1);
  LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("tail", a, 0);

  lval* q = lval_list_take(a, 0);  
  lval* v = lval_qexpr_unquote(q);
  lval_del(lval_list_pop(v, 0));
  return lval_sexpr_quote(v);
}

/* Define variable 
 * (def/= '(a b c) 1 2 3)
 */
lval* builtin_var(lenv* e, lval* a, char* func) {
  LASSERT_TYPE("def", a, 0, LVAL_QEXPR);

  /* First argument is symbol list */
  lval* syms = lval_qexpr_pop(a->value.cell[0]);

 /* Ensure all elements of first list are symbols */
  for (int i = 0; i < syms->count; i++) {
    LASSERT(a, syms->value.cell[i]->type == LVAL_SYM,
      "Function 'def' cannot define non-symbol."
      "Got %s, Expected %s.",
      ltype_name(syms->value.cell[i]->type), ltype_name(LVAL_SYM));
  }

  /* Check correct number of symbols and values */
  LASSERT(a, syms->count == a->count-1,
    "Function 'def' cannot define incorrect "
    "number of values to symbols "
    "Got %i, Expected %i.",
    syms->count, a->count-1);

  /* Assign copies of values to symbols */
  for (int i = 0; i < syms->count; i++) {
    if (strcmp(func, "def") == 0) /* global def */
      lenv_def(e, syms->value.cell[i], a->value.cell[i+1]);
    else if (strcmp(func, "=") == 0)
      lenv_put(e, syms->value.cell[i], a->value.cell[i+1]);

  }

  lval_del(a);
  return lval_list();
}


lval* builtin_def(lenv* e, lval* a) {
  return builtin_var(e, a, "def");
}


lval* builtin_put(lenv* e, lval* a) {
  return builtin_var(e, a, "=");
}


lval* builtin_lambda(lenv* e, lval* a) {
  /* Check Two arguments, each of which are Q-Expressions */
  LASSERT_NUM("lambda", a, 2);
  LASSERT_TYPE("lambda", a, 0, LVAL_QEXPR);
  LASSERT_TYPE("lambda", a, 1, LVAL_QEXPR);

  /* Pop first two arguments and pass them to lval_lambda */
  lval* formals = lval_qexpr_pop(lval_list_pop(a, 0));

  /* Check first Q-Expression contains only Symbols */
   for (int i = 0; i < formals->count; i++) {
    LASSERT(a, (formals->value.cell[i]->type == LVAL_SYM),
      "Cannot define non-symbol. Got %s, Expected %s.",
      ltype_name(formals->value.cell[i]->type),ltype_name(LVAL_SYM));
  }

  lval* body = lval_qexpr_pop(lval_list_pop(a, 0));
  lval_del(a);

  return lval_lambda(formals, body);
}

/* Forward declaration*/
lval* lval_eval(lenv* e, lval* v);

/* Evalute list in q-expr
 * qexpr(list) -> lval
 * '(+ 1 2) -> 3
 */
lval* builtin_qexpr_eval(lenv* e, lval* a) {
  LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);
  
  lval* x = lval_qexpr_unquote(a);
  return lval_eval(e, x);
}


//todo: join what ? Add cons?
/* John two qexpr(list)
 * list(qexpr qexpr ...) -> list 
 * ('(a b c) '(d e)) -> '(a b c d e)
 */
lval* builtin_join(lenv* e, lval* a) {

  for (int i = 0; i < a->count; i++) {
    LASSERT_TYPE("join", a, i, LVAL_QEXPR);
    LASSERT(a, a->value.cell[i]->value.qexpr->type == LVAL_LIST,
      "Function 'join' passed incorrect type."
      "Got %s, Expected: %s. ",
       ltype_name(a->value.cell[i]->value.qexpr->type), ltype_name(LVAL_LIST));
  }
  
  lval* q = lval_list_pop(a, 0);
  lval* x = lval_qexpr_unquote(q);
  
  while (a->count) {
    q = lval_list_pop(a, 0);
    x = lval_list_join(x, lval_qexpr_unquote(q));
  }
  
  lval_del(a);
  return lval_sexpr_quote(x);
}

/* end of add builtin function*/



lval* builtin_op(lenv* e, lval* a, char* op) {
  
  /* Ensure all arguments are numbers */
  for (int i = 0; i < a->count; i++) {
    if (a->value.cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err(LERR_BAD_NUM, "Cannot operate on non-number"); 
    }
  }
  
  /* Pop the first element */
  lval* x = lval_list_pop(a, 0);
  
  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->value.num = -x->value.num;
  }
  
  /* While there are still elements remaining */
  while (a->count > 0) {
  
    /* Pop the next element */
    lval* y = lval_list_pop(a, 0);
    
    /* Perform operation */
    if (strcmp(op, "+") == 0) { x->value.num += y->value.num; }
    if (strcmp(op, "-") == 0) { x->value.num -= y->value.num; }
    if (strcmp(op, "*") == 0) { x->value.num *= y->value.num; }
    if (strcmp(op, "/") == 0) {
      if (y->value.num == 0) {
        lval_del(x); lval_del(y);
        x = lval_err(LERR_DIV_ZERO, "Divison by zero"); 
        break;
      }
      x->value.num /= y->value.num;
    }
    
    /* Delete element now finished with */
    lval_del(y);
  }
  
  /* Delete input expression and return result */
  lval_del(a);
  return x;
}

lval* builtin_add(lenv* e, lval* a) {
  return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
  return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
  return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
  return builtin_op(e, a, "/");
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin builtin) {
  lval* k = lval_sym(name);
  lval* v = lval_builtin(builtin);
  lenv_put(e, k, v);
  lval_del(k); lval_del(v);
}

void lenv_add_builtins(lenv* e) {
  /* Variable Function */
  lenv_add_builtin(e, "def", builtin_def);
  lenv_add_builtin(e, "=", builtin_put);
  /* List Functions */
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_qexpr_eval);
  lenv_add_builtin(e, "join", builtin_join);
  lenv_add_builtin(e, "lambda",   builtin_lambda);

  /* Mathematical Functions */
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);
}

lval* lval_eval_list(lenv* e, lval* v); 
/* Function call
 * fun(e, (x, y), (+ x y)), para(1, 2) 
 *
 * */
lval* lval_call(lenv* env, lval* fun, lval* para) {

  lfun* f = fun->value.fun;
  /* If Builtin then simply apply that */
  if (f->builtin) { return f->builtin(env, para); }

  /* Record Argument Counts */
  int given = para->count;
  int total = f->formals->count;

  /* While arguments still remain to be processed */
  while (para->count) {

    /* If we've ran out of formal arguments to bind */
    if (f->formals->count == 0) {
      lval_del(para); 
      return lval_err(LERR_ERR,
        "Function passed too many arguments. "
        "Got %i, Expected %i.", given, total);
    }

    /* Pop the first symbol from the formals */
    lval* sym = lval_list_pop(f->formals, 0);

    /* Pop the next argument from the list */
    lval* val = lval_list_pop(para, 0);

    /* Bind a copy into the function's environment */
    lenv_put(f->env, sym, val);

    /* Delete symbol and value */
    lval_del(sym); lval_del(val);
  }

  /* Argument list is now bound so can be cleaned up */
  lval_del(para);

  /* If all formals have been bound evaluate */
  if (f->formals->count == 0) {

    /* Set environment parent to evaluation environment */
    f->env->par = env;

    /* Evaluate and return */
    return lval_eval_list(f->env, lval_copy(f->body));
    /*
    return builtin_qexpr_eval(
      f->env, lval_list_add(lval_list(), lval_copy(f->body)));
    */

  } else {

    /* Otherwise return partially evaluated function */
    return lval_copy(fun);
  }
}


lval* lval_eval_list(lenv* e, lval* v) {
  
  /* Evaluate Children */
  for (int i = 0; i < v->count; i++) {
    v->value.cell[i] = lval_eval(e, v->value.cell[i]);
  }
  
  /* Error Checking */
  for (int i = 0; i < v->count; i++) {
    if (v->value.cell[i]->type == LVAL_ERR) { return lval_list_take(v, i); }
  }
  
  /* Empty Expression */
  if (v->count == 0) { return v; }
  
  /* Single Expression */
  if (v->count == 1) { return lval_list_take(v, 0); }
  
  /* Ensure First Element is Function */
  lval* f = lval_list_pop(v, 0);
  if (f->type != LVAL_FUN) { 
    lval* err = lval_err(LERR_ERR,
        "S-Expression starts with incorrect type. "
        "Got %s, Expected %s.",
        ltype_name(f->type), ltype_name(LVAL_FUN));
    lval_del(f); lval_del(v);
    return err;
  }
  
  /* Call builtin with operator */
  lval* result = lval_call(e, f, v);
  lval_del(f);
  return result;
}

lval* lval_eval(lenv* e, lval* v) {

  if (v->type == LVAL_SYM) {
    lval* x = lenv_get(e, v);
    lval_del(v);
    return x;
  }

  /* Evaluate List */
  if (v->type == LVAL_LIST) { return lval_eval_list(e, v); }
  /* All other lval types remain the same */
  return v;
}





