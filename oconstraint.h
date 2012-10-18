#ifndef OCONSTRAINT_H
#define OCONSTRAINT_H

#include "operator.h"
#include "symbol.h"

class SubtreeConstraintOperator : public ConstOperator {
public:

    SubtreeConstraintOperator();
    virtual ~SubtreeConstraintOperator();

    CONST_OPERATOR;
    bool Nested() const;
    bool AllIdentifiers() const;
    bool ConstantInt() const;
    rave_int ConstantIntValue() const;

private:

    bool _nested;
    bool _all_identifiers;
    bool _constant_int;
    rave_int _constant_int_value;

};

#endif