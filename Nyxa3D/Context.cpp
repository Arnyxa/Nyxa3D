#include "Context.h"
#include "Callbacks.h"
#include "Util.h"

#include <glfw/glfw3.h>

#include <set>
#include <algorithm>
#include <fstream>

namespace nx
{

	Context::Context()
		: mDebugger(mInstance)
		, mSwapchain(mDevice, mWindow, mInstance, mPhysicalDevice)
	{}

	void Context::Run()
	{
		Init();
		MainLoop();
	}

	void Context::MainLoop()
	{
		while (!mWindow.ShouldClose())
		{
			mWindow.PollEvents();
			mSwapchain.Draw();
		}

		mDevice.waitIdle();
	}

	void Context::Init()
	{
		std::cout << "Initializing Window...\n";

		mWindow.Init();

		std::cout << "Window initialized.\n\nInitializing Vulkan...\n";

		CreateInstance();
		mDebugger.Init();
		mSwapchain.CreateWindowSurface();
		SelectPhysicalDevice();
		CreateLogicalDevice();
		mSwapchain.Init();

		std::cout << "Vulkan initialized.\n\n";
	}

	void Context::CreateLogicalDevice()
	{
		std::cout << "Creating logical Vulkan Device...\n";

		QueueFamilyIndices myIndices = mSwapchain.FindQueueFamilies(mPhysicalDevice);

		std::vector<vk::DeviceQueueCreateInfo> myQueueCreateInfos;
		std::set<int> myUniqueQueueFamilies = { myIndices.Graphics, myIndices.Present };

		float myQueuePriority = 1.f;
		for (auto& iQueueFamily : myUniqueQueueFamilies)
		{
			vk::DeviceQueueCreateInfo myQueueCreateInfo({}, iQueueFamily, 1, &myQueuePriority);

			myQueueCreateInfos.push_back(myQueueCreateInfo);
		}

		vk::PhysicalDeviceFeatures myDeviceFeatures = {};

		vk::DeviceCreateInfo myDeviceCreateInfo;
		myDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(myQueueCreateInfos.size());
		myDeviceCreateInfo.pQueueCreateInfos = myQueueCreateInfos.data();
		myDeviceCreateInfo.pEnabledFeatures = &myDeviceFeatures;

		myDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size());
		myDeviceCreateInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

		if (VALIDATION_LAYERS_ENABLED)
		{
			myDeviceCreateInfo.enabledLayerCount = mDebugger.GetEnabledLayerCount();
			myDeviceCreateInfo.ppEnabledLayerNames = mDebugger.GetEnabledLayerNames();
		}
		else
			myDeviceCreateInfo.enabledLayerCount = 0;

		mDevice = mPhysicalDevice.createDevice(myDeviceCreateInfo);

		mSwapchain.GetGraphicsQueue() = mDevice.getQueue(myIndices.Graphics, 0);
		mSwapchain.GetPresentQueue() = mDevice.getQueue(myIndices.Present, 0);

		std::cout << "Successfully created logical Vulkan Device.\ngg\n";
	}

	void Context::SelectPhysicalDevice()
	{
		std::cout << "Searching for viable physical device...\n";

		std::vector<vk::PhysicalDevice> myDevices = mInstance.enumeratePhysicalDevices();

		if (myDevices.empty())
			throw std::runtime_error("Failed to find any GPU with Vulkan support.");

		for (const auto& iDevice : myDevices)
		{
			if (IsDeviceSuitable(iDevice))
			{
				mPhysicalDevice = iDevice;
				break;
			}
		}

		if (mPhysicalDevice == vk::PhysicalDevice())
			throw std::runtime_error("Failed to find a suitable GPU for Vulkan.");

		std::cout << "Matching Vulkan-compatible GPU(s) successfully found.\n\n";
	}

	bool Context::IsDeviceSuitable(vk::PhysicalDevice aDevice)
	{
		std::cout << "Evaluating device suitability...\n";

		QueueFamilyIndices myIndices = mSwapchain.FindQueueFamilies(aDevice);

		bool isExtensionsSupported = CheckDeviceExtensionSupport(aDevice);
		bool isSwapchainAdequate = false;

		if (isExtensionsSupported)
		{
			SwapchainDetails mySwapchainSupport = mSwapchain.QuerySupport(aDevice);
			isSwapchainAdequate = !mySwapchainSupport.Formats.empty() && !mySwapchainSupport.PresentModes.empty();
		}

		return myIndices.IsComplete() && isExtensionsSupported && isSwapchainAdequate;
	}

	std::vector<const char*> Context::GetRequiredExtensions()
	{
		std::cout << "Fetching required extensions...\n";

		std::vector<const char*> myExtensions = mWindow.GetRequiredExtensions();

		if (VALIDATION_LAYERS_ENABLED)
			myExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return myExtensions;
	}

	bool Context::CheckDeviceExtensionSupport(vk::PhysicalDevice aDevice)
	{
		std::cout << "Checking device extension support...\n";

		std::vector<vk::ExtensionProperties> myAvailableExtensions = aDevice.enumerateDeviceExtensionProperties();

		std::set<std::string> myRequiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

		for (const auto& iExtension : myAvailableExtensions)
			myRequiredExtensions.erase(iExtension.extensionName);

		return myRequiredExtensions.empty();
	}

	void Context::CreateInstance()
	{
		if (VALIDATION_LAYERS_ENABLED && !mDebugger.CheckValidationLayerSupport())
			throw std::runtime_error("Validation layers requested, but not available.");

		vk::ApplicationInfo myAppInfo("Binch", VK_MAKE_VERSION(1, 0, 0), "Nyxa3D", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_1);

		auto myExtensions = GetRequiredExtensions();

		vk::InstanceCreateInfo myCreateInfo({}, &myAppInfo);
		myCreateInfo.pApplicationInfo = &myAppInfo;

		myCreateInfo.enabledExtensionCount = static_cast<uint32_t>(myExtensions.size());
		myCreateInfo.ppEnabledExtensionNames = myExtensions.data();

		if (VALIDATION_LAYERS_ENABLED)
		{
			myCreateInfo.enabledLayerCount = mDebugger.GetEnabledLayerCount();
			myCreateInfo.ppEnabledLayerNames = mDebugger.GetEnabledLayerNames();
		}
		else
			myCreateInfo.enabledLayerCount = 0;

		std::vector<vk::ExtensionProperties> myExtensionProperties = vk::enumerateInstanceExtensionProperties();

		std::cout << "Available extensions:\n";
		for (const auto& iProperty : myExtensionProperties)
		{
			std::cout << "\t" << iProperty.extensionName << "\n";
		}
		std::cout << myExtensionProperties.size() << " extensions found in total.\n\n";

		std::cout << "Checking for GLFW extension compatibility...\n";

		// check whether extensions required by GLFW are in available extension list
		for (size_t i = 0; i < myExtensions.size(); ++i)
		{
			bool isFound = false;
			for (const auto& iProperty : myExtensionProperties)
			{
				if (std::strcmp(myExtensions[i], iProperty.extensionName) == 0)
				{
					isFound = true;
					break;
				}
			}

			if (!isFound)
				throw std::runtime_error("Could not find required GLFW extension for Vulkan on this system.");

			std::cout << "        " << myExtensions[i] << " found.\n";
		}

		std::cout << "Extension check successful.\n\n";

		mInstance = vk::createInstance(myCreateInfo);

		std::cout << "Created Vulkan instance.\n\n";
	}

	Context::~Context()
	{
		std::cout << "Destroying Context objects...\n";

		mSwapchain.Destroy();
		mDevice.destroy();
		mDebugger.Destroy();
		mInstance.destroy();

		std::cout << "Context successfully destroyed.\n\n";
	}

}