#include "ostatic.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"

#define IMPLEMENT_OPERATOR StaticOperator
#define IMPLEMENT_TYPE IMPLEMENT_CONST

StaticOperator::StaticOperator( int* errors )
: _errors( errors )
, _type( Type::Void() )
, _return_type( Type::Void() )
, _return_path( false )
, _declare_globals( false )
, _declarations( false )
{
}

StaticOperator::~StaticOperator()
{
}

void StaticOperator::Error( const Ast& arg, const std::string& text )
{
    if ( !*_errors )
        std::cout << "\n";
    if ( *_errors == 64 )
        std::cout << "[more errors...]\n";
    else if ( *_errors < 64 )
        std::cout << arg.GetFileInfo() << " line " << arg.GetLineInfo() << ":\t" << text << "\n";
    ++*_errors;
}

/***************************************************************
* Implementations
***************************************************************/
// TODO: insert int-to-float casts

IMPLEMENT( Constant )
{
    _type = arg._is_int ? Type::Int() : Type::Float();
}

IMPLEMENT( Identifier )
{
    if ( !_table.HasEntry( arg._id ) )
        Error( arg, "undeclared identifier '" + arg._id + "'" );
    _type = _table.GetEntry( arg._id );
}

IMPLEMENT( TernaryOp )
{
    Operate( arg._expr );
    Type expr = _type;
    Operate( arg._left );
    Type left = _type;
    Operate( arg._right );
    Type right = _type;

    if ( !expr.ConvertsTo( Type::Int() ) && expr != Type::Void() )
        Error( arg, "cannot apply '?' to '" + expr.Typename() + "'" );
    _type = left.Generalise( right );
    if ( _type == Type::Void() && left != Type::Void() && right != Type::Void() )
        Error( arg, "'?': cannot generalise '" + left.Typename() +
               "' with '" + right.Typename() + "'" );
}

IMPLEMENT( BinaryOp )
{
    Operate( arg._left );
    Type left = _type;
    Operate( arg._right );
    Type right = _type;

    std::string op =
        ( arg._type == BINARY_OP_OR ? "||" :
          arg._type == BINARY_OP_AND ? "&&" :
          arg._type == BINARY_OP_EQ ? "==" :
          arg._type == BINARY_OP_NE ? "!=" :
          arg._type == BINARY_OP_GT ? ">" :
          arg._type == BINARY_OP_GE ? ">=" :
          arg._type == BINARY_OP_LT ? "<" :
          arg._type == BINARY_OP_LE ? "<=" :
          arg._type == BINARY_OP_BIT_OR ? "|" :
          arg._type == BINARY_OP_BIT_AND ? "&" :
          arg._type == BINARY_OP_BIT_XOR ? "'" :
          arg._type == BINARY_OP_LSHIFT ? "<<" :
          arg._type == BINARY_OP_RSHIFT ? ">>" :
          arg._type == BINARY_OP_ADD ? "+" :
          arg._type == BINARY_OP_SUB ? "-" :
          arg._type == BINARY_OP_MUL ? "*" :
          arg._type == BINARY_OP_DIV ? "/" :
          arg._type == BINARY_OP_MOD ? "%" :
          arg._type == BINARY_OP_EXP ? "^" : "" );

    if ( arg._type == BINARY_OP_OR || arg._type == BINARY_OP_AND ||
              arg._type == BINARY_OP_BIT_OR || arg._type == BINARY_OP_BIT_AND ||
              arg._type == BINARY_OP_BIT_XOR ||
              arg._type == BINARY_OP_LSHIFT || arg._type == BINARY_OP_RSHIFT ) {
        if ( !( left.ConvertsTo( Type::Int() ) && left != Type::Void() ) ||
             !( right.ConvertsTo( Type::Int() ) && right != Type::Void() ) )
            Error( arg, "cannot apply '" + op + "' to '" +
                   left.Typename() + "', '" + right.Typename() + "'" );
        _type = Type::Int();
    }
    else if ( arg._type == BINARY_OP_EQ || arg._type == BINARY_OP_NE ) {
        if ( left.Generalise( right ) == Type::Void() &&
             left != Type::Void() && right != Type::Void() )
            Error( arg, "cannot apply '" + op + "' to '" +
                   left.Typename() + "', '" + right.Typename() + "'" );
        _type = Type::Int();
    }
    else if ( arg._type == BINARY_OP_GT || arg._type == BINARY_OP_GE ||
              arg._type == BINARY_OP_LT || arg._type == BINARY_OP_LE ) {
        if ( ( !left.ConvertsTo( Type::Int() ) && !left.ConvertsTo( Type::Float() ) &&
               left != Type::Void() ) ||
             ( !right.ConvertsTo( Type::Int() ) && !right.ConvertsTo( Type::Float() ) &&
               right != Type::Void() ) )
            Error( arg, "cannot apply '" + op + "' to '" +
                   left.Typename() + "', '" + right.Typename() + "'" );
        _type = Type::Int();
    }
    else if ( arg._type == BINARY_OP_ADD || arg._type == BINARY_OP_SUB ||
              arg._type == BINARY_OP_MUL || arg._type == BINARY_OP_DIV ||
              arg._type == BINARY_OP_MOD || arg._type == BINARY_OP_EXP ) {
        // TODO: const-checking for illegal operations?
        if ( ( !left.ConvertsTo( Type::Int() ) && !left.ConvertsTo( Type::Float() ) &&
               left != Type::Void() ) ||
             ( !right.ConvertsTo( Type::Int() ) && !right.ConvertsTo( Type::Float() ) &&
               right != Type::Void() ) )
            Error( arg, "cannot apply '" + op + "' to '" +
                   left.Typename() + "', '" + right.Typename() + "'" );
        _type = left.ConvertsTo( Type::Int() ) && right.ConvertsTo( Type::Int() ) ?
                Type::Int() : Type::Float();
    }
    else {
        Error( arg, "unsupported binary operation" );
        _type = Type::Void();
    }
}


IMPLEMENT( UnaryOp )
{
    Operate( arg._expr );
    std::string op = arg._type == UNARY_OP_NOT ? "!" :
                     arg._type == UNARY_OP_BIT_NOT ? "\\" :
                     arg._type == UNARY_OP_NEGATION ? "-" :
                     arg._type == UNARY_OP_FLOOR ? "[]" : "";

    if ( arg._type == UNARY_OP_NOT || arg._type == UNARY_OP_BIT_NOT ) {
        if ( !_type.ConvertsTo( Type::Int() ) && _type != Type::Void() )
            Error( arg, "cannot apply '" + op + "' to '" + _type.Typename() + "'" );
        _type = Type::Int();
    }
    else if ( arg._type == UNARY_OP_NEGATION ) {
        if ( !_type.ConvertsTo( Type::Int() ) && !_type.ConvertsTo( Type::Float() ) &&
             _type != Type::Void() )
            Error( arg, "cannot apply '" + op + "' to '" + _type.Typename() + "'" );
        _type = _type.ConvertsTo( Type::Int() ) ? Type::Int() : Type::Float();
    }
    else if ( arg._type == UNARY_OP_FLOOR ) {
        if ( _type != Type::Float() && _type != Type::Void() )
            Error( arg, "cannot apply '" + op + "' to '" + _type.Typename() + "'" );
        _type = Type::Int();
    }
    else {
        Error( arg, "unsupported unary operation" );
        _type = Type::Void();
    }
}

IMPLEMENT( TypeOp )
{
    if ( arg._left )
        Operate( arg._left );
    if ( arg._right )
        Operate( arg._right );
    _type = Type::Int();
}

IMPLEMENT( TupleConstruct )
{
    Type::TypeList list;
    for ( std::size_t i = 0; i < arg._list.size(); ++i ) {
        Operate( arg._list[ i ] );
        list.push_back( _type );
    }
    _type = Type::Tuple( list );
}

IMPLEMENT( TupleExtract )
{
    Operate( arg._tuple );
    if ( _type == Type::Void() )
        return;

    if ( !_type.IsTuple() ) {
        Error( arg, "cannot apply '[]' to '" + _type.Typename() + "'" );
        _type = Type::Void();
        return;
    }
    if ( arg._index < 0 || arg._index >= _type.TypeArgs().size() ) {
        std::stringstream ss;
        ss << "cannot apply '[" << arg._index << "]' to '" << _type.Typename() << "'";
        Error( arg, ss.str() );
        _type = Type::Void();
        return;
    }
    _type = _type.TypeArgs()[ arg._index ];
}

IMPLEMENT( TupleReplace )
{
    Operate( arg._tuple );
    Type tuple = _type;
    Operate( arg._expr );
    Type expr = _type;

    if ( tuple == Type::Void() ) {
        _type = Type::Void();
        return;
    }

    if ( !tuple.IsTuple() ) {
        Error( arg, "cannot apply '[/]' to '" + tuple.Typename() + "'" );
        _type = tuple;
        return;
    }
    if ( arg._index < 0 || arg._index >= tuple.TypeArgs().size() ) {
        std::stringstream ss;
        ss << "cannot apply '[" << arg._index << "/]' to '" << tuple.Typename() << "'";
        Error( arg, ss.str() );
        _type = tuple;
        return;
    }
    Type::TypeList list;
    for ( std::size_t i = 0; i < tuple.TypeArgs().size(); ++i )
        list.push_back( i == arg._index ? expr : tuple.TypeArgs()[ i ] );
    _type = Type::Tuple( list );
}

IMPLEMENT( FunctionCall )
{
    Operate( arg._function );
    Type function = _type;
    Type::TypeList list;
    for ( std::size_t i = 0; i < arg._args.size(); ++i ) {
        Operate( arg._args[ i ] );
        list.push_back( _type );
    }

    if ( !function.IsFunction() && !function.IsSequence() ) {
        if ( function != Type::Void() )
            Error( arg, "cannot apply '()' to '" + function.Typename() + "'" );
        _type = Type::Void();
        return;
    }

    for ( std::size_t i = 0; i < list.size() && i < function.TypeArgs().size(); ++i ) {
        if ( !list[ i ].ConvertsTo( function.TypeArgs()[ i ] ) && list[ i ] != Type::Void() &&
             function.TypeArgs()[ i ] != Type::Void() )
            Error( *arg._args[ i ], "argument: cannot convert '" + list[ i ].Typename() +
                   "' to '" + function.TypeArgs()[ i ].Typename() + "'" );
    }
    
    if ( list.size() > function.TypeArgs().size() ) {
        Error( arg, "too many arguments for '" + function.Typename() + "'" );
        _type = function.IsFunction() ?
                function.ReturnType() : Type::Sequence( Type::TypeList() );
        return;
    }

    if ( list.size() == function.TypeArgs().size() ) {
        _type = function.IsFunction() ?
                function.ReturnType() : Type::Sequence( Type::TypeList() );
        return;
    }

    Type::TypeList new_list;
    for ( std::size_t i = list.size(); i < function.TypeArgs().size(); ++i ) {
        new_list.push_back( function.TypeArgs()[ i ] );
    }
    _type = function.IsFunction() ?
        Type::Function( function.ReturnType(), new_list ) : Type::Sequence( new_list );
}

IMPLEMENT( Body )
{
    bool b = _return_path;
    bool unreachable = false;
    _return_path = false;
    for ( std::size_t i = 0; i < arg._steps.size(); ++i ) {
        if ( _return_path && !unreachable ) {
            unreachable = true;
            Error( *arg._steps[ i ], "unreachable code" );
        }
        Operate( arg._steps[ i ] );
    }
    _type = Type::Void();
    _return_path = b || _return_path;
}

IMPLEMENT( Return )
{
    Operate( arg._expr );
    if ( _return_type == Type::Void() )
        _return_type = _type;
    else {
        Type t = _return_type.Generalise( _type );
        if ( t == Type::Void() ) {
            if ( _return_type != Type::Void() && _type != Type::Void() )
                Error( arg, "return value: cannot generalise '" + _return_type.Typename() +
                       "' with '" + _type.Typename() + "'" );
        }
        else
            _return_type = t;
    }
    _return_path = true;
    _type = Type::Void();
}

IMPLEMENT( Guard )
{
    bool b = _return_path;
    _return_path = false;
    Operate( arg._expr );
    if ( !_type.ConvertsTo( Type::Int() ) && _type != Type::Void() )
        Error( arg, "condition: cannot convert '" + _type.Typename() +
               "' to '" + Type::Int().Typename() + "'" );
    Operate( arg._then );
    if ( arg._otherwise )
        Operate( arg._otherwise );
    _type = Type::Void();
    _return_path = b;
}

IMPLEMENT( Let )
{
    bool b = _return_path;
    _return_path = false;
    Operate( arg._expr );
    _table.Push();
    _table.AddEntry( arg._id, _type );
    Operate( arg._in );
    _table.Pop();
    _type = Type::Void();
    _return_path = b || _return_path;
}

IMPLEMENT( Block )
{
    Operate( arg._scope_set );
    for ( std::size_t i = 0; i < arg._statements.size(); ++i )
        Operate( arg._statements[ i ] );
    _type = Type::Void();
}

IMPLEMENT( ScopeSet )
{
    for ( std::size_t i = 0; i < arg._scope_defs.size(); ++i )
        Operate( arg._scope_defs[ i ] );
    _type = Type::Void();
}

IMPLEMENT( ScopeDef )
{
    // TODO: id-check and type-check, constant-checking for flags?
    Operate( arg._expr );
    _type = Type::Void();
}

IMPLEMENT( Loop )
{
    Operate( arg._begin );
    Type begin = _type;
    Operate( arg._end );
    Type end = _type;
    if ( !begin.ConvertsTo( Type::Int() ) && begin != Type::Void() )
        Error( arg, "loop index: cannot convert '" + begin.Typename() +
               "' to '" + Type::Int().Typename() + "'" );
    if ( !end.ConvertsTo( Type::Int() ) && end != Type::Void() )
        Error( arg, "loop index: cannot convert '" + end.Typename() +
               "' to '" + Type::Int().Typename() + "'" );

    _table.Push();
    _table.AddEntry( arg._id, Type::Int() );
    Operate( arg._in );
    _table.Pop();
    _type = Type::Void();
}

IMPLEMENT( SequenceCall )
{
    Operate( arg._sequence );
    Type sequence = _type;
    Type::TypeList list;
    for ( std::size_t i = 0; i < arg._args.size(); ++i ) {
        Operate( arg._args[ i ] );
        list.push_back( _type );
    }

    if ( !sequence.IsSequence() ) {
        if ( sequence != Type::Void() )
            Error( arg, "cannot call '" + sequence.Typename() + "'" );
        _type = Type::Void();
        return;
    }

    for ( std::size_t i = 0; i < list.size() && i < sequence.TypeArgs().size(); ++i ) {
        if ( !list[ i ].ConvertsTo( sequence.TypeArgs()[ i ] ) && list[ i ] != Type::Void() &&
             sequence.TypeArgs()[ i ] != Type::Void() )
            Error( *arg._args[ i ], "argument: cannot convert '" + list[ i ].Typename() +
                   "' to '" + sequence.TypeArgs()[ i ].Typename() + "'" );
    }
    
    if ( list.size() > sequence.TypeArgs().size() )
        Error( arg, "too many arguments for '" + sequence.Typename() + "'" );

    if ( list.size() < sequence.TypeArgs().size() )
        Error( arg, "too few arguments for '" + sequence.Typename() + "'" );

    _type = Type::Void();
}

IMPLEMENT( FxStatement )
{
    // TODO: fx type check
    Operate( arg._expr );
    _type = Type::Void();
}

IMPLEMENT( SplitStatement )
{
    for ( std::size_t i = 0; i < arg._layers.size(); ++i )
        Operate( arg._layers[ i ] );
    _type = Type::Void();
}

IMPLEMENT( Layer )
{
    // TODO: allow all combinations in grammar and check here
    Type order = Type::Void();
    Type fx = Type::Void();
    if ( arg._order ) {
        Operate( arg._order );
        order = _type;
    }
    if ( arg._fx ) {
        Operate( arg._fx );
        fx = _type;
    }
    if ( arg._order && !order.ConvertsTo( Type::Int() ) && order != Type::Void() )
        Error( arg, "layer index: cannot convert '" + order.Typename() +
               "' to '" + Type::Int().Typename() + "'" );
    // TODO: fx type check
    Operate( arg._statement );
    _type = Type::Void();
}

IMPLEMENT( Argument )
{
    if ( _declarations ) {
        _declaration_list.push_back( arg._type );
        return;
    }
    if ( !_table.AddEntry( arg._id, arg._type ) )
        Error( arg, "identifier '" + arg._id + "' already defined" );
    _type = Type::Void();
}

IMPLEMENT( FuncDef )
{
    if ( _declarations ) {
        if ( _declare_globals && arg._modifiers & MODIFIER_LOCAL )
            return;
        _declaration_list.clear();
        for ( std::size_t i = 0; i < arg._args.size(); ++i )
            Operate( arg._args[ i ] );
        if ( !_table.AddEntry( arg._id, Type::Function( arg._return_type, _declaration_list ) ) )
            Error( arg, "identifier '" + arg._id + "' already defined" );
        return;
    }

    if ( arg._modifiers & MODIFIER_LOCAL && _table.Depth() == 0 )
        Error( arg, "'local' modifier illegal at global scope" );
    _table.Push();
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
    _return_type = Type::Void();
    _return_path = false;
    Operate( arg._expr );
    if ( !_return_type.ConvertsTo( arg._return_type ) && _return_type != Type::Void() )
        Error( arg, "'" + arg._id + "': cannot convert '" + _return_type.Typename() +
               "' to '" + arg._return_type.Typename() + "'" );
    if ( !_return_path )
        Error( arg, "'" + arg._id + "': not all code paths return a value" );
    _table.Pop();
    _type = Type::Void();
}

IMPLEMENT( SeqDef )
{
    if ( _declarations ) {
        if ( _declare_globals && arg._modifiers & MODIFIER_LOCAL )
            return;
        _declaration_list.clear();
        for ( std::size_t i = 0; i < arg._args.size(); ++i )
            Operate( arg._args[ i ] );
        if ( !_table.AddEntry( arg._id, Type::Sequence( _declaration_list ) ) )
            Error( arg, "identifier '" + arg._id + "' already defined" );
        return;
    }

    if ( arg._modifiers & MODIFIER_CACHE )
        Error( arg, "'cache' modifier illegal on sequence definitions" );
    if ( arg._modifiers & MODIFIER_LOCAL && _table.Depth() == 0 )
        Error( arg, "'local' modifier illegal at global scope" );
    _table.Push();
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
    Operate( arg._statement );
    _table.Pop();
    _type = Type::Void();
}

IMPLEMENT( VidDef )
{
    if ( _declarations )
        return;

    if ( arg._modifiers & MODIFIER_CACHE )
        Error( arg, "'cache' modifier illegal on video definitions" );
    if ( arg._modifiers & MODIFIER_LOCAL )
        Error( arg, "'local' modifier illegal on video definitions" );
    Operate( arg._frame_count );
    if ( !_type.ConvertsTo( Type::Int() ) && _type != Type::Void() )
        Error( arg, "frame count: cannot convert '" + _type.Typename() +
               "' to '" + Type::Int().Typename() + "'" );
    _table.Push();
    _table.AddEntry( "frame", Type::Int() );
    Operate( arg._statement );
    _table.Pop();
    _type = Type::Void();
}

IMPLEMENT( TypeDef )
{
    if ( _declarations )
        return;

    if ( arg._modifiers & MODIFIER_CACHE )
        Error( arg, "'cache' modifier illegal on type definitions" );
    if ( arg._modifiers & MODIFIER_LOCAL && _table.Depth() == 0 )
        Error( arg, "'local' modifier illegal at global scope" );
    _type = Type::Void();
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
    _type = Type::Void();
}