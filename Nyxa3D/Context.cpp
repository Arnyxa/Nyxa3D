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
		DbgPrint("Initializing Window...\n");

		mWindow.Init();

		DbgPrint("Window initialized.\n\nInitializing Vulkan...\n");

		CreateInstance();
		mDebugger.Init();
		mSwapchain.CreateWindowSurface();
		SelectPhysicalDevice();
		CreateLogicalDevice();
		mSwapchain.Init();

		DbgPrint("Vulkan initialized.\n\n");
	}

	void Context::CreateLogicalDevice()
	{
		DeepPrint("Creating logical Vulkan Device...\n");

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

		DeepPrint("Successfully created logical Vulkan Device.\n\n");
	}

	void Context::SelectPhysicalDevice()
	{
		DeepPrint("Searching for viable physical device...\n");

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

		DeepPrint("Matching Vulkan-compatible GPU(s) successfully found.\n\n");
	}

	bool Context::IsDeviceSuitable(vk::PhysicalDevice aDevice)
	{
		DeepPrint("Evaluating device suitability...\n");

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
		DeepPrint("Fetching required extensions...\n");

		std::vector<const char*> myExtensions = mWindow.GetRequiredExtensions();

		if (VALIDATION_LAYERS_ENABLED)
			myExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return myExtensions;
	}

	bool Context::CheckDeviceExtensionSupport(vk::PhysicalDevice aDevice)
	{
		DeepPrint("Checking device extension support...\n");

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

		DeepPrint("Available extensions:\n");
		for (const auto& iProperty : myExtensionProperties)
		{
			DeepPrint(std::string("\t") + iProperty.extensionName + "\n");
		}
		DeepPrint(myExtensionProperties.size() + " extensions found in total.\n\n");

		DeepPrint("Checking for GLFW extension compatibility...\n");

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

			DeepPrint(std::string("        ") + myExtensions[i] + " found.\n");
		}

		DeepPrint("Extension check successful.\n\n");

		mInstance = vk::createInstance(myCreateInfo);

		DeepPrint("Created Vulkan instance.\n\n");
	}

	Context::~Context()
	{
		DbgPrint("Destroying Context objects...\n");

		mSwapchain.Destroy();
		mDevice.destroy();
		mDebugger.Destroy();
		mInstance.destroy();

		DbgPrint("Context successfully destroyed.\n\n");
	}

}