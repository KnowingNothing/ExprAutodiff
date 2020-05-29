/****************************************************/
/* File: auto.bison                                 */
/* The auto Yacc/Bison specification file           */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */
#define YYERROR_VERBOSE
#define YYDEBUG 1
// int yydebug = 1;
#include "globals_.h"
#include "util.h"
// #include "genIR.h"

#define YYSTYPE TreeNode *
static int savedLineNo;  
static TreeNode * savedTree; /* stores syntax tree for later return */
int lineno = 0;
int Error = FALSE;
FILE* source; /* source code text file */
FILE* listing ;/* send listing to screen */
static int yylex(void);

#define MAXTOKENLEN 100
/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN + 1];

extern char IDname[MAXTOKENLEN + 1];

extern int numvalue;

extern double floatvalue;

TokenType getToken(void);



%}

%token ID INTV FLOATV
%token ASSIGN PLUS MINUS TIMES DIV FLOORDIV MOD
%token LPAREN RPAREN LBRACK RBRACK SEMI COMMA
%token ERROR 

%left PLUS MINUS
%left TIMES DIV FLOORDIV MOD

%% /* Grammar for auto */

program     : stmt
                {savedTree = $1;}
            | program stmt
                {
                    YYSTYPE t = $1;
                    while (t->sibling != NULL)
                        t = t->sibling;
                    t->sibling = $2;
                    $$ = $1; 
                } 

stmt        : tref ASSIGN rhs SEMI
                {
                    $$ = newNode(StmtK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                }


rhs         : rhs PLUS  rhs
                {
                    $$ = newNode(RhsK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = PLUS;
                }
            | rhs TIMES rhs
                {
                    $$ = newNode(RhsK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = TIMES;
                }
            | rhs MINUS rhs
                {
                    $$ = newNode(RhsK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = MINUS;
                }
            | rhs DIV rhs
                {
                    $$ = newNode(RhsK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = DIV;
                }
            | rhs MOD rhs
                {
                    $$ = newNode(RhsK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = MOD;
                }
            | rhs FLOORDIV  rhs
                {
                    $$ = newNode(RhsK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = FLOORDIV;
                }
            | assist_ref
                {$$ = $1;}
            | tref
                {$$ = $1;}
            | const
                {$$ = $1;}

tref        : assist_ref LPAREN alist RPAREN
                {
                    $$ = newNode(TrefK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                }

assist_ref  : ID LBRACK clist RBRACK
                {
                    $$ = newNode(ArefK);
                    $$->child[0] = newNode(IdK);
                    $$->child[0]->name = copyString(IDname);
                    $$->child[1] = $3;
                }

clist       : intv {$$ = $1;}
            | clist COMMA intv
                {
                    YYSTYPE t = $1;
                    while (t->sibling != NULL)
                        t = t->sibling;
                    t->sibling = $3;
                    $$ = $1; 
                }

alist       : idexp {$$ = $1;}
            | alist COMMA idexp
                {
                    YYSTYPE t = $1;
                    while (t->sibling != NULL)
                        t = t->sibling;
                    t->sibling = $3;
                    $$ = $1; 
                }

idexp       : ID
                {
                    $$ = newNode(IdK);
                    $$->name = copyString(IDname);
                }
            | idexp PLUS idexp
                {
                    $$ = newNode(IdexprK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = PLUS;
                }
            | idexp PLUS intv
                {
                    $$ = newNode(IdexprK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = PLUS;
                }
            | idexp MINUS intv
                {
                    $$ = newNode(IdexprK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = MINUS;
                }
            | idexp TIMES intv
                {
                    $$ = newNode(IdexprK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = TIMES;
                }
            | idexp FLOORDIV intv
                {
                    $$ = newNode(IdexprK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = FLOORDIV;
                }
            | idexp MOD intv
                {
                    $$ = newNode(IdexprK);
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                    $$->op = MOD;
                }

intv        : INTV
                {
                    $$ = newNode(IntvK);
                    $$->val = numvalue;
                }

const       : intv{$$ = $1;}
            | FLOATV
                {
                    $$ = newNode(FloatvK);
                    $$->fval = floatvalue;
                }


%%

int yyerror(char *message)
{
    fprintf(listing, "Syntax error at line %d: %s\n", lineno, message);
    fprintf(listing, "Current token: ");
    fprintf(listing,"%d %s\n",yychar,tokenString);
    Error = TRUE;
    exit(0);
    return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the auto scanner
 */
static int yylex(void)
{
    int a = getToken();
    return a;
}

TreeNode *parse(char* name)
{
    if (name)
    { 
        source = fopen(name, "r");
        if (!source)
        {
            fprintf(stderr, "could not open %s\n", name);
            exit(1);
        }
    }
    else
        source = stdin;

    listing = stdout;

    yyparse();
    return savedTree;
}