#include "operator.h"
#include "ast.h"

void Operator::Operate( Ast* operand )
{
    operand->Accept( *this );
}

void ConstOperator::Operate( const Ast* operand )
{
    operand->Accept( *this );
}