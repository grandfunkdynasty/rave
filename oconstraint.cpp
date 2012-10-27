#include "oconstraint.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"

#define IMPLEMENT_OPERATOR SubtreeConstraintOperator
#define IMPLEMENT_TYPE IMPLEMENT_CONST

SubtreeConstraintOperator::SubtreeConstraintOperator( const SymbolTable< Type >& table )
    : _table( table )
    , _single_identifier( true )
    , _valid_let_variables( true )
    , _constant_int( true )
    , _constant_int_value( 0 )
{
}

SubtreeConstraintOperator::~SubtreeConstraintOperator()
{
}

bool SubtreeConstraintOperator::SingleIdentifier() const
{
    return _single_identifier;
}

const std::string& SubtreeConstraintOperator::SingleIdentifierName() const
{
    return _single_identifier_name;
}

bool SubtreeConstraintOperator::ValidLetVariables() const
{
    return _valid_let_variables;
}

bool SubtreeConstraintOperator::ConstantInt() const
{
    return _constant_int;
}

rave_int SubtreeConstraintOperator::ConstantIntValue() const
{
    return _constant_int_value;
}

/***************************************************************
* Implementations
***************************************************************/

IMPLEMENT( Identifier )
{
    _constant_int = false;
    _single_identifier_name = arg._id;
}

IMPLEMENT( Constant )
{
    _single_identifier = _valid_let_variables = false;
    _constant_int = arg._is_int;
    if ( _constant_int )
        _constant_int_value = arg._int_value;
}

IMPLEMENT( TernaryOp )
{
    _constant_int = _single_identifier = _valid_let_variables = false;
}

IMPLEMENT( BinaryOp )
{
    _constant_int = _single_identifier = _valid_let_variables = false;
}

IMPLEMENT( UnaryOp )
{
    _constant_int = _single_identifier = _valid_let_variables = false;
}

IMPLEMENT( TypeOp )
{
    _constant_int = _single_identifier = _valid_let_variables = false;
}

IMPLEMENT( TupleConstruct )
{
    _constant_int = _single_identifier = false;
    for ( std::size_t i = 0; i < arg._list.size(); ++i )
        Operate( arg._list[ i ] );
}

IMPLEMENT( TupleExtract )
{
    _constant_int = _single_identifier = _valid_let_variables = false;
}

IMPLEMENT( TupleReplace )
{
    _constant_int = _single_identifier = _valid_let_variables = false;
}

IMPLEMENT( FunctionCall )
{
    _constant_int = _single_identifier = false;
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
}

IMPLEMENT( Converter )
{
    _constant_int = _single_identifier = _valid_let_variables = false;
}

IMPLEMENT_EMPTY( ParseError );
IMPLEMENT_EMPTY( Body );
IMPLEMENT_EMPTY( Return );
IMPLEMENT_EMPTY( Guard );
IMPLEMENT_EMPTY( Let );
IMPLEMENT_EMPTY( Block );
IMPLEMENT_EMPTY( ScopeSet );
IMPLEMENT_EMPTY( ScopeDef );
IMPLEMENT_EMPTY( Loop );
IMPLEMENT_EMPTY( SequenceCall );
IMPLEMENT_EMPTY( FxStatement );
IMPLEMENT_EMPTY( SplitStatement );
IMPLEMENT_EMPTY( Layer );
IMPLEMENT_EMPTY( Argument );
IMPLEMENT_EMPTY( FuncDef );
IMPLEMENT_EMPTY( SeqDef );
IMPLEMENT_EMPTY( VidDef );
IMPLEMENT_EMPTY( TypeDef );
IMPLEMENT_EMPTY( Program );