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
  }
  
  /* Free the memory allocated for the "lval" struct itself */
  free(v);
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
  }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }


