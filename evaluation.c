
#include <string.h>
#include "mpc.h"
#include "lval.h"

#define LASSERT(args, cond, msg) \
  if (!(cond)) { lval_del(args); return lval_err(LERR_ERR, msg); }

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
  LASSERT(a, a->count == 1,
    "Function 'head' passed too many arguments.");
  LASSERT(a, a->value.cell[0]->type == LVAL_QEXPR,
    "Function 'head' passed incorrect type.");
  LASSERT(a, a->value.cell[0]->value.qexpr->count != 0,
    "Function 'head' passed {}.");
  
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
  LASSERT(a, a->count == 1,
    "Function 'tail' passed too many arguments.");
  LASSERT(a, a->value.cell[0]->type == LVAL_QEXPR,
    "Function 'tail' passed incorrect type.");
  LASSERT(a, a->value.cell[0]->value.qexpr->count != 0,
    "Function 'tail' passed {}.");

  lval* q = lval_list_take(a, 0);  
  lval* v = lval_qexpr_unquote(q);
  lval_del(lval_list_pop(v, 0));
  return lval_sexpr_quote(v);
}


lval* builtin_def(lenv* e, lval* a) {
  LASSERT(a, a->value.cell[0]->type == LVAL_QEXPR,
    "Function 'def' passed incorrect type!");

  /* First argument is symbol list */
  lval* syms = lval_qexpr_pop(a->value.cell[0]);

 /* Ensure all elements of first list are symbols */
  for (int i = 0; i < syms->count; i++) {
    LASSERT(a, syms->value.cell[i]->type == LVAL_SYM,
      "Function 'def' cannot define non-symbol");
  }

  /* Check correct number of symbols and values */
  LASSERT(a, syms->count == a->count-1,
    "Function 'def' cannot define incorrect "
    "number of values to symbols");

  /* Assign copies of values to symbols */
  for (int i = 0; i < syms->count; i++) {
    lenv_put(e, syms->value.cell[i], a->value.cell[i+1]);
  }

  lval_del(a);
  return lval_list();
}

/* Forward declaration*/
lval* lval_eval(lenv* e, lval* v);

/* Evalute list in q-expr
 * qexpr(list) -> lval
 * '(+ 1 2) -> 3
 */
lval* builtin_qexpr_eval(lenv* e, lval* a) {

 LASSERT(a, a->type == LVAL_QEXPR,
    "Function 'eval' passed incorrect type.");
  
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
    LASSERT(a, a->value.cell[i]->type == LVAL_QEXPR,
      "Function 'join' passed incorrect type.");
    LASSERT(a, a->value.cell[i]->value.qexpr->type == LVAL_LIST,
      "Function 'join' passed incorrect type.");
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

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
  lval* k = lval_sym(name);
  lval* v = lval_fun(func);
  lenv_put(e, k, v);
  lval_del(k); lval_del(v);
}

void lenv_add_builtins(lenv* e) {
  /* Variable Function */
  lenv_add_builtin(e, "def", builtin_def);
  /* List Functions */
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_qexpr_eval);
  lenv_add_builtin(e, "join", builtin_join);

  /* Mathematical Functions */
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);
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
    lval_del(f); lval_del(v);
    return lval_err(LERR_BAD_LIST, "List does not start with symbol"); 
  }
  
  /* Call builtin with operator */
  lval* result = f->value.fun(e, v);
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





