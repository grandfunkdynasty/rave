#include "odeclare.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"
#include "otype.h"

#define IMPLEMENT_OPERATOR DeclareOperator
#define IMPLEMENT_TYPE IMPLEMENT_CONST

DeclareOperator::DeclareOperator( TypeOperator& otype, bool resolve )
: _otype( &otype )
, _ostatic( 0 )
, _oirgen( 0 )
, _owner( resolve ? DECLARE_TYPE_RESOLVE : DECLARE_TYPE )
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

void DeclareOperator::DeclareAlgebraicConstructors( const Ast& arg, const Type& type )
{
    auto i = type.Typedef().find( "." );
    if ( i == std::string::npos )
        DeclareAlgebraicConstructors( arg, "", type );
    else
        DeclareAlgebraicConstructors( arg, type.Typedef().substr( 0, i + 1 ), type );
}

void DeclareOperator::DeclareAlgebraicConstructors( const Ast& arg, const std::string& rel_name, const Type& type )
{
    if ( type.IsFunction() )
        DeclareAlgebraicConstructors( arg, rel_name, type.ReturnType() );
    if ( type.IsSequence() || type.IsFunction() || type.IsTuple() ) {
        for ( std::size_t i = 0; i < type.TypeArgs().size(); ++i )
            DeclareAlgebraicConstructors( arg, rel_name, type.TypeArgs()[ i ] );
    }
    if ( !type.IsAlgebraic() )
        return;
    auto it = type.TypeArgsMap().begin();
    for ( ; it != type.TypeArgsMap().end(); ++it ) {
        const std::string& name = it->first;
        std::string scoped_name = _namespace + rel_name + name;
        Type::TypeList args;
        if ( it->second != Type::Void() )
            args.push_back( it->second );
        if ( _ostatic->_table.HasEntry( scoped_name ) ) {
            if ( !_ostatic->_table.HasEntry( "constructor:" + scoped_name ) ||
                 !_ostatic->_table.GetEntry( scoped_name ).Equivalent( Type::Function( type, args ) ) )
                _ostatic->Error( arg, "identifier `" + name + "' already declared in this scope" );
            continue;
        }
        _ostatic->_table.AddEntry( scoped_name, Type::Function( type, args ) );
        _ostatic->_table.AddEntry( "constructor:" + scoped_name, Type::Void() );
    }
}

void DeclareOperator::IrGenAlgebraicConstructors( const Ast& arg, const Type& type )
{
    auto i = type.Typedef().find( "." );
    if ( i == std::string::npos )
        IrGenAlgebraicConstructors( arg, "", type );
    else
        IrGenAlgebraicConstructors( arg, type.Typedef().substr( 0, i + 1 ), type );
}

void DeclareOperator::IrGenAlgebraicConstructors( const Ast& arg, const std::string& rel_name, const Type& type )
{
    if ( type.IsFunction() )
        IrGenAlgebraicConstructors( arg, rel_name, type.ReturnType() );
    if ( type.IsSequence() || type.IsFunction() || type.IsTuple() ) {
        for ( std::size_t i = 0; i < type.TypeArgs().size(); ++i )
            IrGenAlgebraicConstructors( arg, rel_name, type.TypeArgs()[ i ] );
    }
    if ( !type.IsAlgebraic() )
        return;

    for ( std::size_t i = 0; i < _oirgen->_algebraic_constructor_list.size(); ++i ) {
        const Type& t = _oirgen->_algebraic_constructor_list[ i ].first;
        const auto& list = _oirgen->_algebraic_constructor_list[ i ].second;
        if ( t.Equivalent( type ) ) {
            auto it = type.TypeArgsMap().begin();
            std::size_t index = 0;
            for ( ; it != type.TypeArgsMap().end(); ++it, ++index ) {
                std::string scoped_name = _namespace + rel_name + it->first;
                _oirgen->_table.AddEntry( scoped_name, list[ index ] );
            }
            return;
        }
    }

    Type::TypeList internal_args;
    internal_args.push_back( Type::Int() );
    auto it = type.TypeArgsMap().begin();
    for ( ; it != type.TypeArgsMap().end(); ++it ) {
        if ( it->second != Type::Void() )
            internal_args.push_back( it->second );
    }

    std::vector< llvm::Function* > list;
    rave_int index = 0;
    rave_int offset = 0;
    it = type.TypeArgsMap().begin();
    for ( ; it != type.TypeArgsMap().end(); ++it, ++index ) {
        const std::string& name = it->first;
        std::string scoped_name = _namespace + rel_name + name;
        if ( _oirgen->_table.HasEntry( scoped_name ) )
            continue;

        IrGenOperator::LlvmTypeList args;
        if ( it->second != Type::Void() )
            args.push_back( it->second.LlvmType( _oirgen->_builder.getContext() ) );
        auto return_type = type.LlvmType( _oirgen->_builder.getContext() );
        auto type =
            llvm::FunctionType::get( return_type, args, false );
        auto linkage = llvm::Function::PrivateLinkage;
        auto llvm_function =
            llvm::Function::Create( type, linkage, name, _oirgen->_module );

        auto arg_it = llvm_function->arg_begin();
        if ( it->second != Type::Void() )
            arg_it->setName( "value" );
        auto bb =
            llvm::BasicBlock::Create( _oirgen->_builder.getContext(), "entry", llvm_function );
        _oirgen->_builder.SetInsertPoint( bb );

        llvm::Value* value = _oirgen->ConstantStruct( internal_args );
        value = _oirgen->_builder.CreateInsertValue(
            value, _oirgen->ConstantInt( index ), 0, "index" );
        if ( it->second != Type::Void() )
            value = _oirgen->_builder.CreateInsertValue(
                value, arg_it, ++offset, "val" );
        _oirgen->_builder.CreateRet( value );
        _oirgen->_table.AddEntry( scoped_name, llvm_function );
        list.push_back( llvm_function );
    }
    _oirgen->_algebraic_constructor_list.push_back( std::make_pair( type, list ) );
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
    if ( _owner == DECLARE_STATIC ) {
        _declaration_list.push_back( arg._type );
        DeclareAlgebraicConstructors( arg, arg._type );
        return;
    }

    if ( _owner != DECLARE_IRGEN )
        return;
    IrGenAlgebraicConstructors( arg, arg._type );
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
        DeclareAlgebraicConstructors( arg, arg._return_type );
        return;
    }

    if ( _owner != DECLARE_IRGEN || _declare_prototypes )
        return;
    _oirgen->_table.AddEntry( _namespace + arg._id, arg._llvm_function );
    for ( std::size_t i = 0; i < arg._args.size(); ++i )
        Operate( arg._args[ i ] );
    IrGenAlgebraicConstructors( arg, arg._return_type );
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
    if ( _declare_globals && arg._modifiers & MODIFIER_LOCAL )
        return;

    if ( _owner == DECLARE_STATIC ) {
        DeclareAlgebraicConstructors( arg, arg._type );
        return;
    }

    if ( _owner == DECLARE_IRGEN ) {
        IrGenAlgebraicConstructors( arg, arg._type );
        return;
    }

    if ( _owner == DECLARE_TYPE ) {
        if ( !_otype->_table.AddEntry( _namespace + arg._id, _otype->Resolve( arg, arg._type, _namespace ) ) )
            _otype->Error( arg, "type `~" + arg._id + "' already declared in this scope" );
    }

    if ( _owner != DECLARE_TYPE_RESOLVE )
        return;
    //_otype->_table.GetEntry( _namespace + arg._id ) = _otype->Resolve( arg, arg._type, _namespace );
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