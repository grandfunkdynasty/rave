#ifndef OEXPAND_H
#define OEXPAND_H

#include "operator.h"
#include "symbol.h"

class ExpandOperator : public Operator {
public:

    ExpandOperator();
    virtual ~ExpandOperator();

    OPERATOR;

};

#endif