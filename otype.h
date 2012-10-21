#ifndef OTYPE_H
#define OTYPE_H

#include "operator.h"
#include "symbol.h"

class TypeOperator : public Operator {
public:

    TypeOperator( int* errors );
    virtual ~TypeOperator();

    OPERATOR;

private:

    friend class DeclareOperator;
    void Error( const Ast& arg, const std::string& text );
    int* _errors;

    Type Resolve( const Ast& arg, const Type& type );
    SymbolTable< Type > _table;

};

#endif