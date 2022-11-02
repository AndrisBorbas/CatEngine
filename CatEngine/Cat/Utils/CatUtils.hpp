#ifndef CATENGINE_CATUTILS_HPP
#define CATENGINE_CATUTILS_HPP

#include <functional>

namespace cat
{
// from: https://stackoverflow.com/a/57595105
template < typename T, typename... Rest >
void HashCombine( std::size_t& seed, const T& v, const Rest&... rest ) {
	seed ^= std::hash< T >{}( v ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
	( HashCombine( seed, rest ), ... );
};

// Define a property
#define CAT_PROPERTY( _member, _getter, _setter, _propertyName )                            \
	[[nodiscard]] virtual const decltype( _member )& _getter() const { return _member; }    \
	[[nodiscard]] virtual decltype( _member )& _getter() { return _member; }                \
	virtual void _setter( const decltype( _member )& _paramName ) { _member = _paramName; } \
	__declspec( property( get = _getter, put = _setter ) ) decltype( _member ) _propertyName;

// Define a read-only property.
#define CAT_READONLY_PROPERTY( _member, _getter, _propertyName )                         \
	[[nodiscard]] virtual const decltype( _member )& _getter() const { return _member; } \
	[[nodiscard]] virtual decltype( _member )& _getter() { return _member; }             \
	__declspec( property( get = _getter ) ) decltype( _member ) _propertyName;

// Define a const-only read-only property.
#define CAT_CONST_READONLY_PROPERTY( _member, _getter, _propertyName )                   \
	[[nodiscard]] virtual const decltype( _member )& _getter() const { return _member; } \
	__declspec( property( get = _getter ) ) decltype( _member ) _propertyName;

// Define a read-only pointer reference property.
#define CAT_READONLY_POINTER_REF_PROPERTY( _member, _getter, _propertyName )   \
	[[nodiscard]] virtual decltype( *_member )& _getter() { return *_member; } \
	__declspec( property( get = _getter ) ) decltype( _member ) _propertyName;

} // namespace cat

#endif // CATENGINE_CATUTILS_HPP
