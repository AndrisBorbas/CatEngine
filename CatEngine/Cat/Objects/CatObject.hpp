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
	glm::vec3 rotation{};
	glm::vec3 scale{ 1.f, 1.f, 1.f };

	// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
	// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
	// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	[[nodiscard]] glm::mat4 mat4() const;

	[[nodiscard]] glm::mat3 normalMatrix() const;
};

class CatObject
{
public:
	using id_t = uint32_t;
	using Map = std::unordered_map< id_t, std::unique_ptr< CatObject > >;

	void updateTransform( const glm::vec3& vTranslation, const glm::vec3& vRotation, const glm::vec3& vScale );
	void updateTransform( const glm::vec3& vTranslation, const glm::vec3& vRotation );
	void updateTransform( const glm::vec3& vTranslation );

	[[nodiscard]] static std::unique_ptr< CatObject > create( const std::string& sName, const std::string& sFile )
	{
		return std::make_unique< CatObject >( sName, sFile );
	}

	[[nodiscard]] CatObject( std::string sName, std::string sFile )
		: m_id( currentId++ ), m_sName( std::move( sName ) ), m_sFile( std::move( sFile ) )
	{
	}

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

	virtual ~CatObject() = default;

protected:
	const id_t m_id;
	std::string m_sName;
	std::string m_sFile;

private:
	static id_t currentId;
};

} // namespace cat

#endif // CATENGINE_CATOBJECT_HPP
