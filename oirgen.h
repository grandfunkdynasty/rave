#ifndef OIRGEN_H
#define OIRGEN_H

#include "operator.h"
#include "symbol.h"

#pragma warning(push, 0)
#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/IRBuilder.h"
#pragma warning(pop)

class IrGenOperator : public ConstOperator {
public:

    IrGenOperator( llvm::Module* module, llvm::IRBuilder<>& builder );
    virtual ~IrGenOperator();

    CONST_OPERATOR;

private:

    IrGenOperator& operator=( const IrGenOperator& rhs ) { return *this; }

    llvm::Value* GenSwitch( llvm::Value* expr, llvm::Value* left, llvm::Value* right, Type type );
    llvm::Value* GenConvert( llvm::Value* expr, Type from, Type to );

    llvm::Constant* ConstantBool( bool value );
    llvm::Constant* ConstantInt( rave_int value );
    llvm::Constant* ConstantFloat( rave_float value );
    llvm::Constant* ConstantStruct( const Type::TypeList& tuple_args );

    typedef std::vector< llvm::Value* > ValueList;
    typedef std::vector< llvm::Type* > LlvmTypeList;

    llvm::Value* _value;
    llvm::Module* _module;
    llvm::IRBuilder<>& _builder;
    SymbolTable< llvm::Value* > _table;

    Type _return_type;
    llvm::Function::arg_iterator _arg_iterator;
    llvm::BasicBlock* _success_bb;
    llvm::BasicBlock* _fallthrough_bb;

};

#endif