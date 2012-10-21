#include "ostring.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"
#include <iomanip>

#define IMPLEMENT_OPERATOR StringOperator
#define IMPLEMENT_TYPE IMPLEMENT_CONST

StringOperator::StringOperator()
    : _indent( 0 )
{
    _result << std::setprecision( 8 ) << std::fixed;
}

StringOperator::~StringOperator()
{
}

std::string StringOperator::Result() const
{
    return _result.str();
}

std::string StringOperator::Indent() const
{
    return std::string( 4 * _indent, ' ' );
}

std::string modifiers( int modifiers )
{
    std::string s = "";
    if ( modifiers & MODIFIER_LOCAL )
        s += "local ";
    if ( modifiers & MODIFIER_CACHE )
        s += "cache ";
    return s;
}

/***************************************************************
* Implementations
***************************************************************/

IMPLEMENT( ParseError )
{
    _result << "<<ERROR>>";
}

IMPLEMENT( Constant )
{
    if ( arg._is_int )
        _result << arg._int_value;
    else
        _result << arg._float_value;
}

IMPLEMENT( Identifier )
{
    _result << arg._id;
}

IMPLEMENT( TernaryOp )
{
    _result << "(";
    Operate( arg._expr );
    _result << " ? ";
    Operate( arg._left );
    _result << " : ";
    Operate( arg._right );
    _result << ")";
}

IMPLEMENT( BinaryOp )
{
    _result << "(";
    Operate( arg._left );
    _result << ( arg._type == BINARY_OP_OR ? " || " :
                 arg._type == BINARY_OP_AND ? " && " :
                 arg._type == BINARY_OP_EQ ? " == " :
                 arg._type == BINARY_OP_NE ? " != " :
                 arg._type == BINARY_OP_GT ? " > " :
                 arg._type == BINARY_OP_GE ? " >= " :
                 arg._type == BINARY_OP_LT ? " < " :
                 arg._type == BINARY_OP_LE ? " <= " :
                 arg._type == BINARY_OP_BIT_OR ? " | " :
                 arg._type == BINARY_OP_BIT_AND ? " & " :
                 arg._type == BINARY_OP_BIT_XOR ? " ' " :
                 arg._type == BINARY_OP_LSHIFT ? " << " :
                 arg._type == BINARY_OP_RSHIFT ? " >> " :
                 arg._type == BINARY_OP_ADD ? " + " :
                 arg._type == BINARY_OP_SUB ? " - " :
                 arg._type == BINARY_OP_MUL ? " * " :
                 arg._type == BINARY_OP_DIV ? " / " :
                 arg._type == BINARY_OP_MOD ? " % " :
                 arg._type == BINARY_OP_EXP ? "^" : "" );
    Operate( arg._right );
    _result << ")";
}

IMPLEMENT( UnaryOp )
{
    _result << ( arg._type == UNARY_OP_NOT ? "!" :
                 arg._type == UNARY_OP_BIT_NOT ? "\\" :
                 arg._type == UNARY_OP_NEGATION ? "-" :
                 arg._type == UNARY_OP_FLOOR ? "[" : "" );
    Operate( arg._expr );
    if ( arg._type == UNARY_OP_FLOOR )
        _result << "]";
}

IMPLEMENT( TypeOp )
{
    _result << "(";
    if ( arg._left )
        Operate( arg._left );
    else
        _result << arg._left_type.Typename();
    _result << ( arg._type == TYPE_OP_EQ ? " <~> " :
                 arg._type == TYPE_OP_TO ? " ~> " :
                 arg._type == TYPE_OP_FROM ? " <~ " : "" );
    if ( arg._right )
        Operate( arg._right );
    else
        _result << arg._right_type.Typename();
    _result << ")";
}

IMPLEMENT( TupleConstruct )
{
    _result << "(";
    bool first = true;
    for ( std::size_t i = 0; i < arg._list.size(); ++i ) {
        if ( !first )
            _result << ", ";
        Operate( arg._list[ i ] );
        first = false;
    }
    _result << ")";
}

IMPLEMENT( TupleExtract )
{
    Operate( arg._tuple );
    _result << "[";
    Operate( arg._index );
    _result << "]";
}

IMPLEMENT( TupleReplace )
{
    Operate( arg._tuple );
    _result << "[";
    Operate( arg._index );
    _result << " // ";
    Operate( arg._expr );
    _result << "]";
}

IMPLEMENT( FunctionCall )
{
    Operate( arg._function );
    _result << "(";
    bool first = true;
    for ( std::size_t i = 0; i < arg._args.size(); ++i ) {
        if ( !first )
            _result << ", ";
        Operate( arg._args[ i ] );
        first = false;
    }
    _result << ")";
}

IMPLEMENT( Converter )
{
    _result << arg._to.Typename() << "(";
    Operate( arg._expr );
    _result << ")";
}

IMPLEMENT( Body )
{
    _result << "{\n";
    ++_indent;
    for ( std::size_t i = 0; i < arg._steps.size(); ++i ) {
        _result << Indent();
        Operate( arg._steps[ i ] );
        _result << "\n";
    }
    --_indent;
    _result << Indent() << "}";
}

IMPLEMENT( Return )
{
    Operate( arg._expr );
    _result << ";";
}

IMPLEMENT( Guard )
{
    Operate( arg._expr );
    _result << ": ";
    Operate( arg._then );
    if ( !arg._otherwise )
        return;
    _result << "\n" << Indent() << "else: ";
    Operate( arg._otherwise );
}

IMPLEMENT( Let )
{
    Operate( arg._ids );
    _result << " = ";
    Operate( arg._expr );
    _result << ": ";
    Operate( arg._in );
}

IMPLEMENT( Block )
{
    _result << "{\n";
    ++_indent;
    Operate( arg._scope_set );
    for ( std::size_t i = 0; i < arg._statements.size(); ++i ) {
        _result << Indent();
        Operate( arg._statements[ i ] );
        _result << "\n";
    }
    --_indent;
    _result << Indent() << "}";
}

IMPLEMENT( ScopeSet )
{
    for ( std::size_t i = 0; i < arg._scope_defs.size(); ++i ) {
        _result << Indent();
        Operate( arg._scope_defs[ i ] );
        _result << "\n";
    }
}

IMPLEMENT( ScopeDef )
{
    _result << "use " << arg._id << ": ";
    Operate( arg._expr );
    _result << ";";
}

IMPLEMENT( Loop )
{
    Operate( arg._id );
    _result << " = ";
    Operate( arg._begin );
    _result << " .. ";
    Operate( arg._end );
    _result << ": ";
    Operate( arg._in );
}

IMPLEMENT( SequenceCall )
{
    Operate( arg._sequence );
    _result << "(";
    bool first = true;
    for ( std::size_t i = 0; i < arg._args.size(); ++i ) {
        if ( !first )
            _result << ", ";
        Operate( arg._args[ i ] );
        first = false;
    }
    _result << ");";
}

IMPLEMENT( FxStatement )
{
    _result << "fx: ";
    Operate( arg._expr );
    _result << ";";
}

IMPLEMENT( SplitStatement )
{
    _result << "split: {\n";
    ++_indent;
    for ( std::size_t i = 0; i < arg._layers.size(); ++i ) {
        _result << Indent();
        Operate( arg._layers[ i ] );
        _result << "\n";
    }
    --_indent;
    _result << Indent() << "}";
}

IMPLEMENT( Layer )
{
    if ( arg._type & LAYER_BASE )
        _result << "base";
    else if ( arg._type & LAYER_LAYER )
        _result << "layer";
    else if ( arg._type & LAYER_COPY )
        _result << "copy";
    if ( arg._order ) {
        _result << " ";
        Operate( arg._order );
    }
    if ( arg._type & LAYER_FX ) {
        _result << " merge ";
        Operate( arg._fx );
    }
    _result << ": ";
    Operate( arg._statement );
}

IMPLEMENT( Argument )
{
    _result << arg._type.Typename() << " " << arg._id;
}

IMPLEMENT( FuncDef )
{
    _result << modifiers( arg._modifiers ) << arg._return_type.Typename() << " " << arg._id << "(";
    bool first = true;
    for ( std::size_t i = 0; i < arg._args.size(); ++i ) {
        if ( !first )
            _result << ", ";
        Operate( arg._args[ i ] );
        first = false;
    }
    _result << ") ";
    Operate( arg._expr );
}

IMPLEMENT( SeqDef )
{
    _result << modifiers( arg._modifiers ) << arg._id << "(";
    bool first = true;
    for ( std::size_t i = 0; i < arg._args.size(); ++i ) {
        if ( !first )
            _result << ", ";
        Operate( arg._args[ i ] );
        first = false;
    }
    _result << ") ";
    Operate( arg._statement );
}

IMPLEMENT( VidDef )
{
    _result << modifiers( arg._modifiers ) << "video " << arg._id << " ";
    Operate( arg._frame_count );
    _result << ": ";
    Operate( arg._statement );
}

IMPLEMENT( TypeDef )
{
    _result << arg._type.Typename() << " ~" << arg._id << ";";
}

IMPLEMENT( Program )
{
    if ( !arg._scope_name.empty() )
        _result << arg._scope_name << " ";
    _result << "{\n";
    ++_indent;
    for ( std::size_t i = 0; i < arg._elements.size(); ++i ) {
        _result << Indent();
        Operate( arg._elements[ i ] );
        _result << "\n\n";
    }
    --_indent;
    _result << Indent() << "}";
}