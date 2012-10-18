#ifndef OSTRING_H
#define OSTRING_H

#include "operator.h"
#include <string>
#include <sstream>

class StringOperator : public ConstOperator {
public:

    StringOperator();
    virtual ~StringOperator();

    CONST_OPERATOR;
    std::string Result() const;

private:

    std::string Indent() const;
    std::stringstream _result;
    int _indent;

};

#endif