#ifndef CATENGINE_CATOBJECT_HPP
#define CATENGINE_CATOBJECT_HPP

#include "CatModel.hpp"

#include "glm/gtc/matrix_transform.hpp"

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
	[[nodiscard]] glm::mat4 mat4() const;

	[[nodiscard]] glm::mat3 normalMatrix() const;
};

struct PointLightComponent
{
	float lightIntensity = 1.0f;
};

class CatObject
{
public:
	using id_t = uint32_t;
	using Map = std::unordered_map< id_t, CatObject >;

	[[nodiscard]] static CatObject createObject( const std::string& sName, const std::string& sFile )
	{
		static id_t currentId = 1;
		return CatObject{ currentId++, sName, sFile };
	}

	[[nodiscard]] static CatObject makePointLight( const std::string& sName = "Light",
		const float fIntensity = 10.f,
		const float fRadius = .1f,
		const glm::vec3 vColor = glm::vec3( 1.f ) );

	CatObject( const CatObject& ) = delete;
	CatObject& operator=( const CatObject& ) = delete;
	CatObject( CatObject&& ) = default;
	CatObject& operator=( CatObject&& ) = default;

	[[nodiscard]] id_t getId() const { return m_id; }
	[[nodiscard]] std::string getName() const { return m_sName; }
	[[nodiscard]] std::string getFileName() const { return m_sFile; }

	glm::vec3 m_vColor{};
	TransformComponent m_transform{};

	std::shared_ptr< CatModel > m_pModel{};
	std::unique_ptr< PointLightComponent > m_pPointLight = nullptr;

private:
	CatObject( const id_t objId, std::string sName, std::string sFile )
		: m_id{ objId }, m_sName( std::move( sName ) ), m_sFile( std::move( sFile ) )
	{
	}

	const id_t m_id;
	std::string m_sName;
	std::string m_sFile;
};
} // namespace cat


#endif // CATENGINE_CATOBJECT_HPP
