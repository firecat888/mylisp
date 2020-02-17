#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lval.h"


/* Construct a pointer to a new Number lval */ 
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->value.num = x;
  return v;
}

/* Construct a pointer to a new Error lval */ 
lval* lval_err(int code, char* msg) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->value.err.code = code;
  v->value.err.msg  = strdup(msg);
  return v;
}

/* Construct a pointer to a new Symbol lval */ 
lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->value.sym = malloc(strlen(s) + 1);
  strcpy(v->value.sym, s);
  return v;
}

/* A pointer to a new empty list lval */
lval* lval_list(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_LIST;
  v->count = 0;
  v->value.cell = NULL;
  return v;
}

/* A pointer to a new empty qexpr lval */
lval* lval_qexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->value.qexpr = NULL;
  return v;
}

/* Create a pointer to a new Function lval */
lval *lval_fun(lbuiltin func) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->value.fun  = func;
  return v;
}

/* Free var of lval type */
void lval_del(lval* v) {

  switch (v->type) {
    /* Do nothing special for number type */
    case LVAL_NUM: 
      break;
    case LVAL_ERR:
      free(v->value.err.msg);
      break;
    
    /* For Sym free the string data */
    case LVAL_SYM: 
      free(v->value.sym); 
      break;
    
    /* If List then delete all elements inside */
    case LVAL_LIST:
      for (int i = 0; i < v->count; i++) {
        lval_del(v->value.cell[i]);
      }
      /* Also free the memory allocated to contain the pointers */
      free(v->value.cell);
    break;

    case LVAL_QEXPR:
      if (v->value.qexpr != NULL)
        lval_del(v->value.qexpr);
      break;
    
    case LVAL_FUN:
      break;
  }
  
  /* Free the memory allocated for the "lval" struct itself */
  free(v);
}

/* Copy a lval to a new lval */
lval* lval_copy(lval* v) {

  lval* x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type) {

    /* Copy Functions and Numbers Directly */
    case LVAL_FUN: x->value.fun = v->value.fun; break;
    case LVAL_NUM: x->value.num = v->value.num; break;

    /* Copy Strings using malloc and strcpy */
    case LVAL_ERR:
      x->value.err.code = v->value.err.code;
      x->value.err.msg  = strdup(v->value.err.msg);
      break;

    case LVAL_SYM:
      x->value.sym = strdup(v->value.sym);
      break;

    /* Copy Lists by copying each sub-expression */
    case LVAL_LIST:
      x->count = v->count;
      x->value.cell = malloc(sizeof(lval*) * x->count);
      for (int i = 0; i < x->count; i++) {
        x->value.cell[i] = lval_copy(v->value.cell[i]);
      }
      break;
    case LVAL_QEXPR:
      x->value.qexpr = lval_copy(v->value.qexpr);
      break;
  }

  return x;
}


/* Add x to sub element of list v */
lval* lval_list_add(lval* v, lval* x) {

  v->count++;
  v->value.cell = realloc(v->value.cell, sizeof(lval*) * v->count);
  v->value.cell[v->count-1] = x;
  return v;
}

/* Pop up i-th sub element of list v */
lval* lval_list_pop(lval* v, int i) {
  /* Find the item at "i" */
  lval* x = v->value.cell[i];
  
  /* Shift memory after the item at "i" over the top */
  memmove(&v->value.cell[i], &v->value.cell[i+1],
    sizeof(lval*) * (v->count-i-1));
  
  /* Decrease the count of items in the list */
  v->count--;
  
  /* Reallocate the memory used */
  v->value.cell = realloc(v->value.cell, sizeof(lval*) * v->count);
  return x;
}

/*Take i-th sub element and delete list v */
lval* lval_list_take(lval* v, int i) {
  lval* x = lval_list_pop(v, i);
  lval_del(v);
  return x;
}

/* John two list */
// list, list -> list
// (a b c) (d e) -> (a b c d e)
lval* lval_list_join(lval* x, lval* y) {

  while (y->count) {
    x = lval_list_add(x, lval_list_pop(y, 0));
  }

  lval_del(y);  
  return x;
}

/* Take sub element from qexpr, delete qexpr
 * qexpr -> list
 * '(a b c) -> (a b c)
 */
lval* lval_qexpr_unquote(lval* q) {
  assert(q->value.qexpr != NULL);
  lval* v = q->value.qexpr;
  q->value.qexpr = NULL;
  lval_del(q);
  return v;
}

/* Quote s-expr 
 * s-expr -> q-expr
 * a -> 'a, (a b c) -> '(a b c)
 */
lval * lval_sexpr_quote(lval* x) {
  lval* q = lval_qexpr();
  q->value.qexpr = x;
  return q;
}

/* Append sub lval to qexpr */
lval* lval_qexpr_add(lval* q, lval* x) {
  q->value.qexpr = x;
  return q;
}

/* Print an List type lval */
void lval_list_print(lval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    
    /* Print Value contained within */
    lval_print(v->value.cell[i]);
    
    /* Don't print trailing space if last element */
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

/* Print an Qexpr type lval */
void lval_qexpr_print(lval* q) {

  assert(q->value.qexpr != NULL);

  printf("'");
  lval_print(q->value.qexpr);
}

void lval_print(lval* v) {
  switch (v->type) {
    case LVAL_NUM:   printf("%li", v->value.num); break;

    /* In the case the type is an error */
    case LVAL_ERR:
      /* Check what type of error it is and print it */
      if (v->value.err.code == LERR_DIV_ZERO) {
        printf("ERR_DIV_ZERO: %s !", v->value.err.msg);
      }
      else if (v->value.err.code == LERR_BAD_OP)   {
        printf("ERR_BAD_OP: %s !", v->value.err.msg);
      }
      else if (v->value.err.code == LERR_BAD_NUM)  {
        printf("ERR_BAD_NUM: %s !", v->value.err.msg);
      }
      else if (v->value.err.code == LERR_BAD_LIST) {
        printf("ERR_BAD_LIST: %s !", v->value.err.msg);
      }
      break;
 
    case LVAL_SYM:   printf("%s", v->value.sym); break;
    case LVAL_LIST:  lval_list_print(v, '(', ')'); break;
    case LVAL_QEXPR: lval_qexpr_print(v); break;
    case LVAL_FUN:   printf("<funtion>"); break;
  }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }


/* Create a new env */
lenv* lenv_new(void) {
  lenv* e = malloc(sizeof(lenv));
  e->count = 0;
  e->vars  = NULL;
  return e;
}

/* Delete a env */
void lenv_del(lenv* e) {
  for (int i = 0; i<e->count; i++) {
    free(e->vars[i].sym);
    lval_del(e->vars[i].val);
  }
  free(e->vars);
  free(e);
}

/* Get a val by its symbol name from lenv */
lval* lenv_get(lenv* e, lval* k) {

  /* Iterate over all items in environment */
  for (int i = 0; i < e->count; i++) {
    /* Check if the stored string matches the symbol string */
    /* If it does, return a copy of the value */
    if (strcmp(e->vars[i].sym, k->value.sym) == 0) {
      return lval_copy(e->vars[i].val);
    }
  }
  /* If no symbol found return error */
  return lval_err(LERR_ERR, "unbound symbol!");
}

/* Replace var in env
 * lenv, lval, lval -> void
 * find k->sym in e; if found, replace it with v,
 *                   if not found, add v to e
 */
void lenv_put(lenv* e, lval* k, lval* v) {

  /* Iterate over all items in environment */
  /* This is to see if variable already exists */
  for (int i = 0; i < e->count; i++) {

    /* If variable is found delete item at that position */
    /* And replace with variable supplied by user */
    if (strcmp(e->vars[i].sym, k->value.sym) == 0) {
      lval_del(e->vars[i].val);
      e->vars[i].val = lval_copy(v);
      return;
    }
  }

  /* If no existing entry found allocate space for new entry */
  e->count++;
  e->vars = realloc(e->vars, sizeof(lvar) * e->count);

  /* Copy contents of lval and symbol string into new location */
  e->vars[e->count-1].val = lval_copy(v);
  e->vars[e->count-1].sym = strdup(k->value.sym);
}
