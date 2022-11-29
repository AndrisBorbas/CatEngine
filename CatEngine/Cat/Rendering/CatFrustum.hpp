#ifndef CATENGINE_CATFRUSTUM_HPP
#define CATENGINE_CATFRUSTUM_HPP

#include "Globals.hpp"

#include <glm/glm.hpp>

namespace cat
{

class CatFrustum
{
protected:
	std::array< glm::vec4, 6 > m_aPlanes;

public:
	void update( const glm::mat4 mxPV )
	{
		m_aPlanes[0].x = mxPV[0].w + mxPV[0].x;
		m_aPlanes[0].y = mxPV[1].w + mxPV[1].x;
		m_aPlanes[0].z = mxPV[2].w + mxPV[2].x;
		m_aPlanes[0].w = mxPV[3].w + mxPV[3].x;

		m_aPlanes[1].x = mxPV[0].w - mxPV[0].x;
		m_aPlanes[1].y = mxPV[1].w - mxPV[1].x;
		m_aPlanes[1].z = mxPV[2].w - mxPV[2].x;
		m_aPlanes[1].w = mxPV[3].w - mxPV[3].x;

		m_aPlanes[2].x = mxPV[0].w - mxPV[0].y;
		m_aPlanes[2].y = mxPV[1].w - mxPV[1].y;
		m_aPlanes[2].z = mxPV[2].w - mxPV[2].y;
		m_aPlanes[2].w = mxPV[3].w - mxPV[3].y;

		m_aPlanes[3].x = mxPV[0].w + mxPV[0].y;
		m_aPlanes[3].y = mxPV[1].w + mxPV[1].y;
		m_aPlanes[3].z = mxPV[2].w + mxPV[2].y;
		m_aPlanes[3].w = mxPV[3].w + mxPV[3].y;

		m_aPlanes[4].x = mxPV[0].w + mxPV[0].z;
		m_aPlanes[4].y = mxPV[1].w + mxPV[1].z;
		m_aPlanes[4].z = mxPV[2].w + mxPV[2].z;
		m_aPlanes[4].w = mxPV[3].w + mxPV[3].z;

		m_aPlanes[5].x = mxPV[0].w - mxPV[0].z;
		m_aPlanes[5].y = mxPV[1].w - mxPV[1].z;
		m_aPlanes[5].z = mxPV[2].w - mxPV[2].z;
		m_aPlanes[5].w = mxPV[3].w - mxPV[3].z;

		for ( auto& vPlane : m_aPlanes )
		{
			vPlane /= glm::length( glm::vec3( vPlane ) );
		}
	}

	bool checkSphere( glm::vec3 vPosition, float fRadius )
	{
		for ( auto& vPlane : m_aPlanes )
		{
			if ( glm::dot( glm::vec3( vPlane ), vPosition ) + vPlane.w + fRadius < 0.0f ) return false;
		}
		return true;
	}


	CAT_READONLY_PROPERTY( m_aPlanes, getPlanes, m_APlanes );
};

} // namespace cat

#endif // CATENGINE_CATFRUSTUM_HPP
