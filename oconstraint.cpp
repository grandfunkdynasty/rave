#include "oconstraint.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"

#define IMPLEMENT_OPERATOR SubtreeConstraintOperator
#define IMPLEMENT_TYPE IMPLEMENT_CONST

SubtreeConstraintOperator::SubtreeConstraintOperator()
    : _nested( false )
    , _all_identifiers( true )
    , _constant_int( true )
    , _constant_int_value( 0 )
{
}

SubtreeConstraintOperator::~SubtreeConstraintOperator()
{
}

bool SubtreeConstraintOperator::Nested() const
{
    return _nested;
}

bool SubtreeConstraintOperator::AllIdentifiers() const
{
    return _all_identifiers;
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

IMPLEMENT( Constant )
{
    _all_identifiers = false;
    _constant_int = arg._is_int;
    if ( _constant_int )
        _constant_int_value = arg._int_value;
}

IMPLEMENT( TernaryOp )
{
    _constant_int = _all_identifiers = false;
}

IMPLEMENT( BinaryOp )
{
    _constant_int = _all_identifiers = false;
}

IMPLEMENT( UnaryOp )
{
    _constant_int = _all_identifiers = false;
}

IMPLEMENT( TypeOp )
{
    _constant_int = _all_identifiers = false;
}

IMPLEMENT( TupleConstruct )
{
    _nested = true;
    for ( std::size_t i = 0; i < arg._list.size(); ++i )
        Operate( arg._list[ i ] );
    _constant_int = false;
}

IMPLEMENT( TupleExtract )
{
    _constant_int = _all_identifiers = false;
}

IMPLEMENT( TupleReplace )
{
    _constant_int = _all_identifiers = false;
}

IMPLEMENT( FunctionCall )
{
    _constant_int = _all_identifiers = false;
}

IMPLEMENT( Converter )
{
    _constant_int = _all_identifiers = false;
}

IMPLEMENT_EMPTY( ParseError );
IMPLEMENT_EMPTY( Identifier );
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