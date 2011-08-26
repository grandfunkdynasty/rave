#include "type.h"

Type::InternalSet Type::_type_set;

/***************************************************************
* Primitive types
***************************************************************/

Type Type::Void()
{
    static Internal result( TYPE_VOID, 0, Internal::TypeArgs() );
    return result;
}

Type Type::Int()
{
    static Internal result( TYPE_INT, 0, Internal::TypeArgs() );
    return result;
}

Type Type::Float()
{
    static Internal result( TYPE_FLOAT, 0, Internal::TypeArgs() );
    return result;
}

/***************************************************************
* Compound types
***************************************************************/

Type Type::Tuple( const TypeList& type_args )
{
    Internal::TypeArgs internal_args;
    for ( std::size_t i = 0; i < type_args.size(); ++i )
        internal_args.push_back( type_args[ i ]._type );

    Internal result( TYPE_TUPLE, 0, internal_args );
    return *_type_set.insert( result ).first;
}

Type Type::Function( const Type& return_type, const TypeList& arg_types )
{
    Internal::TypeArgs internal_args;
    for ( std::size_t i = 0; i < arg_types.size(); ++i )
        internal_args.push_back( arg_types[ i ]._type );

    Internal result( TYPE_FUNCTION, return_type._type, internal_args );
    return *_type_set.insert( result ).first;
}

Type Type::Sequence( const TypeList& arg_types )
{
    Internal::TypeArgs internal_args;
    for ( std::size_t i = 0; i < arg_types.size(); ++i )
        internal_args.push_back( arg_types[ i ]._type );

    Internal result( TYPE_SEQUENCE, 0, internal_args );
    return *_type_set.insert( result ).first;
}

/***************************************************************
* Type interface
***************************************************************/

Type::~Type()
{
}

Type::Type( const Type& type )
: _type( type._type )
{
}

const Type& Type::operator=( const Type& type )
{
    _type = type._type;
    return *this;
}

bool Type::operator==( const Type& type ) const
{
    return _type == type._type;
}

bool Type::operator!=( const Type& type ) const
{
    return _type != type._type;
}

bool Type::ConvertsTo( const Type& type ) const
{
    return _type->ConvertsTo( *type._type );
}

Type Type::Generalise( const Type& type ) const
{
    return _type->Generalise( *type._type );
}

std::string Type::TypeName() const
{
    return _type->TypeName();
}

bool Type::IsTuple() const
{
    return _type->RawType() == TYPE_TUPLE;
}

bool Type::IsFunction() const
{
    return _type->RawType() == TYPE_FUNCTION;
}

bool Type::IsSequence() const
{
    return _type->RawType() == TYPE_SEQUENCE;
}

Type Type::ReturnType() const
{
    if ( !_type->ReturnType() )
        return Type::Void();
    return *_type->ReturnType();
}

std::size_t Type::ArgTypeSize() const
{
    return _type->ArgTypes().size();
}

Type Type::ArgType( std::size_t index ) const
{
    if ( index >= _type->ArgTypes().size() )
        return Type::Void();
    return *_type->ArgTypes()[ index ];
}

/***************************************************************
* Internals
***************************************************************/

Type::Internal::Internal( int raw_type, const Internal* return_type, const TypeArgs& type_args )
: _raw_type( raw_type )
, _return_type( return_type )
, _type_args( type_args )
{
}

Type::Internal::Internal( const Internal& type )
: _raw_type( type._raw_type )
, _return_type( type._return_type )
, _type_args( type._type_args )
{
}

bool Type::Internal::operator==( const Internal& type ) const
{
    if ( _raw_type != type._raw_type || _return_type != type._return_type ||
         _type_args.size() != type._type_args.size() )
        return false;
    for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
        if ( _type_args[ i ] != type._type_args[ i ] )
            return false;
    }
    return true;
}

bool Type::Internal::operator!=( const Internal& type ) const
{
    return !operator==( type );
}

const Type::Internal& Type::Internal::operator=( const Internal& type )
{
    _raw_type = type._raw_type;
    _return_type = type._return_type;
    _type_args = type._type_args;
    return *this;
}

std::size_t Type::Internal::Hash::operator()( const Internal& type ) const
{
    std::size_t t = 17;
    boost::hash_combine( t, type._raw_type );
    boost::hash_combine( t, type._return_type );
    for ( std::size_t i = 0; i < type._type_args.size(); ++i )
        boost::hash_combine( t, type._type_args[ i ] );
    return t;
}

Type::Type( const Internal& type )
: _type( &type )
{
}

/***************************************************************
* Converts to
***************************************************************/

bool Type::Internal::ConvertsTo( const Internal& type ) const
{
    if ( _raw_type == TYPE_VOID || type._raw_type == TYPE_VOID )
        return false;

    if ( _raw_type == TYPE_INT )
        return type._raw_type == TYPE_INT || type._raw_type == TYPE_FLOAT;
    if ( _raw_type == TYPE_FLOAT )
        return type._raw_type == TYPE_FLOAT;

    if ( _raw_type == TYPE_TUPLE ) {
        if ( type._raw_type != TYPE_TUPLE || _type_args.size() != type._type_args.size() )
            return false;
        for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
            if ( !_type_args[ i ]->ConvertsTo( *type._type_args[ i ] ) )
                return false;
        }
        return true;
    }

    if ( _raw_type == TYPE_FUNCTION ) {
        if ( type._raw_type != TYPE_FUNCTION || *_return_type != *type._return_type )
            return false;
    }
    if ( _raw_type == TYPE_SEQUENCE && type._raw_type != TYPE_SEQUENCE )
        return false;
    if ( _type_args.size() != type._type_args.size() )
        return false;
    for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
        if ( *type._type_args[ i ] != *_type_args[ i ] )
            return false;
    }
    return true;
}

/***************************************************************
* Generalise
***************************************************************/

Type Type::Internal::Generalise( const Internal& type ) const
{
    if ( _raw_type == TYPE_VOID || type._raw_type == TYPE_VOID )
        return Void();
    if ( _raw_type == TYPE_INT )
        return type._raw_type == TYPE_INT ? Int() :
               type._raw_type == TYPE_FLOAT ? Float() : Void();
    if ( _raw_type == TYPE_FLOAT )
        return type._raw_type == TYPE_INT || type._raw_type == TYPE_FLOAT ?
               Float() : Void();

    if ( _raw_type == TYPE_TUPLE ) {
        if ( _type_args.size() != type._type_args.size() )
            return Void();
        TypeList type_args;
        for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
            Type t = _type_args[ i ]->Generalise( *type._type_args[ i ] );
            if ( t == Void() )
                return Void();
            type_args.push_back( t );
        }
        return Tuple( type_args );
    }

    if ( _raw_type != type._raw_type )
        return Void();
    TypeList arg_types;
    for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
        if ( *_type_args[ i ] != *type._type_args[ i ] )
            return Void();
        arg_types.push_back( *_type_args[ i ] );
    }
    if ( _raw_type == TYPE_SEQUENCE )
        return Sequence( arg_types );
    if ( _return_type != type._return_type )
        return Void();
    return Function( *_return_type, arg_types );
}

/***************************************************************
* Accessors
***************************************************************/

std::string Type::Internal::TypeName() const
{
    if ( _raw_type == TYPE_VOID )
        return "void";
    if ( _raw_type == TYPE_INT )
        return "int";
    if ( _raw_type == TYPE_FLOAT )
        return "float";

    std::string s = _raw_type == TYPE_FUNCTION ? _return_type->TypeName() + " function" :
                    _raw_type == TYPE_SEQUENCE ? "sequence" : "";

    bool first = true;
    s += "(";
    for ( std::size_t i = 0; i < _type_args.size(); ++i ) {
        if ( !first )
            s += ", ";
        s += _type_args[ i ]->TypeName();
        first = false;
    }
    return s + ")";
}

int Type::Internal::RawType() const
{
    return _raw_type;
}

const Type::Internal* Type::Internal::ReturnType() const
{
    return _return_type;
}

const Type::Internal::TypeArgs& Type::Internal::ArgTypes() const
{
    return _type_args;
}