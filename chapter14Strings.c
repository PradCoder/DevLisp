/*
Linux
cc -std=c99 -Wall -ggdb prompt.c mpc.c -ledit -lm -o prompt

Windows
cc -std=c99 -Wall -ggdb prompts.c mpc.c -prompts

valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./chapter12Funcs

Pesara Amarasekera
2019-07-21

In this file Strings are finally implemented
*/

#include "mpc.h"

#ifdef _WIN32
    #include <string.h>
    static char buffer[2048]

    char* readline(char* prompt){
        fgets(prompt,stdout);
        fputs(buffer,2048,stdin);
        char* cpy = malloc(strlen(buffer)+1);
        strcpy(cpy,buffer);
        cpy[strlen(cpy)-1] = '\0';
        return cpy;
    }

    char* add_history(char* unused){}
#else
    #include <editline/readline.h>
    #include <editline/history.h>
#endif

#define LASSERT(args,cond, fmt, ...) \
    if(!(cond)){\
        lval* err = lval_err(fmt, ##__VA_ARGS__);\
        lval_del(args);\
        return err;\
    }

#define LASSERT_NUM(func,args,expec)\
    if(args->count != expec){\
        lval* err = lval_err("Function '%s' passed incorrect number of arguments. " \
                            "Got %i, Expected %i", \
                            func, args->count, expec);\
        lval_del(args); \
        return err;\
    }

#define LASSERT_TYPE(func, args, num, gtype) \
    if(args->cell[num]->type != gtype){ \
        lval* err = lval_err("Function '%s' passed incorrect type for argument %i. " \
                            "Got %s, Expected %s.",  \
                            func, num,ltype_name(args->cell[num]->type), ltype_name(gtype)); \
        lval_del(args); \
        return err;\
    }

struct lval;
struct lenv;
typedef struct lval lval; 
typedef struct lenv lenv;

enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_STR,
       LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR};

typedef lval* (*lbuiltin) (lenv*, lval*);

struct lval{
    int type;

    /* Basic */
    long num;
    char* sym;
    char* err;
    char* str;

    /* Function */
    lbuiltin builtin;
    lenv* env;
    lval* formals;
    lval* body;
    
    /* Expression */
    int count;
    lval** cell;
};

struct lenv{
    lenv* par;
    int count;
    lval** vals;
    char** syms;
};

lval* lval_num(long x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* s);
lval* lval_str(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_fun(lbuiltin func);
int lval_eq(lval* x, lval* y);
void lval_del(lval* v);
lval* lval_copy(lval* v);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_add(lval* v,lval* x);
lval* lval_read_str(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
void lval_expr_print(lval* v,char open, char close);
void lval_print_str(lval* v);
void lval_print(lval* v);
void lval_println(lval* v);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* x, lval* y);
long lval_len(lval* y);
lval* lval_init(lval* x, lval* y);
char* ltype_name(int t);

lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lval_lambda(lval* formals, lval* body);
lval* lenv_get(lenv* e, lval* k);
lenv* lenv_copy(lenv* e);
void lenv_put(lenv* e, lval* k, lval* v);
void lenv_def(lenv* e,lval* k, lval* v);
lval* lval_call(lenv* e, lval* f, lval* a);

lval* builtin_var(lenv* e, lval* a, char* func);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);
lval* builtin_lambda(lenv* e, lval* a);
lval* builtin_fun(lenv* e, lval* a);
lval* builtin_if(lenv* e, lval* a);

lval* builtin_op(lenv* e, lval* a,char* op);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_mod(lenv* e, lval* a);

lval* builtin_list(lenv* e, lval* a);
lval* builtin_head(lenv* e,lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_cons(lenv* e, lval* a);
lval* builtin_len(lenv* e, lval* a);
lval* builtin_init(lenv* e, lval* a);

lval* builtin_logic(lenv* e, lval* a, char* op);
lval* builtin_and(lenv* e, lval* a);
lval* builtin_or(lenv* e, lval* a);
lval* builtin_not(lenv* e, lval* a);

lval* builtin_cmp(lenv* e, lval* a, char* op);
lval* builtin_eq(lenv* e, lval* a);
lval* builtin_ne(lenv* e, lval* a);
lval* builtin_ord(lenv* e,lval* a, char* op);
lval* builtin_gt(lenv* e, lval* a);
lval* builtin_lt(lenv* e, lval* a);
lval* builtin_ge(lenv* e, lval* a);
lval* builtin_le(lenv* e, lval* a);

lval* builtin_eval(lenv* e, lval* a);
void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);
lval* lval_eval_sexpr(lenv* e,lval* v);
lval* lval_eval(lenv* e, lval* v);

/* Construct a pointer to a new Number lval */
lval* lval_num(long x){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

/* Construct a pointer to a new Error lval */
lval* lval_err(char* fmt, ...){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;

    /* Create a va list and initialize it */
    va_list va;
    va_start(va, fmt);
    
    /* Allocate 512 bytes of space */
    v->err = malloc(512);

    /* printf the error string with a maximum of 511 */
    vsnprintf(v->err, 511, fmt, va);

    /* Reallocate to number of bytes actually used */
    v->err = realloc(v->err, strlen(v->err)+1);

    /* Cleanup our va list */
    va_end(va);

    return v;
}

/* Construct a pointer to a new Symbol lval */
lval* lval_sym(char* s){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s)+1);
    strcpy(v->sym,s);
    return v;
}

/* Construct a pointer to a String */
lval* lval_str(char* s){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_STR;
    v->str = malloc(strlen(s)+1);
    strcpy(v->str,s);
    return v;
}

/* Sexpr pointer constructor */
lval* lval_sexpr(void){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

/* Qexpr pointer constructor */
lval* lval_qexpr(void){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lval* lval_fun(lbuiltin func){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->builtin = func;
    return v;
}

int lval_eq(lval* x, lval* y){

    /*Different types are always unequal */
    if(x->type != y->type) {return 0;}

    /* Compare based on types */
    switch (x->type) {
        /* Compare Number values */
        case LVAL_NUM: return (x->num == y->num);
        
        /* Compare String Values */
        case LVAL_ERR: return (strcmp(x->err,y->err) == 0);
        case LVAL_SYM: return (strcmp(x->sym,y->sym) == 0);
        case LVAL_STR: return (strcmp(x->str,y->str) == 0);
        /* If builtin compare, otherwise compare formals and body */
        case LVAL_FUN:
            if (x->builtin || y->builtin){
                return x->builtin == y->builtin;
            } else {
                return lval_eq(x->formals, y->formals)
                && lval_eq(x->body, y->body);
            }
        
        /* If list compare every individual element */
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if(x->count != y->count) { return 0; }
            for (int i = 0; i < x->count; i++){
                /* If any element not equal then whole list is not equal */
                if (!lval_eq(x->cell[i],y->cell[i])) { return 0; }
            }
            /* Otherwise lists must be equal */
            return 1;
        break;
    }
    return 0;
}

lval* lval_copy(lval* v){

    lval* x = malloc(sizeof(lval));
    x->type = v->type;

    switch(v->type){
        /* Copy Functions and Numbers Directly */
        case LVAL_FUN: 
            if(v->builtin){
                x->builtin = v->builtin;
            } else {
                x->builtin = NULL;
                x->env = lenv_copy(v->env);
                x->formals = lval_copy(v->formals);
                x->body = lval_copy(v->body); 
            }
        break;
        case LVAL_NUM: x->num = v->num; break;

        /* Copy Strings using malloc and strcpy */
        case LVAL_ERR:
            x->err = malloc(strlen(v->err)+1);
            strcpy(x->err,v->err); break;
        
        case LVAL_SYM:
            x->sym = malloc(strlen(v->sym)+1);
            strcpy(x->sym,v->sym); break;
        case LVAL_STR:
            x->str = malloc(strlen(v->str) + 1);
            strcpy(x->str,v->str); break;
        /* Copy Lists by copying each sub-expression */
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            x->count = v->count;
            x->cell = malloc(sizeof(lval*) * x->count);
            for(int i=0;i<x->count;i++){
                x->cell[i] = lval_copy(v->cell[i]);
            }
        break;
    }
    return x;
}

void lval_del(lval* v){
    switch (v->type){
        case LVAL_NUM: break;
        case LVAL_FUN: 
            if(!v->builtin){
                lenv_del(v->env);
                lval_del(v->formals);
                lval_del(v->body);
            }
        break;
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;
        case LVAL_STR: free(v->str); break;
        /*If Qexpr or Sexpr then delete all elements inside*/
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            for(int i=0;i<v->count;i++){
                lval_del(v->cell[i]);
            }
            /* Also free the memory allocated to contain the pointers */
            free(v->cell);
        break; 
    }
    free(v);
}

lval* lval_read_num(mpc_ast_t* t){
    errno = 0;
    long x = strtol(t->contents,NULL,10);
    return errno != ERANGE ? 
        lval_num(x) : lval_err("invalid number");
}

lval* lval_add(lval* v,lval* x){
    v->count++;
    v->cell = realloc(v->cell,sizeof(lval*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

lval* lval_read_str(mpc_ast_t* t){
    /* Cut off the final quote character */
    t->contents[strlen(t->contents)-1] = '\0';
    /* Copy the string missing out the first quote character */
    char* unescaped = malloc(strlen(t->contents+1)+1);
    strcpy(unescaped, t->contents+1);
    /* Pass through the unescape function */
    unescaped = mpcf_unescape(unescaped);
    /* Construct a new lval using the string */
    lval* str = lval_str(unescaped);
    /* Free the string and return */
    free(unescaped);
    return str;
}

lval* lval_read(mpc_ast_t* t){
    /*If Symbol or Number return conversion to that type*/
    if(strstr(t->tag,"number")) {return lval_read_num(t);}
    if(strstr(t->tag,"symbol")) {return lval_sym(t->contents);}
    if(strstr(t->tag,"string")) {return lval_read_str(t);}

    /*If root(>), sexpr, or qexpr then create empty list*/
    lval* x = NULL;
    if(strcmp(t->tag,">") == 0) { x = lval_sexpr();}
    if(strstr(t->tag,"sexpr"))  { x = lval_sexpr();}
    if(strstr(t->tag,"qexpr"))  { x = lval_qexpr();}

    /*Fill this list with any valid expression contained within*/
    for(int i=0;i<t->children_num;i++){
        if(strcmp(t->children[i]->contents,"(") == 0){continue;}
        if(strcmp(t->children[i]->contents,")") == 0){continue;}
        if(strcmp(t->children[i]->contents,"{") == 0){continue;}
        if(strcmp(t->children[i]->contents,"}") == 0){continue;}
        if(strcmp(t->children[i]->tag,"regex")  == 0){continue;}
        x = lval_add(x,lval_read(t->children[i]));
    }

    return x;
}

void lval_expr_print(lval* v,char open, char close){
    putchar(open);
    for(int i=0;i<v->count;i++){
        /*Print value contained within*/
        lval_print(v->cell[i]);

        /*Don't print trailing space if last element*/
        if(i != v->count-1){
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_print_str(lval* v){
    /* Make a Copy of the string */
    char* escaped = malloc(strlen(v->str)+1);
    strcpy(escaped, v->str);
    /* Pass it through the escape function */
    escaped = mpcf_escape(escaped);
    /* Print it between " characters */
    printf("\"%s\"", escaped);
    /* free the copied string */
    free(escaped);
}

void lval_print(lval* v){
    switch (v->type) {
        case LVAL_NUM: printf("%li",v->num); break;
        case LVAL_ERR: printf("Error: %s",v->err); break;
        case LVAL_SYM: printf("%s", v->sym); break;
        case LVAL_STR: lval_print_str(v); break;
        case LVAL_SEXPR: lval_expr_print(v,'(',')'); break;
        case LVAL_QEXPR: lval_expr_print(v,'{','}'); break;
        case LVAL_FUN: 
            if(v->builtin){
                printf("<builtin>");
            } else {
                printf("\\"); lval_print(v->formals);
                putchar(' '); lval_print(v->body); putchar(')');
            } 
        break;
    }
}

void lval_println(lval* v){lval_print(v); putchar('\n');}

lval* lval_pop(lval* v, int i){
    /* Find the item at i */
    lval* x = v->cell[i];

    /* Shift memory after the item "i" is over the top */
    memmove(&v->cell[i],&v->cell[i+1],
        sizeof(lval*) * (v->count-i-1));

    /* Decrease the count of items in the list */
    v->count--;
    
    /*reallocate the memory used*/
    v->cell = realloc(v->cell,sizeof(lval*) * v->count);

    return x;
}

lval* lval_take(lval* v, int i){
    lval* x = lval_pop(v,i);
    lval_del(v);
    return x;
}

lval* lval_join(lval* x, lval* y){
    /*For each cell in 'y' add it to 'x'*/
    while(y->count){
        x = lval_add(x,lval_pop(y,0));
    }
    lval_del(y);
    return x;
}

long lval_len(lval* y){
    long x = 0;
    while(y->count){
        lval* temp = lval_pop(y,0);
        x++; 
        lval_del(temp);
    }
    lval_del(y);
    return x;
}

lval* lval_init(lval* x, lval* y){
    while(y->count-1 != 0){
        x = lval_add(x,lval_pop(y,0));
    }
    lval_del(y);
    return x;
}

char* ltype_name(int t){
    switch(t){
        case LVAL_FUN: return "Function";
        case LVAL_NUM: return "Number";
        case LVAL_ERR: return "Error";
        case LVAL_SYM: return "Symbol";
        case LVAL_STR: return "String";
        case LVAL_SEXPR: return "S-Expression";
        case LVAL_QEXPR: return "Q-Expression";
        default: return "Unknown";
    }
}

lenv* lenv_new(void){
    lenv* e = malloc(sizeof(lenv));
    e->par = NULL;
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

void lenv_del(lenv* e){
    for(int i=0;i<e->count;i++){
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

lval* lval_lambda(lval* formals, lval* body){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;

    /* Set Builtin to Null */
    v->builtin = NULL;

    /* Build new environment */
    v->env = lenv_new();

    /* Set Formals and Body */
    v->formals = formals;
    v->body = body;
    return v;
}

lval* lenv_get(lenv* e, lval* k){
    /* Iterate over all items in environment */
    for(int i = 0;i<e->count;i++){
        /*Check if the stored string matches the symbol string*/
        /* If it does, return a copy of the value */
        if (strcmp(e->syms[i],k->sym)==0){
            return lval_copy(e->vals[i]);
        }
    }
    /* If no symbol check in parent otherwise error */
    if(e->par){
        return lenv_get(e->par, k);
    }

    return lval_err("Unbound Symbol '%s'",k->sym);
}

lenv* lenv_copy(lenv* e){
    lenv* n = malloc(sizeof(lenv));
    n->par = e->par;
    n->count = e->count;
    n->syms = malloc(sizeof(char*) * n->count);
    n->vals = malloc(sizeof(lval*) * n->count);
    for(int i = 0; i< e->count; i++){
        n->syms[i] = malloc(strlen(e->syms[i])+1);
        strcpy(n->syms[i],e->syms[i]);
        n->vals[i] = lval_copy(e->vals[i]);
    }
    return n;
}

void lenv_put(lenv* e, lval* k, lval* v){
    /* Iterate over all items in environment */
    /* This is to see if variable already exists */
    for(int i=0;i<e->count;i++){

        /* If variable is found delete item at that position */
        /* And replace with variable supplied by user */
        if(strcmp(e->syms[i],k->sym) == 0){
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }

    /* If no existing entry found allocate space for new entry */
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);

    /* Copy contents of lval and symbol string into new location */
    e->vals[e->count-1] = lval_copy(v);
    e->syms[e->count-1] = malloc(strlen(k->sym)+1);
    strcpy(e->syms[e->count-1],k->sym);
}

void lenv_def(lenv* e,lval* k, lval* v){
    /* Iterate till e has no parent */
    while(e->par) {
        e = e->par;
    }
    /* Put value in e */
    lenv_put(e,k,v);
}

lval* lval_call(lenv* e, lval* f, lval* a){
    /* If Builtin then simply call that */
    if(f->builtin){ return f->builtin(e,a); }

    /* Record Argument Counts */
    int given = a->count;
    int total = f->formals->count;

    /* If variable number of arguments are to be passed check if there is at least 1 of those*/
    if( (total-2>=0) && strcmp(f->formals->cell[total-2]->sym,"&") == 0 && given < (total-1)){
        lval_del(a);
        return lval_err("Function not called with proper number of arguments. "
            "Got %i, Expected %i.", given, total);
    }    

    /* While arguments still remain to be processed */
    while(a->count){

        /* If we've ran out of formal arguments to bind */
        if(f->formals->count == 0){
            lval_del(a); return lval_err(
                "Function passed too many arguments. "
                "Got %i, Expected %i.", given, total);
        }

        /* Pop the first symbol from the formals */
        lval* sym = lval_pop(f->formals, 0);

        /* Special Case to deal with '&' */
        if(strcmp(sym->sym,"&") == 0){

            /* Ensure '&' is followed by another symbol */
            if(f->formals->count != 1){
                lval_del(a);
                return lval_err("Function format invalid. "
                    "Symbol '&' not followed by single symbol.");
            }

            /* Next formal should be bound to remaining arguments */
            lval* nsym = lval_pop(f->formals,0);
            lenv_put(f->env, nsym, builtin_list(e,a));
            lval_del(sym); lval_del(nsym);
            break;
        }

        /* Pop the next argument from the list */
        lval* val = lval_pop(a,0);

        /* Bind a copy into the function's environment */
        lenv_put(f->env, sym, val);

        /* Delete symbol and value */
        lval_del(sym); lval_del(val);
    }

    /* Argument list is now bound so can be cleaned up */
    lval_del(a);

    /*If & remains in formal list bind to empty list */
    if(f->formals->count > 0 &&
        strcmp(f->formals->cell[0]->sym, "&") == 0){
        
        /* Check to ensure that & is not passed invalidly. */
        if(f->formals->count != 2){
            return lval_err("Function format invalid. "
                "Symbol '&' not followed by single symbol.");
        }

        /* Pop and delete '&' symbol */
        lval_del(lval_pop(f->formals,0));

        /* Pop next symbol and create empty list */
        lval* sym = lval_pop(f->formals, 0);
        lval* val = lval_qexpr();

        /* Bind to environment and delete */
        lenv_put(f->env, sym, val);
        lval_del(sym); lval_del(val);
    }

    /* If all formals have been bound evaluate */
    if(f->formals->count == 0){

        /* Set environment parent to evaluation environment */
        f->env->par = e;

        /* Evaluate and return */
        return builtin_eval(
            f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    }
    /* Otherwise return partially evaluated function */
    return lval_copy(f);
}

lval* builtin_var(lenv* e, lval* a, char* func){
    LASSERT_TYPE(func, a, 0, LVAL_QEXPR);
    
    /* First argument is symbol */
    lval* syms = a->cell[0];
    /* Ensure all elements of first list are symbols */
    for(int i=0;i<syms->count;i++){
        LASSERT(a,syms->cell[i]->type == LVAL_SYM,
            "Function 'def' cannot define non-symbol. "
            "Got %s, Expected %s.",func,
            ltype_name(syms->cell[i]->type),
            ltype_name(LVAL_SYM));
    }

    /* Check correct number of symbols and values */
    LASSERT(a, syms->count == a->count-1,
        "Function '%s' passed too many arguments for symbols. "
        "Got %i, Expected %i.",func, syms->count,a->count-1);
    
    for(int i =0;i<syms->count;i++){
        /* If 'def' define in globally. If 'put' define in locally */
        if(strcmp(func,"def") == 0){

            LASSERT(a,(strcmp("list",syms->cell[i]->sym) != 0 && strcmp("head",syms->cell[i]->sym) != 0 && strcmp("tail",syms->cell[i]->sym) != 0 &&
            strcmp("eval",syms->cell[i]->sym) != 0 && strcmp("join",syms->cell[i]->sym) != 0 && strcmp("cons",syms->cell[i]->sym) != 0 &&
            strcmp("init",syms->cell[i]->sym) != 0 && strcmp("len",syms->cell[i]->sym) != 0 && strcmp("def",syms->cell[i]->sym) != 0 &&
            strcmp("head",syms->cell[i]->sym) != 0 && strcmp("+",syms->cell[i]->sym) != 0 && strcmp("-",syms->cell[i]->sym) != 0 &&
            strcmp("*",syms->cell[i]->sym) != 0 && strcmp("/",syms->cell[i]->sym) != 0 && strcmp("%",syms->cell[i]->sym) != 0 &&
            strcmp("=",syms->cell[i]->sym) != 0 && strcmp("\\",syms->cell[i]->sym) != 0 && strcmp("fun",syms->cell[i]->sym) != 0),
            "Redefinition of '%s' is not allowed",syms->cell[i]->sym);

            lenv_def(e, syms->cell[i], a->cell[i+1]);
        }

        if(strcmp(func,"=") == 0){
            lenv_put(e,syms->cell[i],a->cell[i+1]);
        }
    }

    lval_del(a);
    return lval_sexpr();
}

lval* builtin_def(lenv* e, lval* a){
    return builtin_var(e,a,"def");
}

lval* builtin_put(lenv* e, lval* a){
    return builtin_var(e,a,"=");
}

lval* builtin_if(lenv* e, lval* a){
    /* Check Two arguments, each of which are Q-Expressions */
    LASSERT_NUM("if",a,3);
    LASSERT_TYPE("if",a,0,LVAL_NUM);
    LASSERT_TYPE("if",a,1,LVAL_QEXPR);
    LASSERT_TYPE("if",a,2,LVAL_QEXPR);

    /* Mark Both Expressions as evaluable */
    lval* x;
    a->cell[1]->type = LVAL_SEXPR;
    a->cell[2]->type = LVAL_SEXPR;

    if(a->cell[0]->num){
        /* If condition is true evaluate first expression */
        x = lval_eval(e,lval_pop(a,1));
    } else {
        /* Otherwise evaluate second expression */
        x = lval_eval(e,lval_pop(a,2));
    }
    /* Delete argument list and return */    
    lval_del(a);
    return x;
}

lval* builtin_lambda(lenv* e, lval* a){
    /* Check Two arguments, each of which are Q-Expressions */
    LASSERT_NUM("\\",a,2);
    LASSERT_TYPE("\\",a,0,LVAL_QEXPR);
    LASSERT_TYPE("\\",a,1,LVAL_QEXPR);

    /* Check first Q-Expression contains only Symbols */
    for(int i=0; i< a->cell[0]->count; i++){
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
            "Cannot define non-symbol. Got %s, Expected %s.",
            ltype_name(a->cell[0]->cell[i]->type),ltype_name(LVAL_SYM));
    }

    /* Pop first two arguments and pass them to lval_lambda */
    lval* formals = lval_pop(a,0);
    lval* body = lval_pop(a,0);
    
    lval_del(a);
    return lval_lambda(formals,body);
}

lval* builtin_fun(lenv* e, lval* a){
    /* Check Two arguments, each of which are Q-Expressions */
    LASSERT_NUM("fun",a,2);
    LASSERT_TYPE("fun",a,0,LVAL_QEXPR);
    LASSERT_TYPE("fun",a,1,LVAL_QEXPR);

    /* Check first Q-Expression contains only Symbols */
    for(int i=0; i< a->cell[0]->count; i++){
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
            "Cannot define non-symbol. Got %s, Expected %s.",
            ltype_name(a->cell[0]->cell[i]->type),ltype_name(LVAL_SYM));
    }
    
    lval* definition = lval_pop(a,0);
    lval* name = lval_pop(definition,0);
    lval* formals = definition;
    lval* body = lval_pop(a,0);

    lval* lambda = lval_lambda(formals,body);

    lval* temp1 = lval_qexpr();
    lval* temp2 = lval_sexpr();

    temp1 = lval_add(temp1,name);
    lval* function = lval_add(temp2,temp1);

    function = lval_add(function, lambda);

    lval_del(a);

    return builtin_var(e,function,"def");
}

lval* builtin_op(lenv* e, lval* a,char* op){
    /* Ensure all arguments are numbers */
    for(int i=0;i<a->count;i++){
        LASSERT_TYPE(op,a,i,LVAL_NUM);
    }

    /*Pop the first element*/
    lval* x = lval_pop(a,0);

    /*If no arguments and subexpressions perform unary negation*/
    if((strcmp(op,"-") == 0) && a->count == 0){
        x->num = -x->num;
    }

    /*While there are still elements remaining*/
    while(a->count > 0){
        /*Pop the next element*/
        lval* y = lval_pop(a,0);
        if(strcmp(op,"+")==0){ x->num += y->num;}
        if(strcmp(op,"-")==0){ x->num -= y->num;}
        if(strcmp(op,"*")==0){ x->num *= y->num;}
        if(strcmp(op,"%")==0){
            if(y->num==0){
                lval_del(x);lval_del(y);
                x = lval_err("Modulo By Zero!");break;
            }
             x->num = (long) x->num % (long) y->num;
        }
        if(strcmp(op,"/")==0){
            if(y->num==0){
                lval_del(x); lval_del(y);
                x = lval_err("Division By Zero!"); break;
            }
            x->num /= y->num;
        }
        lval_del(y);
    }
    
    lval_del(a); 
    return x;
}

lval* builtin_add(lenv* e, lval* a){
    return builtin_op(e,a,"+");
}

lval* builtin_sub(lenv* e, lval* a){
    return builtin_op(e,a,"-");
}

lval* builtin_mul(lenv* e, lval* a){
    return builtin_op(e,a,"*");
}

lval* builtin_div(lenv* e, lval* a){
    return builtin_op(e,a,"/");
}

lval* builtin_mod(lenv* e, lval* a){
    return builtin_op(e,a,"%");
}

lval* builtin_logic(lenv* e, lval* a, char* op){
    /* Ensure all arguments are numbers */
    for(int i=0;i<a->count;i++){
        LASSERT_TYPE(op,a,i,LVAL_NUM);
    }

    if ((strcmp(op,"!") == 0) && a->count != 0){
        LASSERT_NUM(op,a,1);
    }

    /*Pop the first element*/
    lval* x = lval_pop(a,0);

    /*If no arguments and subexpressions perform unary negation*/
    int r = x->num;

    if((strcmp(op,"!") == 0) && a->count == 0){
        r = !(r);
    }

    /*While there are still elements remaining*/
    while(a->count > 0){
        /*Pop the next element*/
        lval* y = lval_pop(a,0);
        if(strcmp(op,"&&")==0){ r = (r && y->num);}
        if(strcmp(op,"||")==0){ r = (r || y->num);}
        lval_del(y);
    }
    
    lval_del(x);
    lval_del(a); 
    return lval_num(r);
}

lval* builtin_and(lenv* e, lval* a){
    return builtin_logic(e,a,"&&");
}

lval* builtin_or(lenv* e, lval* a){
    return builtin_logic(e,a,"||");
}

lval* builtin_not(lenv* e, lval* a){
    return builtin_logic(e,a,"!");
}

lval* builtin_cmp(lenv* e, lval* a, char* op){
    LASSERT_NUM(op, a, 2);
    int r;
    if (strcmp(op,"==") == 0){
        r = lval_eq(a->cell[0], a->cell[1]);
    }
    if (strcmp(op,"!=") == 0){
        r = !lval_eq(a->cell[0], a->cell[1]);
    }
    lval_del(a);
    return lval_num(r);
}

lval* builtin_eq(lenv* e, lval* a){
    return builtin_cmp(e,a,"==");
}

lval* builtin_ne(lenv* e, lval* a){
    return builtin_cmp(e,a,"!=");
}

lval* builtin_ord(lenv* e,lval* a, char* op){
    /* Ensure all arguments are numbers */
    LASSERT_NUM(op,a,2);
    LASSERT_TYPE(op,a,0,LVAL_NUM);
    LASSERT_TYPE(op,a,1,LVAL_NUM);

    int r;
    if(strcmp(op,">")==0){ r = (a->cell[0]->num > a->cell[1]->num);}
    if(strcmp(op,"<")==0){ r = (a->cell[0]->num < a->cell[1]->num);}
    if(strcmp(op,">=")==0){ r = (a->cell[0]->num >= a->cell[1]->num);}
    if(strcmp(op,"<=")==0){ r = (a->cell[0]->num <= a->cell[1]->num);}
    
    lval_del(a); 
    return lval_num(r);
}

lval* builtin_gt(lenv* e, lval* a){
    return builtin_ord(e,a,">");
}

lval* builtin_lt(lenv* e, lval* a){
    return builtin_ord(e,a,"<");
}

lval* builtin_ge(lenv* e, lval* a){
    return builtin_ord(e,a,">=");
}

lval* builtin_le(lenv* e, lval* a){
    return builtin_ord(e,a,"<=");
}

lval* builtin_list(lenv* e, lval* a){
    a->type = LVAL_QEXPR;
    return a;
}

lval* builtin_head(lenv* e,lval* a){
    LASSERT_NUM("head",a,1);
    LASSERT_TYPE("head",a,0,LVAL_QEXPR);
    LASSERT(a,a->cell[0]->count != 0,
        "Function 'head' passed {}!");

    /* Otherwise take first argument */
    lval* v = lval_take(a,0);

    /* Delete all elements that are not head and return */
    while(v->count > 1) {lval_del(lval_pop(v,1));}
    return v;
}

lval* builtin_tail(lenv* e, lval* a){
    /* Check Error Conditions */
    LASSERT_NUM("tail",a,1);
    LASSERT_TYPE("tail",a,0,LVAL_QEXPR);
    LASSERT(a,a->cell[0]->count != 0,
        "Function 'tail' passed {}!");

    /* Take first argument */
    lval* v = lval_take(a,0);

    /* Delete first element and return */
    lval_del(lval_pop(v,0));
    return v;
}

lval* builtin_join(lenv* e, lval* a){
    for(int i = 0;i<a->count;i++){
        LASSERT_TYPE("join",a,i,LVAL_QEXPR);
    }

    lval* x = lval_pop(a,0);
    while(a->count){
        x = lval_join(x, lval_pop(a,0));
    }

    lval_del(a);
    return x;
}

lval* builtin_cons(lenv* e, lval* a){
    LASSERT_NUM("cons",a,2);

    LASSERT(a,a->cell[0]->type != LVAL_ERR,
        "Function 'cons' passed incorrect type!");

    LASSERT_TYPE("cons",a,1,LVAL_QEXPR);

    lval* x = lval_qexpr();
    x = lval_add(x,lval_pop(a,0));
    while(a->count){
        x = lval_join(x,lval_pop(a,0));
    }

    lval_del(a);
    return x;
}

lval* builtin_len(lenv* e, lval* a){
    LASSERT_NUM("len",a,1);
    LASSERT_TYPE("len",a,0,LVAL_QEXPR);

    long x = 0;
    x = lval_len(lval_pop(a,0));

    lval* y = lval_num(x);
    lval_del(a);
    return y;
}

lval* builtin_init(lenv* e, lval* a){
    LASSERT_NUM("init",a,1);
    LASSERT_TYPE("init",a,0,LVAL_QEXPR);
    LASSERT(a,a->cell[0]->count != 0,
    "Function 'init' passed {}!");

    lval* x = lval_qexpr();
    x = lval_init(x,lval_pop(a,0));

    lval_del(a);
    return x;
}

lval* builtin_eval(lenv* e, lval* a){
    LASSERT_NUM("eval",a,1)
    LASSERT_TYPE("eval",a, 0,LVAL_QEXPR)
    
    lval* x = lval_take(a,0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func){
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);
    lenv_put(e,k,v);
    lval_del(k);lval_del(v);
}

void lenv_add_builtins(lenv* e){
    /* List Functions */
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "cons", builtin_cons);
    lenv_add_builtin(e, "init", builtin_init);
    lenv_add_builtin(e, "len", builtin_len);
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "=", builtin_put);
    lenv_add_builtin(e, "\\", builtin_lambda);
    lenv_add_builtin(e, "fun", builtin_fun);

    /*Mathematical Functions*/
    lenv_add_builtin(e,"+",builtin_add);
    lenv_add_builtin(e,"-",builtin_sub);
    lenv_add_builtin(e,"*",builtin_mul);
    lenv_add_builtin(e,"/",builtin_div);
    lenv_add_builtin(e,"%",builtin_mod);

    /*Comparison Functions*/
    lenv_add_builtin(e, "if", builtin_if);
    lenv_add_builtin(e,"==",builtin_eq);
    lenv_add_builtin(e,"!=",builtin_ne);
    lenv_add_builtin(e,">",builtin_gt);
    lenv_add_builtin(e,"<",builtin_lt);
    lenv_add_builtin(e,">=",builtin_ge);
    lenv_add_builtin(e,"<=",builtin_le);

    /* Logical Functions */
    lenv_add_builtin(e,"&&",builtin_and);
    lenv_add_builtin(e,"||",builtin_or);
    lenv_add_builtin(e,"!",builtin_not);

}

lval* lval_eval_sexpr(lenv* e,lval* v){
    /*Evaluate Children*/
    for(int i =0;i<v->count;i++){
        v->cell[i] = lval_eval(e, v->cell[i]);
    }

    /* Error checking */
    for(int i=0;i<v->count;i++){
        if(v->cell[i]->type==LVAL_ERR){return lval_take(v,i);}
    }

    if(v->count==0){return v;}
    if(v->count==1){return lval_take(v,0);}

    /* Ensure First element is a function after evaluation */
    lval* f = lval_pop(v,0);
    if(f->type!=LVAL_FUN){
        lval* err = lval_err(
            "S-Expression starts with incorrect type. "
            "Got %s, Expected %s.",
            ltype_name(f->type), ltype_name(LVAL_FUN));
        lval_del(v);lval_del(f);
        return err;
    }

    /* If so call function to get result */
    lval* result = lval_call(e, f, v);
    lval_del(f);
    return result;
}

lval* lval_eval(lenv* e, lval* v){
    if(v->type == LVAL_SYM) {
        lval* x = lenv_get(e,v);
        lval_del(v);
        return x;
    }
    
    if(v->type == LVAL_SEXPR){ return lval_eval_sexpr(e, v);}
    return v;
}

int main(int argc, char** argv){
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* String = mpc_new("string");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                       \
        number : /-?[0-9]+/ ;                               \
        symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&|]+/;         \
        string : /\"(\\\\.|[^\"])*\"/;                      \
        sexpr  : '(' <expr>* ')' ;                          \
        qexpr  : '{' <expr>* '}' ;                          \
        expr   :  <number> | <symbol> | <string>            \
               | <sexpr> | <qexpr> ;                        \
        lispy  : /^/<expr>*/$/ ;                            \
    ",
    Number, Symbol, String, Sexpr, Qexpr, Expr, Lispy);

    lenv* e = lenv_new();
    lenv_add_builtins(e);

    puts("Lispy version 0.0.0.1.0"); 
    puts("Press Ctrl-c to Exit\n");

    int state =  1;

    while(state){

        mpc_result_t r;
        char* input  = readline("Input> ");
        add_history(input);

        if(strcmp(input,"exit")==0){
            state = 0;
            free(input);
            continue;
        }

        if(mpc_parse("<stdin>",input,Lispy,&r)){

            lval* x = lval_eval(e, lval_read(r.output));
            lval_println(x);
            lval_del(x);

            mpc_ast_delete(r.output);
        }else{
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    lenv_del(e);
    mpc_cleanup(7,Number, Symbol, String, Sexpr, Qexpr, Expr, Lispy);
    return 0;
}