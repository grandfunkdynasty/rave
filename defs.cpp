#include "defs.h"

/***************************************************************
* Body
***************************************************************/

Body::Body( const AstList& steps )
: _steps( steps )
{
}

Body::~Body()
{
    for ( std::size_t i = 0; i < _steps.size(); ++i )
        delete _steps[ i ];
}

/***************************************************************
* Return
***************************************************************/

Return::Return( Ast* expr )
: _expr( expr )
{
}

Return::~Return()
{
    delete _expr;
}

/***************************************************************
* Guard
***************************************************************/

Guard::Guard( Ast* expr, Ast* then )
: _expr( expr )
, _then( then )
, _otherwise( 0 )
{
}

Guard::Guard( Ast* expr, Ast* then, Ast* otherwise )
: _expr( expr )
, _then( then )
, _otherwise( otherwise )
{
}

Guard::~Guard()
{
    delete _expr;
    delete _then;
    if ( _otherwise )
        delete _otherwise;
}

/***************************************************************
* Let
***************************************************************/

Let::Let( const std::string& id, Ast* expr, Ast* in )
: _id( id )
, _expr( expr )
, _in( in )
{
}

Let::~Let()
{
    delete _expr;
    delete _in;
}

/***************************************************************
* Block
***************************************************************/

Block::Block( Ast* scope_set, const AstList& statements )
: _scope_set( scope_set )
, _statements( statements )
{
}

Block::~Block()
{
    delete _scope_set;
    for ( std::size_t i = 0; i < _statements.size(); ++i )
        delete _statements[ i ];
}

/***************************************************************
* Scope set
***************************************************************/

ScopeSet::ScopeSet( const AstList& scope_defs )
: _scope_defs( scope_defs )
{
}

ScopeSet::~ScopeSet()
{
    for ( std::size_t i = 0; i < _scope_defs.size(); ++i )
        delete _scope_defs[ i ];
}

/***************************************************************
* Scope definition
***************************************************************/

ScopeDef::ScopeDef( const std::string& id, Ast* expr )
: _id( id )
, _expr( expr )
{
}

ScopeDef::~ScopeDef()
{
    delete _expr;
}

/***************************************************************
* Loop
***************************************************************/

Loop::Loop( const std::string& id, Ast* begin, Ast* end, Ast* in )
: _id( id )
, _begin( begin )
, _end( end )
, _in( in )
{
}

Loop::~Loop()
{
    delete _begin;
    delete _end;
    delete _in;
}

/***************************************************************
* Sequence call
***************************************************************/

SequenceCall::SequenceCall( Ast* sequence, const AstList& args )
: _sequence( sequence )
, _args( args )
{
}

SequenceCall::~SequenceCall()
{
    delete _sequence;
    for ( std::size_t i = 0; i < _args.size(); ++i )
        delete _args[ i ];
}

/***************************************************************
* FX statement
***************************************************************/

FxStatement::FxStatement( Ast* expr )
: _expr( expr )
{
}

FxStatement::~FxStatement()
{
    delete _expr;
}

/***************************************************************
* Split statement
***************************************************************/

SplitStatement::SplitStatement( const AstList& layers )
: _layers( layers )
{
}

SplitStatement::~SplitStatement()
{
    for ( std::size_t i = 0; i < _layers.size(); ++i )
        delete _layers[ i ];
}

/***************************************************************
* Layer
***************************************************************/

Layer::Layer( int type, Ast* statement )
: _type( type )
, _order( 0 )
, _fx( 0 )
, _statement( statement )
{
}

Layer::Layer( int type, Ast* order, Ast* statement )
: _type( type )
, _order( order )
, _fx( 0 )
, _statement( statement )
{
}

Layer::Layer( int type, Ast* order, Ast* fx, Ast* statement )
: _type( type )
, _order( order )
, _fx( fx )
, _statement( statement )
{
}

Layer::~Layer()
{
    if ( _order )
        delete _order;
    if ( _fx )
        delete _fx;
    delete _statement;
}