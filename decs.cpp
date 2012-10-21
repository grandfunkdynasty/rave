#include "decs.h"

/***************************************************************
* Argument
***************************************************************/

Argument::Argument( Type type, const std::string& id )
: _type( type )
, _id( id )
{
}

Argument::~Argument()
{
}

/***************************************************************
* Function definition
***************************************************************/

FuncDef::FuncDef( int modifiers, Type return_type, const std::string& id,
             const AstList& args, Ast* expr )
: _modifiers( modifiers )
, _return_type( return_type )
, _id( id )
, _args( args )
, _expr( expr )
, _llvm_function( 0 )
{
}

FuncDef::~FuncDef()
{
    for ( std::size_t i = 0; i < _args.size(); ++i )
        delete _args[ i ];
    delete _expr;
}

/***************************************************************
* Sequence definition
***************************************************************/

SeqDef::SeqDef( int modifiers, const std::string& id,
                const AstList& args, Ast* statement )
: _modifiers( modifiers )
, _id( id )
, _args( args )
, _statement( statement )
{
}

SeqDef::~SeqDef()
{
    for ( std::size_t i = 0; i < _args.size(); ++i )
        delete _args[ i ];
    delete _statement;
}

/***************************************************************
* Video definition
***************************************************************/

VidDef::VidDef( int modifiers, const std::string& id,
                Ast* frame_count, Ast* statement )
: _modifiers( modifiers )
, _id( id )
, _frame_count( frame_count )
, _statement( statement )
{
}

VidDef::~VidDef()
{
    delete _frame_count;
    delete _statement;
}

/***************************************************************
* Type definition
***************************************************************/

TypeDef::TypeDef( int modifiers, Type type, const std::string& id )
: _modifiers( modifiers )
, _type( type )
, _id( id )
{
}

TypeDef::~TypeDef()
{
}

/***************************************************************
* Program
***************************************************************/

Program::Program( int modifiers, const AstList& elements, const std::string& scope_name )
: _modifiers( modifiers )
, _elements( elements )
, _scope_name( scope_name )
{
}

Program::~Program()
{
    for ( std::size_t i = 0; i < _elements.size(); ++i )
        delete _elements[ i ];
}