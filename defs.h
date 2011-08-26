#ifndef DEFS_H
#define DEFS_H

#include "ast.h"

/***************************************************************
* Body
***************************************************************/

class Body : public Ast {
public:

    Body( const AstList& steps );
    virtual ~Body();

    ACCEPT( Body );

private:

    AstList _steps;

};

/***************************************************************
* Return
***************************************************************/

class Return : public Ast {
public:

    Return( Ast* expr );
    virtual ~Return();

    ACCEPT( Return );

private:

    Ast* _expr;

};

/***************************************************************
* Guard
***************************************************************/

class Guard : public Ast {
public:

    Guard( Ast* expr, Ast* then );
    Guard( Ast* expr, Ast* then, Ast* otherwise );
    virtual ~Guard();

    ACCEPT( Guard );

private:

    Ast* _expr;
    Ast* _then;
    Ast* _otherwise;

};

/***************************************************************
* Let
***************************************************************/

class Let : public Ast {
public:

    Let( const std::string& id, Ast* expr, Ast* in );
    virtual ~Let();

    ACCEPT( Let );

private:

    std::string _id;
    Ast* _expr;
    Ast* _in;

};

/***************************************************************
* Block
***************************************************************/

class Block : public Ast {
public:

    Block( Ast* scope_set, const AstList& statements );
    virtual ~Block();

    ACCEPT( Block );

private:

    Ast* _scope_set;
    AstList _statements;

};

/***************************************************************
* Scope set
***************************************************************/

class ScopeSet : public Ast {
public:

    ScopeSet( const AstList& scope_defs );
    virtual ~ScopeSet();

    ACCEPT( ScopeSet );

private:

    AstList _scope_defs;

};

/***************************************************************
* Scope definition
***************************************************************/

class ScopeDef : public Ast {
public:

    ScopeDef( const std::string& id, Ast* expr );
    virtual ~ScopeDef();

    ACCEPT( ScopeDef );

private:

    std::string _id;
    Ast* _expr;

};

/***************************************************************
* Loop
***************************************************************/

class Loop : public Ast {
public:

    Loop( const std::string& id, Ast* begin, Ast* end, Ast* in );
    virtual ~Loop();

    ACCEPT( Loop );

private:

    std::string _id;
    Ast* _begin;
    Ast* _end;
    Ast* _in;

};

/***************************************************************
* Sequence call
***************************************************************/

class SequenceCall : public Ast {
public:

    SequenceCall( Ast* sequence, const AstList& args );
    virtual ~SequenceCall();

    ACCEPT( SequenceCall );

private:

    Ast* _sequence;
    AstList _args;

};

/***************************************************************
* FX statement
***************************************************************/

class FxStatement : public Ast {
public:

    FxStatement( Ast* _expr );
    virtual ~FxStatement();

    ACCEPT( FxStatement );

private:

    Ast* _expr;

};

/***************************************************************
* Split statement
***************************************************************/

class SplitStatement : public Ast {
public:

    SplitStatement( const AstList& layers );
    virtual ~SplitStatement();

    ACCEPT( SplitStatement );

private:

    AstList _layers;

};

/***************************************************************
* Layer
***************************************************************/

class Layer : public Ast {
public:

    Layer( int type, Ast* statement );
    Layer( int type, Ast* order, Ast* statement );
    Layer( int type, Ast* order, Ast* fx, Ast* statement );
    virtual ~Layer();

    ACCEPT( Layer );

private:

    int _type;
    Ast* _order;
    Ast* _fx;
    Ast* _statement;

};

#endif