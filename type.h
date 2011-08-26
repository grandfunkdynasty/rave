#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <vector>
#include <boost/tr1/unordered_set.hpp>

/***************************************************************
* Raw primitive types
***************************************************************/

typedef long rave_int;
typedef double rave_float;

/***************************************************************
* Types
***************************************************************/

class Type {
public:

    typedef std::vector<Type> TypeList;

    static Type Void();
    static Type Int();
    static Type Float();
    static Type Tuple( const TypeList& type_args );
    static Type Function( const Type& return_type, const TypeList& arg_types );
    static Type Sequence( const TypeList& arg_types );

    ~Type();

    Type( const Type& type );
    const Type& operator=( const Type& type );

    bool operator==( const Type& type ) const;
    bool operator!=( const Type& type ) const;
    bool ConvertsTo( const Type& type ) const;
    Type Generalise( const Type& type ) const;

    std::string TypeName() const;
    bool IsTuple() const;
    bool IsFunction() const;
    bool IsSequence() const;
    Type ReturnType() const;
    std::size_t ArgTypeSize() const;
    Type ArgType( std::size_t index ) const;

/***************************************************************
* Internals
***************************************************************/

private:

    enum RawType {
        TYPE_VOID,
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_TUPLE,
        TYPE_FUNCTION,
        TYPE_SEQUENCE
    };

    class Internal {
    public:

        typedef std::vector<const Internal*> TypeArgs;

        Internal( int raw_type, const Internal* return_type, const TypeArgs& type_args );

        Internal( const Internal& type );
        const Internal& operator=( const Internal& type );

        bool operator==( const Internal& type ) const;
        bool operator!=( const Internal& type ) const;
        bool ConvertsTo( const Internal& type ) const;
        Type Generalise( const Internal& type ) const;

        std::string TypeName() const;
        int RawType() const;
        const Internal* ReturnType() const;
        const TypeArgs& ArgTypes() const;

        class Hash {
        public:

            std::size_t operator()( const Internal& type ) const;

        };

    private:

        int _raw_type;
        const Internal* _return_type;
        TypeArgs _type_args;

    };

    const Internal* _type;
    Type( const Internal& type );

    typedef boost::unordered_set<Internal, Internal::Hash> InternalSet;
    static InternalSet _type_set;

};

#endif