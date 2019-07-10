/*

Linux
cc -std=c99 -Wall -ggdb prompt.c mpc.c -ledit -lm -o prompt
Windows
cc -std=c99 -Wall -ggdb prompts.c mpc.c -prompts

Pesara Amarasekera
2019-07-09

In this file Quoted Expressions will be implemented
*/
#include "mpc.h"
#ifdef _WIN32
    #include <string.h>

    static char buffer[2048];

    char* readline(char* prompt){
        fputs(prompt,stdout);
        fgets(buffer,2048,stdin);
        char* cpy = malloc(strlen(buffer)+1);
        strcpy(cpy,buffer);
        cpy[strlen(buffer)] ='\0';
        return cpy;
    }

    char* add_history(char* unused) {}
#else
    #include <editline/readline.h>
    #include <editline/history.h>
#endif

enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR };

typedef struct lval{
    int type;
    long num;
    /* Error and Symbol types have some string data */
    char* err;
    char* sym;
    /* Count and Pointer to a list of "lval" */
    int count;
    struct lval** cell;
} lval;

/*  Construct a pointer to a new Number lval */
lval* lval_num(long x){
    lval* v  = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

/* Construct a pointer to a new Error lval */
lval* lval_err(char* m){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m)+1);
    strcpy(v->err,m);
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

lval* lval_sexpr(void){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}
/* A pointer to a new empty Qexpr lval */
lval* lval_qexpr(void){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void lval_del(lval* v){
    switch (v->type){
        case LVAL_NUM: break;
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;

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
    return errno!= ERANGE ? 
        lval_num(x) : lval_err("invalid number");
}

void lval_print(lval* v){
    switch (v->type) {
        case LVAL_NUM: printf("%li",v->num); break;
        case LVAL_ERR: printf("Error: %s",v->err); break;
        case LVAL_SYM: printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_expr_print(v,'(',')'); break;
        case LVAL_QEXPR: lval_expr_print(v,'{','}'); break;
    }
}



lval* lval_read(mpc_ast_t* t){
    /*If Symbol or Number return conversion to that type*/
    if(strstr(t->tag,"number")) {return lval_read_num(t);}
    if(strstr(t->tag,"symbol")) {return lval_sym(t->contents);}

    /*If root(>) or sexpr then create empty list*/
    lval* x = NULL;
    if(strcmp(t->tag,">") == 0) { x = lval_sexpr();}
    if(strstr(t->tag,"sexpr"))  { x = lval_sexpr();}

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

int main(int argc, char** argv){
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                       \
        number : /-?[0-9]+/ ;                               \
        symbol : '+'|'-'|'*'|'/'|'%' ;                      \
        sexpr  : '(' <expr>* ')' ;                          \
        qexpr  : '{' <expr>* '}' ;                          \
        expr   :  <number> | <symbol> | <sexpr> | <qexpr> ; \
        lispy  : /^/<expr>*/$/ ;                            \
    ",
    Number, Symbol, Sexpr, Qexpr, Expr, Lispy);


    mpc_cleanup(6,Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
    return 0;
}