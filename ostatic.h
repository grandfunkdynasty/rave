#ifndef OSTATIC_H
#define OSTATIC_H

#include "operator.h"
#include "symbol.h"

class StaticOperator : public Operator {
public:

    StaticOperator( int* errors );
    virtual ~StaticOperator();

    OPERATOR;

    const Type& InferredType() const;

private:

    friend class DeclareOperator;
    void Error( const Ast& arg, const std::string& text );
    int* _errors;

    Ast* Convert( Ast* expr, Type from, Type to );

    Type _type;
    Type _return_type;
    bool _return_path;
    bool _let_variables;
    Type _let_type;
    SymbolTable< Type > _table;

};

#endif