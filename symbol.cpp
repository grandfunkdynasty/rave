#include "symbol.h"

Type SymbolTable::_void( Type::Void() );

SymbolTable::SymbolTable()
{
}

SymbolTable::~SymbolTable()
{
}

void SymbolTable::Push()
{
    _scope_stack.push_back( ScopeTable() );
}

void SymbolTable::Pop()
{
    if ( _scope_stack.size() > 0 )
        _scope_stack.erase( _scope_stack.end() - 1 );
}

std::size_t SymbolTable::Depth() const
{
    return _scope_stack.size() - 1;
}

bool SymbolTable::AddEntry( const std::string& id, const Type& type )
{
    ScopeTable& scope = _scope_stack[ Depth() ];
    return scope.insert( std::make_pair( id, type ) ).second;
}

const Type& SymbolTable::GetEntry( const std::string& id ) const
{
    for ( std::size_t i = Depth(); ; --i ) {
        const ScopeTable& scope = _scope_stack[ i ];
        ScopeTable::const_iterator it = scope.find( id );
        if ( it != scope.end() )
            return it->second;
        if ( !i )
            break;
    }
    return _void;
}