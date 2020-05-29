/****************************************************/
/* File: globals.h                                  */
/* to supprot the parsing process                   */
/****************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


#ifdef PARSER
    typedef int TokenType;
#else
    typedef enum 

   {ENDFILE = 256,ERROR,
    ID,INTV,FLOATV,
    ASSIGN,PLUS,MINUS,TIMES,DIV,FLOORDIV,MOD,
    LPAREN,RPAREN,LBRACK,RBRACK,SEMI,COMMA
   } TokenType;
#endif


extern FILE* source; /* source code text file */
extern FILE* listing; /* listing output text file */
extern FILE* code; /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

// /**************************************************/
// /***********   Syntax tree for parsing ************/
// /**************************************************/

typedef enum {StmtK,RhsK,TrefK,ArefK,IdK,IdexprK,IntvK,FloatvK} NodeKind;

/* ExpType is used for type checking */
typedef enum {Void,Integer,Boolean} ExpType;

#define MAXCHILDREN 3

typedef struct treeNode
   { struct treeNode * child[MAXCHILDREN];
     struct treeNode * sibling;
     int lineno;
     NodeKind nodekind;
     TokenType op;
     int val;
     double fval;
     char * name;
   } TreeNode;

#endif
