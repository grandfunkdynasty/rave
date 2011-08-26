#ifndef DECS_H
#define DECS_H

#include "ast.h"
#include "type.h"

/***************************************************************
* Argument
***************************************************************/

class Argument : public Ast {
public:

    Argument( Type type, const std::string& id );
    virtual ~Argument();

    ACCEPT( Argument );

private:

    Type _type;
    std::string _id;

};

/***************************************************************
* Function definition
***************************************************************/

class FuncDef : public Ast {
public:

    FuncDef( int modifiers, Type return_type, const std::string& id,
             const AstList& args, Ast* expr );
    virtual ~FuncDef();

    ACCEPT( FuncDef );

private:

    int _modifiers;
    Type _return_type;
    std::string _id;
    AstList _args;
    Ast* _expr;

};

/***************************************************************
* Sequence definition
***************************************************************/

class SeqDef : public Ast {
public:

    SeqDef( int modifiers, const std::string& id,
            const AstList& args, Ast* statement );
    virtual ~SeqDef();

    ACCEPT( SeqDef );

private:

    int _modifiers;
    std::string _id;
    AstList _args;
    Ast* _statement;

};

/***************************************************************
* Video definition
***************************************************************/

class VidDef : public Ast {
public:

    VidDef( int modifiers, const std::string& id,
            Ast* frame_count, Ast* statement );
    virtual ~VidDef();

    ACCEPT( VidDef );

private:

    int _modifiers;
    std::string _id;
    Ast* _frame_count;
    Ast* _statement;

};

/***************************************************************
* Type definition
***************************************************************/

class TypeDef : public Ast {
public:

    TypeDef( int modifiers, Type type, const std::string& id );
    virtual ~TypeDef();

    ACCEPT( TypeDef );

private:

    int _modifiers;
    Type _type;
    std::string _id;

};

/***************************************************************
* Program
***************************************************************/

class Program : public Ast {
public:

    Program( const AstList& elements );
    virtual ~Program();

    ACCEPT( Program );

private:

    AstList _elements;

};

#endif