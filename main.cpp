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

    std::cout << "checking...";
    StaticOperator ostatic;
    ostatic.Operate( ast );
    if ( ostatic.Errors() )
        std::cout << ostatic.Errors() << " error(s) detected.\n";
    else
        std::cout << " success\n";

    StringOperator ostring;
    ostring.Operate( ast );
    std::cout << ostring.Result();
    delete ast;
    return 0;
}