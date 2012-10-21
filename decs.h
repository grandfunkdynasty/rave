#ifndef DECS_H
#define DECS_H

#include "ast.h"
#include "type.h"
namespace llvm {
    class Function;
}

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
    mutable Type::TypeList _arg_types;
    mutable llvm::Function* _llvm_function;

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
    mutable Type::TypeList _arg_types;

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

    Program( int modifiers, const AstList& elements, const std::string& scope_name );
    virtual ~Program();

    ACCEPT( Program );

private:

    int _modifiers;
    AstList _elements;
    std::string _scope_name;

};

#endif