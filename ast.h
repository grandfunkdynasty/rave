#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

/***************************************************************
* AST base class
***************************************************************/

class Operator;
class ConstOperator;

class Ast {
public:

    typedef std::vector<Ast*> AstList;

    Ast();
    virtual ~Ast();
    void SetInfo( const std::string& file, int line );

    const std::string& GetFileInfo() const;
    int GetLineInfo() const;

    virtual void Accept( Operator& op ) = 0;
    virtual void Accept( ConstOperator& op ) const = 0;

private:

    std::string _file;
    int _line;

};

/***************************************************************
* Operator implementation magic
***************************************************************/

#include "otype.h"
#include "ostatic.h"
#include "ostring.h"

#define ACCEPT( type )                                                  \
                                                                        \
    virtual void Accept( Operator& op )                                 \
    {                                                                   \
        op.Operate##type( *this );                                      \
    }                                                                   \
                                                                        \
    virtual void Accept( ConstOperator& op ) const                      \
    {                                                                   \
        op.Operate##type( *this );                                      \
    }                                                                   \
                                                                        \
    friend void TypeOperator::Operate##type( type& arg );               \
    friend void StaticOperator::Operate##type( type& arg );       \
    friend void StringOperator::Operate##type( const type& arg )

#endif