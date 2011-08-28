#include "expr.h"

/***************************************************************
* Constant
***************************************************************/

Constant::Constant( rave_int value )
: _is_int( true )
, _int_value( value )
{
}

Constant::Constant( rave_float value )
: _is_int( false )
, _float_value( value )
{
}

Constant::~Constant()
{
}

/***************************************************************
* Identifier
***************************************************************/

Identifier::Identifier( const std::string& id )
: _id( id )
{
}

Identifier::~Identifier()
{
}

/***************************************************************
* Unary operator
***************************************************************/

UnaryOp::UnaryOp( int type, Ast* expr )
: _type( type )
, _expr( expr )
{
}

UnaryOp::~UnaryOp()
{
    delete _expr;
}

/***************************************************************
* Binary operator
***************************************************************/

BinaryOp::BinaryOp( int type, Ast* left, Ast* right )
: _type( type )
, _left( left )
, _right( right )
{
}

BinaryOp::~BinaryOp()
{
    delete _left;
    delete _right;
}

/***************************************************************
* Type operator
***************************************************************/

TypeOp::TypeOp( int type, Ast* left, Ast* right )
: _type( type )
, _left( left )
, _right( right )
, _left_type( Type::Void() )
, _right_type( Type::Void() )
{
}

TypeOp::TypeOp( int type, Type left, Ast* right )
: _type( type )
, _left( 0 )
, _right( right )
, _left_type( left )
, _right_type( Type::Void() )
{
}

TypeOp::TypeOp( int type, Ast* left, Type right )
: _type( type )
, _left( left )
, _right( 0 )
, _left_type( Type::Void() )
, _right_type( right )
{
}

TypeOp::TypeOp( int type, Type left, Type right )
: _type( type )
, _left( 0 )
, _right( 0 )
, _left_type( left )
, _right_type( right )
{
}

TypeOp::~TypeOp()
{
    if ( _left )
        delete _left;
    if ( _right )
        delete _right;
}

/***************************************************************
* Ternary operator
***************************************************************/

TernaryOp::TernaryOp( Ast* expr, Ast* left, Ast* right )
: _expr( expr )
, _left( left )
, _right( right )
{
}

TernaryOp::~TernaryOp()
{
    delete _expr;
    delete _left;
    delete _right;
}

/***************************************************************
* Tuple construction
***************************************************************/

TupleConstruct::TupleConstruct( const AstList& list )
: _list( list )
{
}

TupleConstruct::~TupleConstruct()
{
    for ( std::size_t i = 0; i < _list.size(); ++i )
        delete _list[ i ];
}

/***************************************************************
* Tuple extraction
***************************************************************/

TupleExtract::TupleExtract( Ast* tuple, rave_int index )
: _tuple( tuple )
, _index( index )
{
}

TupleExtract::~TupleExtract()
{
    delete _tuple;
}

/***************************************************************
* Tuple replacement
***************************************************************/

TupleReplace::TupleReplace( Ast* tuple, rave_int index, Ast* expr )
: _tuple( tuple )
, _index( index )
, _expr( expr )
{
}

TupleReplace::~TupleReplace()
{
    delete _tuple;
    delete _expr;
}

/***************************************************************
* Function call
***************************************************************/

FunctionCall::FunctionCall( Ast* function, const AstList& list )
: _function( function )
, _args( list )
{
}

FunctionCall::~FunctionCall()
{
    delete _function;
    for ( std::size_t i = 0; i < _args.size(); ++i )
        delete _args[ i ];
}

/***************************************************************
* Promote
***************************************************************/

Promoter::Promoter( Ast* expr, Type to )
: _expr( expr )
, _to( to )
{
}

Promoter::~Promoter()
{
    delete _expr;
}