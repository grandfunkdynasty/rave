#include "oirgen.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"

#define IMPLEMENT_OPERATOR IrGenOperator
#define IMPLEMENT_TYPE IMPLEMENT_CONST

IrGenOperator::IrGenOperator( llvm::Module* module, llvm::IRBuilder<>& builder )
    : _builder( builder )
    , _module( module )
    , _return_type( Type::Void() )
    , _success_bb( 0 )
    , _fallthrough_bb( 0 )
{
}

IrGenOperator::~IrGenOperator()
{
}

llvm::Value* IrGenOperator::GenSwitch( llvm::Value* expr, llvm::Value* left, llvm::Value* right, Type type )
{
    auto parent = _builder.GetInsertBlock()->getParent();
    auto left_bb = llvm::BasicBlock::Create( _builder.getContext(), "left", parent );
    auto right_bb = llvm::BasicBlock::Create( _builder.getContext(), "right" );
    auto merge_bb = llvm::BasicBlock::Create( _builder.getContext(), "merge" );

    _builder.CreateCondBr( expr, left_bb, right_bb );

    _builder.SetInsertPoint( left_bb );
    _builder.CreateBr( merge_bb );
    left_bb = _builder.GetInsertBlock();

    parent->getBasicBlockList().push_back( right_bb );
    _builder.SetInsertPoint( right_bb );
    _builder.CreateBr( merge_bb );
    right_bb = _builder.GetInsertBlock();

    parent->getBasicBlockList().push_back( merge_bb );
    _builder.SetInsertPoint( merge_bb );
    auto phi = _builder.CreatePHI( type.LlvmType( _builder.getContext() ), 2, "trntmp" );
      
    phi->addIncoming( left, left_bb );
    phi->addIncoming( right, right_bb );
    return phi;
}

llvm::Value* IrGenOperator::GenConvert( llvm::Value* expr, Type from, Type to )
{
    if ( from == Type::Int() && to == Type::Bool() )
        return _builder.CreateICmpNE( expr, ConstantInt( 0 ), "tmbool" );
    if ( from == Type::Bool() && to == Type::Int() )
        return _builder.CreateZExt( expr, llvm::Type::getInt32Ty( _builder.getContext() ), "tmpint" );
    if ( from == Type::Int() && to == Type::Float() )
        return _builder.CreateSIToFP( expr, llvm::Type::getDoubleTy( _builder.getContext() ), "tfloat" );

    if ( !from.IsTuple() || !to.IsTuple() )
        return expr;

    llvm::Value* tuple = ConstantStruct( to.TypeArgs() );
    for ( std::size_t i = 0; i < to.TypeArgs().size(); ++i ) {
        auto convert = GenConvert( _builder.CreateExtractValue( expr, i, "tmpext" ), from.TypeArgs()[ i ], to.TypeArgs()[ i ] );
        tuple = _builder.CreateInsertValue( tuple, convert, i, "tmprep" );
    }
    return tuple;
}

llvm::Constant* IrGenOperator::ConstantBool( bool value )
{
    return llvm::ConstantInt::get( _builder.getContext(), llvm::APInt( 1, value ? 1 : 0, true ) );
}

llvm::Constant* IrGenOperator::ConstantInt( rave_int value )
{
    int64_t t = value; // TODO ~ what if it's too big?
    return llvm::ConstantInt::get( _builder.getContext(), llvm::APInt( 8 * sizeof( rave_int ), *( uint64_t* )&t, true ) );
}

llvm::Constant* IrGenOperator::ConstantFloat( rave_float value )
{
    return llvm::ConstantFP::get( _builder.getContext(), llvm::APFloat( value ) );
}

llvm::Constant* IrGenOperator::ConstantStruct( const Type::TypeList& tuple_args )
{
    std::vector< llvm::Constant* > values;
    for ( std::size_t i = 0; i < tuple_args.size(); ++i ) {
        if ( tuple_args[ i ] == Type::Bool() )
            values.push_back( ConstantBool( false ) );
        else if ( tuple_args[ i ] == Type::Int() )
            values.push_back( ConstantInt( 0 ) );
        else if ( tuple_args[ i ] == Type::Float() )
            values.push_back( ConstantFloat( 0.0 ) );
        else if ( tuple_args[ i ].IsTuple() )
            values.push_back( ConstantStruct( tuple_args[ i ].TypeArgs() ) );
        else
            values.push_back( 0 ); // TODO ~ function & sequence defaults?
    }
    return llvm::ConstantStruct::get( ( llvm::StructType* )Type::Tuple( tuple_args ).LlvmType( _builder.getContext() ), values );
}

/***************************************************************
* Implementations
***************************************************************/

IMPLEMENT_EMPTY( ParseError );

IMPLEMENT( Converter )
{
    Operate( arg._expr );
    _value = GenConvert( _value, arg._from, arg._to );
}

IMPLEMENT( Constant )
{
    if ( arg._is_int )
        _value = ConstantInt( arg._int_value );
    else
        _value = ConstantFloat( arg._float_value );
}

IMPLEMENT( Identifier )
{
    _value = _table.GetEntry( arg._id );
}

IMPLEMENT( TernaryOp )
{
    Operate( arg._expr );
    auto expr = _value;
    Operate( arg._left );
    auto left = _value;
    Operate( arg._right );
    auto right = _value;

    _value = GenSwitch( expr, left, right, arg._value_type );
}

IMPLEMENT( BinaryOp )
{
    Operate( arg._left );
    auto left = _value;
    Operate( arg._right );
    auto right = _value;

    if ( arg._type == BINARY_OP_OR )
        _value = _builder.CreateOr( left, right, "lortmp" );
    else if ( arg._type == BINARY_OP_AND )
        _value = _builder.CreateAnd( left, right, "andtmp" );
    else if ( arg._type == BINARY_OP_BIT_AND )
        _value = _builder.CreateAnd( left, right, "bndtmp" );
    else if ( arg._type == BINARY_OP_BIT_OR )
        _value = _builder.CreateOr( left, right, "bortmp" );
    else if ( arg._type == BINARY_OP_BIT_XOR )
        _value = _builder.CreateXor( left, right, "xortmp" );
    else if ( arg._type == BINARY_OP_LSHIFT )
        _value = _builder.CreateShl( left, right, "shltmp" );
    else if ( arg._type == BINARY_OP_RSHIFT )
        _value = _builder.CreateAShr( left, right, "shrtmp" );
    else if ( arg._type == BINARY_OP_EQ || arg._type == BINARY_OP_NE ) {
        _value = 0;
        typedef std::vector< rave_int > index_list;
        std::vector< index_list > stack;
        stack.push_back( index_list() );

        while ( !stack.empty() ) {
            auto indices = stack[ stack.size() - 1 ];
            stack.pop_back();

            Type t = arg._op_type;
            for ( std::size_t i = 0; i < indices.size(); ++i )
                t = t.TypeArgs()[ indices[ i ] ];
            if ( t.IsTuple() ) {
                for ( std::size_t i = 0; i < t.TypeArgs().size(); ++i ) {
                    stack.push_back( indices );
                    stack[ stack.size() - 1 ].push_back( i );
                }
                continue;
            }
            llvm::Value* vt = 0;
            if ( t.IsFunction() || t.IsSequence() ) // TODO ~ right now just false
                vt = ConstantBool( arg._type != BINARY_OP_EQ );
            else {
                auto lt = left;
                auto rt = right;
                for ( std::size_t i = 0; i < indices.size(); ++i ) {
                    lt = _builder.CreateExtractValue( lt, indices[ i ], "cextmp" );
                    rt = _builder.CreateExtractValue( rt, indices[ i ], "cextmp" );
                }
                if ( t == Type::Float() )
                    vt = arg._type == BINARY_OP_EQ ? _builder.CreateFCmpOEQ( lt, rt, "feqtmp" ) : _builder.CreateFCmpONE( lt, rt, "fnetmp" );
                else
                    vt = arg._type == BINARY_OP_EQ ? _builder.CreateICmpEQ( lt, rt, "ieqtmp" ) : _builder.CreateICmpNE( lt, rt, "inetmp" );
            }
            _value = _value == 0 ? vt : arg._type == BINARY_OP_EQ ? _builder.CreateAnd( _value, vt, "ceqtmp" ) : _builder.CreateOr( _value, vt, "cnetmp" );
        }
    }
    else if ( arg._type == BINARY_OP_GT )
        _value = arg._op_type == Type::Int() ? _builder.CreateICmpSGT( left, right, "igttmp" )
                                             : _builder.CreateFCmpOGT( left, right, "fgttmp" );
    else if ( arg._type == BINARY_OP_GE )
        _value = arg._op_type == Type::Int() ? _builder.CreateICmpSGE( left, right, "igetmp" )
                                             : _builder.CreateFCmpOGE( left, right, "fgetmp" );
    else if ( arg._type == BINARY_OP_LT )
        _value = arg._op_type == Type::Int() ? _builder.CreateICmpSLT( left, right, "ilttmp" )
                                             : _builder.CreateFCmpOLT( left, right, "flttmp" );
    else if ( arg._type == BINARY_OP_LE )
        _value = arg._op_type == Type::Int() ? _builder.CreateICmpSLE( left, right, "iletmp" )
                                             : _builder.CreateFCmpOLE( left, right, "fletmp" );
    else if ( arg._type == BINARY_OP_ADD )
        _value = arg._op_type == Type::Int() ? _builder.CreateAdd(  left, right, "addtmp" )
                                             : _builder.CreateFAdd( left, right, "fddtmp" );
    else if ( arg._type == BINARY_OP_SUB )
        _value = arg._op_type == Type::Int() ? _builder.CreateSub(  left, right, "subtmp" )
                                             : _builder.CreateFSub( left, right, "fubtmp" );
    else if ( arg._type == BINARY_OP_MUL )
        _value = arg._op_type == Type::Int() ? _builder.CreateMul(  left, right, "multmp" )
                                             : _builder.CreateFMul( left, right, "fultmp" );
    else if ( arg._type == BINARY_OP_DIV ) {
        if ( arg._op_type != Type::Int() )
            _value = _builder.CreateFDiv( left, right, "fivtmp" );
        else {
            auto l_check = _builder.CreateICmpSGE( left, ConstantInt( 0 ), "diftmp" );
            auto r_check = _builder.CreateICmpSGE( right, ConstantInt( 0 ), "diftmp" );
            auto if_pos = _builder.CreateSDiv( left, right, "divtmp" );
            auto correction = GenSwitch( r_check, _builder.CreateSub( left, _builder.CreateSub( right, ConstantInt( 1 ), "dubtmp" ), "dubtmp" ),
                                                          _builder.CreateAdd( left, _builder.CreateAdd( right, ConstantInt( 1 ), "dddtmp" ), "dddtmp" ), Type::Int() );
            auto if_neg = _builder.CreateSDiv( correction, right, "divtmp" );
            _value = GenSwitch( l_check, if_pos, if_neg, Type::Int() );
        }
    }
    else if ( arg._type == BINARY_OP_MOD ) {
        if ( arg._op_type != Type::Int() )
            _value = _builder.CreateFRem( left, right, "fodtmp" );
        else {
            auto l_check = _builder.CreateICmpSGE( left, ConstantInt( 0 ), "miftmp" );
            auto r_check = _builder.CreateICmpSGE( right, ConstantInt( 0 ), "miftmp" );
            auto if_pos = _builder.CreateSRem( left, right, "modtmp" );
            auto r_abs = GenSwitch( r_check, right, _builder.CreateSub( ConstantInt( 0 ), right, "mubtmp" ), Type::Int() );
            auto if_neg = _builder.CreateAdd( r_abs, if_pos, "mddtmp" );
            _value = GenSwitch( l_check, if_pos, if_neg, Type::Int() );
        }
    }
    else if ( arg._type == BINARY_OP_EXP )
        _value = 0; // TODO
}

IMPLEMENT( UnaryOp )
{
    Operate( arg._expr );
    uint64_t max = UINT64_MAX;

    if ( arg._type == UNARY_OP_NOT )
        _value = _builder.CreateICmpEQ( _value, ConstantInt( 0 ), "nottmp" );
    else if ( arg._type == UNARY_OP_BIT_NOT )
        _value = _builder.CreateXor( _value, ConstantInt( ( rave_int )max ), "bnttmp" );
    else if ( arg._type == UNARY_OP_NEGATION ) {
        if ( arg._op_type == Type::Int() )
            _value = _builder.CreateSub( ConstantInt( 0 ), _value, "negtmp" );
        else
            _value = _builder.CreateFSub( ConstantFloat( 0.0 ), _value, "fngtmp" );
    }
    else if ( arg._type == UNARY_OP_FLOOR ) {
        auto check = _builder.CreateFCmpOGE( _value, ConstantFloat( 0.0 ), "fiftmp" );
        auto floor = _builder.CreateFPToSI( _value, llvm::Type::getInt32Ty( _builder.getContext() ), "flrtmp" );
       _value = GenSwitch( check, floor, _builder.CreateSub( floor, ConstantInt( 1 ) ), Type::Int() );
    }
}

IMPLEMENT( TypeOp )
{
}

IMPLEMENT( TupleConstruct )
{
    ValueList values;
    for ( std::size_t i = 0; i < arg._list.size(); ++i ) {
        Operate( arg._list[ i ] );
        values.push_back( _value );
    }

    _value = ConstantStruct( arg._value_type.TypeArgs() );
    for ( std::size_t i = 0; i < arg._list.size(); ++i )
        _value = _builder.CreateInsertValue( _value, values[ i ], i, "contmp" );
}

IMPLEMENT( TupleExtract )
{
    Operate( arg._tuple );
    _value = _builder.CreateExtractValue( _value, arg._constant_index, "exttmp" );
}

IMPLEMENT( TupleReplace )
{
    Operate( arg._tuple );
    auto tuple = _value;
    Operate( arg._expr );
    auto expr = _value;

    _value = ConstantStruct( arg._value_type.TypeArgs() );
    for ( std::size_t i = 0; i < arg._value_type.TypeArgs().size(); ++i ) {
        auto v = signed( i ) == arg._constant_index ?
            expr : _builder.CreateExtractValue( tuple, i, "rextmp" );
        _value = _builder.CreateInsertValue( _value, v, i, "reptmp" );
    }
}

IMPLEMENT( FunctionCall )
{
    // TODO
}

IMPLEMENT( Body )
{
    auto entry_bb = _builder.GetInsertBlock();
    auto parent = entry_bb->getParent();

    auto default_bb =
        llvm::BasicBlock::Create( _builder.getContext(), "default", parent );
    auto merge_bb =
        llvm::BasicBlock::Create( _builder.getContext(), "merge" );

    parent->getBasicBlockList().push_back( merge_bb );
    _builder.SetInsertPoint( merge_bb );
    auto phi =
        _builder.CreatePHI( _return_type.LlvmType( _builder.getContext() ), arg._steps.size(), "rettmp" );
    merge_bb = _builder.GetInsertBlock();

    auto i = arg._steps.rbegin();
    _builder.SetInsertPoint( default_bb );
    Operate( *i );
    _builder.CreateBr( merge_bb );
    default_bb = _builder.GetInsertBlock();
    phi->addIncoming( _value, default_bb );

    _fallthrough_bb = default_bb;
    for ( ++i; i != arg._steps.rend(); ++i ) {
        _success_bb =
            llvm::BasicBlock::Create( _builder.getContext(), "success", parent, _fallthrough_bb );
        auto check_bb =
            i + 1 == arg._steps.rend() ? entry_bb :
            llvm::BasicBlock::Create( _builder.getContext(), "check", parent, _success_bb );

        _builder.SetInsertPoint( check_bb );
        Operate( *i );
        _builder.CreateBr( merge_bb );
        _success_bb = _builder.GetInsertBlock();
        phi->addIncoming( _value, _success_bb );

        _fallthrough_bb = check_bb;
    }

    _builder.SetInsertPoint( merge_bb );
    _value = phi;
}

IMPLEMENT( Return )
{
    Operate( arg._expr );
}

IMPLEMENT( Guard )
{
    Operate( arg._expr );
    auto expr = _value;

    _builder.CreateCondBr( expr, _success_bb, _fallthrough_bb );
    _builder.SetInsertPoint( _success_bb );
    Operate( arg._then );

    // TODO ~ otherwise (sequence-only) not handled
}

IMPLEMENT( Let )
{
}

IMPLEMENT( Block )
{
}

IMPLEMENT( ScopeSet )
{
}

IMPLEMENT( ScopeDef )
{
}

IMPLEMENT( Loop )
{
}

IMPLEMENT( SequenceCall )
{
}

IMPLEMENT( FxStatement )
{
}

IMPLEMENT( SplitStatement )
{
}

IMPLEMENT( Layer )
{
}

IMPLEMENT( Argument )
{
    _arg_iterator->setName( arg._id );
    _table.AddEntry( arg._id, _arg_iterator );
}

IMPLEMENT( FuncDef )
{
    LlvmTypeList types;
    for ( std::size_t i = 0; i < arg._arg_types.size(); ++i )
        types.push_back( arg._arg_types[ i ].LlvmType( _builder.getContext() ) );
    auto type =
        llvm::FunctionType::get( arg._return_type.LlvmType( _builder.getContext() ), types, false );
    auto linkage = arg._modifiers & MODIFIER_LOCAL ?
        llvm::Function::PrivateLinkage : llvm::Function::ExternalLinkage;
    auto func =
        llvm::Function::Create( type, linkage, arg._id, _module );

    _return_type = arg._return_type;
    _table.Push();
    _arg_iterator = func->arg_begin();
    for ( std::size_t i = 0; _arg_iterator != func->arg_end(); ++_arg_iterator, ++i )
        Operate( arg._args[ i ] );
    auto bb =
        llvm::BasicBlock::Create( _builder.getContext(), "entry", func );
    _builder.SetInsertPoint( bb );
    Operate( arg._expr );
    _builder.CreateRet( _value );
    _table.Pop();
}

IMPLEMENT( SeqDef )
{
}

IMPLEMENT( VidDef )
{
}

IMPLEMENT( TypeDef )
{
}

IMPLEMENT( Program )
{
    _table.Push();
    for ( std::size_t i = 0; i < arg._elements.size(); ++i )
        Operate( arg._elements[ i ] );
    _table.Pop();
}