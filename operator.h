#ifndef OPERATOR_H
#define OPERATOR_H

/***************************************************************
* AST classes
***************************************************************/

#define CLASS_LIST( X )     \
    X( ParseError );        \
    X( Constant );          \
    X( Identifier );        \
    X( TernaryOp );         \
    X( BinaryOp );          \
    X( UnaryOp );           \
    X( TypeOp );            \
    X( TupleConstruct );    \
    X( TupleExtract );      \
    X( TupleReplace );      \
    X( FunctionCall );      \
    X( Promoter );          \
    X( Body );              \
    X( Return );            \
    X( Guard );             \
    X( Let );               \
    X( Block );             \
    X( ScopeSet );          \
    X( ScopeDef );          \
    X( Loop );              \
    X( SequenceCall );      \
    X( FxStatement );       \
    X( SplitStatement );    \
    X( Layer );             \
    X( Argument );          \
    X( FuncDef );           \
    X( SeqDef );            \
    X( VidDef );            \
    X( TypeDef );           \
    X( Program )

/***************************************************************
* Forward declarations
***************************************************************/

#define FORWARD( type )     \
    class type
CLASS_LIST( FORWARD );
#undef FORWARD
class Ast;

/***************************************************************
* Operator
***************************************************************/

class Operator {
public:

    #define OPERATION( type )       \
        virtual void Operate##type( type& arg ) = 0
    CLASS_LIST( OPERATION );
    #undef OPERATION

    void Operate( Ast* operand );

};

/***************************************************************
* Constant operator
***************************************************************/

class ConstOperator {
public:
    
    #define OPERATION( type )       \
        virtual void Operate##type( const type& arg ) = 0
    CLASS_LIST( OPERATION );
    #undef OPERATION

    void Operate( const Ast* operand );

};

/***************************************************************
* Operator implementation magic
***************************************************************/

#define OPERATION( type )           \
    virtual void Operate##type( type& arg )
#define CONST_OPERATION( type )     \
    virtual void Operate##type( const type& arg )
#define OPERATOR                    \
        CLASS_LIST( OPERATION )
#define CONST_OPERATOR              \
        CLASS_LIST( CONST_OPERATION )
#define IMPLEMENT( type )           \
    void IMPLEMENT_OPERATOR::Operate##type( IMPLEMENT_TYPE type& arg )
#define IMPLEMENT_EMPTY( type )     \
    void IMPLEMENT_OPERATOR::Operate##type( IMPLEMENT_TYPE type& arg ) { }
#define IMPLEMENT_CONST const
#define IMPLEMENT_NON_CONST

#endif