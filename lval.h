#if !defined(__LVAL_H__)
#define __LVAL_H__

/* Create Enumeration of Possible Error Types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_ERR };

/* Declare New lval Struct */
typedef struct {
  int type;
  long num;
  int err;
} lval;



/* Create a new number type lval */
lval lval_num(long x);

/* Create a new error type lval */
lval lval_err(int x);

/* Print an "lval" */
void lval_print(lval v);

/* Print an "lval" followed by a newline */
void lval_println(lval v);

#endif
