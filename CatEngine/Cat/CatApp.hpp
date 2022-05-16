#ifndef CATENGINE_CATAPP_HPP
#define CATENGINE_CATAPP_HPP

#include <future>

#include "Cat/Rendering/CatDescriptors.hpp"
#include "Cat/Rendering/CatDevice.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/Rendering/CatRenderer.hpp"
#include "CatWindow.hpp"
#include "CatFrameInfo.hpp"

#include <memory>
#include <vector>


namespace cat
{

class CatApp
{
public:
	static constexpr int WIDTH = 1200;
	static constexpr int HEIGHT = 800;

	CatApp();
	~CatApp();

	CatApp( const CatApp& ) = delete;
	CatApp& operator=( const CatApp& ) = delete;

	void run();

	[[nodiscard]] auto const& getObjects() const { return m_mObjects; }
	[[nodiscard]] CatFrameInfo& getFrameInfo() const { return *m_pFrameInfo; }

	void saveLevel( const std::string& sFileName ) const;
	void loadLevel( const std::string& sFileName, bool bClearPrevious = true );

	std::future< void > m_jLevelLoad{};
	std::vector< std::future< std::pair< json, std::shared_ptr< CatModel > > > > m_aLoadingObjects{};

	[[nodiscard]] auto getDevice() { return &m_device; }
	[[nodiscard]] auto getRenderer() { return &m_renderer; }
	[[nodiscard]] auto getFrameTime() const { return m_dFrameTime; }
	[[nodiscard]] auto getFrameRate() const { return m_dFrameRate; }

private:
	void loadGameObjects();

	CatWindow m_window{ WIDTH, HEIGHT, "Cat Engine" };
	CatDevice m_device{ m_window };
	CatRenderer m_renderer{ m_window, m_device };

	// note: order of declarations matters
	std::unique_ptr< CatDescriptorPool > m_pGlobalPool{};
	CatObject::Map m_mObjects;

	std::unique_ptr< CatFrameInfo > m_pFrameInfo = nullptr;

	double m_dFrameTime = 0.0;
	double m_dDeltaTime = 0.0;
	double m_dFrameRate = 0.0;
};

[[nodiscard]] extern CatApp* GetEditorInstance();
extern void CreateEditorInstance();
extern void DestroyGameInstance();

} // namespace cat

#endif // CATENGINE_CATAPP_HPP
