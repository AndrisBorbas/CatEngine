#ifndef CATENGINE_CATOBJECT_HPP
#define CATENGINE_CATOBJECT_HPP

#include "CatModel.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace cat
{

struct TransformComponent
{
	glm::vec3 translation{};
	glm::vec3 scale{ 1.f, 1.f, 1.f };
	glm::vec3 rotation{};

	// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
	// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
	// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	glm::mat4 mat4();

	glm::mat3 normalMatrix();
};
class CatObject
{
public:
	using id_t = uint32_t;

	static CatObject createObject()
	{
		static id_t currentId = 1;
		return CatObject{ currentId++ };
	}

	CatObject( const CatObject& ) = delete;
	CatObject& operator=( const CatObject& ) = delete;
	CatObject( CatObject&& ) = default;
	CatObject& operator=( CatObject&& ) = default;

	id_t getId() { return m_id; }

	std::shared_ptr< CatModel > m_pModel{};
	glm::vec3 m_vColor{};
	TransformComponent m_transform{};

private:
	CatObject( id_t objId ) : m_id{ objId } {}

	id_t m_id;
};
} // namespace cat


#endif // CATENGINE_CATOBJECT_HPP
