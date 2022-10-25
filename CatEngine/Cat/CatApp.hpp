#ifndef CATENGINE_CATAPP_HPP
#define CATENGINE_CATAPP_HPP

#include <future>

#include "Cat/Rendering/CatDescriptors.hpp"
#include "Cat/Rendering/CatDevice.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/Rendering/CatRenderer.hpp"
#include "CatWindow.hpp"
#include "CatFrameInfo.hpp"
#include "Cat/Level/CatLevel.hpp"
#include "Cat/Objects/CatAssetLoader.hpp"

#include <memory>
#include <vector>

#include <concurrentqueue.h>
#include <BS_thread_pool.hpp>


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

	[[nodiscard]] auto getRenderer() { return &m_renderer; }

private:
	void loadDefaultExampleMap();

	// note: order of declarations matters
	CatWindow m_window{ WIDTH, HEIGHT, "Cat Engine", false };
	CatDevice m_device{ m_window };
	CatRenderer m_renderer{ m_window, m_device };

	std::unique_ptr< CatDescriptorPool > m_pGlobalDescriptorPool{};
	CatObject::Map m_mObjects;

	std::unique_ptr< CatFrameInfo > m_pFrameInfo = nullptr;

	double m_dFrameTime = 0.0;
	double m_dDeltaTime = 0.0;
	double m_dFrameRate = 0.0;

	BS::thread_pool m_tAssetLoader{ 4 };
	BS::thread_pool m_tObjectLoader{ 4 };
	BS::thread_pool m_tLevelLoader{ 1 };
	moodycamel::ConcurrentQueue< const char* > m_qLoadAssets{ 1 << 16 };
	CatAssetLoader m_assetLoader{};

public:
	std::unique_ptr< CatLevel > m_pCurrentLevel;

public:
	CAT_READONLY_PROPERTY( m_device, getDevice, device, m_Device );
	CAT_READONLY_PROPERTY( m_dFrameRate, getFrameRate, dFrameRate, m_DFrameRate );
	CAT_READONLY_PROPERTY( m_dFrameTime, getFrameTime, dFrameTime, m_DFrameTime );
	CAT_READONLY_PROPERTY( m_dDeltaTime, getDeltaTime, dDeltaTime, m_DDeltaTime );
	CAT_READONLY_PROPERTY( m_window, getWindow, window, m_Window );
	CAT_READONLY_PROPERTY( m_qLoadAssets, getLoadAssetsQueue, qLoadAssets, m_QLoadAssets );
	CAT_READONLY_PROPERTY( m_tAssetLoader, getAssetLoaderThreadPool, tAssetLoader, m_TAssetLoader );
	CAT_READONLY_PROPERTY( m_tObjectLoader, getObjectLoaderThreadPool, tObjectLoader, m_TObjectLoader );
	CAT_READONLY_PROPERTY( m_tLevelLoader, getLevelLoaderThreadPool, tLevelLoader, m_TLevelLoader );
	CAT_READONLY_PROPERTY( m_assetLoader, getAssetLoader, assetLoader, m_AssetLoader );
};

[[nodiscard]] extern CatApp* GetEditorInstance();
extern void CreateEditorInstance();
extern void DestroyGameInstance();

} // namespace cat

#endif // CATENGINE_CATAPP_HPP
