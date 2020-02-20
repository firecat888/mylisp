#if !defined(__LVAL_H__)
#define __LVAL_H__

/* Create Enumeration of Possible Error Types */
enum { LERR_ERR, LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM, LERR_BAD_LIST };

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

/* Create Enumeration of Possible lval Types */
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, 
       LVAL_FUN, LVAL_LIST, LVAL_QEXPR};

typedef lval* (*lbuiltin) (lenv*, lval*);

typedef struct lerr {
  int code;
  char* msg;
} lerr;

/* Declare New lval Struct */
struct lval {
  int type;
  union {
    long num;           /* type == LVAL_NUM */
    lerr err;           /* type == LVAL_ERR */
    char* sym;          /* type == LVAL_SYM */
    lbuiltin fun;       /* type == LVAL_FUN */
    struct lval* qexpr; /* type == LVAL_QEXPR */
    struct lval** cell; /* type == LVAL_LIST */
  } value;
  /* Count and Pointer to a list of "lval*"; */
  int count;
};


/* Create a new number type lval */
lval* lval_num(long x);

/* Create a new error type lval */
lval* lval_err(int code, char* msg);

/* Create a pointer to a new Symbol lval */
lval* lval_sym(char* s);

/* A pointer to a new empty list lval */
lval* lval_list(void);

/* A pointer to a new empty qexpr lval */
lval* lval_qexpr(void);

/* Create a pointer to a new Function lval */
lval* lval_fun(lbuiltin func);

/* Free var of lval type */
void lval_del(lval* v);

/* Copy a lval to new lval */
lval* lval_copy(lval* v);

/* Add x to sub element of list v */
lval* lval_list_add(lval* v, lval* x);

/* Pop up i-th sub element of list v */
lval* lval_list_pop(lval* v, int i);

/*Take i-th sub element and delete list v */
lval* lval_list_take(lval* v, int i);

/* John two list */
lval* lval_list_join(lval* x, lval* y);

/* Take sub element from qexpr, delete qexpr */
lval* lval_qexpr_unquote(lval* q);

/* Pop sub element from qexpr */
lval* lval_qexpr_pop(lval* q);

/* Quote s-expr */
lval* lval_sexpr_quote(lval* x);

/* Add x to sub element of qexpr q */
lval* lval_qexpr_add(lval* q, lval* x);

/* Print an "lval" */
void lval_print(lval* v);

/* Print an "lval" followed by a newline */
void lval_println(lval* v);

/* Print an List type lval */
void lval_list_print(lval* v, char open, char close);


/* environment */

typedef struct lvar {
  char* sym;
  lval* val;
} lvar;

struct lenv {
  int count;
  lvar* vars;
};

lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);
 
#endif
