#include <iostream>
#include "parse.h"
#include "ast.h"
#include "ostatic.h"
#include "ostring.h"

int main( int argc, char** argv )
{
    int errors = 0;
    parse_set_error( &errors );
    Ast* ast = parse( argv[ 1 ] );

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

    IrGenOperator oir;
    oir.Operate( ast );

    delete ast;
    return 0;
}