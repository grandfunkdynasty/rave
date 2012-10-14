#include "oidentifier.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"

#define IMPLEMENT_OPERATOR IdentifierOperator
#define IMPLEMENT_TYPE IMPLEMENT_CONST

IdentifierOperator::IdentifierOperator()
    : _nested( false )
    , _allIdentifiers( true )
{
}

IdentifierOperator::~IdentifierOperator()
{
}

bool IdentifierOperator::Nested() const
{
    return _nested;
}

bool IdentifierOperator::AllIdentifiers() const
{
    return _allIdentifiers;
}


/***************************************************************
* Implementations
***************************************************************/

IMPLEMENT( Constant )
{
    _allIdentifiers = false;
}

IMPLEMENT( TernaryOp )
{
    _allIdentifiers = false;
}

IMPLEMENT( BinaryOp )
{
    _allIdentifiers = false;
}

IMPLEMENT( UnaryOp )
{
    _allIdentifiers = false;
}

IMPLEMENT( TypeOp )
{
    _allIdentifiers = false;
}

IMPLEMENT( TupleConstruct )
{
    _nested = true;
    for ( std::size_t i = 0; i < arg._list.size(); ++i )
        Operate( arg._list[ i ] );
}

IMPLEMENT( TupleExtract )
{
    _allIdentifiers = false;
}


IMPLEMENT( TupleReplace )
{
    _allIdentifiers = false;
}

IMPLEMENT( FunctionCall )
{
    _allIdentifiers = false;
}

IMPLEMENT( Promoter )
{
    _allIdentifiers = false;
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