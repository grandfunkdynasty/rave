#ifndef ODECLARE_H
#define ODECLARE_H

#include "operator.h"
#include "symbol.h"
class TypeOperator;
class StaticOperator;
class IrGenOperator;

class DeclareOperator : public ConstOperator {
public:

    DeclareOperator( TypeOperator& otype );
    DeclareOperator( StaticOperator& ostatic );
    DeclareOperator( IrGenOperator& ostatic );
    virtual ~DeclareOperator();

    CONST_OPERATOR;

private:

    enum DeclareOwner {
        DECLARE_TYPE,
        DECLARE_STATIC,
        DECLARE_IRGEN
    };

    DeclareOperator& operator=( const DeclareOperator& rhs ) { return *this; }

    DeclareOwner _owner;
    TypeOperator* _otype;
    StaticOperator* _ostatic;
    IrGenOperator* _oirgen;

    bool _global_scope;
    std::string _namespace;
    std::size_t _declare_globals;
    std::size_t _declare_prototypes;
    Type::TypeList _declaration_list;

};

#endif