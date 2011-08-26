#include "ast.h"

Ast::Ast()
: _file( "" )
, _line( 0 )
{
}

Ast::~Ast()
{
}

void Ast::SetInfo( const std::string& file, int line )
{
    _file = file;
    _line = line;
}

const std::string& Ast::GetFileInfo() const
{
    return _file;
}

int Ast::GetLineInfo() const
{
    return _line;
}