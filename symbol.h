#ifndef SYMBOL_H
#define SYMBOL_H

#include <boost/tr1/unordered_map.hpp>
#include <vector>
#include "type.h"

/***************************************************************
* Symbol table
***************************************************************/

template< typename T >
class SymbolTable {
public:

    SymbolTable();
    ~SymbolTable();

    void Push();
    void Pop();
    std::size_t Depth() const;

    bool AddEntry( const std::string& id, const T& t );
    bool HasEntry( const std::string& id ) const;
    const T& GetEntry( const std::string& id ) const;

/***************************************************************
* Internals
***************************************************************/

private:

    static T _void;
    typedef boost::unordered_map< std::string, T > ScopeTable;
    typedef std::vector< ScopeTable > ScopeStack;
    ScopeStack _entry_stack;

};

template< typename T >
T SymbolTable< T >::_void( 0 );
template<>
Type SymbolTable< Type >::_void( Type::Void() );

template< typename T >
SymbolTable< T >::SymbolTable()
{
}

template< typename T >
SymbolTable< T >::~SymbolTable()
{
}

template< typename T >
void SymbolTable< T >::Push()
{
    _entry_stack.push_back( ScopeTable() );
}

template< typename T >
void SymbolTable< T >::Pop()
{
    if ( _entry_stack.size() > 0 )
        _entry_stack.erase( _entry_stack.end() - 1 );
}

template< typename T >
std::size_t SymbolTable< T >::Depth() const
{
    return _entry_stack.size() - 1;
}

template< typename T >
bool SymbolTable< T >::AddEntry( const std::string& id, const T& t )
{
    auto& scope = _entry_stack[ Depth() ];
    return scope.insert( std::make_pair( id, t ) ).second;
}

template< typename T >
bool SymbolTable< T >::HasEntry( const std::string& id ) const
{
    for ( std::size_t i = Depth(); ; --i ) {
        const auto& scope = _entry_stack[ i ];
        auto it = scope.find( id );
        if ( it != scope.end() )
            return true;
        if ( !i )
            break;
    }
    return false;
}

template< typename T >
const T& SymbolTable< T >::GetEntry( const std::string& id ) const
{
    for ( std::size_t i = Depth(); ; --i ) {
        const auto& scope = _entry_stack[ i ];
        auto it = scope.find( id );
        if ( it != scope.end() )
            return it->second;
        if ( !i )
            break;
    }
    return _void;
}

#endif