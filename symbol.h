#ifndef SYMBOL_H
#define SYMBOL_H

#include <boost/tr1/unordered_map.hpp>
#include <vector>
#include "type.h"

/***************************************************************
* Symbol table
***************************************************************/

class SymbolTable {
public:

    SymbolTable();
    ~SymbolTable();

    void Push();
    void Pop();
    std::size_t Depth() const;
    bool AddEntry( const std::string& id, const Type& type );
    const Type& GetEntry( const std::string& id ) const;

/***************************************************************
* Internals
***************************************************************/

private:

    static Type _void;
    typedef boost::unordered_map<std::string, Type> ScopeTable;
    typedef std::vector< ScopeTable > ScopeStack;
    ScopeStack _scope_stack;

};

#endif