#ifndef ODECLARE_H
#define ODECLARE_H

#include "operator.h"
#include "symbol.h"
class TypeOperator;
class StaticOperator;
class IrGenOperator;

class DeclareOperator : public ConstOperator {
public:

    DeclareOperator( TypeOperator& otype, bool resolve );
    DeclareOperator( StaticOperator& ostatic );
    DeclareOperator( IrGenOperator& ostatic );
    virtual ~DeclareOperator();

    CONST_OPERATOR;

private:

    enum DeclareOwner {
        DECLARE_TYPE,
        DECLARE_TYPE_RESOLVE,
        DECLARE_STATIC,
        DECLARE_IRGEN
    };

    DeclareOperator& operator=( const DeclareOperator& rhs ) { return *this; }
    void DeclareAlgebraicConstructors( const Ast& arg, const Type& type );
    void DeclareAlgebraicConstructors( const Ast& arg, const std::string& rel_name,
                                       const Type& type );
    void IrGenAlgebraicConstructors( const Ast& arg, const Type& type );
    void IrGenAlgebraicConstructors( const Ast& arg, const std::string& rel_name,
                                     const Type& type );

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