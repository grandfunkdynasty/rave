#include "type.h"
#pragma warning(push, 0)
#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#pragma warning(pop)

Type::InternalSet Type::_type_set;

/***************************************************************
* Type internals
***************************************************************/

class Internal {
public:

    enum RawType {
        TYPE_VOID,
        TYPE_BOOL,
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_TUPLE,
        TYPE_ALGEBRAIC,
        TYPE_FUNCTION,
        TYPE_SEQUENCE
    };

    Internal( int raw_type, Type return_type, const Type::TypeList& type_args );
    Internal( int raw_type, const Type::TypeMap& type_map );

    Internal( const Internal& type );
    Internal& operator=( const Internal& type );

    bool operator==( const Internal& type ) const;
    bool operator!=( const Internal& type ) const;
    bool Equivalent( const Internal& type ) const;
    bool ConvertsTo( const Internal& type ) const;
    Type Generalise( const Internal& type ) const;

    std::string Typename() const;
    int RawType() const;
    const Type& ReturnType() const;
    const Type::TypeList& TypeArgs() const;
    const Type::TypeMap& TypeArgsMap() const;
    llvm::Type* LlvmType( llvm::LLVMContext& context ) const;

private:

    int _raw_type;
    Type _return_type;
    Type::TypeList _type_args;
    Type::TypeMap _type_map;

};

/***************************************************************
* Primitive types
***************************************************************/

Type Type::Void()
{
    static Internal result( Internal::TYPE_VOID, Type::Void(), TypeList() );
    return result;
}

Type Type::Bool()
{
    static Internal result( Internal::TYPE_BOOL, Type::Void(), TypeList() );
    return result;
}

Type Type::Int()
{
    static Internal result( Internal::TYPE_INT, Type::Void(), TypeList() );
    return result;
}

Type Type::Float()
{
    static Internal result( Internal::TYPE_FLOAT, Type::Void(), TypeList() );
    return result;
}

/***************************************************************
* Compound types
***************************************************************/

Type Type::Tuple( const TypeList& type_args )
{
    Internal result( Internal::TYPE_TUPLE, Type::Void(), type_args );
    return *_type_set.insert( result ).first;
}

Type Type::Algebraic( const TypeMap& type_map )
{
    Internal result( Internal::TYPE_ALGEBRAIC, type_map );
    return *_type_set.insert( result ).first;
}

Type Type::Function( const Type& return_type, const TypeList& arg_types )
{
    Internal result( Internal::TYPE_FUNCTION, return_type, arg_types );
    return *_type_set.insert( result ).first;
}

Type Type::Sequence( const TypeList& arg_types )
{
    Internal result( Internal::TYPE_SEQUENCE, Type::Void(), arg_types );
    return *_type_set.insert( result ).first;
}

Type Type::Typedef( const std::string& name )
{
    return name;
}

Type Type::Typedef( const std::string& name, const Type& type )
{
    return Type( name, *type._type );
}

/***************************************************************
* Type interface
***************************************************************/

Type::~Type()
{
}

Type::Type( const Type& type )
: _type( type._type )
, _typename( type._typename )
{
}

Type& Type::operator=( const Type& type )
{
    _type = type._type;
    _typename = type._typename;
    return *this;
}

bool Type::operator==( const Type& type ) const
{
    return _type == type._type && _typename == type._typename;
}

bool Type::operator!=( const Type& type ) const
{
    return _type != type._type;
}

bool Type::Equivalent( const Type& type ) const
{
    if ( !_type || !type._type )
        return false;
    return _type->Equivalent( *type._type );
}

bool Type::ConvertsTo( const Type& type ) const
{
    if ( !_type || !type._type )
        return false;
    return _type->ConvertsTo( *type._type );
}

Type Type::Generalise( const Type& type ) const
{
    if ( !_type || !type._type )
        return Type::Void();
    return _type->Generalise( *type._type );
}

std::string Type::Typename() const
{
    if ( _typename != "" )
        return "~" + _typename;
    if ( !_type )
        return "undefined";
    return _type->Typename();
}

bool Type::IsUnresolved() const
{
    return !_type;
}

const std::string& Type::Typedef() const
{
    return _typename;
}

bool Type::IsTuple() const
{
    if ( !_type )
        return false;
    return _type->RawType() == Internal::TYPE_TUPLE;
}

bool Type::IsAlgebraic() const
{
    if ( !_type )
        return false;
    return _type->RawType() == Internal::TYPE_ALGEBRAIC;
}

bool Type::IsFunction() const
{
    if ( !_type )
        return false;
    return _type->RawType() == Internal::TYPE_FUNCTION;
}

bool Type::IsSequence() const
{
    if ( !_type )
        return false;
    return _type->RawType() == Internal::TYPE_SEQUENCE;
}

const Type& Type::ReturnType() const
{
    return _type->ReturnType();
}

const Type::TypeList& Type::TypeArgs() const
{
    return _type->TypeArgs();
}

const Type::TypeMap& Type::TypeArgsMap() const
{
    return _type->TypeArgsMap();
}

llvm::Type* Type::LlvmType( llvm::LLVMContext& context ) const
{
    return _type->LlvmType( context );
}

/***************************************************************
* Internals
***************************************************************/

Internal::Internal( int raw_type, Type return_type, const Type::TypeList& type_args )
: _raw_type( raw_type )
, _return_type( return_type )
, _type_args( type_args )
{
}

Internal::Internal( int raw_type, const Type::TypeMap& type_map )
: _raw_type( raw_type )
, _return_type( Type::Void() )
, _type_map( type_map )
{
}

Internal::Internal( const Internal& type )
: _raw_type( type._raw_type )
, _return_type( type._return_type )
, _type_args( type._type_args )
, _type_map( type._type_map )
{
}

bool Internal::operator==( const Internal& type ) const
{
    if ( _raw_type != type._raw_type || _return_type != type._return_type ||
         _type_args.size() != type._type_args.size() ||
         _type_map.size() != type._type_map.size() )
        return false;
    for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
        if ( _type_args[ i ] != type._type_args[ i ] )
            return false;
    }
    auto i = _type_map.begin();
    auto j = type._type_map.begin();
    for ( ; i != _type_map.end() && j != type._type_map.end(); ++i, ++j ) {
        if ( i->first != j->first || i->second != j->second )
            return false;
    }
    return true;
}

bool Internal::operator!=( const Internal& type ) const
{
    return !operator==( type );
}

Internal& Internal::operator=( const Internal& type )
{
    _raw_type = type._raw_type;
    _return_type = type._return_type;
    _type_args = type._type_args;
    _type_map = type._type_map;
    return *this;
}

std::size_t Type::Hash::operator()( const Internal& type ) const
{
    std::size_t t = 17;
    boost::hash_combine( t, type.RawType() );
    boost::hash_combine( t, type.ReturnType()._type );
    boost::hash_combine( t, type.ReturnType()._typename );
    for ( std::size_t i = 0; i < type.TypeArgs().size(); ++i ) {
        boost::hash_combine( t, type.TypeArgs()[ i ]._type );
        boost::hash_combine( t, type.TypeArgs()[ i ]._typename );
    }
    auto i = type.TypeArgsMap().begin();
    for ( ; i != type.TypeArgsMap().end(); ++i ) {
        boost::hash_combine( t, i->first );
        boost::hash_combine( t, i->second._type );
        boost::hash_combine( t, i->second._typename );
    }
    return t;
}

Type::Type()
: _type( Type::Void()._type )
, _typename( Type::Void()._typename )
{
}

Type::Type( const Internal& type )
: _type( &type )
, _typename( "" )
{
}

Type::Type( const std::string& name )
: _type( 0 )
, _typename( name )
{
}

Type::Type( const std::string& name, const Internal& type )
: _type( &type )
, _typename( name )
{
}

/***************************************************************
* Equivalent
***************************************************************/

bool Internal::Equivalent( const Internal& type ) const
{
    if ( _raw_type != type._raw_type )
        return false;
    if ( _raw_type == TYPE_VOID || _raw_type == TYPE_BOOL ||
         _raw_type == TYPE_INT || _raw_type == TYPE_FLOAT )
        return true;

    if ( _raw_type == TYPE_TUPLE ) {
        if ( _type_args.size() != type._type_args.size() )
            return false;
        for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
            if ( !_type_args[ i ].Equivalent( type._type_args[ i ] ) )
                return false;
        }
        return true;
    }

    if ( _raw_type == TYPE_ALGEBRAIC ) {
        if ( _type_map.size() != type._type_map.size() )
            return false;
        auto i = _type_map.begin();
        auto j = type._type_map.begin();
        for ( ; i != _type_map.end() && j != type._type_map.end(); ++i, ++j ) {
            if ( i->first != j->first || !i->second.Equivalent( j->second ) )
                return false;
        }
        return true;
    }

    if ( _raw_type == TYPE_FUNCTION && !_return_type.Equivalent( type._return_type ) )
        return false;
    if ( _type_args.size() != type._type_args.size() )
        return false;
    for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
        if ( !type._type_args[ i ].Equivalent( _type_args[ i ] ) )
            return false;
    }
    return true;
}

/***************************************************************
* Converts to
***************************************************************/

bool Internal::ConvertsTo( const Internal& type ) const
{
    if ( _raw_type == TYPE_VOID )
        return type._raw_type == TYPE_VOID;
    if ( _raw_type == TYPE_INT || _raw_type == TYPE_BOOL )
        return type._raw_type == TYPE_INT ||
               type._raw_type == TYPE_FLOAT || type._raw_type == TYPE_BOOL;
    if ( _raw_type == TYPE_FLOAT )
        return type._raw_type == TYPE_FLOAT;

    if ( _raw_type == TYPE_TUPLE ) {
        if ( type._raw_type != TYPE_TUPLE || _type_args.size() != type._type_args.size() )
            return false;
        for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
            if ( !_type_args[ i ].ConvertsTo( type._type_args[ i ] ) )
                return false;
        }
        return true;
    }

    if ( _raw_type == TYPE_ALGEBRAIC ) {
        if ( type._raw_type != TYPE_ALGEBRAIC || _type_map.size() != type._type_map.size() )
            return false;
        auto i = _type_map.begin();
        auto j = type._type_map.begin();
        for ( ; i != _type_map.end() && j != type._type_map.end(); ++i, ++j ) {
            if ( i->first != j->first || !i->second.ConvertsTo( j->second ) )
                return false;
        }
        return true;
    }

    if ( _raw_type == TYPE_FUNCTION ) {
        if ( type._raw_type != TYPE_FUNCTION || !_return_type.Equivalent( type._return_type ) )
            return false;
    }
    if ( _raw_type == TYPE_SEQUENCE && type._raw_type != TYPE_SEQUENCE )
        return false;
    if ( _type_args.size() != type._type_args.size() )
        return false;
    for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
        if ( !type._type_args[ i ].Equivalent( _type_args[ i ] ) )
            return false;
    }
    return true;
}

/***************************************************************
* Generalise
***************************************************************/

Type Internal::Generalise( const Internal& type ) const
{
    if ( _raw_type == TYPE_VOID || type._raw_type == TYPE_VOID )
        return Type::Void();
    if ( _raw_type == TYPE_BOOL )
        return type._raw_type == TYPE_BOOL ? Type::Bool() :
               type._raw_type == TYPE_INT ? Type::Int() :
               type._raw_type == TYPE_FLOAT ? Type::Float() : Type::Void();
    if ( _raw_type == TYPE_INT )
        return type._raw_type == TYPE_INT ? Type::Int() :
               type._raw_type == TYPE_FLOAT ? Type::Float() : Type::Void();
    if ( _raw_type == TYPE_FLOAT )
        return type._raw_type == TYPE_BOOL || type._raw_type == TYPE_INT ||
               type._raw_type == TYPE_FLOAT ? Type::Float() : Type::Void();

    if ( _raw_type == TYPE_TUPLE ) {
        if ( _type_args.size() != type._type_args.size() )
            return Type::Void();
        Type::TypeList type_args;
        for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
            Type t = _type_args[ i ].Generalise( type._type_args[ i ] );
            if ( t == Type::Void() )
                return Type::Void();
            type_args.push_back( t );
        }
        return Type::Tuple( type_args );
    }

    if ( _raw_type == TYPE_ALGEBRAIC ) {
        if ( _type_map.size() != type._type_map.size() )
            return Type::Void();
        Type::TypeMap type_map;
        auto i = _type_map.begin();
        auto j = type._type_map.begin();
        for ( ; i != _type_map.end() && j != type._type_map.end(); ++i, ++j ) {
            if ( i->first != j->first )
                return Type::Void();
            Type t = i->second.Generalise( j->second );
            if ( t == Type::Void() &&
                 !( i->second == Type::Void() && j->second == Type::Void() ) )
                return Type::Void();
            type_map[ i->first ] = t;
        }
        return Type::Algebraic( type_map );
    }

    if ( _raw_type != type._raw_type )
        return Type::Void();
    Type::TypeList arg_types;
    for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
        if ( _type_args[ i ] != type._type_args[ i ] )
            return Type::Void();
        arg_types.push_back( _type_args[ i ] );
    }
    if ( _raw_type == TYPE_SEQUENCE )
        return Type::Sequence( arg_types );
    if ( _return_type != type._return_type )
        return Type::Void();
    return Type::Function( _return_type, arg_types );
}

/***************************************************************
* Accessors
***************************************************************/

std::string Internal::Typename() const
{
    if ( _raw_type == TYPE_VOID )
        return "void";
    if ( _raw_type == TYPE_BOOL )
        return "bool";
    if ( _raw_type == TYPE_INT )
        return "int";
    if ( _raw_type == TYPE_FLOAT )
        return "float";
    if ( _raw_type == TYPE_ALGEBRAIC ) {
        bool first = true;
        std::string s = "<";
        for ( auto i = _type_map.begin(); i != _type_map.end(); ++i ) {
            if ( !first )
                s += " | ";
            s += i->first;
            if ( i->second != Type::Void() )
                s += " " + i->second.Typename();
            first = false;
        }
        return s + ">";
    }

    std::string s = _raw_type == TYPE_FUNCTION ? _return_type.Typename() + " function" :
                    _raw_type == TYPE_SEQUENCE ? "sequence" : "";

    bool first = true;
    s += "(";
    for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
        if ( !first )
            s += ", ";
        s += _type_args[ i ].Typename();
        first = false;
    }
    return s + ")";
}

int Internal::RawType() const
{
    return _raw_type;
}

const Type& Internal::ReturnType() const
{
    return _return_type;
}

const Type::TypeList& Internal::TypeArgs() const
{
    return _type_args;
}

const Type::TypeMap& Internal::TypeArgsMap() const
{
    return _type_map;
}

llvm::Type* Internal::LlvmType( llvm::LLVMContext& context ) const
{
    if ( _raw_type == TYPE_VOID )
        return llvm::Type::getVoidTy( context );
    if ( _raw_type == TYPE_BOOL )
        return llvm::Type::getInt1Ty( context );
    if ( _raw_type == TYPE_INT )
        return llvm::Type::getInt32Ty( context );
    if ( _raw_type == TYPE_FLOAT )
        return llvm::Type::getDoubleTy( context );
    
    std::vector< llvm::Type* > args;
    if ( _raw_type == TYPE_ALGEBRAIC ) {
        args.push_back( llvm::Type::getInt32Ty( context ) );
        auto i = _type_map.begin();
        for ( ; i != _type_map.end(); ++i ) {
            if ( i->second != Type::Void() )
                args.push_back( i->second.LlvmType( context ) );
        }
        return llvm::StructType::get( context, args, true );
    }

    for ( std::size_t i = 0; i < _type_args.size(); ++i )
        args.push_back( _type_args[ i ].LlvmType( context ) );
    if ( _raw_type == TYPE_TUPLE )
        return llvm::StructType::get( context, args, true );
    if ( _raw_type == TYPE_FUNCTION ) {
        auto t = llvm::FunctionType::get( _return_type.LlvmType( context ), args, false );
        return llvm::PointerType::get( t, 0 );
    }
    if ( _raw_type == TYPE_SEQUENCE ) {
        auto t = llvm::FunctionType::get( llvm::Type::getVoidTy( context ), args, false );
        return llvm::PointerType::get( t, 0 );
    }
    return llvm::Type::getVoidTy( context );
}