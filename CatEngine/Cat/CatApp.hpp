#ifndef CATENGINE_CATAPP_HPP
#define CATENGINE_CATAPP_HPP

#include <future>

#include "Cat/VulkanRHI/CatDescriptors.hpp"
#include "Cat/VulkanRHI/CatDevice.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/VulkanRHI/CatRenderer.hpp"
#include "Cat/CatWindow.hpp"
#include "Cat/CatFrameInfo.hpp"
#include "Cat/Level/CatLevel.hpp"
#include "Cat/Objects/CatAssetLoader.hpp"
#include "Cat/CatImgui.hpp"
#include "Cat/Controller/CatInput.hpp"

#include <memory>
#include <vector>

#include <concurrentqueue.h>
#include <BS_thread_pool.hpp>
#include <ImGuizmo.h>


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

	void init();

	void run();

	[[nodiscard]] auto const& getObjects() const { return m_mObjects; }
	[[nodiscard]] CatFrameInfo const& getFrameInfo() const { return *m_pFrameInfo; }
	[[nodiscard]] CatFrameInfo& getFrameInfo() { return *m_pFrameInfo; }

	void saveLevel( const std::string& sFileName ) const;
	void loadLevel( const std::string& sFileName, bool bClearPrevious = true );

	std::future< void > m_jLevelLoad{};
	std::vector< std::future< std::pair< json, std::shared_ptr< CatModel > > > > m_aLoadingObjects{};

	bool m_bTerrain = false;


private:
	CatWindow* m_pWindow;
	CatDevice* m_pDevice;
	CatRenderer* m_pRenderer;

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
	float m_fCameraSpeed = 12.33f;

	std::vector< std::unique_ptr< CatBuffer > > m_aUboBuffers;
	std::unique_ptr< CatDescriptorSetLayout > m_pGlobalDescriptorSetLayout;
	std::vector< vk::DescriptorSet > m_aGlobalDescriptorSets;

	CatImgui* m_pImgui = nullptr;

	CatCamera m_camera;
	std::unique_ptr< CatObject > m_pCameraObject;
	CatInput m_cameraController;

	GlobalUbo m_ubo;

	ImGuizmo::OPERATION m_eGizmoOperation = ImGuizmo::UNIVERSAL;
	ImGuizmo::MODE m_eGizmoMode = ImGuizmo::WORLD;

	std::unique_ptr< CatLevel > m_pCurrentLevel;

	GLFWkeyfun m_fKeyCallback = nullptr;

public:
	__declspec( property( get = getFrameInfo ) ) CatFrameInfo& m_RFrameInfo;

	CAT_READONLY_PROPERTY( m_pDevice, getDevice, m_PDevice );
	CAT_READONLY_PROPERTY( m_dFrameRate, getFrameRate, m_DFrameRate );
	CAT_READONLY_PROPERTY( m_dFrameTime, getFrameTime, m_DFrameTime );
	CAT_READONLY_PROPERTY( m_dDeltaTime, getDeltaTime, m_DDeltaTime );
	CAT_READONLY_PROPERTY( m_pWindow, getWindow, m_PWindow );
	CAT_READONLY_PROPERTY( m_qLoadAssets, getLoadAssetsQueue, m_QLoadAssets );
	CAT_READONLY_PROPERTY( m_tAssetLoader, getAssetLoaderThreadPool, m_TAssetLoader );
	CAT_READONLY_PROPERTY( m_tObjectLoader, getObjectLoaderThreadPool, m_TObjectLoader );
	CAT_READONLY_PROPERTY( m_tLevelLoader, getLevelLoaderThreadPool, m_TLevelLoader );
	CAT_READONLY_PROPERTY( m_assetLoader, getAssetLoader, m_AssetLoader );
	CAT_READONLY_PROPERTY( m_fCameraSpeed, getCameraSpeed, m_FCameraSpeed );
	CAT_READONLY_PROPERTY( m_pCurrentLevel, getCurrentLevel, m_PCurrentLevel );
	CAT_PROPERTY( m_fKeyCallback, getKeyCallback, setKeyCallback, m_FKeyCallback );
};

[[nodiscard]] extern CatApp* GetEditorInstance();
[[nodiscard]] extern CatApp* GEI();

extern void CreateEditorInstance();
extern void DestroyGameInstance();

} // namespace cat

#endif // CATENGINE_CATAPP_HPP
