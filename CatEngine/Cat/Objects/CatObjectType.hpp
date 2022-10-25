#ifndef CATENGINE_CATOBJECTTYPE_HPP
#define CATENGINE_CATOBJECTTYPE_HPP

#include "Globals.hpp"

namespace cat
{

enum class ObjectType
{
	eNotSaved = ( 1u << 0 ),
	eDynamic = ( 1u << 1 ),
	eEditorObject = ( 1u << 2 ) | eNotSaved,
	eGameObject = ( 1u << 3 ),
	eCamera = ( 1u << 4 ) | eEditorObject,
	eLight = ( 1u << 5 ),
	eVolume = ( 1u << 6 ),
	eGrid = ( 1u << 7 ) | eEditorObject,
};

inline ObjectType operator|( ObjectType& lhs, ObjectType& rhs )
{
	return static_cast< ObjectType >( static_cast< uint32_t >( lhs ) | static_cast< uint32_t >( rhs ) );
}

inline ObjectType operator&( ObjectType& lhs, ObjectType& rhs )
{
	return static_cast< ObjectType >( static_cast< uint32_t >( lhs ) & static_cast< uint32_t >( rhs ) );
}

// Contains operator
// Does lhs contain rhs
static bool operator>=( ObjectType lhs, ObjectType rhs )
{
	return ( lhs & rhs ) == rhs;
}

} // namespace cat

#endif // CATENGINE_CATOBJECTTYPE_HPP
