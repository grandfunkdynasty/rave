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
    ~IrGenOperator();

    CONST_OPERATOR;

    llvm::Value* LlvmValue() const;

private:

    IrGenOperator& operator=( const IrGenOperator& rhs ) { return *this; }

    llvm::Value* _value;
    llvm::IRBuilder<>& _builder;
    SymbolTable< llvm::Value* > _table;

};

#endif