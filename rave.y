/***************************************************************
* Parser
***************************************************************/

%{
#include "../parse.h"
#include <stdio.h>
#include <string.h>

extern int yylex();
extern int yyline;
extern const char* yyname;
extern void write_error( const char* name, int line, const char* next, const char* text, int unexpected );
extern char* yytext;
struct Node* parse_tree;

int yyserror( const char* text )
{
    return 0;
}

int yyerror( const char* text )
{
    write_error( yyname, yyline, yytext, text, 1 );
    return 0;
}

struct Node* error( const char* text )
{
    return (struct Node*)yyerror( text );
}
%}

%union {
    long   integer;
    double number;
    char*  string;
    int    type;
    
    struct Node* node;
}

/***************************************************************
* Tokens
***************************************************************/

%token              T_VIDEO
%token              T_SEQUENCE
%token              T_FUNCTION
%token              T_INT
%token              T_FLOAT
%token              T_ELSE
%token              T_TO
%token              T_SPLIT
%token              T_FX
%token              T_LOCAL
%token              T_CACHE
%token              T_LAYER
%token              T_COPY
%token              T_BASE
%token              T_MERGE
%token              T_USE
%token              T_TERNARY_OP_0
%token <type>       T_TERNARY_OP_1
%token <type>       T_BINARY_OP_0
%token <type>       T_BINARY_OP_1
%token <type>       T_BINARY_OP_2
%token <type>       T_BINARY_OP_3
%token <type>       T_BINARY_OP_4
%token <type>       T_BINARY_OP_5
%token <type>       T_BINARY_OP_6
%token <type>       T_BINARY_OP_7
%token <type>       T_BINARY_OP_8
%token <type>       T_BINARY_OP_9
%token <type>       T_BINARY_OP_10
%token <type>       T_BINARY_OP_11
%token <type>       T_UNARY_OP
%token <type>       T_TYPE_OP
%token <integer>    T_INT_LITERAL
%token <number>     T_FLOAT_LITERAL
%token <string>     T_ID

/***************************************************************
* Precedence
***************************************************************/

%left               T_TERNARY_OP_0              /* ?: */
%left               T_BINARY_OP_0               /* || */
%left               T_BINARY_OP_1               /* && */
%left               T_BINARY_OP_2               /* == != */
%left               T_TYPE_OP                   /* <~ <~> ~> */
%left               T_BINARY_OP_3               /* < > <= >= */
%left               T_BINARY_OP_4               /* | */
%left               T_BINARY_OP_5               /* ' */
%left               T_BINARY_OP_6               /* & */
%left               T_BINARY_OP_7               /* << >> */
%left               T_BINARY_OP_8 '+' '-'       /* + - */
%left               T_BINARY_OP_9 '/' '*' '%'   /* * / % */
%right              T_UNARY_OP                  /* - ! \ */
%right              T_BINARY_OP_10              /* ^ */
%right              T_BINARY_OP_11 '['          /* [] */
%right              T_TERNARY_OP_1              /* [=] */
%left               T_FUNCTION_OP '('           /* () */

/***************************************************************
* Types
***************************************************************/

%type <integer>     modifier_list modifier layer_type
%type <node>        type_args type_args_list type expr t_expr expr_list body body_list
%type <node>        layer layer_optional_expr layer_optional_fx layer_list statement o_statement c_statement statement_list scope_def_list scope_def
%type <node>        argument_def argument_list func_def seq_def vid_def type_def program_scope program
%start program

%%

/***************************************************************
* Types
***************************************************************/

type_args : '(' ')'                                                 { $$ = alloc_node( NODE_UNDEFINED, 0 ); }
          | '(' type_args_list ')'                                  { $$ = $2; }
          ;
          
type_args_list : type type_args_optional_id                         { $$ = alloc_node( NODE_UNDEFINED, 0 );
                                                                      push_back( $$, $1 ); }
               | type_args_list ',' type type_args_optional_id      { $$ = $1;
                                                                      push_back( $$, $3 ); }
               ;
               
type_args_optional_id : T_ID
                      |
                      ;
                      
type : T_INT                                                        { $$ = alloc_node( NODE_TYPE, TYPE_INT ); }
     | T_FLOAT                                                      { $$ = alloc_node( NODE_TYPE, TYPE_FLOAT ); }
     | type T_FUNCTION type_args                                    { $$ = $3;
                                                                      set_type( $$, NODE_TYPE, TYPE_FUNCTION );
                                                                      push_front( $$, $1 ); }
     | T_SEQUENCE type_args                                         { $$ = $2;
                                                                      set_type( $$, NODE_TYPE, TYPE_SEQUENCE ); }
     | '(' type type_args_optional_id ',' type_args_list ')'        { $$ = $5;
                                                                      set_type( $$, NODE_TYPE, TYPE_TUPLE );
                                                                      push_front( $$, $2 ); }
     | '~' T_ID                                                     { $$ = alloc_node( NODE_TYPE, TYPE_TYPEDEF );
                                                                      $$->string_data = $2; }
                                                                      
/***************************************************************
* Expressions
***************************************************************/

expr : t_expr T_TERNARY_OP_0 expr ':' expr                          { $$ = alloc_node( NODE_TERNARY_OP, 0 );
                                                                      push_back( $$, $1 );
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 ); }
     | t_expr;
                                                                      
t_expr : T_INT_LITERAL                                              { $$ = alloc_node( NODE_LITERAL, LITERAL_INT );
                                                                      $$->int_data = $1; }
       | T_FLOAT_LITERAL                                            { $$ = alloc_node( NODE_LITERAL, LITERAL_FLOAT );
                                                                      $$->float_data = $1; }
       | T_ID                                                       { $$ = alloc_node( NODE_IDENTIFIER, 0 );
                                                                      $$->string_data = $1; }
       | t_expr T_BINARY_OP_0 t_expr                                { $$ = alloc_binary_node( $2, $1, $3 ); }
       | t_expr T_BINARY_OP_1 t_expr                                { $$ = alloc_binary_node( $2, $1, $3 ); }
       | t_expr T_BINARY_OP_2 t_expr                                { $$ = alloc_binary_node( $2, $1, $3 ); }
       | t_expr T_BINARY_OP_3 t_expr                                { $$ = alloc_binary_node( $2, $1, $3 ); }
       | t_expr T_BINARY_OP_4 t_expr                                { $$ = alloc_binary_node( $2, $1, $3 ); }
       | t_expr T_BINARY_OP_5 t_expr                                { $$ = alloc_binary_node( $2, $1, $3 ); }
       | t_expr T_BINARY_OP_6 t_expr                                { $$ = alloc_binary_node( $2, $1, $3 ); }
       | t_expr T_BINARY_OP_7 t_expr                                { $$ = alloc_binary_node( $2, $1, $3 ); }
       | t_expr '+' t_expr                                          { $$ = alloc_binary_node( BINARY_OP_ADD, $1, $3 ); }
       | t_expr '-' t_expr                                          { $$ = alloc_binary_node( BINARY_OP_SUB, $1, $3 ); }
       | t_expr '/' t_expr                                          { $$ = alloc_binary_node( BINARY_OP_DIV, $1, $3 ); }
       | t_expr '*' t_expr                                          { $$ = alloc_binary_node( BINARY_OP_MUL, $1, $3 ); }
       | t_expr '%' t_expr                                          { $$ = alloc_binary_node( BINARY_OP_MOD, $1, $3 ); }
       | t_expr T_BINARY_OP_10 t_expr                               { $$ = alloc_binary_node( $2, $1, $3 ); }
       | t_expr '[' T_INT_LITERAL ']'                               { $$ = alloc_node( NODE_TUPLE_EXTRACT, 0 );
                                                                      push_back( $$, $1 );
                                                                      $$->int_data = $3; }
       | T_UNARY_OP t_expr                                          { $$ = alloc_node( NODE_UNARY_OP, $1 );
                                                                      push_back( $$, $2 ); }
       | '-' t_expr %prec T_UNARY_OP                                { $$ = alloc_node( NODE_UNARY_OP, UNARY_OP_NEGATION );
                                                                      push_back( $$, $2 ); }
       | '[' expr ']' %prec T_UNARY_OP                              { $$ = alloc_node( NODE_UNARY_OP, UNARY_OP_FLOOR );
                                                                      push_back( $$, $2 ); }
       | t_expr '[' T_INT_LITERAL '/' expr ']'                      { $$ = alloc_node( NODE_TUPLE_REPLACE, 0 );
                                                                      push_back( $$, $1 );
                                                                      $$->int_data = $3;
                                                                      push_back( $$, $5 ); }
       | type T_TYPE_OP type                                        { $$ = alloc_typeop_node( $2, $1, $3 ); }
       | t_expr T_TYPE_OP type                                      { $$ = alloc_typeop_node( $2, $1, $3 ); }
       | type T_TYPE_OP t_expr                                      { $$ = alloc_typeop_node( $2, $1, $3 ); }
       | t_expr T_TYPE_OP t_expr                                    { $$ = alloc_typeop_node( $2, $1, $3 ); }
       | '(' expr ',' expr_list ')'                                 { $$ = $4;
                                                                      set_type( $$, NODE_TUPLE_CONSTRUCT, 0 );
                                                                      push_front( $$, $2 ); }
       | t_expr '(' ')'                                             { $$ = alloc_node( NODE_FUNCTION_CALL, 0 );
                                                                      push_front( $$, $1 ); }
       | t_expr '(' expr_list ')'                                   { $$ = $3;
                                                                      set_type( $$, NODE_FUNCTION_CALL, 0 );
                                                                      push_front( $$, $1 ); }
       | '(' expr ')'                                               { $$ = $2; }
       ;
       
expr_list : expr                                                    { $$ = alloc_node( NODE_UNDEFINED, 0 );
                                                                      push_back( $$, $1 ); }
          | expr_list ',' expr                                      { $$ = $1;
                                                                      push_back( $$, $3 ); }
          ;
          
/***************************************************************
* Functions
***************************************************************/
     
body : '{' body_list '}'                                            { $$ = $2;
                                                                      set_type( $$, NODE_BODY, 0 ); }
     | expr ':' body                                                { $$ = alloc_node( NODE_GUARD, 0 );
                                                                      push_back( $$, $1 );
                                                                      push_back( $$, $3 ); }
     | T_ID '=' expr ':' body                                       { $$ = alloc_node( NODE_LET, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 ); }
     | expr ';'                                                     { $$ = alloc_node( NODE_RETURN, 0 );
                                                                      push_back( $$, $1 ); }
     ;
     
body_list : body_list body                                          { $$ = $1;
                                                                      push_back( $$, $2 ); }
          |                                                         { $$ = alloc_node( NODE_UNDEFINED, 0 ); }
          ;
     
/***************************************************************
* Layers
***************************************************************/

layer_type : T_LAYER                                                { $$ = LAYER_LAYER; }
           | T_COPY                                                 { $$ = LAYER_COPY; }
           | T_BASE                                                 { $$ = LAYER_BASE; }
           ;
           
layer_optional_expr : expr
                    |                                               { $$ = 0; }
                    ;
                    
layer_optional_fx : T_MERGE expr                                    { $$ = $2; }
                  |                                                 { $$ = 0; }
                  ;

layer : layer_type layer_optional_expr layer_optional_fx ':'
        statement                                                   { $$ = alloc_node( NODE_LAYER, $1 | ( $3 ? LAYER_FX : 0 ) );
                                                                      if ( $2 )
                                                                          push_back( $$, $2 );
                                                                      if ( $3 )
                                                                          push_back( $$, $3 );
                                                                      push_back( $$, $5 ); }

layer_list : layer_list layer                                       { $$ = $1;
                                                                      push_back( $$, $2 ); }
           |                                                        { $$ = alloc_node( NODE_UNDEFINED, 0 ); }
           ;
           
/***************************************************************
* Statements
***************************************************************/
           
statement : o_statement
          | c_statement
          ;

o_statement : expr ':' statement                                    { $$ = alloc_node( NODE_GUARD, 0 );
                                                                      push_back( $$, $1 );
                                                                      push_back( $$, $3 ); }
            | expr ':' c_statement T_ELSE ':' o_statement           { $$ = alloc_node( NODE_GUARD, 0 );
                                                                      push_back( $$, $1 );
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $6 ); }
            | T_ID '=' expr T_TO expr ':' o_statement               { $$ = alloc_node( NODE_LOOP, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 );
                                                                      push_back( $$, $7 ); }
            | T_ID '=' expr ':' o_statement                         { $$ = alloc_node( NODE_LET, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 ); }
            ;
                                                                      
c_statement : '{' scope_def_list statement_list '}'                 { $$ = $3;
                                                                      set_type( $2, NODE_SCOPE_SET, 0 );
                                                                      set_type( $$, NODE_BLOCK, 0 );
                                                                      push_front( $$, $2 ); }
            | expr ':' c_statement T_ELSE ':' c_statement           { $$ = alloc_node( NODE_GUARD, 0 );
                                                                      push_back( $$, $1 );
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $6 ); }
            | T_ID '=' expr T_TO expr ':' c_statement               { $$ = alloc_node( NODE_LOOP, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 );
                                                                      push_back( $$, $7 ); }
            | T_ID '=' expr ':' c_statement                         { $$ = alloc_node( NODE_LET, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 ); }
            | t_expr '(' ')' ';'                                    { $$ = alloc_node( NODE_SEQUENCE_CALL, 0 );
                                                                      push_front( $$, $1 ); }
            | t_expr '(' expr_list ')' ';'                          { $$ = $3;
                                                                      set_type( $$, NODE_SEQUENCE_CALL, 0 );
                                                                      push_front( $$, $1 ); }
            | T_FX ':' expr ';'                                     { $$ = alloc_node( NODE_FX, 0 );
                                                                      push_back( $$, $3 ); }
            | T_SPLIT ':' '{' layer_list '}'                        { $$ = $4;
                                                                      set_type( $$, NODE_SPLIT, 0 ); }
            | ';'                                                   { $$ = alloc_node( NODE_BLOCK, 0 );
                                                                      push_back( $$, alloc_node( NODE_SCOPE_SET, 0 ) ); }
            ;
            
/***************************************************************
* Blocks
***************************************************************/
                                                                      
statement_list : statement_list statement                           { $$ = $1;
                                                                      push_back( $$, $2 ); }
               |                                                    { $$ = alloc_node( NODE_UNDEFINED, 0 ); }
               ;
                                                                      
scope_def_list : scope_def_list scope_def                           { $$ = $1;
                                                                      push_back( $$, $2 ); }
               |                                                    { $$ = alloc_node( NODE_UNDEFINED, 0 ); }
               ;
                                                                      
scope_def : T_USE T_ID ':' expr ';'                                 { $$ = alloc_node( NODE_SCOPE_DEF, 0 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, $4 ); }
          ;
          
/***************************************************************
* Definition helpers
***************************************************************/
          
modifier_list : modifier_list modifier                              { $$ = $1;
                                                                      $$ |= $2; }
              |                                                     { $$ = 0; }
              ;
              
modifier : T_LOCAL                                                  { $$ = MODIFIER_LOCAL; }
         | T_CACHE                                                  { $$ = MODIFIER_CACHE; }
         ;

argument_def : type T_ID                                            { $$ = alloc_node( NODE_ARGUMENT, 0 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, $1 ); }
             ;
                          
argument_list : argument_def                                        { $$ = alloc_node( NODE_UNDEFINED, 0 );
                                                                      push_back( $$, $1 ); }
              | argument_list ',' argument_def                      { $$ = $1;
                                                                      push_back( $$, $3 ); }
              ;
              
/***************************************************************
* Top-level program elements
***************************************************************/

func_def : modifier_list type T_ID '(' ')' body                     { $$ = alloc_node( NODE_FUNC_DEF, $1 );
                                                                      push_back( $$, $2 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, $6 ); }
         | modifier_list type T_ID '(' argument_list ')' body       { $$ = $5;
                                                                      set_type( $$, NODE_FUNC_DEF, $1 );
                                                                      push_front( $$, $2 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, $7 ); }
        ;
                                                                      
seq_def : modifier_list T_ID '(' ')' statement                      { $$ = alloc_node( NODE_SEQ_DEF, $1 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, $5 ); }
seq_def : modifier_list T_ID '(' argument_list ')' statement        { $$ = $4;
                                                                      set_type( $$, NODE_SEQ_DEF, $1 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, $6 ); }
        ;
                                                                      
vid_def : modifier_list T_VIDEO T_ID expr ':' statement             { $$ = alloc_node( NODE_VID_DEF, $1 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, $4 );
                                                                      push_back( $$, $6 ); }
        ;
                                                                      
type_def : modifier_list type '~' T_ID ';'                          { $$ = alloc_node( NODE_TYPE_DEF, $1 );
                                                                      $$->string_data = $4;
                                                                      push_back( $$, $2 ); }
         ;
         
/***************************************************************
* Program
***************************************************************/
         
program_scope : program_scope '{' program_scope '}'                 { $$ = $1;
                                                                      push_back( $$, $3 ); }
              | program_scope func_def                              { $$ = $1;
                                                                      push_back( $$, $2 ); }
              | program_scope seq_def                               { $$ = $1;
                                                                      push_back( $$, $2 ); }
              | program_scope vid_def                               { $$ = $1;
                                                                      push_back( $$, $2 ); }
              | program_scope type_def                              { $$ = $1;
                                                                      push_back( $$, $2 ); }
              | program_scope scope_def                             { $$ = $1;
                                                                      push_back( $$, $2 ); }
              |                                                     { $$ = alloc_node( NODE_PROGRAM, 0 );
                                                                      parse_tree = $$; }
              ;
        
program : program_scope                                             { $$ = $1;
                                                                      parse_tree = $$; }
        ;

/***************************************************************
* Error handling
***************************************************************/

type_args : '(' error ')'                                           { error( "expected type list" );
                                                                      $$ = alloc_node( NODE_ERROR, 0 ); }
          | '(' type_args_list error                                { error( "expected ')'" );
                                                                      $$ = $2; }
          ;
          
type : type T_FUNCTION error                                        { error( "expected '('" );
                                                                      $$ = alloc_node( NODE_TYPE, TYPE_FUNCTION );
                                                                      push_front( $$, $1 ); }
     | T_SEQUENCE error                                             { $$ = error( "expected '('" );
                                                                      $$ = alloc_node( NODE_TYPE, TYPE_SEQUENCE ); }
     | '(' type type_args_optional_id error ')'                     { error( "expected ','" );
                                                                      $$ = $2; }
     | '~' error                                                    { $$ = error( "expected identifier" );
                                                                      $$ = alloc_node( NODE_ERROR, 0 ); }
     ;
     
expr : t_expr T_TERNARY_OP_0 expr error expr                        { error( "expected ':'" );
                                                                      $$ = alloc_node( NODE_TERNARY_OP, 0 );
                                                                      push_back( $$, $1 );
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 ); }
     ;
     
t_expr : t_expr '[' error ']'                                       { error( "expected literal" );
                                                                      $$ = alloc_node( NODE_TUPLE_EXTRACT, 0 );
                                                                      $$->int_data = 0;
                                                                      push_back( $$, $1 ); }
       | '[' error ']' %prec T_UNARY_OP                             { error( "expected expression" );
                                                                      $$ = alloc_node( NODE_UNARY_OP, UNARY_OP_FLOOR );
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
       | '[' expr error %prec T_UNARY_OP                            { error( "expected ']'" );
                                                                      $$ = alloc_node( NODE_UNARY_OP, UNARY_OP_FLOOR );
                                                                      push_back( $$, $2 ); }
       | '(' error ')'                                              { error( "expected expression" );
                                                                      $$ = alloc_node( NODE_ERROR, 0 ); }
       | t_expr '(' error ')'                                       { error( "expected argument list" );
                                                                      $$ = $1; }
       | t_expr '(' expr_list error                                 { error( "expected ')'" );
                                                                      $$ = $3;
                                                                      set_type( $$, NODE_FUNCTION_CALL, 0 );
                                                                      push_front( $$, $1 ); }
       | '(' expr error                                             { error( "expected ')'" );
                                                                      $$ = $2; }
       | '(' expr ',' error ')'                                     { error( "expected expression" );
                                                                      $$ = alloc_node( NODE_TUPLE_CONSTRUCT, 0 );
                                                                      push_back( $$, $2 );
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
       | '(' expr ',' expr_list error                               { error( "expected ')'" );
                                                                      $$ = $4;
                                                                      set_type( $$, NODE_TUPLE_CONSTRUCT, 0 );
                                                                      push_front( $$, $2 ); }
       ;
       
body : '{' error '}'                                                { error( "expected function body" );
                                                                      $$ = alloc_node( NODE_ERROR, 0 ); }
     | expr error body                                              { error( "expected ':'" );
                                                                      $$ = alloc_node( NODE_GUARD, 0 );
                                                                      push_back( $$, $1 );
                                                                      push_back( $$, $3 ); }
     | T_ID '=' error ':' body                                          { error( "expected expression" );
                                                                      $$ = alloc_node( NODE_LET, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) );
                                                                      push_back( $$, $5 ); }     
     | T_ID '=' expr error body                                     { error( "expected ':'" );
                                                                      $$ = alloc_node( NODE_LET, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 ); }
     ;
     
layer : layer_type error statement                                  { error( "expected layer" );
                                                                      $$ = alloc_node( NODE_LAYER, $1 );
                                                                      push_back( $$, $3 ); }
      ;
      
layer_optional_fx : T_MERGE error                                   { error( "expected expression" );
                                                                      $$ = alloc_node( NODE_ERROR, 0 ); }
                  ;
                  
o_statement : T_ID '=' expr error o_statement                       { error( "expected ':'" );
                                                                      $$ = alloc_node( NODE_LET, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 ); }
            | T_ID '=' error ':' o_statement                        { error( "expected expression" );
                                                                      $$ = alloc_node( NODE_LET, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) );
                                                                      push_back( $$, $5 ); }
            ;
            
c_statement : '{' error '}'                                         { error( "expected block" );
                                                                      $$ = alloc_node( NODE_ERROR, 0 ); }
            | expr ':' error                                        { error( "expected statement" );
                                                                      $$ = alloc_node( NODE_GUARD, 0 );
                                                                      push_back( $$, $1 );
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
            | expr ':' c_statement T_ELSE error c_statement         { error( "expected ':'" );
                                                                      $$ = alloc_node( NODE_GUARD, 0 );
                                                                      push_back( $$, $1 );
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $6 ); }
            | expr ':' c_statement T_ELSE ':' error                 { error( "expected statement" );
                                                                      $$ = alloc_node( NODE_GUARD, 0 );
                                                                      push_back( $$, $1 );
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
            | T_ID '=' error ':' c_statement                        { error( "expected expression" );
                                                                      $$ = alloc_node( NODE_LET, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) );
                                                                      push_back( $$, $5 ); }
            | T_ID '=' expr T_TO expr error c_statement             { error( "expected ':'" );
                                                                      $$ = alloc_node( NODE_LOOP, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 );
                                                                      push_back( $$, $7 ); }
            | T_ID '=' expr T_TO expr ':' error                     { error( "expected statement" );
                                                                      $$ = alloc_node( NODE_LOOP, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, $5 );
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
            | T_ID '=' expr ':' error                               { error( "expected statement" );
                                                                      $$ = alloc_node( NODE_LET, 0 );
                                                                      $$->string_data = $1;
                                                                      push_back( $$, $3 );
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
            | t_expr '(' error ')' ';'                              { error( "expected argument list" );
                                                                      $$ = $1; }
            | t_expr '(' error ';'                                  { error( "expected ')'" );
                                                                      $$ = $1; }
            | t_expr '(' expr_list error ';'                        { error( "expected ')'" );
                                                                      $$ = $3;
                                                                      set_type( $$, NODE_SEQUENCE_CALL, 0 );
                                                                      push_front( $$, $1 ); }
            | t_expr '(' ')' error                                  { error( "expected ';'" );
                                                                      $$ = alloc_node( NODE_SEQUENCE_CALL, 0 );
                                                                      push_front( $$, $1 ); }
            | t_expr '(' expr_list ')' error                        { error( "expected ';'" );
                                                                      $$ = $3;
                                                                      set_type( $$, NODE_SEQUENCE_CALL, 0 );
                                                                      push_front( $$, $1 ); }
            | T_SPLIT ':' '{' error '}'                             { error( "expected layer list" );
                                                                      $$ = alloc_node( NODE_SPLIT, 0 ); }
            | T_SPLIT ':' error '}'                                 { error( "expected '{'" );
                                                                      $$ = alloc_node( NODE_SPLIT, 0 ); }
            | T_SPLIT error '}'                                     { error( "expected ':'" );
                                                                      $$ = alloc_node( NODE_SPLIT, 0 );  }
            | T_FX ':' error ';'                                    { error( "expected expression" );
                                                                      $$ = alloc_node( NODE_FX, 0 );
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
            | T_FX error ';'                                        { error( "expected ':'" );
                                                                      $$ = alloc_node( NODE_FX, 0 );
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
            | T_FX ':' expr error                                   { error( "expected ';'" );
                                                                      $$ = alloc_node( NODE_FX, 0 );
                                                                      push_back( $$, $3 ); }
            ;
            
scope_def : T_USE error ';'                                         { error( "expected identifier" );
                                                                      $$ = alloc_node( NODE_ERROR, 0 ); }
          | T_USE T_ID error ';'                                    { error( "expected ':'" );
                                                                      $$ = alloc_node( NODE_SCOPE_DEF, 0 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
          | T_USE T_ID ':' error ';'                                { error( "expected expression" );
                                                                      $$ = alloc_node( NODE_SCOPE_DEF, 0 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
          | T_USE T_ID ':' expr error                               { error( "expected ';'" );
                                                                      $$ = alloc_node( NODE_SCOPE_DEF, 0 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, $4 ); }
          ;
          
argument_def : type error                                           { error( "expected identifier" );
                                                                      $$ = alloc_node( NODE_ERROR, 0 ); }
             ;
             
argument_list : argument_list error argument_def                    { error( "expected ','" );
                                                                      $$ = $1;
                                                                      push_back( $$, $3 ); }
              ;
              
func_def : modifier_list type T_ID '(' error ')' body               { error( "expected argument list" );
                                                                      $$ = alloc_node( NODE_FUNC_DEF, $1 );
                                                                      push_back( $$, $2 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, $7 ); }
         | modifier_list type T_ID error body                       { error( "expected '('" );
                                                                      $$ = alloc_node( NODE_FUNC_DEF, $1 );
                                                                      push_back( $$, $2 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, $5 ); }
         | modifier_list type T_ID '(' error body                   { error( "expected ')'" );
                                                                      $$ = alloc_node( NODE_FUNC_DEF, $1 );
                                                                      push_back( $$, $2 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, $6 ); }
         | modifier_list type T_ID '(' argument_list error body     { error( "expected ')'" );
                                                                      $$ = $5;
                                                                      set_type( $$, NODE_FUNC_DEF, $1 );
                                                                      push_front( $$, $2 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, $7 ); }
         | modifier_list type T_ID '(' ')' error                    { error( "expected statement" );
                                                                      $$ = alloc_node( NODE_FUNC_DEF, $1 );
                                                                      push_back( $$, $2 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
         | modifier_list type T_ID '(' argument_list ')' error      { error( "expected statement" );
                                                                      $$ = $5;
                                                                      set_type( $$, NODE_FUNC_DEF, $1 );
                                                                      push_front( $$, $2 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
         ;
         
seq_def : modifier_list T_ID '(' error ')' statement                { error( "expected argument list" );
                                                                      $$ = alloc_node( NODE_SEQ_DEF, $1 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, $6 ); }
        | modifier_list T_ID error statement                        { error( "expected '('" );
                                                                      $$ = alloc_node( NODE_SEQ_DEF, $1 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, $4 ); }
        | modifier_list T_ID '(' error statement                    { error( "expected ')'" );
                                                                      $$ = alloc_node( NODE_SEQ_DEF, $1 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, $5 ); }
        | modifier_list T_ID '(' argument_list error statement      { error( "expected ')'" );
                                                                      $$ = $4;
                                                                      set_type( $$, NODE_SEQ_DEF, $1 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, $6 ); }
        | modifier_list T_ID '(' ')' error                          { error( "expected statement" );
                                                                      $$ = alloc_node( NODE_SEQ_DEF, $1 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
        | modifier_list T_ID '(' argument_list ')' error            { error( "expected statement" );
                                                                      $$ = $4;
                                                                      set_type( $$, NODE_SEQ_DEF, $1 );
                                                                      $$->string_data = $2;
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
        ;
        
vid_def : modifier_list T_VIDEO error statement                     { error( "expected identifier" );
                                                                      $$ = alloc_node( NODE_VID_DEF, $1 );
                                                                      $$->string_data = "unnamed";
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) );
                                                                      push_back( $$, $4 ); }
        | modifier_list T_VIDEO T_ID error statement                { error( "expected expression" );
                                                                      $$ = alloc_node( NODE_VID_DEF, $1 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) );
                                                                      push_back( $$, $5 ); }
        | modifier_list T_VIDEO T_ID expr error statement           { error( "expected ':'" );
                                                                      $$ = alloc_node( NODE_VID_DEF, $1 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, $4 );
                                                                      push_back( $$, $6 ); }
        | modifier_list T_VIDEO T_ID expr ':' error                 { error( "expected statement" );
                                                                      $$ = alloc_node( NODE_VID_DEF, $1 );
                                                                      $$->string_data = $3;
                                                                      push_back( $$, $4 );
                                                                      push_back( $$, alloc_node( NODE_ERROR, 0 ) ); }
        ;
        
type_def : modifier_list type error ';'                             { error( "expected '~'" );
                                                                      $$ = alloc_node( NODE_TYPE_DEF, $1 );
                                                                      $$->string_data = "unnamed";
                                                                      push_back( $$, $2 ); }
         | modifier_list type '~' error ';'                         { error( "expected identifier" );
                                                                      $$ = alloc_node( NODE_TYPE_DEF, $1 );
                                                                      $$->string_data = "unnamed";
                                                                      push_back( $$, $2 ); }
         | modifier_list type '~' T_ID error                        { error( "expected ';'" );
                                                                      $$ = alloc_node( NODE_TYPE_DEF, $1 );
                                                                      $$->string_data = $4;
                                                                      push_back( $$, $2 ); }
         ;
         
program_scope : program_scope '{' error '}'                         { error( "expected program" );
                                                                      $$ = $1; }
              ;

%%
