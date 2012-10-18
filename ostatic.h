#ifndef OSTATIC_H
#define OSTATIC_H

#include "operator.h"
#include "symbol.h"

class StaticOperator : public Operator {
public:

    StaticOperator( int* errors );
    ~StaticOperator();

    OPERATOR;

    const Type& InferredType() const;

private:

    void Error( const Ast& arg, const std::string& text );
    int* _errors;

    Ast* Convert( Ast* expr, Type from, Type to );

    Type _type;
    Type _return_type;
    bool _return_path;
    bool _declare_globals;
    bool _declarations;
    bool _let_variables;
    Type _let_type;
    Type::TypeList _declaration_list;
    SymbolTable< Type > _table;

};

#endif