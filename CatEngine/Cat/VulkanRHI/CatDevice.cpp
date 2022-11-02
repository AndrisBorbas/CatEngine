#include "CatDevice.hpp"

#include <loguru.hpp>

#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <unordered_set>

namespace cat
{
// local callback functions

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData )
{
	auto verbosity = loguru::Verbosity_FATAL;
	switch ( messageSeverity )
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			verbosity = loguru::Verbosity_ERROR;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			verbosity = loguru::Verbosity_WARNING;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			verbosity = loguru::Verbosity_INFO;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			verbosity = loguru::Verbosity_MAX;
			break;
	}

	VLOG_F( verbosity, "-- validation layer error: %s\n", pCallbackData->pMessage );

	return VK_FALSE;
}

vk::Result CreateDebugUtilsMessengerEXT( vk::Instance& rInstance,
	const vk::DebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const vk::AllocationCallbacks* pAllocator,
	vk::DebugUtilsMessengerEXT* pDebugMessenger )
{
	auto dldy = vk::DispatchLoaderDynamic( rInstance, vkGetInstanceProcAddr );
	return rInstance.createDebugUtilsMessengerEXT( pCreateInfo, pAllocator, pDebugMessenger, dldy );
}

void DestroyDebugUtilsMessengerEXT( const vk::Instance& rInstance,
	vk::DebugUtilsMessengerEXT& rDebugMessenger,
	const vk::AllocationCallbacks* pAllocator )
{
	auto dldy = vk::DispatchLoaderDynamic( rInstance, vkGetInstanceProcAddr );
	rInstance.destroyDebugUtilsMessengerEXT( rDebugMessenger, pAllocator, dldy );
}


// class member functions

CatDevice::CatDevice( CatWindow* pWindow ) : m_pWindow{ pWindow }
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createCommandPool();
}

CatDevice::~CatDevice()
{
	m_device.destroyCommandPool( m_pDrawCommandPool, nullptr );
	m_device.destroy( nullptr );

	if ( m_bEnableValidationLayers )
	{
		DestroyDebugUtilsMessengerEXT( m_instance, m_debugMessenger, nullptr );
	}

	m_instance.destroySurfaceKHR( m_surface, nullptr );
	m_instance.destroy( nullptr );
}

void CatDevice::createInstance()
{
	if ( m_bEnableValidationLayers && !checkValidationLayerSupport() )
	{
		throw std::runtime_error( "validation layers requested, but not available!" );
	}

	const vk::ApplicationInfo appInfo = {
		.pApplicationName = "CatEditor",
		.applicationVersion = VK_MAKE_VERSION( 0, 3, 0 ),
		.pEngineName = "CatEngine",
		.engineVersion = VK_MAKE_VERSION( 0, 3, 0 ),
		.apiVersion = VK_API_VERSION_1_2,
	};

	auto extensions = getRequiredExtensions();

	vk::InstanceCreateInfo createInfo = {
		.pApplicationInfo = &appInfo,
		.enabledExtensionCount = static_cast< uint32_t >( extensions.size() ),
		.ppEnabledExtensionNames = extensions.data(),
	};


	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if ( m_bEnableValidationLayers )
	{
		createInfo.enabledLayerCount = static_cast< uint32_t >( m_aValidationLayers.size() );
		createInfo.ppEnabledLayerNames = m_aValidationLayers.data();

		populateDebugMessengerCreateInfo( debugCreateInfo );
		createInfo.pNext = (vk::DebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if ( vk::createInstance( &createInfo, nullptr, &m_instance ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create instance!" );
	}

	hasGflwRequiredInstanceExtensions();
}

vk::SampleCountFlagBits CatDevice::getMaxUsableSampleCount( const vk::PhysicalDevice rPhysicalDevice )
{
	// Limit MSAA to 1 because otherwise undocked imgui crashes
	return vk::SampleCountFlagBits::e1;

	vk::PhysicalDeviceProperties physicalDeviceProperties;
	rPhysicalDevice.getProperties( &physicalDeviceProperties );

	vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
								  & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if ( counts & vk::SampleCountFlagBits::e64 ) return vk::SampleCountFlagBits::e64;
	if ( counts & vk::SampleCountFlagBits::e32 ) return vk::SampleCountFlagBits::e32;
	if ( counts & vk::SampleCountFlagBits::e16 ) return vk::SampleCountFlagBits::e16;
	if ( counts & vk::SampleCountFlagBits::e8 ) return vk::SampleCountFlagBits::e8;
	if ( counts & vk::SampleCountFlagBits::e4 ) return vk::SampleCountFlagBits::e4;
	if ( counts & vk::SampleCountFlagBits::e2 ) return vk::SampleCountFlagBits::e2;

	return vk::SampleCountFlagBits::e1;
}

bool CatDevice::isDeviceSuitable( const vk::PhysicalDevice rPhysicalDevice )
{
	if ( !findQueueFamilies( rPhysicalDevice ).isComplete() )
	{
		return false;
	}
	if ( !checkDeviceExtensionSupport( rPhysicalDevice ) )
	{
		return false;
	}

	SwapChainSupportDetails swapChainSupport = querySwapChainSupport( rPhysicalDevice );
	if ( swapChainSupport.aFormats.empty() || swapChainSupport.aPresentModes.empty() )
	{
		return false;
	}

	vk::PhysicalDeviceFeatures2 supportedFeatures;
	rPhysicalDevice.getFeatures2( &supportedFeatures );
	if ( !supportedFeatures.features.samplerAnisotropy )
	{
		return false;
	}

	return true;
}

int CatDevice::rateDeviceSuitability( const vk::PhysicalDevice rPhysicalDevice )
{
	if ( !isDeviceSuitable( rPhysicalDevice ) )
	{
		return -1;
	}

	vk::PhysicalDeviceProperties deviceProperties;
	rPhysicalDevice.getProperties( &deviceProperties );

	vk::PhysicalDeviceFeatures deviceFeatures;
	rPhysicalDevice.getFeatures( &deviceFeatures );

	// Application can't function without geometry shaders
	if ( !deviceFeatures.fillModeNonSolid )
	{
		return 0;
	}

	// Application can't function without geometry shaders
	if ( !deviceFeatures.geometryShader )
	{
		return 0;
	}

	int score = 0;

	// Discrete GPUs have a significant performance advantage
	if ( deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu )
	{
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	return score;
}

void CatDevice::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	m_instance.enumeratePhysicalDevices( &deviceCount, nullptr );
	if ( deviceCount == 0 )
	{
		throw std::runtime_error( "failed to find GPUs with Vulkan support!" );
	}
	std::vector< vk::PhysicalDevice > physicalDevices( deviceCount );
	m_instance.enumeratePhysicalDevices( &deviceCount, physicalDevices.data() );

	// Use an ordered map to automatically sort candidates by increasing score
	std::multimap< int, vk::PhysicalDevice > candidates;

	for ( const auto& physicalDevice : physicalDevices )
	{
		int score = rateDeviceSuitability( physicalDevice );

		candidates.insert( std::make_pair( score, physicalDevice ) );
	}

	// Check if the best candidate is suitable at all
	if ( candidates.rbegin()->first > 0 )
	{
		m_physicalDevice = candidates.rbegin()->second;
		LOG_F( INFO, "%s", m_physicalDevice.getProperties().deviceName );
	}
	else
	{
		throw std::runtime_error( "failed to find a suitable GPU!" );
	}

	m_msaaSamples = getMaxUsableSampleCount( m_physicalDevice );
	LOG_F( INFO, ( std::string( "MSAA: " ) + to_string( m_msaaSamples ) ).c_str() );
}

void CatDevice::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies( m_physicalDevice );

	std::vector< vk::DeviceQueueCreateInfo > queueCreateInfos;
	std::set< uint32_t > uniqueQueueFamilies = {
		indices.nGraphicsFamily.value(),
		indices.nPresentFamily.value(),
		indices.nTransferFamily.value(),
	};

	float queuePriority = 1.0f;
	for ( uint32_t queueFamily : uniqueQueueFamilies )
	{
		vk::DeviceQueueCreateInfo queueCreateInfo = {
			.queueFamilyIndex = queueFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};

		queueCreateInfos.push_back( queueCreateInfo );
	}

	vk::PhysicalDeviceFeatures deviceFeatures = {
		.tessellationShader = true,
		.sampleRateShading = true,
		.fillModeNonSolid = true,
		.wideLines = true,
		.samplerAnisotropy = true,
		.variableMultisampleRate = true,
	};

	vk::DeviceCreateInfo createInfo = {
		.queueCreateInfoCount = static_cast< uint32_t >( queueCreateInfos.size() ),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledExtensionCount = static_cast< uint32_t >( m_aDeviceExtensions.size() ),
		.ppEnabledExtensionNames = m_aDeviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures,
	};


	// might not really be necessary anymore because device specific validation layers
	// have been deprecated
	if ( m_bEnableValidationLayers )
	{
		createInfo.enabledLayerCount = static_cast< uint32_t >( m_aValidationLayers.size() );
		createInfo.ppEnabledLayerNames = m_aValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if ( m_physicalDevice.createDevice( &createInfo, nullptr, &m_device ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create logical device!" );
	}

	m_device.getQueue( indices.nGraphicsFamily.value(), 0, &m_graphicsQueue );
	m_device.getQueue( indices.nPresentFamily.value(), 0, &m_presentQueue );
	m_device.getQueue( indices.nTransferFamily.value(), 0, &m_transferQueue );
}

void CatDevice::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

	vk::CommandPoolCreateInfo drawPoolInfo = {
		.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = queueFamilyIndices.nGraphicsFamily.value(),
	};


	if ( m_device.createCommandPool( &drawPoolInfo, nullptr, &m_pDrawCommandPool ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create draw command pool!" );
	}

	vk::CommandPoolCreateInfo transferPoolInfo = {
		.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer ,
		.queueFamilyIndex = queueFamilyIndices.nTransferFamily.value(),
	};


	if ( m_device.createCommandPool( &transferPoolInfo, nullptr, &m_pTransferCommandPool ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create transfer command pool!" );
	}
}

void CatDevice::createSurface()
{
	auto tempSurface = VkSurfaceKHR( m_surface );
	m_pWindow->createWindowSurface( m_instance, &tempSurface );
	m_surface = tempSurface;
}

void CatDevice::populateDebugMessengerCreateInfo( vk::DebugUtilsMessengerCreateInfoEXT& createInfo )
{
	createInfo = {
		.messageSeverity =
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
		.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
					   | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
		.pfnUserCallback = debugCallback,
		.pUserData = nullptr, // Optional
	};
}

void CatDevice::setupDebugMessenger()
{
	if ( !m_bEnableValidationLayers ) return;
	vk::DebugUtilsMessengerCreateInfoEXT createInfo = {
		.messageSeverity =
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
		.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
					   | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
		.pfnUserCallback = debugCallback,
	};
	populateDebugMessengerCreateInfo( createInfo );
	if ( CreateDebugUtilsMessengerEXT( m_instance, &createInfo, nullptr, &m_debugMessenger ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to set up debug messenger!" );
	}
}

bool CatDevice::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vk::enumerateInstanceLayerProperties( &layerCount, nullptr );

	std::vector< vk::LayerProperties > availableLayers( layerCount );
	vk::enumerateInstanceLayerProperties( &layerCount, availableLayers.data() );

	for ( const char* layerName : m_aValidationLayers )
	{
		bool layerFound = false;

		for ( const auto& layerProperties : availableLayers )
		{
			if ( strcmp( layerName, layerProperties.layerName ) == 0 )
			{
				layerFound = true;
				break;
			}
		}

		if ( !layerFound )
		{
			return false;
		}
	}

	return true;
}

std::vector< const char* > CatDevice::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

	std::vector< const char* > extensions( glfwExtensions, glfwExtensions + glfwExtensionCount );

	if ( m_bEnableValidationLayers )
	{
		extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
	}

	return extensions;
}

void CatDevice::hasGflwRequiredInstanceExtensions()
{
	uint32_t extensionCount = 0;
	vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
	std::vector< vk::ExtensionProperties > extensions( extensionCount );
	vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions.data() );

	std::cout << "available extensions:" << std::endl;
	std::unordered_set< std::string > available;
	for ( const auto& extension : extensions )
	{
		std::cout << "\t" << extension.extensionName << std::endl;
		available.insert( extension.extensionName );
	}

	std::cout << "required extensions:" << std::endl;
	auto requiredExtensions = getRequiredExtensions();
	for ( const auto& required : requiredExtensions )
	{
		std::cout << "\t" << required << std::endl;
		if ( available.find( required ) == available.end() )
		{
			throw std::runtime_error( "Missing required glfw extension" );
		}
	}
}

bool CatDevice::checkDeviceExtensionSupport( const vk::PhysicalDevice rPhysicalDevice )
{
	uint32_t extensionCount;
	rPhysicalDevice.enumerateDeviceExtensionProperties( nullptr, &extensionCount, nullptr );

	std::vector< vk::ExtensionProperties > availableExtensions( extensionCount );
	rPhysicalDevice.enumerateDeviceExtensionProperties( nullptr, &extensionCount, availableExtensions.data() );


	std::set< std::string > requiredExtensions( m_aDeviceExtensions.begin(), m_aDeviceExtensions.end() );


	for ( const auto& extension : availableExtensions )
	{
		requiredExtensions.erase( extension.extensionName );
		if ( requiredExtensions.empty() )
		{
			break;
		}
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices CatDevice::findQueueFamilies( const vk::PhysicalDevice rPhysicalDevice )
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	rPhysicalDevice.getQueueFamilyProperties( &queueFamilyCount, nullptr );

	std::vector< vk::QueueFamilyProperties > queueFamilies( queueFamilyCount );
	rPhysicalDevice.getQueueFamilyProperties( &queueFamilyCount, queueFamilies.data() );

	int i = 0;
	for ( const auto& queueFamily : queueFamilies )
	{
		if ( queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics )
		{
			indices.nGraphicsFamily = i;
		}
		vk::Bool32 presentSupport = false;
		rPhysicalDevice.getSurfaceSupportKHR( i, m_surface, &presentSupport );
		if ( queueFamily.queueCount > 0 && presentSupport )
		{
			indices.nPresentFamily = i;
		}
		if ( queueFamilyCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eTransfer )
		{
			indices.nTransferFamily = i;
		}
		if ( indices.isComplete() )
		{
			break;
		}

		i++;
	}

	return indices;
}

SwapChainSupportDetails CatDevice::querySwapChainSupport( const vk::PhysicalDevice rPhysicalDevice )
{
	SwapChainSupportDetails details;
	rPhysicalDevice.getSurfaceCapabilitiesKHR( m_surface, &details.capabilities );

	uint32_t formatCount;
	rPhysicalDevice.getSurfaceFormatsKHR( m_surface, &formatCount, nullptr );

	if ( formatCount != 0 )
	{
		details.aFormats.resize( formatCount );
		rPhysicalDevice.getSurfaceFormatsKHR( m_surface, &formatCount, details.aFormats.data() );
	}

	uint32_t presentModeCount;
	rPhysicalDevice.getSurfacePresentModesKHR( m_surface, &presentModeCount, nullptr );

	if ( presentModeCount != 0 )
	{
		details.aPresentModes.resize( presentModeCount );
		rPhysicalDevice.getSurfacePresentModesKHR( m_surface, &presentModeCount, details.aPresentModes.data() );
	}
	return details;
}

vk::Format CatDevice::findSupportedFormat( const std::vector< vk::Format >& candidates,
	const vk::ImageTiling tiling,
	const vk::FormatFeatureFlags features )
{
	for ( const vk::Format format : candidates )
	{
		vk::FormatProperties props;
		m_physicalDevice.getFormatProperties( format, &props );

		if ( tiling == vk::ImageTiling::eLinear && ( props.linearTilingFeatures & features ) == features )
		{
			return format;
		}
		else if ( tiling == vk::ImageTiling::eOptimal && ( props.optimalTilingFeatures & features ) == features )
		{
			return format;
		}
	}
	throw std::runtime_error( "failed to find supported format!" );
}

uint32_t CatDevice::findMemoryType( uint32_t typeFilter, vk::MemoryPropertyFlags rProperties )
{
	vk::PhysicalDeviceMemoryProperties memProperties;
	m_physicalDevice.getMemoryProperties( &memProperties );
	for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
	{
		if ( ( typeFilter & ( 1 << i ) ) && ( memProperties.memoryTypes[i].propertyFlags & rProperties ) == rProperties )
		{
			return i;
		}
	}

	throw std::runtime_error( "failed to find suitable m_pMemory type!" );
}

void CatDevice::createBuffer( vk::DeviceSize size,
	vk::BufferUsageFlags usage,
	vk::MemoryPropertyFlags properties,
	vk::Buffer& buffer,
	vk::DeviceMemory& bufferMemory )
{
	vk::BufferCreateInfo bufferInfo{
		.size = size,
		.usage = usage,
		.sharingMode = vk::SharingMode::eExclusive,
	};


	if ( m_device.createBuffer( &bufferInfo, nullptr, &buffer ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create vertex m_Buffer!" );
	}

	vk::MemoryRequirements memRequirements;
	m_device.getBufferMemoryRequirements( buffer, &memRequirements );

	vk::MemoryAllocateInfo allocInfo{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, properties ),
	};

	if ( m_device.allocateMemory( &allocInfo, VK_NULL_HANDLE, &bufferMemory ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to allocate vertex m_Buffer m_pMemory!" );
	}

	m_device.bindBufferMemory( buffer, bufferMemory, 0 );
}

vk::CommandBuffer CatDevice::beginSingleTimeCommands()
{
	vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = m_pTransferCommandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1,
	};


	vk::CommandBuffer commandBuffer;
	m_device.allocateCommandBuffers( &allocInfo, &commandBuffer );

	vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };

	commandBuffer.begin( beginInfo );
	return commandBuffer;
}

void CatDevice::endSingleTimeCommands( vk::CommandBuffer commandBuffer ) const
{
	vkEndCommandBuffer( commandBuffer );

	vk::SubmitInfo submitInfo{
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
	};

	const std::lock_guard lock( m_mutex );
	m_transferQueue.submit( 1, &submitInfo, VK_NULL_HANDLE );
	m_transferQueue.waitIdle();

	m_device.freeCommandBuffers( m_pTransferCommandPool, 1, &commandBuffer );
}

void CatDevice::copyBuffer( vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size )
{
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

	vk::BufferCopy copyRegion{
		.srcOffset = 0, // Optional
		.dstOffset = 0, // Optional
		.size = size,
	};

	commandBuffer.copyBuffer( srcBuffer, dstBuffer, 1, &copyRegion );

	endSingleTimeCommands( commandBuffer );
}

void CatDevice::copyBufferToImage( vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount )
{
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

	vk::BufferImageCopy region{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,

		.imageSubresource{
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = layerCount,
		},

		.imageOffset = { 0, 0, 0 },
		.imageExtent = { width, height, 1 },
	};


	commandBuffer.copyBufferToImage( buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region );
	endSingleTimeCommands( commandBuffer );
}

void CatDevice::createImageWithInfo( const vk::ImageCreateInfo& imageInfo,
	vk::MemoryPropertyFlags properties,
	vk::Image& image,
	vk::DeviceMemory& imageMemory )
{
	if ( m_device.createImage( &imageInfo, nullptr, &image ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to create image!" );
	}

	vk::MemoryRequirements memRequirements;
	m_device.getImageMemoryRequirements( image, &memRequirements );

	vk::MemoryAllocateInfo allocInfo{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, properties ),
	};


	if ( m_device.allocateMemory( &allocInfo, nullptr, &imageMemory ) != vk::Result::eSuccess )
	{
		throw std::runtime_error( "failed to allocate image m_pMemory!" );
	}

	m_device.bindImageMemory( image, imageMemory, 0 );
}
} // namespace cat
