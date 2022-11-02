#ifndef CATENGINE_CATOBJECT_HPP
#define CATENGINE_CATOBJECT_HPP

#include "CatModel.hpp"
#include "CatObjectType.hpp"

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
	using Map = std::unordered_map< id_t, std::shared_ptr< CatObject > >;


	[[nodiscard]] static std::unique_ptr< CatObject > create( const std::string& sName,
		const std::string& sFile,
		const ObjectType& eType = ObjectType::eGameObject )
	{
		return std::unique_ptr< CatObject >( new CatObject( sName, sFile, eType ) );
	}


	CatObject( const CatObject& ) = delete;
	CatObject& operator=( const CatObject& ) = delete;
	CatObject( CatObject&& ) = default;
	CatObject& operator=( CatObject&& ) = default;

	// TODO: Add load() which loads from json and creates correct object automatically.

	virtual json save();
	virtual void load( const json& object );

	void updateTransform( const glm::vec3& vTranslation, const glm::vec3& vRotation, const glm::vec3& vScale );
	void updateTransform( const glm::vec3& vTranslation, const glm::vec3& vRotation );
	void updateTransform( const glm::vec3& vTranslation );

	[[nodiscard]] id_t getId() const { return m_id; }
	[[nodiscard]] auto& getName() const { return m_sName; }
	[[nodiscard]] auto& getFileName() const { return m_sFile; }
	[[nodiscard]] virtual const ObjectType& getType() const { return m_eType; }

	glm::vec3 m_vColor{};
	TransformComponent m_transform{};

	std::shared_ptr< CatModel > m_pModel{};

	virtual ~CatObject() = default;

protected:
	[[nodiscard]] explicit CatObject( std::string sName, std::string sFile, const ObjectType& eType = ObjectType::eGameObject )
		: m_id( M_ID_CURRENT++ ), m_sName( std::move( sName ) ), m_sFile( std::move( sFile ) ), m_eType( eType )
	{
	}

	const id_t m_id;
	std::string m_sName;
	std::string m_sFile;
	ObjectType m_eType;
	bool m_bVisible;

private:
	static id_t M_ID_CURRENT;

public:
	CAT_PROPERTY( m_bVisible, getVisible, setVisible, m_BVisible );
};

} // namespace cat

#endif // CATENGINE_CATOBJECT_HPP
