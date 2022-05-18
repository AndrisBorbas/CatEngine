#ifndef CATENGINE_CATUTILS_HPP
#define CATENGINE_CATUTILS_HPP

#include <functional>

namespace cat
{
// from: https://stackoverflow.com/a/57595105
template < typename T, typename... Rest >
void hashCombine( std::size_t& seed, const T& v, const Rest&... rest )
{
	seed ^= std::hash< T >{}( v ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
	( hashCombine( seed, rest ), ... );
};


#define CAT_PROPERTY( _member, _getter, _setter, _paramName, _propertyName )                \
	[[nodiscard]] virtual const decltype( _member )& _getter() const { return _member; }    \
	virtual void _setter( const decltype( _member )& _paramName ) { _member = _paramName; } \
	__declspec( property( get = _getter, put = _setter ) ) decltype( _member ) _propertyName;

} // namespace cat

#endif // CATENGINE_CATUTILS_HPP
