#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "lval.h"


/* Construct a pointer to a new Number lval */ 
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->value.num = x;
  return v;
}


/* Get type name */
char* ltype_name(int t) {
  switch(t) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_LIST: return "List";
    case LVAL_QEXPR: return "Q-Expr";
    default: return "Unknown";
  }
}


/* Construct a pointer to a new Error lval */ 
lval* lval_err(int code, char* fmt, ...) {
  #define MSG_LEN 1024
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->value.err.code = code;
  v->value.err.msg  = malloc(MSG_LEN);
  memset(v->value.err.msg, 0, MSG_LEN);

  va_list va;
  va_start(va, fmt);
  vsnprintf(v->value.err.msg, MSG_LEN-1, fmt, va);
  va_end(va);

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
lval* lval_qexpr(lval* x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->value.qexpr = x;
  return v;
}

/* Create a pointer to a new Function lval */
lval* lval_builtin(lbuiltin f) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->value.fun = malloc(sizeof(lfun));
  v->value.fun->builtin = f;
  return v;
}


lval* lval_lambda(lval* formals, lval* body) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->value.fun = malloc(sizeof(lfun));

  /* Set Builtin to Null */
  v->value.fun->builtin = NULL;

  /* Build new environment */
  v->value.fun->env = lenv_new();

  /* Set Formals and Body */
  v->value.fun->formals = formals;
  v->value.fun->body = body;
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
      if (!IS_BUILTIN(v)) {
        lenv_del(v->value.fun->env);
        lval_del(v->value.fun->formals);
        lval_del(v->value.fun->body);
      }
      free(v->value.fun);
      break;
  }
  
  /* Free the memory allocated for the "lval" struct itself */
  free(v);
}

/* Copy a lval to a new lval */
lval* lval_copy(lval* v) {

  lfun* func;
  lval* x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type) {

    /* Copy Numbers Directly */
    case LVAL_NUM: x->value.num = v->value.num; break;

    /* Copy Functions and Numbers Directly */
    case LVAL_FUN: 
      func = malloc(sizeof(lfun));
      if (IS_BUILTIN(v))
        func->builtin = v->value.fun->builtin;
      else {
        func->builtin = NULL;
        func->env = lenv_copy(v->value.fun->env);
        func->formals = lval_copy(v->value.fun->formals);
        func->body = lval_copy(v->value.fun->body);
      }
      x->value.fun = func;
      break;

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

/* Pop sub element from qexpr (not free qexpr)
 * qexpr -> list
 * '(a b c) -> (a b c)
 */
lval* lval_qexpr_pop(lval* q) {
  assert(q->value.qexpr != NULL);
  lval* v = q->value.qexpr;
  q->value.qexpr = NULL;
  return v;
}


/* Quote s-expr 
 * s-expr -> q-expr
 * a -> 'a, (a b c) -> '(a b c)
 */
lval * lval_sexpr_quote(lval* x) {
  lval* q = lval_qexpr(x);
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
        printf("Error: %s", v->value.err.msg);
        break;
 
    case LVAL_SYM:   printf("%s", v->value.sym); break;
    case LVAL_LIST:  lval_list_print(v, '(', ')'); break;
    case LVAL_QEXPR: lval_qexpr_print(v); break;
    case LVAL_FUN:  
      if (IS_BUILTIN(v))
        printf("<builtin>"); 
      else {
        printf("(lambda "); lval_print(v->value.fun->formals);
        putchar(' '); lval_print(v->value.fun->body); putchar(')');
      }
      break;
  }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }


/* Create a new env */
lenv* lenv_new(void) {
  lenv* e = malloc(sizeof(lenv));
  e->par = NULL;
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


/* Duplicate a environment */
lenv* lenv_copy(lenv* e) {
  lenv* n = malloc(sizeof(lenv));
  n->par = e->par;
  n->count = e->count;
  n->vars = malloc(sizeof(lvar) * n->count);
  for (int i = 0; i < e->count; i++) {
    n->vars[i].sym = strdup(e->vars[i].sym);
    n->vars[i].val = lval_copy(e->vars[i].val);
  }
  return n;
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

  if (e->par) {
    return lenv_get(e->par, k);
  } else {
    /* If no symbol found return error */
    return lval_err(LERR_ERR, "unbound symbol!");
  }
}

/* Replace var in env or add new var to env
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

/* Define new var in global env */
void lenv_def(lenv* e, lval* k, lval* v) {
  /* Iterate till e has no parent */
  while (e->par) { e = e->par; }
  /* Put value in e */
  lenv_put(e, k, v);
}
