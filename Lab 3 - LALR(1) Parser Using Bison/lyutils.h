//Davie Truong
//dtruong8@ucsc.edu
//1524861

// $Id: lyutils.h,v 1.11 2017-10-11 14:19:04-07 - - $

#ifndef __UTILS_H__
#define __UTILS_H__

// Lex and Bison interface utility.

#include <string>
#include <vector>
using namespace std;

#include <stdio.h>

#include "astree.h"
#include "auxlib.h"

#define YYEOF 0

extern FILE *tok_file;
extern FILE *ast_file;

extern int depth;

extern FILE* yyin;
extern char* yytext;          // Is what is going to be passed into astree
                              // points to the buffer/identifier 
extern int yy_flex_debug;     
extern int yydebug;
extern size_t yyleng; 

int yylex();                  // Returns a token code, element of vt
int yylex_destroy();
int yyparse();                // The parser not called til asgn 3
void yyerror (const char* message);

int yylval_token (int symbol);   // returns symbol and fills tok_file with data
astree* new_parseroot();

struct lexer {
   static bool interactive;
   static location lloc;
   static size_t last_yyleng;
   static vector<string> filenames;
   static const string* filename (int filenr);
   static void newfilename (const string& filename);
   static void advance();                             // Move ahead
   static void newline();                             // Called when recognize a newline character
   static void badchar (unsigned char bad);
   static void badtoken (char* lexeme);
   static void include();
};

struct parser {
   static astree* root;
   static const char* get_tname (int symbol);
};

//#define YYSTYPE_IS_DECLARED
//typedef astree* YYSTYPE;              // The type of the look ahead symbol
#define YYSTYPE astree*
#include "yyparse.h"                  // Generated by Bison

#endif

