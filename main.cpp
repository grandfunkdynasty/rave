#include <iostream>
#include "parse.h"
#include "ast.h"
#include "ostatic.h"
#include "ostring.h"

int main( int argc, char** argv )
{
    Ast* ast = parse( argv[ 1 ] );
    if ( !ast )
        return 1;

    int errors = 0;
    std::cout << "checking semantics...";
    TypeOperator otype( &errors );
    otype.Operate( ast );

    StaticOperator ostatic ( &errors );
    ostatic.Operate( ast );
    if ( errors ) {
        std::cout << errors << " error(s) detected.\n";
        return 1;
    }
    std::cout << " success\n";

    StringOperator ostring;
    ostring.Operate( ast );
    std::cout << ostring.Result();
    delete ast;
    return 0;
}