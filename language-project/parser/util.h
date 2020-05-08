/****************************************************/
/* File: util.h                                     */
/* Utility functions                                */
/****************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken( TokenType);

/* Function newNode creates a new statement
 * node for syntax tree construction
 */
TreeNode *newNode(NodeKind kind);

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString( char * );

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * );

void PrintAll(TreeNode* root) ;

void pushname( char *);

char * popname ();

#endif
