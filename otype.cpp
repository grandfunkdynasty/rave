#include "otype.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"

#define IMPLEMENT_OPERATOR TypeOperator
#define IMPLEMENT_TYPE IMPLEMENT_NON_CONST

TypeOperator::TypeOperator( int* errors )
: _errors( errors )
, _declare_globals( false )
, _declarations( false )
{
}

TypeOperator::~TypeOperator()
{
}

void TypeOperator::Error( const Ast& arg, const std::string& text )
{
    if ( !*_errors )
        std::cout << "\n";
    if ( *_errors == 64 )
        std::cout << "[more errors...]\n";
    else if ( *_errors < 64 )
        std::cout << arg.GetFileInfo() << " line " << arg.GetLineInfo() << ":\t" << text << "\n";
    ++*_errors;
}

Type TypeOperator::Resolve( const Ast& arg, const Type& type )
{
    if ( type == Type::Void() || type == Type::Int() || type == Type::Float() )
        return type;
    if ( type.Typedef() != "" && !type.IsUnresolved() )
        return type;
    if ( type.Typedef() != "" && type.IsUnresolved() ) {
        if ( !_table.HasEntry( type.Typedef() ) ) {
            Error( arg, "undeclared type '~" + type.Typedef() + "'" );
            return Type::Typedef( type.Typedef(), Type::Void() );
        }
        return Type::Typedef( type.Typedef(), _table.GetEntry( type.Typedef() ) );
    }
    Type::TypeList list;
    for ( std::size_t i = 0; i < type.TypeArgs().size(); ++i )
        list.push_back( Resolve( arg, type.TypeArgs()[ i ] ) );
    if ( type.IsTuple() )
        return Type::Tuple( list );
    if ( type.IsSequence() )
        return Type::Sequence( list );
    return Type::Function( Resolve( arg, type.ReturnType() ), list );
}

/***************************************************************
* Implementations
***************************************************************/

IMPLEMENT_EMPTY( Constant );
IMPLEMENT_EMPTY( Identifier );

IMPLEMENT( TernaryOp )
{
    Operate( arg._expr );
    Operate( arg._left );
    Operate( arg._right );
}

IMPLEMENT( BinaryOp )
{
    Operate( arg._left );
    Operate( arg._right );
}


IMPLEMENT( UnaryOp )
{
    Operate( arg._expr );
}

IMPLEMENT( TypeOp )
{
    if ( arg._left )
        Operate( arg._left );
    else
        arg._left_type = Resolve( arg, arg._left_type );
    if ( arg._right )
        Operate( arg._right );
    else
        arg._right_type = Resolve( arg, arg._right_type );
}

IMPLEMENT( TupleConstruct )
{
    for ( std::size_t i = 0; i < arg._list.size(); ++i )
        Operate( arg._list[ i ] );
}

IMPLEMENT( FunctionCall )
{
    Operate( arg._function );
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
}

IMPLEMENT( Body )
{
    for ( std::size_t i = 0; i < arg._steps.size(); ++i )
        Operate( arg._steps[ i ] );
}

IMPLEMENT( Return )
{
    Operate( arg._expr );
}

IMPLEMENT( Guard )
{
    Operate( arg._expr );
    Operate( arg._then );
    if ( arg._otherwise )
        Operate( arg._otherwise );
}

IMPLEMENT( Let )
{
    Operate( arg._expr );
    Operate( arg._in );
}

IMPLEMENT( Block )
{
    Operate( arg._scope_set );
    for ( std::size_t i = 0; i < arg._statements.size(); ++i )
        Operate( arg._statements[ i ] );
}

IMPLEMENT( ScopeSet )
{
    for ( std::size_t i = 0; i < arg._scope_defs.size(); ++i )
        Operate( arg._scope_defs[ i ] );
}

IMPLEMENT( ScopeDef )
{
    Operate( arg._expr );
}

IMPLEMENT( Loop )
{
    Operate( arg._begin );
    Operate( arg._end );
    Operate( arg._in );
}

IMPLEMENT( SequenceCall )
{
    Operate( arg._sequence );
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
}

IMPLEMENT( FxStatement )
{
    Operate( arg._expr );
}

IMPLEMENT( SplitStatement )
{
    for ( std::size_t i = 0; i < arg._layers.size(); ++i )
        Operate( arg._layers[ i ] );
}

IMPLEMENT( Layer )
{
    if ( arg._order )
        Operate( arg._order );
    if ( arg._fx )
        Operate( arg._fx );
    Operate( arg._statement );
}

IMPLEMENT( Argument )
{
    arg._type = Resolve( arg, arg._type );
}

IMPLEMENT( FuncDef )
{
    if ( _declarations )
        return;
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
    Operate( arg._expr );
    arg._return_type = Resolve( arg, arg._return_type );
}

IMPLEMENT( SeqDef )
{
    if ( _declarations )
        return;
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
    Operate( arg._statement );
}

IMPLEMENT( VidDef )
{
    if ( _declarations )
        return;
    Operate( arg._frame_count );
    Operate( arg._statement );
}

IMPLEMENT( TypeDef )
{
    if ( !_declarations )
        return;
    if ( _declare_globals && arg._modifiers & MODIFIER_LOCAL )
        return;
    if ( !_table.AddEntry( arg._id, Resolve( arg, arg._type ) ) )
        Error( arg, "type '~" + arg._id + "' already defined" );
}

IMPLEMENT( Program )
{
    if ( _declarations ) {
        if ( _declare_globals )
            return;
        _declare_globals = true;
        for ( std::size_t i = 0; i < arg._elements.size(); ++i )
            Operate( arg._elements[ i ] );
        _declare_globals = false;
        return;
    }

    _table.Push();
    _declarations = true;
    for ( std::size_t i = 0; i < arg._elements.size(); ++i )
        Operate( arg._elements[ i ] );
    _declarations = false;
    for ( std::size_t i = 0; i < arg._elements.size(); ++i )
        Operate( arg._elements[ i ] );
    _table.Pop();
}