#include "expr.h"

/***************************************************************
* Parse error
***************************************************************/

ParseError::ParseError()
{
}

ParseError::~ParseError()
{
}

/***************************************************************
* Constant
***************************************************************/

Constant::Constant( rave_int value )
: _is_int( true )
, _int_value( value )
, _float_value( 0.0 )
{
}

Constant::Constant( rave_float value )
: _is_int( false )
, _int_value( 0 )
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
, _op_type( Type::Void() )
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
, _op_type( Type::Void() )
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
, _value_type( Type::Void() )
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
, _value_type( Type::Void() )
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

TupleExtract::TupleExtract( Ast* tuple, Ast* index )
: _tuple( tuple )
, _index( index )
, _constant_index( 0 )
{
}

TupleExtract::~TupleExtract()
{
    delete _tuple;
    delete _index;
}

/***************************************************************
* Tuple replacement
***************************************************************/

TupleReplace::TupleReplace( Ast* tuple, Ast* index, Ast* expr )
: _tuple( tuple )
, _index( index )
, _constant_index( 0 )
, _expr( expr )
, _value_type( Type::Void() )
{
}

TupleReplace::~TupleReplace()
{
    delete _tuple;
    delete _index;
    delete _expr;
}

/***************************************************************
* Function call
***************************************************************/

FunctionCall::FunctionCall( Ast* function, const AstList& list )
: _function( function )
, _args( list )
, _let_value( 0 )
, _let_index( 0 )
{
}

FunctionCall::~FunctionCall()
{
    delete _function;
    for ( std::size_t i = 0; i < _args.size(); ++i )
        delete _args[ i ];
}

/***************************************************************
* Convert
***************************************************************/

Converter::Converter( Ast* expr, Type from, Type to )
: _expr( expr )
, _from( from )
, _to( to )
{
}

Converter::~Converter()
{
    delete _expr;
}