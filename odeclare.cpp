#include "odeclare.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"
#include "otype.h"

#define IMPLEMENT_OPERATOR DeclareOperator
#define IMPLEMENT_TYPE IMPLEMENT_CONST

DeclareOperator::DeclareOperator( TypeOperator& otype )
: _otype( &otype )
, _ostatic( 0 )
, _oirgen( 0 )
, _owner( DECLARE_TYPE )
, _global_scope( true )
, _declare_globals( 0 )
, _declare_prototypes( 0 )
{
}

DeclareOperator::DeclareOperator( StaticOperator& ostatic )
: _otype( 0 )
, _ostatic( &ostatic )
, _oirgen( 0 )
, _owner( DECLARE_STATIC )
, _global_scope( true )
, _declare_globals( 0 )
, _declare_prototypes( 0 )
{
}

DeclareOperator::DeclareOperator( IrGenOperator& oirgen )
: _otype( 0 )
, _ostatic( 0 )
, _oirgen( &oirgen )
, _owner( DECLARE_IRGEN )
, _global_scope( true )
, _declare_globals( 0 )
, _declare_prototypes( 0 )
{
}

DeclareOperator::~DeclareOperator()
{
}

/***************************************************************
* Implementations
***************************************************************/

IMPLEMENT_EMPTY( ParseError );
IMPLEMENT_EMPTY( Constant );
IMPLEMENT_EMPTY( Identifier );
IMPLEMENT_EMPTY( TernaryOp );
IMPLEMENT_EMPTY( BinaryOp );
IMPLEMENT_EMPTY( UnaryOp );
IMPLEMENT_EMPTY( TypeOp );
IMPLEMENT_EMPTY( TupleConstruct );
IMPLEMENT_EMPTY( TupleExtract );
IMPLEMENT_EMPTY( TupleReplace );
IMPLEMENT_EMPTY( FunctionCall );
IMPLEMENT_EMPTY( Converter );
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

IMPLEMENT( Argument )
{
    if ( _owner == DECLARE_STATIC )
        _declaration_list.push_back( arg._type );
}

IMPLEMENT( FuncDef )
{
    if ( !arg._llvm_function && _owner == DECLARE_IRGEN ) {
        IrGenOperator::LlvmTypeList types;
        for ( std::size_t i = 0; i < arg._arg_types.size(); ++i )
            types.push_back( arg._arg_types[ i ].LlvmType( _oirgen->_builder.getContext() ) );
        auto return_type = arg._return_type.LlvmType( _oirgen->_builder.getContext() );
        auto type =
            llvm::FunctionType::get( return_type, types, false );
        auto linkage = arg._modifiers & MODIFIER_LOCAL ?
            llvm::Function::PrivateLinkage : llvm::Function::ExternalLinkage;
        arg._llvm_function =
            llvm::Function::Create( type, linkage, arg._id, _oirgen->_module );
    }

    if ( _declare_globals && arg._modifiers & MODIFIER_LOCAL )
        return;
    if ( _owner == DECLARE_STATIC ) {
        _declaration_list.clear();
        for ( std::size_t i = 0; i < arg._args.size(); ++i )
            Operate( arg._args[ i ] );
        if ( !_ostatic->_table.AddEntry( _namespace + arg._id, Type::Function( arg._return_type, _declaration_list ) ) )
            _ostatic->Error( arg, "identifier `" + arg._id + "' already declared in this scope" );
        arg._arg_types = _declaration_list;
        return;
    }

    if ( _owner != DECLARE_IRGEN || _declare_prototypes )
        return;
    _oirgen->_table.AddEntry( _namespace + arg._id, arg._llvm_function );
}

IMPLEMENT( SeqDef )
{
    if ( _declare_globals && arg._modifiers & MODIFIER_LOCAL )
        return;
    if ( _owner != DECLARE_STATIC )
        return;
    _declaration_list.clear();
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
    if ( !_ostatic->_table.AddEntry( _namespace + arg._id, Type::Sequence( _declaration_list ) ) )
        _ostatic->Error( arg, "identifier `" + arg._id + "' already declared in this scope" );
    arg._arg_types = _declaration_list;
    return;
}

IMPLEMENT_EMPTY( VidDef );

IMPLEMENT( TypeDef )
{
    if ( _owner != DECLARE_TYPE || ( _declare_globals && arg._modifiers & MODIFIER_LOCAL ) )
        return;
    if ( !_otype->_table.AddEntry( _namespace + arg._id, _otype->Resolve( arg, arg._type, _namespace ) ) )
        _otype->Error( arg, "type `~" + arg._id + "' already declared in this scope" );
}

IMPLEMENT( Program )
{
    if ( _global_scope ) {
        _global_scope = false;
        for ( std::size_t i = 0; i < arg._elements.size(); ++i )
            Operate( arg._elements[ i ] );
        return;
    }

    if ( _declare_globals && ( arg._modifiers & MODIFIER_LOCAL || arg._scope_name.empty() ) ) {
        if ( _owner != DECLARE_IRGEN )
            return;
        ++_declare_prototypes;
        for ( std::size_t i = 0; i < arg._elements.size(); ++i )
            Operate( arg._elements[ i ] );
        --_declare_prototypes;
        return;
    }
    ++_declare_globals;
    _namespace += arg._scope_name.empty() ? "" : arg._scope_name + ".";
    for ( std::size_t i = 0; i < arg._elements.size(); ++i )
        Operate( arg._elements[ i ] );
    --_declare_globals;
    _namespace = _namespace.substr( 0, 1 + _namespace.find_last_of( '.', _namespace.length() - 2 ) );
}