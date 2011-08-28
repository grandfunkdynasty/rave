#include <iostream>
#include <string>
#include "parse.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"

#ifndef LINUX
#define OPEN( t, x ) fopen_s( &t, x, "r" )
#else
#define OPEN( t, x ) ( ( t = fopen( x, "r" ) ) == 0 )
#endif

extern "C" {
    int yyparse();
    extern int yyline;
    extern int include_depth;
    extern IncludeList* include_list;
    extern char* yyname;
    Node* parse_tree;
    int parse_error;
    extern FILE* yyin;
}

/***************************************************************
* Parse data
***************************************************************/

class NodeImpl : public Node {
public:
    NodeImpl()
    {
    }
    ~NodeImpl()
    {
        if ( string_data )
            free( string_data );
        if ( file )
            free( file );
        if ( string_data )
            free( string_data );
        if ( begin )
            delete begin;
    }
};

class NodeListImpl : public NodeList {
public:
    NodeListImpl()
    {
    }
    ~NodeListImpl()
    {
        delete elem;
        if ( next )
            delete next;
    }
};

/***************************************************************
* Parse helper functions
***************************************************************/

Node* alloc_node( int type, int sub_type )
{
    Node* node = new NodeImpl();

    node->type = type;
    node->sub_type = sub_type;
    node->string_data = 0;
    node->int_data = 0;
    node->float_data = 0;
    node->begin = 0;
    node->end = 0;
    node->file = strdup( yyname );
    node->line = yyline;
    
    return node;
}

Node* alloc_binary_node( int sub_type, Node* left, Node* right )
{
    Node* node = alloc_node( NODE_BINARY_OP, sub_type );
    if ( left )
        push_back( node, left );
    if ( right )
        push_back( node, right );
    return node;
}

Node* alloc_typeop_node( int sub_type, Node* left, Node* right )
{
    Node* node = alloc_node( NODE_TYPE_OP, sub_type );
    if ( left )
        push_back( node, left );
    if ( right )
        push_back( node, right );
    return node;
}

void set_type( Node* node, int type, int sub_type )
{
    if ( !node )
        return;
    node->type = type;
    node->sub_type = sub_type;
    node->file = strdup( yyname );
    node->line = yyline;
}

void push_front( Node* parent, Node* child )
{
    if ( !parent || !child )
        return;

    NodeList* list = new NodeListImpl();
    list->elem = child;
    list->next = parent->begin;

    parent->begin = list;
    if ( !parent->end )
        parent->end = list;
}

void push_back( Node* parent, Node* child )
{
    if ( !parent || !child )
        return;

    NodeList* list = new NodeListImpl();
    list->elem = child;
    list->next = 0;

    if ( parent->end )
        parent->end->next = list;
    else
        parent->begin = list;
    parent->end = list;
}

/***************************************************************
* Parse to AST construction
***************************************************************/

Type construct_type( Node* node )
{
    if ( node->sub_type == TYPE_TYPEDEF )
        return Type::Typedef( node->string_data );
    if ( node->sub_type == TYPE_INT )
        return Type::Int();
    if ( node->sub_type == TYPE_FLOAT )
        return Type::Float();

    if ( node->sub_type == TYPE_TUPLE ) {
        Type::TypeList list;
        for ( NodeList* t = node->begin; t; t = t->next )
            list.push_back( construct_type( t->elem ) );
        return Type::Tuple( list );
    }

    Type::TypeList list;
    NodeList* t = node->begin;
    for ( t = node->sub_type == TYPE_FUNCTION ? t->next : t; t; t = t->next )
        list.push_back( construct_type( t->elem ) );
    if ( node->sub_type == TYPE_FUNCTION ) {
        Type return_type = construct_type( node->begin->elem );
        return Type::Function( return_type, list );
    }
    return Type::Sequence( list );
}

Ast* construct( Node* node );

Ast* construct( Node* node, bool helper )
{
    (void)helper;
    int type = node->type;
    int sub_type = node->sub_type;
    NodeList* t = node->begin;

    // Literal
    if ( type == NODE_LITERAL ) {
        if ( sub_type == LITERAL_INT )
            return new Constant( node->int_data );
        else if ( sub_type == LITERAL_FLOAT )
            return new Constant( node->float_data );
    }
    // Identifer
    if ( type == NODE_IDENTIFIER )
        return new Identifier( node->string_data );

    // Unary operator
    if ( type == NODE_UNARY_OP )
        return new UnaryOp( sub_type, construct( t->elem ) );
    // Binaryu operator
    if ( type == NODE_BINARY_OP )
        return new BinaryOp( sub_type, construct( t->elem ),
                             construct( t->next->elem ) );
    // Ternary operator
    if ( type == NODE_TERNARY_OP )
        return new TernaryOp( construct( t->elem ),
                              construct( t->next->elem ),
                              construct( t->next->next->elem ) );

    // Type operator
    if ( type == NODE_TYPE_OP ) {
        bool left = t->elem->type == NODE_TYPE;
        bool right = t->next->elem->type == NODE_TYPE;
        if ( left && right )
            return new TypeOp( sub_type, construct_type( t->elem ),
                           construct_type( t->next->elem ) );
        else if ( left && !right )
            return new TypeOp( sub_type, construct_type( t->elem ),
                           construct( t->next->elem ) );
        else if ( !left && right )
            return new TypeOp( sub_type, construct( t->elem ),
                           construct_type( t->next->elem ) );
        else
            return new TypeOp( sub_type, construct( t->elem ),
                   construct( t->next->elem ) );
    }

    // Tuple construction
    if ( type == NODE_TUPLE_CONSTRUCT ) {
        Ast::AstList list;
        for ( ; t; t = t->next )
            list.push_back( construct( t->elem ) );
        return new TupleConstruct( list );
    }

    // Tuple extraction
    if ( type == NODE_TUPLE_EXTRACT )
        return new TupleExtract( construct( t->elem ), node->int_data );

    // Tuple replacement
    if ( type == NODE_TUPLE_REPLACE )
        return new TupleReplace( construct( t->elem ), node->int_data,
                                 construct( t->next->elem ) );

    // Function call
    if ( type == NODE_FUNCTION_CALL ) {
        Ast* function = construct( t->elem );
        Ast::AstList list;
        for ( t = t->next; t; t = t->next )
            list.push_back( construct( t->elem ) );
        return new FunctionCall( function, list );
    }

    // Body
    if ( type == NODE_BODY ) {
        Ast::AstList list;
        for ( ; t; t = t->next )
            list.push_back( construct( t->elem ) );
        return new Body( list );
    }

    // Return
    if ( type == NODE_RETURN )
        return new Return( construct( t->elem ) );

    // Guard
    if ( type == NODE_GUARD ) {
        Ast* expr = construct( t->elem );
        Ast* then = construct( t->next->elem );
        if ( t->next == node->end )
            return new Guard( expr, then );
        Ast* otherwise = construct( t->next->next->elem );
        return new Guard( expr, then, otherwise );
    }

    // Let
    if ( type == NODE_LET )
        return new Let( node->string_data, construct( t->elem ), construct( t->next->elem ) );

    // Block
    if ( type == NODE_BLOCK ) {
        Ast* scope_defs = construct( t->elem );
        Ast::AstList list;
        for ( t = t->next; t; t = t->next )
            list.push_back( construct( t->elem ) );
        return new Block( scope_defs, list );
    }

    // Scope set
    if ( type == NODE_SCOPE_SET ) {
        Ast::AstList list;
        for ( ; t; t = t->next )
            list.push_back( construct( t->elem ) );
        return new ScopeSet( list );
    }

    // Scope definition
    if ( type == NODE_SCOPE_DEF )
        return new ScopeDef( node->string_data, construct( t->elem ) );

    // Loop
    if ( type == NODE_LOOP )
        return new Loop( node->string_data, construct( t->elem ), construct( t->next->elem ),
                         construct( t->next->next->elem ) );

    // Sequence call
    if ( type == NODE_SEQUENCE_CALL ) {
        Ast* sequence = construct( t->elem );
        Ast::AstList list;
        for ( t = t->next; t; t = t->next )
            list.push_back( construct( t->elem ) );
        return new SequenceCall( sequence, list );
    }

    // FX statement
    if ( type == NODE_FX )
        return new FxStatement( construct( t->elem ) );

    // Split statement
    if ( type == NODE_SPLIT ) {
        Ast::AstList list;
        for ( ; t; t = t->next )
            list.push_back( construct( t->elem ) );
        return new SplitStatement( list );
    }

    // Layer
    if ( type == NODE_LAYER ) {
        if ( t == node->end )
            return new Layer( node->sub_type, construct( t->elem ) );
        if ( t->next == node->end )
            return node->sub_type & LAYER_FX ?
            new Layer( node->sub_type, 0, construct( t->elem ), construct( t->next->elem ) ) :
            new Layer( node->sub_type, construct( t->elem ), construct( t->next->elem ) );
        return new Layer( node->sub_type, construct( t->elem ), construct( t->next->elem ),
                          construct( t->next->next->elem ) );
    }

    // Argument
    if ( type == NODE_ARGUMENT )
        return new Argument( construct_type( t->elem ), node->string_data );

    // Function definition
    if ( type == NODE_FUNC_DEF ) {
        Type return_type = construct_type( t->elem );
        Ast::AstList list;
        for ( t = t->next; t != node->end; t = t->next )
            list.push_back( construct( t->elem ) );
        Ast* expr = construct( t->elem );
        return new FuncDef( sub_type, return_type, node->string_data, list, expr );
    }

    // Sequence definition
    if ( type == NODE_SEQ_DEF ) {
        Ast::AstList list;
        for ( ; t != node->end; t = t->next )
            list.push_back( construct( t->elem ) );
        Ast* expr = construct( t->elem );
        return new SeqDef( sub_type, node->string_data, list, expr );
    }

    // Video definition
    if ( type == NODE_VID_DEF )
        return new VidDef( sub_type, node->string_data,
                           construct( t->elem ), construct( t->next->elem ) );

    // Type definition
    if ( type == NODE_TYPE_DEF )
        return new TypeDef( sub_type, construct_type( t->elem ), node->string_data );

    // Program
    if ( type == NODE_PROGRAM ) {
        Ast::AstList list;
        for ( ; t; t = t->next )
            list.push_back( construct( t->elem ) );
        return new Program( list );
    }

    return 0;
}

Ast* construct( Node* node )
{
    Ast* r = construct( node, true );
    if ( r )
        r->SetInfo( node->file, node->line );
    return r;
}

/***************************************************************
* Parse function
***************************************************************/

Ast* parse( const std::string& path )
{
    FILE* t = 0;
    if ( OPEN( t, path.c_str() ) ) {
        std::cout << "could not open input file " << path << "\n";
        return 0;
    }
    std::cout << "parsing...";

    yyline = 1;
    yyname = ( char* )path.c_str();
    include_depth = 0;
    parse_error = 0;
    yyin = t;
    yyparse();

    for ( IncludeList* list = include_list; list; ) {
        free( list->file );
        IncludeList* t = list;
        list = list->next;
        free( t );
    }
    include_list = 0;

    fclose( t );
    if ( parse_error ) {
        std::cout << parse_error << " error(s) detected.\n";
        return 0;
    }
    std::cout << " success\n";

    Ast* parse = construct( parse_tree );
    delete parse_tree;
    return parse;
}

/***************************************************************
* Errors
***************************************************************/

void write_error( const char* name, int line, const char* next, const char* text, int unexpected )
{
    if ( !parse_error )
        std::cout << "\n";
    if ( parse_error == 64 )
        std::cout << "[more errors...]\n";
    else if ( parse_error < 64 ) {
        std::cout << name << " line " << line << ":\t";
        if ( unexpected )
            std::cout << "unexpected '" << ( *next ? next : "EOF" ) << "': ";
        std::cout << text << "\n";
    }
    ++parse_error;
}
