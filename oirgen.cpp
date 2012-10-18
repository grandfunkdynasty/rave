#include "oirgen.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"

#define IMPLEMENT_OPERATOR IrGenOperator
#define IMPLEMENT_TYPE IMPLEMENT_CONST

IrGenOperator::IrGenOperator( llvm::IRBuilder<>& builder )
    : _builder( builder )
{
}

IrGenOperator::~IrGenOperator()
{
}

llvm::Value* IrGenOperator::LlvmValue() const
{
    return _value;
}

/***************************************************************
* Implementations
***************************************************************/

IMPLEMENT_EMPTY( ParseError );

IMPLEMENT( Converter )
{
    Operate( arg._expr );

    if ( arg._from == Type::Int() && arg._to == Type::Bool() )
        _value = _builder.CreateICmpNE( _value, llvm::ConstantInt::get( _builder.getContext(), llvm::APInt( 8 * sizeof( rave_int ), 0, true ) ), "tmbool" );
    else if ( arg._from == Type::Bool() && arg._to == Type::Int() )
        _value = _builder.CreateZExt( _value, llvm::Type::getInt32Ty( _builder.getContext() ), "tmpint" );
    else if ( arg._from == Type::Int() && arg._to == Type::Float() )
        _value = _builder.CreateSIToFP( _value, llvm::Type::getDoubleTy( _builder.getContext() ), "tfloat" );
}

IMPLEMENT( Constant )
{
    int64_t t = arg._int_value;
    if ( arg._is_int )
        _value = llvm::ConstantInt::get( _builder.getContext(), llvm::APInt( 8 * sizeof( arg._int_value ), *( uint64_t* )&t, true ) );
    else
        _value = llvm::ConstantFP::get( _builder.getContext(), llvm::APFloat( arg._float_value ) );
}

IMPLEMENT( Identifier )
{
    _value = _table.GetEntry( arg._id );
}

IMPLEMENT( TernaryOp )
{
    Operate( arg._expr );
    llvm::Value* expr = _value;

    llvm::Function* parent = _builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* left_bb = llvm::BasicBlock::Create( _builder.getContext(), "left", parent );
    llvm::BasicBlock* right_bb = llvm::BasicBlock::Create( _builder.getContext(), "right" );
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create( _builder.getContext(), "merge" );

    _builder.CreateCondBr( expr, left_bb, right_bb );
    _builder.SetInsertPoint( left_bb );

    Operate( arg._left );
    llvm::Value* left = _value;
    _builder.CreateBr( merge_bb );
    left_bb = _builder.GetInsertBlock();

    parent->getBasicBlockList().push_back( right_bb );
    _builder.SetInsertPoint( right_bb );

    Operate( arg._right );
    llvm::Value* right = _value;
    _builder.CreateBr( merge_bb );
    right_bb = _builder.GetInsertBlock();

    parent->getBasicBlockList().push_back( merge_bb );
    _builder.SetInsertPoint( merge_bb );
    llvm::PHINode* phi = _builder.CreatePHI( arg._value_type.LlvmType( _builder.getContext() ), 2, "trntmp" );
      
    phi->addIncoming( left, left_bb );
    phi->addIncoming( right, right_bb );
    _value = phi;
}

IMPLEMENT( BinaryOp )
{
    Operate( arg._left );
    llvm::Value* left = _value;
    Operate( arg._right );
    llvm::Value* right = _value;

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
        std::vector< std::vector< rave_int > > _stack;
        _stack.push_back( std::vector< rave_int >() );

        while ( !_stack.empty() ) {
            std::vector< rave_int > indices = _stack[ _stack.size() - 1 ];
            _stack.pop_back();

            Type t = arg._op_type;
            for ( std::size_t i = 0; i < indices.size(); ++i )
                t = t.TypeArgs()[ indices[ i ] ];
            if ( t.IsTuple() ) {
                for ( std::size_t i = 0; i < t.TypeArgs().size(); ++i ) {
                    _stack.push_back( indices );
                    _stack[ _stack.size() - 1 ].push_back( i );
                }
                continue;
            }
            llvm::Value* vt = 0;
            if ( t.IsFunction() || t.IsSequence() ) // TODO ~ right now just false
                vt = llvm::ConstantInt::get( _builder.getContext(), llvm::APInt( 1, arg._type == BINARY_OP_EQ ? 0 : 1, false ) );
            else {
                llvm::Value* lt = left;
                llvm::Value* rt = right;
                for ( std::size_t i = 0; i < indices.size(); ++i ) {
                    lt = left; // TODO ~ struct-get left[ indices[ i ] ]
                    rt = right; // TODO ~ struct-get right[ indices[ i ] ]
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
    else if ( arg._type == BINARY_OP_DIV )
        _value = arg._op_type == Type::Int() ? _builder.CreateSDiv( left, right, "divtmp" ) // TODO: this rounds towards 0, want round towards -inf!
                                             : _builder.CreateFDiv( left, right, "fivtmp" );
    else if ( arg._type == BINARY_OP_MOD )
        _value = arg._op_type == Type::Int() ? _builder.CreateSRem( left, right, "modtmp" ) // TODO: similarly wrong
                                             : _builder.CreateFRem( left, right, "fodtmp" );
    else if ( arg._type == BINARY_OP_EXP )
        _value = 0; // TODO
}

IMPLEMENT( UnaryOp )
{
    Operate( arg._expr );

    if ( arg._type == UNARY_OP_NOT )
        _value = _builder.CreateICmpEQ( _value, llvm::ConstantInt::get( _builder.getContext(), llvm::APInt( 8 * sizeof( rave_int ), 0, true ) ), "nottmp" );
    else if ( arg._type == UNARY_OP_BIT_NOT )
        _value = _builder.CreateXor( _value, llvm::ConstantInt::get( _builder.getContext(), llvm::APInt( 8 * sizeof( rave_int ), UINT64_MAX, true ) ), "bnttmp" );
    else if ( arg._type == UNARY_OP_NEGATION )
        _value = _builder.CreateSub( llvm::ConstantInt::get( _builder.getContext(), llvm::APInt( 8 * sizeof( rave_int ), 0, true ) ), _value, "negtmp" );
    else if ( arg._type == UNARY_OP_FLOOR )
        _value = _builder.CreateFPToSI( _value, llvm::Type::getInt32Ty( _builder.getContext() ), "flrtmp" ); // TODO: this rounds towards 0, want round towards -inf!
}

IMPLEMENT( TypeOp )
{
}

IMPLEMENT( TupleConstruct )
{
}

IMPLEMENT( TupleExtract )
{
}

IMPLEMENT( TupleReplace )
{
}

IMPLEMENT( FunctionCall )
{
}

IMPLEMENT( Body )
{
}

IMPLEMENT( Return )
{
}

IMPLEMENT( Guard )
{
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
}

IMPLEMENT( FuncDef )
{
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
}