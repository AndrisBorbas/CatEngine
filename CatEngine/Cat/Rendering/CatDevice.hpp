#ifndef CATENGINE_CATDEVICE_HPP
#define CATENGINE_CATDEVICE_HPP

#include <mutex>

#include "Cat/CatWindow.hpp"

#include <string>
#include <vector>
#include <optional>


namespace cat
{
struct QueueFamilyIndices
{
	std::optional< uint32_t > nGraphicsFamily;
	std::optional< uint32_t > nPresentFamily;
	std::optional< uint32_t > nTransferFamily;

	bool isComplete() const { return nGraphicsFamily.has_value() && nPresentFamily.has_value() && nTransferFamily.has_value(); }
};

struct SwapChainSupportDetails
{
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector< vk::SurfaceFormatKHR > aFormats;
	std::vector< vk::PresentModeKHR > aPresentModes;
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

	[[nodiscard]] vk::CommandPool getCommandPool() const { return m_pDrawCommandPool; }
	[[nodiscard]] vk::Device getDevice() const { return m_device; }
	[[nodiscard]] vk::SurfaceKHR getSurface() const { return m_surface; }
	[[nodiscard]] vk::Queue getGraphicsQueue() const { return m_graphicsQueue; }
	[[nodiscard]] vk::Queue getPresentQueue() const { return m_presentQueue; }
	[[nodiscard]] vk::Queue getTransferQueue() const { return m_transferQueue; }
	[[nodiscard]] vk::Instance getInstance() const { return m_instance; }
	[[nodiscard]] vk::PhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
	[[nodiscard]] uint32_t getGraphicsQueueFamily() { return findPhysicalQueueFamilies().nGraphicsFamily.value(); }
	[[nodiscard]] SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport( m_physicalDevice ); }
	[[nodiscard]] uint32_t findMemoryType( uint32_t typeFilter, vk::MemoryPropertyFlags rProperties );
	[[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies( m_physicalDevice ); }
	[[nodiscard]] vk::Format findSupportedFormat( const std::vector< vk::Format >& candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features );
	[[nodiscard]] auto getMSAA() { return m_msaaSamples; }

	// Buffer Helper Functions
	void createBuffer( vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties,
		vk::Buffer& buffer,
		vk::DeviceMemory& bufferMemory );
	[[nodiscard]] vk::CommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands( vk::CommandBuffer commandBuffer ) const;
	void copyBuffer( vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size );
	void copyBufferToImage( vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount );

	void createImageWithInfo( const vk::ImageCreateInfo& imageInfo,
		vk::MemoryPropertyFlags properties,
		vk::Image& image,
		vk::DeviceMemory& imageMemory );

	vk::PhysicalDeviceProperties m_properties;

	inline static std::mutex m_mutex{};

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
	vk::CommandPool m_pDrawCommandPool;
	vk::CommandPool m_pTransferCommandPool;

	vk::Device m_device;
	vk::SurfaceKHR m_surface;

	// TODO: add transfer queue
	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;
	vk::Queue m_transferQueue;
	vk::SampleCountFlagBits m_msaaSamples;

	std::vector< const char* > m_aDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		// Only works if vulkan configurator is running
		// #ifndef NDEBUG
		//		// TODO:
		//		// https://www.saschawillems.de/blog/2016/05/28/tutorial-on-using-vulkans-vk_ext_debug_marker-with-renderdoc/
		//		VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
		// #endif
	};

	const std::vector< const char* > m_aValidationLayers = { "VK_LAYER_KHRONOS_validation" };
};
} // namespace cat


#endif // CATENGINE_CATDEVICE_HPP
