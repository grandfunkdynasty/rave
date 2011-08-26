#ifndef EXPR_H
#define EXPR_H

#include "ast.h"
#include "type.h"

/***************************************************************
* Constant
***************************************************************/

class Constant : public Ast {
public:

    Constant( rave_int value );
    Constant( rave_float value );
    virtual ~Constant();

    ACCEPT( Constant );

private:

    bool _is_int;
    rave_int _int_value;
    rave_float _float_value;

};

/***************************************************************
* Identifier
***************************************************************/

class Identifier : public Ast {
public:

    Identifier( const std::string& id );
    virtual ~Identifier();

    ACCEPT( Identifier );

private:

    std::string _id;

};

/***************************************************************
* Unary operator
***************************************************************/

class UnaryOp : public Ast {
public:

    UnaryOp( int type, Ast* expr );
    virtual ~UnaryOp();

    ACCEPT( UnaryOp );

private:

    int _type;
    Ast* _expr;

};

/***************************************************************
* Binary operator
***************************************************************/

class BinaryOp : public Ast {
public:

    BinaryOp( int type, Ast* left, Ast* right );
    virtual ~BinaryOp();

    ACCEPT( BinaryOp );

private:

    int _type;
    Ast* _left;
    Ast* _right;

};

/***************************************************************
* Type operator
***************************************************************/

class TypeOp : public Ast {
public:

    TypeOp( int type, Ast* left, Ast* right );
    TypeOp( int type, Type left, Ast* right );
    TypeOp( int type, Ast* left, Type right );
    TypeOp( int type, Type left, Type right );
    virtual ~TypeOp();

    ACCEPT( TypeOp );

private:

    int _type;
    Ast* _left;
    Ast* _right;
    Type _left_type;
    Type _right_type;

};

/***************************************************************
* Ternary operator
***************************************************************/

class TernaryOp : public Ast {
public:

    TernaryOp( int type, Ast* condition, Ast* left, Ast* right );
    virtual ~TernaryOp();

    ACCEPT( TernaryOp );

private:

    int _type;
    Ast* _expr;
    Ast* _left;
    Ast* _right;

};

/***************************************************************
* Tuple construction
***************************************************************/

class TupleConstruct : public Ast {
public:

    TupleConstruct( const AstList& list );
    virtual ~TupleConstruct();

    ACCEPT( TupleConstruct );

private:

    AstList _list;

};

/***************************************************************
* Function call
***************************************************************/

class FunctionCall : public Ast {
public:

    FunctionCall( Ast* function, const AstList& args );
    virtual ~FunctionCall();

    ACCEPT( FunctionCall );

private:

    Ast* _function;
    AstList _args;

};

#endif