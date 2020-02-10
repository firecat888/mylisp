
#include <string.h>
#include "mpc.h"
#include "lval.h"

lval* builtin_op(lval* a, char* op) {
  
  /* Ensure all arguments are numbers */
  for (int i = 0; i < a->count; i++) {
    if (a->value.cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err(LERR_BAD_NUM); /*"Cannot operate on non-number!"); */
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
        x = lval_err(LERR_DIV_ZERO); /*"Division By Zero." */
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


lval* lval_eval(lval* v);

lval* lval_eval_list(lval* v) {
  
  /* Evaluate Children */
  for (int i = 0; i < v->count; i++) {
    v->value.cell[i] = lval_eval(v->value.cell[i]);
  }
  
  /* Error Checking */
  for (int i = 0; i < v->count; i++) {
    if (v->value.cell[i]->type == LVAL_ERR) { return lval_list_take(v, i); }
  }
  
  /* Empty Expression */
  if (v->count == 0) { return v; }
  
  /* Single Expression */
  if (v->count == 1) { return lval_list_take(v, 0); }
  
  /* Ensure First Element is Symbol */
  lval* f = lval_list_pop(v, 0);
  if (f->type != LVAL_SYM) {
    lval_del(f); lval_del(v);
    return lval_err(LERR_BAD_LIST); /*"S-expression Does not start with symbol."); */
  }
  
  /* Call builtin with operator */
  lval* result = builtin_op(v, f->value.sym);
  lval_del(f);
  return result;
}

lval* lval_eval(lval* v) {
  /* Evaluate Sexpressions */
  if (v->type == LVAL_LIST) { return lval_eval_list(v); }
  /* All other lval types remain the same */
  return v;
}




