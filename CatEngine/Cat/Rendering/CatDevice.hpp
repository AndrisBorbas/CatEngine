#ifndef CATENGINE_CATDEVICE_HPP
#define CATENGINE_CATDEVICE_HPP

#include "Cat/CatWindow.hpp"

#include <string>
#include <vector>
#include <optional>


namespace cat
{

struct QueueFamilyIndices
{
	std::optional< uint32_t > m_nGraphicsFamily;
	std::optional< uint32_t > m_nPresentFamily;

	bool isComplete() { return m_nGraphicsFamily.has_value() && m_nPresentFamily.has_value(); }
};

struct SwapChainSupportDetails
{
	vk::SurfaceCapabilitiesKHR m_capabilities;
	std::vector< vk::SurfaceFormatKHR > m_aFormats;
	std::vector< vk::PresentModeKHR > m_aPresentModes;
};

class CatDevice
{
public:
#ifdef NDEBUG
	const bool m_bEnableValidationLayers = false;
#else
	const bool m_bEnableValidationLayers = true;
#endif

	CatDevice( CatWindow& rWindow );
	~CatDevice();

	// Not copyable or movable
	CatDevice( const CatDevice& ) = delete;
	CatDevice& operator=( const CatDevice& ) = delete;
	CatDevice( CatDevice&& ) = delete;
	CatDevice& operator=( CatDevice&& ) = delete;

	vk::CommandPool getCommandPool() const { return m_pCommandPool; }
	vk::Device getDevice() const { return m_device; }
	vk::SurfaceKHR getSurface() const { return m_surface; }
	vk::Queue getGraphicsQueue() const { return m_graphicsQueue; }
	vk::Queue getPresentQueue() const { return m_presentQueue; }
	vk::Instance getInstance() const { return m_instance; }
	vk::PhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
	uint32_t getGraphicsQueueFamily() { return findPhysicalQueueFamilies().m_nGraphicsFamily.value(); }

	SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport( m_physicalDevice ); }
	uint32_t findMemoryType( uint32_t typeFilter, vk::MemoryPropertyFlags rProperties );
	QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies( m_physicalDevice ); }
	vk::Format findSupportedFormat( const std::vector< vk::Format >& candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features );

	// Buffer Helper Functions
	void createBuffer( vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties,
		vk::Buffer& buffer,
		vk::DeviceMemory& bufferMemory );
	vk::CommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands( vk::CommandBuffer commandBuffer ) const;
	void copyBuffer( vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size );
	void copyBufferToImage( vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount );

	void createImageWithInfo( const vk::ImageCreateInfo& imageInfo,
		vk::MemoryPropertyFlags properties,
		vk::Image& image,
		vk::DeviceMemory& imageMemory );

	vk::PhysicalDeviceProperties m_properties;

private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createCommandPool();

	// helper functions
	static vk::SampleCountFlagBits getMaxUsableSampleCount( const vk::PhysicalDevice rPhysicalDevice );
	int rateDeviceSuitability( const vk::PhysicalDevice rPhysicalDevice );
	bool isDeviceSuitable( vk::PhysicalDevice rPhysicalDevice );
	std::vector< const char* > getRequiredExtensions();
	bool checkValidationLayerSupport();
	QueueFamilyIndices findQueueFamilies( const vk::PhysicalDevice rPhysicalDevice );
	void populateDebugMessengerCreateInfo( vk::DebugUtilsMessengerCreateInfoEXT& createInfo );
	void hasGflwRequiredInstanceExtensions();
	bool checkDeviceExtensionSupport( const vk::PhysicalDevice rPhysicalDevice );
	SwapChainSupportDetails querySwapChainSupport( const vk::PhysicalDevice rPhysicalDevice );

	vk::Instance m_instance;
	vk::DebugUtilsMessengerEXT m_debugMessenger;
	vk::PhysicalDevice m_physicalDevice = nullptr;
	CatWindow& m_rWindow;
	vk::CommandPool m_pCommandPool;

	vk::Device m_device;
	vk::SurfaceKHR m_surface;
	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;
	vk::SampleCountFlagBits m_msaaSamples;

	const std::vector< const char* > m_aValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector< const char* > m_aDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};
} // namespace cat


#endif // CATENGINE_CATDEVICE_HPP
