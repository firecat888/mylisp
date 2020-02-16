#if !defined(__LVAL_H__)
#define __LVAL_H__

/* Create Enumeration of Possible Error Types */
enum { LERR_ERR, LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM, LERR_BAD_LIST };


/* Create Enumeration of Possible lval Types */
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_LIST, LVAL_QEXPR};

typedef struct lerr {
  int code;
  char* msg;
} lerr;

/* Declare New lval Struct */
typedef struct lval {
  int type;
  union {
    long num; /* type == LVAL_NUM */
    /* Error and Symbol types have some string data */
    lerr err; /* type == LVAL_ERR */
    char* sym; /* type == LVAL_SYM */
    struct lval* qexpr; /* type == LVAL_QEXPR */
    struct lval** cell; /*type == LVAL_LIST or LVAL_QEXPR */
  } value;
  /* Count and Pointer to a list of "lval*"; */
  int count;
} lval;




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

/* Free var of lval type */
void lval_del(lval* v);

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
 
#endif
