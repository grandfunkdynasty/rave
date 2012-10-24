#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <vector>
#include <boost/tr1/unordered_set.hpp>
#include <map>

/***************************************************************
* Raw primitive types
***************************************************************/

typedef long rave_int;
typedef double rave_float;
class Internal;
namespace llvm {
    class Type;
    class LLVMContext;
}

/***************************************************************
* Types
***************************************************************/

class Type {
public:

    typedef std::vector< Type > TypeList;
    typedef std::map< std::string, Type > TypeMap;

    static Type Void();
    static Type Bool();
    static Type Int();
    static Type Float();
    static Type Tuple( const TypeList& type_args );
    static Type Algebraic( const TypeMap& type_map );
    static Type Function( const Type& return_type, const TypeList& arg_types );
    static Type Sequence( const TypeList& arg_types );
    static Type Typedef( const std::string& name );
    static Type Typedef( const std::string& name, const Type& type );

    ~Type();

    Type( const Type& type );
    const Type& operator=( const Type& type );

    bool operator==( const Type& type ) const;
    bool operator!=( const Type& type ) const;
    bool Equivalent( const Type& type ) const;
    bool ConvertsTo( const Type& type ) const;
    Type Generalise( const Type& type ) const;

    std::string Typename() const;
    bool IsUnresolved() const;
    const std::string& Typedef() const;
    bool IsTuple() const;
    bool IsAlgebraic() const;
    bool IsFunction() const;
    bool IsSequence() const;
    const Type& ReturnType() const;
    const TypeList& TypeArgs() const;
    const TypeMap& TypeArgsMap() const;

    llvm::Type* LlvmType( llvm::LLVMContext& context ) const;

/***************************************************************
* Internals
***************************************************************/

private:

    class Hash {
    public:

        std::size_t operator()( const Internal& type ) const;

    };

    std::string _typename;
    const Internal* _type;
    friend class TypeMap;
    Type();
    Type( const Internal& type );
    Type( const std::string& name );
    Type( const std::string& name, const Internal& type );

    typedef boost::unordered_set< Internal, Hash > InternalSet;
    static InternalSet _type_set;

};

#endif