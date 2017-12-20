%{
// $Id: lexer.l,v 1.9 2014-10-09 18:39:47-07 - - $

#include "lyutils.h"

#define YY_USER_ACTION  { lexer::advance(); }

%}

%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn

LETTER          [a-zA-Z_]
DIGIT           [0-9]
IDENT           {LETTER}({LETTER}|{DIGIT})*
MANTISSA        {DIGIT}+\.?{DIGIT}*|\.{DIGIT}+
EXPONENT        [Ee][+-]?{DIGIT}+
NUMBER          ({MANTISSA})({EXPONENT})?
NOTNUMBER       ({MANTISSA}[Ee][+-]?)
INTEGER         ({DIGIT}+)
CHAR            ('([^\\'\n]|\\[\\'"0nt])')
STRING          (\"([^\\"\n]|\\[\\'"0nt])*\")

%%

"return"        { return yylval_token (TOK_RETURN); }
"string"        { return yylval_token (TOK_STRING); }
"struct"        { return yylval_token (TOK_STRUCT); }
"if"            { return yylval_token (TOK_IF); }
"else"          { return yylval_token (TOK_ELSE); }
"while"         { return yylval_token (TOK_WHILE); }
"null"          { return yylval_token (TOK_NULL); }
"new"           { return yylval_token (TOK_NEW); }
"void"          { return yylval_token (TOK_VOID); }
"char"          { return yylval_token (TOK_CHAR); }
"int"           { return yylval_token (TOK_INT); }
"bool"          { return yylval_token (TOK_BOOL); }

"!="            { return yylval_token (TOK_NE); }
"<="            { return yylval_token (TOK_LE); }
"=="            { return yylval_token (TOK_EQ); }
">="            { return yylval_token (TOK_GE); }

{IDENT}         { return yylval_token (TOK_IDENT); }
{CHAR}          { return yylval_token (TOK_CHARCON); }
{STRING}        { return yylval_token (TOK_STRINGCON); }
{INTEGER}       { return yylval_token (TOK_INTCON); }

"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"%"             { return yylval_token ('%'); }            
"!"             { return yylval_token ('!'); }
"="             { return yylval_token ('='); }
"<"             { return yylval_token (TOK_LT); }
">"             { return yylval_token (TOK_GT); }
","             { return yylval_token (','); }
"."             { return yylval_token ('.'); }
";"             { return yylval_token (';'); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }
"[]"            { return yylval_token (TOK_ARRAY); }

\n              { lexer::newline(); }
[ \t]+          { /* Skip whitespace. */ }
"#".*           { lexer::include(); }

{NOTNUMBER}     { lexer::badtoken (yytext);
                  return yylval_token (NUMBER); }
.               { lexer::badchar (*yytext); }

%%

