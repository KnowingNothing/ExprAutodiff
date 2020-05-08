/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/****************************************************/

#include "globals.h"
#include "util.h"

#define MAXSAVENUM 1000



TreeNode *newNode(NodeKind kind)
{
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else
    {
        for (i = 0; i < MAXCHILDREN; i++)
            t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = kind;
        t->lineno = lineno;
    }
    return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char *copyString(char *s)
{
    int n;
    char *t;
    if (s == NULL)
        return NULL;
    n = strlen(s) + 1;
    t = (char*) malloc(n);
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else
        strcpy(t, s);
    return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno += 3
#define UNINDENT indentno -= 3

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{
    int i;
    for (i = 0; i < indentno; i++)
    {
        if (i % 3 == 0)
            fprintf(listing, "|");
        else
            fprintf(listing, " ");
    }
}


/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken(TokenType token)
{
    switch (token)
    {
    case PLUS:
        fprintf(listing, "+\n");
        break;
    case MINUS:
        fprintf(listing, "-\n");
        break;
    case TIMES:
        fprintf(listing, "*\n");
        break;
    case DIV:
        fprintf(listing, "/\n");
        break;
    case FLOORDIV:
        fprintf(listing, "//\n");
        break;
    case MOD:
        fprintf(listing, "%%\n");
        break;
    default: /* should never happen */
        fprintf(listing, "Unknown token: %d\n", token);
    }
}


/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */

void printTree(TreeNode *tree)
{
    int i;
    INDENT;
    while (tree != NULL)
    {
        printSpaces();
            switch (tree->nodekind)
            {
            case StmtK:
                fprintf(listing, "Stmt\n");
                break;
            case TrefK:
                fprintf(listing, "Tref\n");
                break;
            case ArefK:
                fprintf(listing, "Sref\n");
                break;
            case RhsK:
                fprintf(listing, "Rhs ");
                printToken(tree->op);
                break;
            case IdK:
                fprintf(listing, "Id: %s\n", tree->name);
                break;
            case IdexprK:
                fprintf(listing, "Idexpr ");
                printToken(tree->op);
                break;
            case IntvK:
                fprintf(listing, "Int: %d\n", tree->val);
                break;
            case FloatvK:
                fprintf(listing, "Float: %lf\n", tree->fval);
                break;
            default:
                fprintf(listing, "Unknown ExpNode kind\n");
                break;
            }
        for (i = 0; i < MAXCHILDREN; i++)
        {
            printTree(tree->child[i]);
        }
        tree = tree->sibling;
    }
    UNINDENT;
}


// char *savename[1000];
// int savenum = 0;


// void pushname(char *idname)
// {
//     savename[savenum++] = copyString(idname);
// }

// char *popname()
// {
//     return savename[--savenum];
// }


