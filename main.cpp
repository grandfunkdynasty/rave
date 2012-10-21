#include <iostream>
#include "parse.h"
#include "ast.h"
#include "ostatic.h"
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
    if ( argc == 0 ) {
        std::cout << "no input file(s) specified\n";
        return 1;
    }

    int errors = 0;
    parse_set_error( &errors );
    auto ast = parse( argv[ 1 ] );
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

    llvm::InitializeNativeTarget();
    auto& context = llvm::getGlobalContext();

    std::string rs;
    auto module = new llvm::Module( "anonymous", context );
    auto execution_engine = llvm::EngineBuilder( module ).setErrorStr( &rs ).create();
    if ( !execution_engine ) {
        std::cout << "fatal error:\t" << rs << "\n";
        return 1;
    }

    llvm::IRBuilder<> builder( context );
    IrGenOperator oir( module, builder );
    oir.Operate( ast );
    delete ast;

    std::cout << "ir code:\n";
    module->dump();
    std::cout << "\n\n";
    std::cout << "verifying...";
    if ( llvm::verifyModule( *module ) )
        return 1;
    std::cout << " success\n";

    llvm::FunctionPassManager optimiser( module );
    optimiser.add( new llvm::TargetData( *execution_engine->getTargetData() ) );
    optimiser.add( llvm::createBasicAliasAnalysisPass() );
    optimiser.add( llvm::createInstructionCombiningPass() );
    optimiser.add( llvm::createReassociatePass() );
    optimiser.add( llvm::createGVNPass() );
    optimiser.add( llvm::createCFGSimplificationPass() );
    optimiser.doInitialization();
    auto& list = module->getFunctionList();
    for ( auto i = list.begin(); i != list.end(); ++i )
        optimiser.run( *i );

    std::cout << "optimised ir code:\n";
    module->dump();
    std::cout << "\n\n";

    void* jit_func = execution_engine->getPointerToFunction( &list.back() );
    struct ReturnType { rave_int a; rave_int b; };
    ReturnType r = ( ( ReturnType (*)( rave_int ) )( intptr_t )jit_func )( 5 );
    std::cout << "int value: " << r.a << ", " << r.b << "\n";
    delete module;
    return 0;
}