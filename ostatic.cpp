#include "ostatic.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"

#define IMPLEMENT_OPERATOR StaticOperator
#define IMPLEMENT_TYPE IMPLEMENT_NON_CONST

StaticOperator::StaticOperator( int* errors )
: _errors( errors )
, _type( Type::Void() )
, _return_type( Type::Void() )
, _return_path( false )
, _let_variables( false )
, _let_guard( false )
, _let_type( Type::Void() )
{
}

StaticOperator::~StaticOperator()
{
}

const Type& StaticOperator::InferredType() const
{
    return _type;
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

Ast* StaticOperator::Convert( Ast* expr, Type from, Type to )
{
    if ( from == Type::Void() || to == Type::Void() )
        return expr;
    if ( !from.ConvertsTo( to ) ) {
        Error( *expr, "conversion: cannot convert `" + from.Typename() + "' to `" + to.Typename() + "'" );
        return expr;
    }
    if ( from.Equivalent( to ) )
        return expr;
    return new Converter( expr, from, to );
}

/***************************************************************
* Implementations
***************************************************************/

IMPLEMENT( ParseError )
{
    _type = Type::Void();
}

IMPLEMENT( Constant )
{
    _type = arg._is_int ? Type::Int() : Type::Float();
}

IMPLEMENT( Identifier )
{
    if ( _let_variables ) {
        if ( !_table.AddEntry( arg._id, _let_type ) )
            Error( arg, "identifier `" + arg._id + "' already declared in this scope" );
        if ( arg._id.find( "." ) != std::string::npos )
            Error( arg, "scoped variable names are illegal" );
        return;
    }
    if ( !_table.HasEntry( arg._id ) )
        Error( arg, "undeclared identifier `" + arg._id + "'" );
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

    if ( !expr.ConvertsTo( Type::Bool() ) && expr != Type::Void() )
        Error( arg, "cannot apply `?' to `" + expr.Typename() + "'" );
    else
        arg._expr = Convert( arg._expr, expr, Type::Bool() );
    _type = left.Generalise( right );
    if ( _type == Type::Void() && left != Type::Void() && right != Type::Void() )
        Error( arg, "`?': cannot generalise `" + left.Typename() +
               "' with `" + right.Typename() + "'" );
    arg._left = Convert( arg._left, left, _type );
    arg._right = Convert( arg._right, right, _type );
    arg._value_type = _type;
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

    if ( arg._type == BINARY_OP_OR || arg._type == BINARY_OP_AND ) {
        if ( !( left.ConvertsTo( Type::Bool() ) && left != Type::Void() ) ||
             !( right.ConvertsTo( Type::Bool() ) && right != Type::Void() ) )
            Error( arg, "cannot apply `" + op + "' to `" +
                   left.Typename() + "', `" + right.Typename() + "'" );
        else {
            arg._left = Convert( arg._left, left, Type::Bool() );
            arg._right = Convert( arg._right, right, Type::Bool() );
        }
        _type = Type::Bool();
        arg._op_type = Type::Bool();
    }
    if ( arg._type == BINARY_OP_BIT_OR || arg._type == BINARY_OP_BIT_AND ||
         arg._type == BINARY_OP_BIT_XOR ||
         arg._type == BINARY_OP_LSHIFT || arg._type == BINARY_OP_RSHIFT ) {
        if ( !( left.ConvertsTo( Type::Int() ) && left != Type::Void() ) ||
             !( right.ConvertsTo( Type::Int() ) && right != Type::Void() ) )
            Error( arg, "cannot apply `" + op + "' to `" +
                   left.Typename() + "', `" + right.Typename() + "'" );
        else {
            arg._left = Convert( arg._left, left, Type::Int() );
            arg._right = Convert( arg._right, right, Type::Int() );
        }
        _type = Type::Int();
        arg._op_type = Type::Int();
    }
    else if ( arg._type == BINARY_OP_EQ || arg._type == BINARY_OP_NE ) {
        Type type = left.Generalise( right );
        if ( type == Type::Void() &&
             left != Type::Void() && right != Type::Void() )
            Error( arg, "cannot apply `" + op + "' to `" +
                   left.Typename() + "', `" + right.Typename() + "'" );
        else {
            arg._left = Convert( arg._left, left, type );
            arg._right = Convert( arg._right, right, type );
        }
        _type = Type::Bool();
        arg._op_type = type;
    }
    else if ( arg._type == BINARY_OP_GT || arg._type == BINARY_OP_GE ||
              arg._type == BINARY_OP_LT || arg._type == BINARY_OP_LE ) {
        if ( ( !left.ConvertsTo( Type::Int() ) && !left.ConvertsTo( Type::Float() ) &&
               left != Type::Void() ) ||
             ( !right.ConvertsTo( Type::Int() ) && !right.ConvertsTo( Type::Float() ) &&
               right != Type::Void() ) )
            Error( arg, "cannot apply `" + op + "' to `" +
                   left.Typename() + "', `" + right.Typename() + "'" );
        else {
            Type type = left.Generalise( right );
            arg._left = Convert( arg._left, left, type );
            arg._right = Convert( arg._right, right, type );
            arg._op_type = type;
        }
        _type = Type::Bool();
    }
    else if ( arg._type == BINARY_OP_ADD || arg._type == BINARY_OP_SUB ||
              arg._type == BINARY_OP_MUL || arg._type == BINARY_OP_DIV ||
              arg._type == BINARY_OP_MOD || arg._type == BINARY_OP_EXP ) {
        // TODO: const-checking for illegal operations? <-- I think that means check for expr/0 etc
        if ( ( !left.ConvertsTo( Type::Int() ) && !left.ConvertsTo( Type::Float() ) &&
               left != Type::Void() ) ||
             ( !right.ConvertsTo( Type::Int() ) && !right.ConvertsTo( Type::Float() ) &&
               right != Type::Void() ) ) {
            Error( arg, "cannot apply `" + op + "' to `" +
                   left.Typename() + "', `" + right.Typename() + "'" );
            _type = Type::Float();
        }
        else {
            _type = left.Generalise( right );
            arg._op_type = _type;
            arg._left = Convert( arg._left, left, _type );
            arg._right = Convert( arg._right, right, _type );
        }
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

    if ( arg._type == UNARY_OP_NOT ) {
        if ( !_type.ConvertsTo( Type::Bool() ) && _type != Type::Void() )
            Error( arg, "cannot apply `" + op + "' to `" + _type.Typename() + "'" );
        else
            arg._expr = Convert( arg._expr, _type, Type::Bool() );
        _type = Type::Bool();
    }        
    else if ( arg._type == UNARY_OP_BIT_NOT ) {
        if ( !_type.ConvertsTo( Type::Int() ) && _type != Type::Void() )
            Error( arg, "cannot apply `" + op + "' to `" + _type.Typename() + "'" );
        else
            arg._expr = Convert( arg._expr, _type, Type::Int() );
        _type = Type::Int();
    }
    else if ( arg._type == UNARY_OP_NEGATION ) {
        if ( !_type.ConvertsTo( Type::Int() ) && !_type.ConvertsTo( Type::Float() ) &&
             _type != Type::Void() )
            Error( arg, "cannot apply `" + op + "' to `" + _type.Typename() + "'" );
        else
            arg._expr = Convert( arg._expr, _type, _type.ConvertsTo( Type::Int() ) ? Type::Int() : Type::Float() );
        arg._op_type = _type = _type.ConvertsTo( Type::Int() ) ? Type::Int() : Type::Float();
    }
    else if ( arg._type == UNARY_OP_FLOOR ) {
        if ( _type != Type::Float() && _type != Type::Void() )
            Error( arg, "cannot apply `" + op + "' to `" + _type.Typename() + "'" );
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
    if ( _let_variables ) {
        Type t = _let_type;
        if ( !t.IsTuple() )
            Error( arg, "let variables: " + t.Typename() + " is not a tuple" );
        else if ( t.TypeArgs().size() != arg._list.size() ) {
            std::stringstream ss;
            ss << "let variables: " << t.Typename() << " is not a " << arg._list.size() << "-tuple";
            Error( arg, ss.str() );
        }
        for ( std::size_t i = 0; i < arg._list.size() && i < t.TypeArgs().size() && t.IsTuple(); ++i ) {
            _let_type = t.TypeArgs()[ i ];
            Operate( arg._list[ i ] );
        }
        _let_type = t;
        return;
    }
    Type::TypeList list;
    for ( std::size_t i = 0; i < arg._list.size(); ++i ) {
        Operate( arg._list[ i ] );
        list.push_back( _type );
    }
    _type = arg._value_type = Type::Tuple( list );
}

IMPLEMENT( TupleExtract )
{
    Operate( arg._tuple );
    if ( _type == Type::Void() )
        return;

    bool fail = false;
    if ( !_type.IsTuple() ) {
        Error( arg, "cannot apply `[]' to `" + _type.Typename() + "'" );
        fail = true;
    }
    SubtreeConstraintOperator oconstraint( _table );
    oconstraint.Operate( arg._index );
    if ( !oconstraint.ConstantInt() ) {
        Error( arg, "tuple-extract: index must be constant int" );
        fail = true;
    }
    if ( fail ) {
        _type = Type::Void();
        return;
    }
    rave_int index = oconstraint.ConstantIntValue();
    if ( index < 0 || index >= signed( _type.TypeArgs().size() ) ) {
        std::stringstream ss;
        ss << "cannot apply `[" << index << "]' to `" << _type.Typename() << "'";
        Error( arg, ss.str() );
        _type = Type::Void();
        return;
    }
    arg._constant_index = index;
    _type = _type.TypeArgs()[ index ];
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

    bool fail = false;
    if ( !tuple.IsTuple() ) {
        Error( arg, "cannot apply `[:=]' to `" + tuple.Typename() + "'" );
        fail = true;
    }
    SubtreeConstraintOperator oconstraint( _table );
    oconstraint.Operate( arg._index );
    if ( !oconstraint.ConstantInt() ) {
        Error( arg, "tuple-replace: index must be constant int" );
        fail = true;
    }
    if ( fail ) {
        _type = Type::Void();
        return;
    }
    rave_int index = oconstraint.ConstantIntValue();
    if ( index < 0 || index >= signed( tuple.TypeArgs().size() ) ) {
        std::stringstream ss;
        ss << "cannot apply `[" << index << "//]' to `" << tuple.Typename() << "'";
        Error( arg, ss.str() );
        _type = tuple;
        return;
    }
    Type::TypeList list;
    for ( std::size_t i = 0; i < tuple.TypeArgs().size(); ++i )
        list.push_back( signed( i ) == index ? expr : tuple.TypeArgs()[ i ] );
    arg._constant_index = index;
    _type = arg._value_type = Type::Tuple( list );
}

IMPLEMENT( FunctionCall )
{
    if ( _let_variables ) {
        _let_guard = true;
        SubtreeConstraintOperator oconstraint( _table );
        oconstraint.Operate( arg._function );
        const std::string& identifier = oconstraint.SingleIdentifierName();
        if ( !oconstraint.SingleIdentifier() )
            Error( arg, "let variables: constructor must be an identifier" );
        else if ( !_table.HasEntry( identifier ) )
            Error( arg, "let variables: undeclared identifier `" + identifier + "'" );
        else if ( !_table.HasEntry( "constructor:" + identifier ) )
            Error( arg, "let variables: `" + identifier + "' is not a constructor" );
        else {
            const Type& constructor = _table.GetEntry( identifier );
            Type t = _let_type;
            if ( !t.Equivalent( constructor.ReturnType() ) ) // TODO ~ ConvertsTo, maybe? (need some conversion somewhere)
                Error( arg, "let variables: cannot convert `" + t.Typename() + "' to `" + constructor.ReturnType().Typename() );
            if ( arg._args.size() < constructor.TypeArgs().size() )
                Error( arg, "let variables: too few arguments for constructor `" + identifier + "'" );
            else if ( arg._args.size() > constructor.TypeArgs().size() )
                Error( arg, "let variables: too many arguments for constructor `" + identifier + "'" );
            for ( std::size_t i = 0; i < arg._args.size() && i < constructor.TypeArgs().size(); ++i ) {
                _let_type = constructor.TypeArgs()[ i ];
                Operate( arg._args[ i ] );
            }
            _let_type = t;
        }
        return;
    }

    Operate( arg._function );
    Type function = _type;
    Type::TypeList list;
    for ( std::size_t i = 0; i < arg._args.size(); ++i ) {
        Operate( arg._args[ i ] );
        list.push_back( _type );
    }

    if ( !function.IsFunction() && !function.IsSequence() ) {
        if ( function != Type::Void() )
            Error( arg, "cannot apply `()' to `" + function.Typename() + "'" );
        _type = Type::Void();
        return;
    }

    for ( std::size_t i = 0; i < list.size() && i < function.TypeArgs().size(); ++i ) {
        if ( !list[ i ].ConvertsTo( function.TypeArgs()[ i ] ) && list[ i ] != Type::Void() &&
             function.TypeArgs()[ i ] != Type::Void() )
            Error( *arg._args[ i ], "argument: cannot convert `" + list[ i ].Typename() +
                   "' to `" + function.TypeArgs()[ i ].Typename() + "'" );
        else
            arg._args[ i ] = Convert( arg._args[ i ], list[ i ], function.TypeArgs()[ i ] );
    }
    
    if ( list.size() > function.TypeArgs().size() ) {
        Error( arg, "too many arguments for `" + function.Typename() + "'" );
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

IMPLEMENT( Converter )
{
    Operate( arg._expr );
    if ( !_type.ConvertsTo( arg._to ) )
        Error( arg, "cannot convert `" + _type.Typename() + "' to `" + arg._to.Typename() + "'" );
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

    if ( !_type.ConvertsTo( _return_type ) && _type != Type::Void() )
        Error( arg, "return value: cannot convert `" + _type.Typename() +
                    "' to `" + _return_type.Typename() + "'" );
    else
        arg._expr = Convert( arg._expr, _type, _return_type );
    _return_path = true;
    _type = Type::Void();
}

IMPLEMENT( Guard )
{
    bool b = _return_path;
    _return_path = false;
    Operate( arg._expr );
    if ( !_type.ConvertsTo( Type::Bool() ) && _type != Type::Void() )
        Error( arg, "condition: cannot convert `" + _type.Typename() +
               "' to `" + Type::Bool().Typename() + "'" );
    else
        arg._expr = Convert( arg._expr, _type, Type::Bool() );
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
    SubtreeConstraintOperator oconstraint( _table );
    oconstraint.Operate( arg._ids );
    bool let_guard = false;
    if ( !oconstraint.ValidLetVariables() )
        Error( *arg._ids, "let variables: only identifiers, tuples and constructors are legal" );
    else {
        _let_variables = true;
        _let_guard = false;
        _let_type = _type;
        Operate( arg._ids );
        _let_variables = false;
        let_guard = _let_guard;
        _let_guard = false;
    }
    Operate( arg._in );
    _table.Pop();
    _type = Type::Void();
    _return_path = b || ( _return_path && !let_guard );
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
        Error( *arg._begin, "loop index: cannot convert `" + begin.Typename() +
               "' to `" + Type::Int().Typename() + "'" );
    else
        arg._begin = Convert( arg._begin, _type, Type::Int() );
    if ( !end.ConvertsTo( Type::Int() ) && end != Type::Void() )
        Error( *arg._end, "loop index: cannot convert `" + end.Typename() +
               "' to `" + Type::Int().Typename() + "'" );
    else
        arg._end = Convert( arg._end, _type, Type::Int() );

    _table.Push();
    SubtreeConstraintOperator oconstraint( _table );
    oconstraint.Operate( arg._id );
    if ( !oconstraint.SingleIdentifier() )
        Error( *arg._id, "loop variable: must be identifier" );
    else {
        _let_variables = true;
        _let_type = Type::Int();
        Operate( arg._id );
        _let_variables = false;
    }
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
            Error( arg, "cannot call `" + sequence.Typename() + "'" );
        _type = Type::Void();
        return;
    }

    for ( std::size_t i = 0; i < list.size() && i < sequence.TypeArgs().size(); ++i ) {
        if ( !list[ i ].ConvertsTo( sequence.TypeArgs()[ i ] ) && list[ i ] != Type::Void() &&
             sequence.TypeArgs()[ i ] != Type::Void() )
            Error( *arg._args[ i ], "argument: cannot convert `" + list[ i ].Typename() +
                   "' to `" + sequence.TypeArgs()[ i ].Typename() + "'" );
        else
            arg._args[ i ] = Convert( arg._args[ i ], list[ i ], sequence.TypeArgs()[ i ] );
    }
    
    if ( list.size() > sequence.TypeArgs().size() )
        Error( arg, "too many arguments for `" + sequence.Typename() + "'" );

    if ( list.size() < sequence.TypeArgs().size() )
        Error( arg, "too few arguments for `" + sequence.Typename() + "'" );

    _type = Type::Void();
}

IMPLEMENT( FxStatement )
{
    // TODO: fx type check and Convert
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
    Type order = Type::Void();
    Type fx = Type::Void();
    if ( arg._order ) {
        Operate( arg._order );
        order = _type;
        if ( arg._type & LAYER_BASE )
            Error( arg, "layer index illegal on `base' layer" );
    }
    else if ( arg._type & LAYER_COPY )
        Error( arg, "layer index required on `copy' layer" );
    else if ( arg._type & LAYER_LAYER )
        Error( arg, "layer index required on `layer' layer" );
    if ( arg._fx ) {
        Operate( arg._fx );
        fx = _type;
        if ( arg._type & LAYER_BASE )
            Error( arg, "merge fx illegal on `base' layer" );
    }
    if ( arg._order && !order.ConvertsTo( Type::Int() ) && order != Type::Void() )
        Error( arg, "layer index: cannot convert `" + order.Typename() +
               "' to `" + Type::Int().Typename() + "'" );
    else if ( arg._order )
        arg._order = Convert( arg._order, order, Type::Int() );
    // TODO: fx type check and Convert
    Operate( arg._statement );
    _type = Type::Void();
}

IMPLEMENT( Argument )
{
    if ( !_table.AddEntry( arg._id, arg._type ) )
        Error( arg, "identifier `" + arg._id + "' already declared in this scope" );
    _type = Type::Void();
}

IMPLEMENT( FuncDef )
{
    if ( arg._modifiers & MODIFIER_LOCAL && !_table.Depth() )
        Error( arg, "`local' modifier illegal at global scope" );
    _table.Push();
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
    _return_type = arg._return_type;
    _return_path = false;
    Operate( arg._expr );
    if ( !_return_path )
        Error( arg, "`" + arg._id + "': not all code paths return a value" );
    _table.Pop();
    _type = Type::Void();
}

IMPLEMENT( SeqDef )
{
    if ( arg._modifiers & MODIFIER_CACHE )
        Error( arg, "`cache' modifier illegal on sequence definitions" );
    if ( arg._modifiers & MODIFIER_LOCAL && !_table.Depth() )
        Error( arg, "`local' modifier illegal at global scope" );
    _table.Push();
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
    Operate( arg._statement );
    _table.Pop();
    _type = Type::Void();
}

IMPLEMENT( VidDef )
{
    if ( arg._modifiers & MODIFIER_CACHE )
        Error( arg, "`cache' modifier illegal on video definitions" );
    if ( arg._modifiers & MODIFIER_LOCAL )
        Error( arg, "`local' modifier illegal on video definitions" );
    if ( !_table.AddEntry( "video::" /*+ _namespace*/ + arg._id, Type::Sequence( Type::TypeList() ) ) )
        Error( arg, "video `" + arg._id + "' already declared in this scope" );
    Operate( arg._frame_count );
    if ( !_type.ConvertsTo( Type::Int() ) && _type != Type::Void() )
        Error( arg, "frame count: cannot convert `" + _type.Typename() +
               "' to `" + Type::Int().Typename() + "'" );
    else
        arg._frame_count = Convert( arg._frame_count, _type, Type::Int() );
    _table.Push();
    _table.AddEntry( "frame", Type::Int() );
    Operate( arg._statement );
    _table.Pop();
    _type = Type::Void();
}

IMPLEMENT( TypeDef )
{
    if ( arg._modifiers & MODIFIER_CACHE )
        Error( arg, "`cache' modifier illegal on type definitions" );
    if ( arg._modifiers & MODIFIER_LOCAL && !_table.Depth() )
        Error( arg, "`local' modifier illegal at global scope" );
    _type = Type::Void();
}

IMPLEMENT( Program )
{
    if ( arg._modifiers & MODIFIER_CACHE )
        Error( arg, "`cache' modifier illegal on namespaces" );
    if ( arg._modifiers & MODIFIER_LOCAL && !_table.Depth() )
        Error( arg, "`local' modifier illegal at global scope" );
    if ( arg._modifiers & MODIFIER_LOCAL && arg._scope_name.empty() )
        Error( arg, "`local' modifier illegal on anonymous namespaces" );
    _table.Push();
    DeclareOperator odeclare( *this );
    odeclare.Operate( &arg );
    for ( std::size_t i = 0; i < arg._elements.size(); ++i )
        Operate( arg._elements[ i ] );
    _table.Pop();
    _type = Type::Void();
}