# mylisp
Create my lisp interpreter according to http://buildyourownlisp.com/

Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 3.0

http://creativecommons.org/licenses/by-nc-sa/3.0/

## Branch sexpr

* Difference from original BuildMyLisp
***** S-expression include include atom and list, so the list is named "list" lval instead of "sexpr".

## Branch qexpr

*  Difference from original BuildMyLisp
*****  Quoted expression not use "{1 2 3}" format, just use " '(1 2 3)"
**  Since quoted expression also include atom, so link real expression lval to quoted expression lval

