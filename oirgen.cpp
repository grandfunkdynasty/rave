#include "oirgen.h"
#include "astenum.h"
#include "expr.h"
#include "defs.h"
#include "decs.h"

#define IMPLEMENT_OPERATOR IrGenOperator
#define IMPLEMENT_TYPE IMPLEMENT_CONST

IrGenOperator::IrGenOperator()
    : _builder( llvm::getGlobalContext() )
{
}

IrGenOperator::~IrGenOperator()
{
}

/***************************************************************
* Implementations
***************************************************************/

IMPLEMENT_EMPTY( ParseError );

IMPLEMENT( Constant )
{
    int64_t t = arg._int_value;
    if ( arg._is_int )
        _value = llvm::ConstantInt::get( llvm::getGlobalContext(), llvm::APInt( 8 * sizeof( arg._int_value ), *( uint64_t* )&t, true ) );
    else
        _value = llvm::ConstantFP::get( llvm::getGlobalContext(), llvm::APFloat( arg._float_value ) );
}

IMPLEMENT( Identifier )
{
    _value = _table.GetEntry( arg._id );
}

IMPLEMENT( TernaryOp )
{
    Operate( arg._expr );
    llvm::Value* expr = _value;

    expr = _builder.CreateICmpNE( expr, llvm::ConstantInt::get( llvm::getGlobalContext(), llvm::APInt( 8 * sizeof( rave_int ), 0, true ) ) );

    llvm::Function* parent = _builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* left_bb = llvm::BasicBlock::Create( llvm::getGlobalContext(), "left", parent );
    llvm::BasicBlock* right_bb = llvm::BasicBlock::Create( llvm::getGlobalContext(), "right" );
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create( llvm::getGlobalContext(), "merge" );

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
    llvm::PHINode* phi = _builder.CreatePHI( arg._value_type.LlvmType(), 2, "trntmp" );
      
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

    if ( arg._type == BINARY_OP_OR || arg._type == BINARY_OP_AND ) {
        left = _builder.CreateICmpNE( left, llvm::ConstantInt::get( llvm::getGlobalContext(), llvm::APInt( 8 * sizeof( rave_int ), 0, true ) ) );
        right = _builder.CreateICmpNE( right, llvm::ConstantInt::get( llvm::getGlobalContext(), llvm::APInt( 8 * sizeof( rave_int ), 0, true ) ) );
        _value = arg._type == BINARY_OP_OR ? _builder.CreateOr( left, right, "lortmp" ) : _builder.CreateAnd( left, right, "andtmp" );
    }
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
    else if ( arg._type == BINARY_OP_EQ )
        _value = 0; // TODO ~ need to recursively check
    else if ( arg._type == BINARY_OP_NE )
        _value = 0; // TODO ~ OR and another pass that expands this to thing == thing && thing == thing for tuples; not sure about functions
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
    // TODO
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

IMPLEMENT( Promoter )
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