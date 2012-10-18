#include <iostream>
#include "parse.h"
#include "ast.h"
#include "ostatic.h"
#include "oexpand.h"
#include "ostring.h"

#pragma warning(push, 0)
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/TargetSelect.h"
#pragma warning(pop)

int main( int argc, char** argv )
{
    int errors = 0;
    parse_set_error( &errors );
    Ast* ast = parse( argv[ 1 ] );
    if ( !ast ) {
        std::cout << errors << " error(s) detected.\n";
        return 1;
    }

    TypeOperator otype( &errors );
    otype.Operate( ast );

    StaticOperator ostatic ( &errors );
    ostatic.Operate( ast );
    if ( errors ) {
        std::cout << errors << " error(s) detected.\n";
        return 1;
    }
    std::cout << " success\n\n";

    std::cout << "source code:\n";
    StringOperator ostring;
    ostring.Operate( ast );
    std::cout << ostring.Result() << "\n\n";

    std::cout << "expanding...";
    ExpandOperator oexpand;
    oexpand.Operate( ast );
    std::cout << " success\n";

    llvm::InitializeNativeTarget();
    llvm::LLVMContext& context = llvm::getGlobalContext();

    llvm::Module* module = new llvm::Module( "anonymous", context );
    std::string rs;
    llvm::ExecutionEngine* execution_engine = llvm::EngineBuilder( module ).setErrorStr( &rs ).create();
    if ( !execution_engine ) {
        std::cout << "fatal error:\t" << rs << "\n";
        return 1;
    }

    llvm::IRBuilder<> builder( context );
    llvm::FunctionType* type = llvm::FunctionType::get( ostatic.InferredType().LlvmType( context ), std::vector< llvm::Type* >(), false );
    llvm::Function* func = llvm::Function::Create( type, llvm::Function::ExternalLinkage, "expression", module );
    llvm::BasicBlock *bb = llvm::BasicBlock::Create( context, "entry", func );
    builder.SetInsertPoint( bb );

    IrGenOperator oir( builder );
    oir.Operate( ast );
    delete ast;
    builder.CreateRet( oir.LlvmValue() );
    std::cout << "verifying...";
    llvm::verifyFunction( *func );
    std::cout << " success\n";

    std::cout << "ir code:\n";
    oir.LlvmValue()->dump();
    std::cout << "\n\n";

    llvm::FunctionPassManager optimiser( module );
    optimiser.add( new llvm::TargetData( *execution_engine->getTargetData() ) );
    optimiser.add( llvm::createBasicAliasAnalysisPass() );
    optimiser.add( llvm::createInstructionCombiningPass() );
    optimiser.add( llvm::createReassociatePass() );
    optimiser.add( llvm::createGVNPass() );
    optimiser.add( llvm::createCFGSimplificationPass() );
    optimiser.doInitialization();
    optimiser.run( *func );

    llvm::GenericValue value = execution_engine->runFunction( func, std::vector< llvm::GenericValue >() );
    std::cout << "value:\n";
    std::cout << "int ";
    value.IntVal.dump();
    std::cout << "\ndouble " << value.DoubleVal << "\npointer " << value.PointerVal << "\n\n";

    //delete module;
    return 0;
}