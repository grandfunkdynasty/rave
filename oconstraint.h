#ifndef OCONSTRAINT_H
#define OCONSTRAINT_H

#include "operator.h"
#include "symbol.h"

class SubtreeConstraintOperator : public ConstOperator {
public:

    SubtreeConstraintOperator( const SymbolTable< Type >& table );
    virtual ~SubtreeConstraintOperator();

    CONST_OPERATOR;
    bool SingleIdentifier() const;
    const std::string& SingleIdentifierName() const;
    bool ValidLetVariables() const;
    bool ConstantInt() const;
    rave_int ConstantIntValue() const;

private:

    SubtreeConstraintOperator& operator=( const SubtreeConstraintOperator& o );

    const SymbolTable< Type >& _table;
    bool _single_identifier;
    std::string _single_identifier_name;
    bool _valid_let_variables;
    bool _constant_int;
    rave_int _constant_int_value;

};

#endif