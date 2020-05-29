/****************************************************/
/* File: auto.flex                                  */
/* The auto Lex/Flex    specification file          */
/****************************************************/

%{
#include "globals.h"
// #include "util.h"
// #include "scan.h"

#define MAXTOKENLEN 100
char tokenString[MAXTOKENLEN+1];
char IDname[MAXTOKENLEN+1];
int numvalue;
double floatvalue;

%}

digit       [0-9]
letter      [a-zA-Z_]
identifier  {letter}+({letter}|{digit})*
intnum      -?{digit}+
floatnum    -?{digit}+(\.{digit}+)(e[+-]?{digit}+)?
newline     \n
return      \r\n
whitespace  [ \t]+



%%
{floatnum}      {return FLOATV;}
{intnum}        {return INTV;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{return}        {lineno++;}
{whitespace}    {/* skip whitespace */}
"#".*           {/* skip annotation */}
"="             {return ASSIGN;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"//"            {return FLOORDIV;}
"/"             {return DIV;}
"%"             {return MOD;}
"("             {return LPAREN; /* left parenthesis */}
")"             {return RPAREN;}
"<"             {return LBRACK; /* left brace */}
">"             {return RBRACK;}
";"             {return SEMI; /* semi-colon */}
","             {return COMMA;}
.               {return ERROR;}

%%

// int main()
// {
//     TokenType a;
//     while(a = yylex())
//     {
//         printf("%d\n",a);
//     } 
//     return 0;
// }

TokenType getToken(void)
{
    static int firstTime = TRUE;
    TokenType currentToken;
    if (firstTime)
    {
        firstTime = FALSE;
        lineno++;
        yyin = source;
        yyout = listing;
    }
    currentToken = yylex();
    // printf("gettoken %d\n",currentToken);
    strncpy(tokenString, yytext, MAXTOKENLEN);
    if(currentToken==ID)
        strncpy(IDname, yytext, MAXTOKENLEN);
    if(currentToken==INTV)
        numvalue=atoi(tokenString);
    if(currentToken==FLOATV)
        floatvalue = atof(tokenString);
    // printf("token is %s\n",tokenString);
    // for (int i=0;i<MAXTOKENLEN; i++)
    //     printf("%d ", tokenString[i]);
    // printf("\n");
    // if (TraceScan)
    // {
    //     fprintf(listing, "\t%d: ", lineno);
    //     printToken(currentToken, tokenString);
    // }
    return currentToken;
}

