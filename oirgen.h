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

    IrGenOperator( llvm::IRBuilder<>& builder );
    virtual ~IrGenOperator();

    CONST_OPERATOR;

    llvm::Value* LlvmValue() const;

private:

    IrGenOperator& operator=( const IrGenOperator& rhs ) { return *this; }

    llvm::Value* GenSwitch( llvm::Value* expr, llvm::Value* left, llvm::Value* right, Type type );

    llvm::Constant* ConstantBool( bool value );
    llvm::Constant* ConstantInt( rave_int value );
    llvm::Constant* ConstantFloat( rave_float value );
    llvm::Constant* ConstantStruct( const Type::TypeList& tuple_args );

    llvm::Value* _value;
    llvm::IRBuilder<>& _builder;
    SymbolTable< llvm::Value* > _table;

    typedef std::vector< llvm::Value* > ValueList;

};

#endif