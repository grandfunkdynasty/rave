/***************************************************************
* Lexer
***************************************************************/

%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
#include "../parse.h"

#ifndef LINUX
#define fileno _fileno
#undef strdup
#define strdup( x ) _strdup( x )
#define sprintf( x, y, z ) sprintf_s( x, 512, y, z )
#define strcpy( x, y ) strcpy_s( x, strlen( y ) + 1, y )
#define OPEN( t, x ) fopen_s( &t, x, "r" )
#else
#define OPEN( t, x ) ( ( t = fopen( x, "r" ) ) == 0 )
#endif

extern int yyserror( const char* text );
extern int yyerror( const char* text );

#define MAX_INCLUDE_DEPTH 128
YY_BUFFER_STATE include_includes[ MAX_INCLUDE_DEPTH ];
char* include_names[ MAX_INCLUDE_DEPTH ];
int include_lines[ MAX_INCLUDE_DEPTH ];
int include_depth = 0;
struct IncludeList* include_list = 0;

int yyline = 1;
char* yyname = 0;
%}

%option noyywrap
%x include
%x long_comment

/***************************************************************
* Regular expressions
***************************************************************/

ws          [ \t]+
comment     \/\/.*\n
begin_lc    \/\*
end_lc      \*\/
nl          (\n)|(\r)|(\r\n)
default     .|\n|\r

letter      [A-Za-z_]
digit       [0-9]

id          {letter}({letter}|{digit})*
integer     {digit}+

/***************************************************************
* Rules
***************************************************************/

%%

    /***************************************************************
    * Includes
    ***************************************************************/

<INITIAL>INCLUDE {
    BEGIN( include );
}

<include>[^ \t\n\r]+ {
    if ( include_depth >= MAX_INCLUDE_DEPTH )
        yyserror( "includes nested too deeply" );
    else {
        FILE* t = 0;
        struct IncludeList* list;
        int found = 0;
        int dir = 0;
        char* local = calloc( strlen( yyname ) + strlen( yytext ) + 8, sizeof( char ) );
        char* c;
        
        include_includes[ include_depth ] = YY_CURRENT_BUFFER;
        include_names[ include_depth ] = yyname;
        include_lines[ include_depth ] = yyline;

        strcpy( local, yyname );
        for ( c = local + strlen( yyname ) - 1; c >= local; --c ) {
            if ( *c == '/' || *c == '\\' ) {
                strcpy( c + 1, yyname );
                dir = 1;
                break;
            }
        }

        if ( dir == 0 || OPEN( t, local ) ) {
            if ( OPEN( t, yytext ) ) {
                char buf[ 512 ];
                sprintf( buf, "could not open included file '%s'", yytext );
                yyserror( buf );
            }
            else
                found = 2;
        }
        else
            found = 1;
    
        list = malloc( sizeof( struct IncludeList ) );
        list->file = strdup( found == 1 ? local : yytext );
        list->next = include_list;
        include_list = list;
        
        if ( found ) {
            ++include_depth;
            yyin = t;
            yyline = 1;
            yyname = strdup( found == 1 ? local : yytext );
            yy_switch_to_buffer( yy_create_buffer( yyin, YY_BUF_SIZE ) );
        }
        
        free( local );
    }
    BEGIN( INITIAL );
}

<<EOF>> {
    if ( --include_depth < 0 ) {
        yyterminate();
    }
    else {
        yy_delete_buffer( YY_CURRENT_BUFFER );
        free( yyname );
        yyline = include_lines[ include_depth ];
        yyname = include_names[ include_depth ];
        yy_switch_to_buffer( include_includes[ include_depth ] );
    }
}

    /***************************************************************
    * Comments and whitespace
    ***************************************************************/

<INITIAL>{begin_lc} {
    BEGIN( long_comment );
}

<long_comment>{end_lc} {
    BEGIN( INITIAL );
}

<long_comment>{nl} {
    ++yyline;
}

<long_comment>\* {
}

<long_comment>[^*\n\r]+ {
}

{ws} {
}

{comment}|{nl} {
    ++yyline;
}

    /***************************************************************
    * Keywords
    ***************************************************************/

video {
    return T_VIDEO;
}

sequence {
    return T_SEQUENCE;
}

function {
    return T_FUNCTION;
}

int {
    return T_INT;
}

float {
    return T_FLOAT;
}

else {
    return T_ELSE;
}

\.\. {
    return T_TO;
}

split {
    return T_SPLIT;
}

fx {
    return T_FX;
}

local {
	return T_LOCAL;
}

cache {
    return T_CACHE;
}

layer {
    return T_LAYER;
}

copy {
    return T_COPY;
}

base {
	return T_BASE;
}

merge {
    return T_MERGE;
}

use {
    return T_USE;
}

    /***************************************************************
    * Operators
    ***************************************************************/

"?" {
    return T_TERNARY_OP_0;
}

"||" {
    yylval.type = BINARY_OP_OR;
    return T_BINARY_OP_0;
}

"&&" {
    yylval.type = BINARY_OP_AND;
    return T_BINARY_OP_1;
}

"==" {
    yylval.type = BINARY_OP_EQ;
    return T_BINARY_OP_2;
}

"!=" {
    yylval.type = BINARY_OP_NE;
    return T_BINARY_OP_2;
}

"<~>" {
    yylval.type = TYPE_OP_EQ;
    return T_TYPE_OP;
}

"<~" {
    yylval.type = TYPE_OP_FROM;
    return T_TYPE_OP;
}

"~>" {
    yylval.type = TYPE_OP_TO;
    return T_TYPE_OP;
}

">" {
    yylval.type = BINARY_OP_GT;
    return T_BINARY_OP_3;
}

">=" {
    yylval.type = BINARY_OP_GE;
    return T_BINARY_OP_3;
}

"<" {
    yylval.type = BINARY_OP_LT;
    return T_BINARY_OP_3;
}

"<=" {
    yylval.type = BINARY_OP_LE;
    return T_BINARY_OP_3;
}

"|" {
    yylval.type = BINARY_OP_BIT_OR;
    return T_BINARY_OP_4;
}

"'" {
    yylval.type = BINARY_OP_BIT_XOR;
    return T_BINARY_OP_5;
}

"&" {
    yylval.type = BINARY_OP_BIT_AND;
    return T_BINARY_OP_6;
}

"<<" {
    yylval.type = BINARY_OP_LSHIFT;
    return T_BINARY_OP_7;
}

">>" {
    yylval.type = BINARY_OP_RSHIFT;
    return T_BINARY_OP_7;
}

"^" {
    yylval.type = BINARY_OP_EXP;
    return T_BINARY_OP_10;
}

    /***************************************************************
    * Other
    ***************************************************************/

{id} {
    yylval.string = strdup( yytext );
	return T_ID;
}

{integer} {
    yylval.integer = atol( yytext );
    return T_INT_LITERAL;
}

\.{integer} {
    yylval.number = atof( yytext );
    return T_FLOAT_LITERAL;
}

({integer}\.{integer}) {
    yylval.number = atof( yytext );
	return T_FLOAT_LITERAL;
}

{integer}\./[^\.] {
    yylval.number = atof( yytext );
	return T_FLOAT_LITERAL;
}

{integer}\./(\.\.) {
    yylval.number = atof( yytext );
	return T_FLOAT_LITERAL;
}

{default} {
    #undef  ECHO
    #define ECHO
    return yytext[ 0 ];
}

    /***************************************************************
    * End of rules
    ***************************************************************/
