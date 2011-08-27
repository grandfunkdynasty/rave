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
    _entry_stack.push_back( ScopeTable() );
}

void SymbolTable::Pop()
{
    if ( _entry_stack.size() > 0 )
        _entry_stack.erase( _entry_stack.end() - 1 );
}

std::size_t SymbolTable::Depth() const
{
    return _entry_stack.size() - 1;
}

bool SymbolTable::AddEntry( const std::string& id, const Type& type )
{
    ScopeTable& scope = _entry_stack[ Depth() ];
    return scope.insert( std::make_pair( id, type ) ).second;
}

bool SymbolTable::HasEntry( const std::string& id ) const
{
    for ( std::size_t i = Depth(); ; --i ) {
        const ScopeTable& scope = _entry_stack[ i ];
        ScopeTable::const_iterator it = scope.find( id );
        if ( it != scope.end() )
            return true;
        if ( !i )
            break;
    }
    return false;
}

const Type& SymbolTable::GetEntry( const std::string& id ) const
{
    for ( std::size_t i = Depth(); ; --i ) {
        const ScopeTable& scope = _entry_stack[ i ];
        ScopeTable::const_iterator it = scope.find( id );
        if ( it != scope.end() )
            return it->second;
        if ( !i )
            break;
    }
    return _void;
}