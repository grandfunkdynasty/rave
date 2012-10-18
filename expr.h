#ifndef EXPR_H
#define EXPR_H

#include "ast.h"
#include "type.h"

/***************************************************************
* Parse error
***************************************************************/

class ParseError : public Ast {
public:

    ParseError();
    virtual ~ParseError();

    ACCEPT( ParseError );

};

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
    Type _op_type;

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

    TernaryOp( Ast* condition, Ast* left, Ast* right );
    virtual ~TernaryOp();

    ACCEPT( TernaryOp );

private:

    Ast* _expr;
    Ast* _left;
    Ast* _right;
    Type _value_type;

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
* Tuple extraction
***************************************************************/

class TupleExtract : public Ast {
public:

    TupleExtract( Ast* tuple, Ast* index );
    virtual ~TupleExtract();

    ACCEPT( TupleExtract );

private:

    Ast* _tuple;
    Ast* _index;

};

/***************************************************************
* Tuple replacement
***************************************************************/

class TupleReplace : public Ast {
public:

    TupleReplace( Ast* tuple, Ast* index, Ast* expr );
    virtual ~TupleReplace();

    ACCEPT( TupleReplace );

private:

    Ast* _tuple;
    Ast* _index;
    Ast* _expr;

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

/***************************************************************
* Convert
***************************************************************/

class Converter : public Ast {
public:

    Converter( Ast* expr, Type from, Type to );
    virtual ~Converter();

    ACCEPT( Converter );

private:

    Ast* _expr;
    Type _from;
    Type _to;

};

#endif