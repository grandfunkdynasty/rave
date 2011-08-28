#ifndef ASTENUM_H
#define ASTENUM_H

/***************************************************************
* Node type enums
***************************************************************/

enum NodeType {
    NODE_UNDEFINED,

    // Types
    NODE_TYPE,

    // Expressions
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_TERNARY_OP,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_TYPE_OP,
    NODE_TUPLE_CONSTRUCT,
    NODE_TUPLE_EXTRACT,
    NODE_TUPLE_REPLACE,
    NODE_FUNCTION_CALL,
    NODE_PROMOTE,

    // Functions and sequences
    NODE_BODY,
    NODE_RETURN,
    NODE_GUARD,
    NODE_LET,
    NODE_BLOCK,
    NODE_SCOPE_SET,
    NODE_SCOPE_DEF,
    NODE_LOOP,
    NODE_SEQUENCE_CALL,
    NODE_FX,
    NODE_SPLIT,
    NODE_LAYER,

    // Declarations
    NODE_ARGUMENT,
    NODE_FUNC_DEF,
    NODE_SEQ_DEF,
    NODE_VID_DEF,
    NODE_TYPE_DEF,
    NODE_PROGRAM
};

/***************************************************************
* Node subtype enums
***************************************************************/

enum NodeSubType {
    SUB_TYPE_UNDEFINED = 0,

    // Literal types
    LITERAL_INT = 1,
    LITERAL_FLOAT,

    // Binary operator types
    BINARY_OP_OR = 1,
    BINARY_OP_AND,
    BINARY_OP_EQ,
    BINARY_OP_NE,
    BINARY_OP_GT,
    BINARY_OP_GE,
    BINARY_OP_LT,
    BINARY_OP_LE,
    BINARY_OP_BIT_OR,
    BINARY_OP_BIT_XOR,
    BINARY_OP_BIT_AND,
    BINARY_OP_LSHIFT,
    BINARY_OP_RSHIFT,
    BINARY_OP_ADD,
    BINARY_OP_SUB,
    BINARY_OP_MUL,
    BINARY_OP_DIV,
    BINARY_OP_MOD,
    BINARY_OP_EXP,

    // Unary operator types
    UNARY_OP_NOT = 1,
    UNARY_OP_BIT_NOT,
    UNARY_OP_NEGATION,
    UNARY_OP_FLOOR,

    // Type operator types
    TYPE_OP_EQ = 1,
    TYPE_OP_FROM,
    TYPE_OP_TO,

    // Type types
    TYPE_INT = 1,
    TYPE_FLOAT,
    TYPE_TUPLE,
    TYPE_FUNCTION,
    TYPE_SEQUENCE,
    TYPE_TYPEDEF,

    // Layer types
    LAYER_LAYER = 1,
    LAYER_COPY = 2,
    LAYER_BASE = 4,
    LAYER_FX = 8,

    // Modifier types
    MODIFIER_LOCAL = 1,
    MODIFIER_CACHE = 2
};

#endif