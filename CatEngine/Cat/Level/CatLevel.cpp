#include "CatLevel.hpp"

namespace cat
{
void CatLevel::save() {}

std::unique_ptr< CatLevel > CatLevel::load( const std::string& sName )
{
	return std::unique_ptr< CatLevel >();
}
} // namespace cat
