#ifndef CATENGINE_CATAPP_HPP
#define CATENGINE_CATAPP_HPP

#include "Cat/Rendering/CatDescriptors.hpp"
#include "Cat/Rendering/CatDevice.hpp"
#include "Cat/Objects/CatObject.hpp"
#include "Cat/Rendering/CatRenderer.hpp"
#include "CatWindow.hpp"

#include <memory>
#include <vector>

namespace cat
{

class CatApp
{
public:
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;

	CatApp();
	~CatApp();

	CatApp( const CatApp& ) = delete;
	CatApp& operator=( const CatApp& ) = delete;

	void run();

private:
	void loadGameObjects();

	CatWindow m_window{ WIDTH, HEIGHT, "Cat Engine" };
	CatDevice m_device{ m_window };
	CatRenderer m_renderer{ m_window, m_device };

	// note: order of declarations matters
	std::unique_ptr< CatDescriptorPool > m_pGlobalPool{};
	std::vector< CatObject > m_aObjects;
};
} // namespace cat


#endif // CATENGINE_CATAPP_HPP
