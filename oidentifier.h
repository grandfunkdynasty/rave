#ifndef OIDENTIFIER_H
#define OIDENTIFIER_H

#include "operator.h"
#include "symbol.h"

class IdentifierOperator : public ConstOperator {
public:

    IdentifierOperator();
    ~IdentifierOperator();

    CONST_OPERATOR;
    bool Nested() const;
    bool AllIdentifiers() const;

private:

    bool _nested;
    bool _allIdentifiers;

};

#endif