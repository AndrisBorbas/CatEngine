#ifndef CATENGINE_CATAPP_HPP
#define CATENGINE_CATAPP_HPP

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

private:
	void loadGameObjects();

	CatWindow m_window{ WIDTH, HEIGHT, "Cat Engine" };
	CatDevice m_device{ m_window };
	CatRenderer m_renderer{ m_window, m_device };

	// note: order of declarations matters
	std::unique_ptr< CatDescriptorPool > m_pGlobalPool{};
	CatObject::Map m_mObjects;

	std::unique_ptr< CatFrameInfo > m_pFrameInfo = nullptr;
};
} // namespace cat


#endif // CATENGINE_CATAPP_HPP
