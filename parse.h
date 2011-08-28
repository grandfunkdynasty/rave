#ifndef PARSE_H
#define PARSE_H

#include "astenum.h"

#ifndef LINUX
#define strdup _strdup
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
* Parse data
***************************************************************/

    struct Node {
        int type;
        int sub_type;

        char* string_data;
        long int_data;
        double float_data;

        struct NodeList* begin;
        struct NodeList* end;

        char* file;
        int line;
    };

    struct NodeList {
        struct Node* elem;
        struct NodeList* next;
    };

    struct IncludeList {
        char* file;
        struct IncludeList* next;
    };

/***************************************************************
* Parse helper functions
***************************************************************/

    struct Node* alloc_node( int type, int sub_type );
    struct Node* alloc_binary_node( int sub_type, struct Node* left, struct Node* right );
    struct Node* alloc_typeop_node( int sub_type, struct Node* left, struct Node* right );
    void set_type( struct Node* node, int type, int sub_type );
    void push_front( struct Node* parent, struct Node* child );
    void push_back( struct Node* parent, struct Node* child );

/***************************************************************
* Parse functions
***************************************************************/

#ifdef __cplusplus
    class Ast;
    Ast* parse( const std::string& path );
#endif

    void parse_set_error( int* errors );
    void write_error( const char* name, int line, const char* next, const char* text, int unexpected );

#ifdef __cplusplus
}
#endif

#endif