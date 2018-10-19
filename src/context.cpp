#include "context.hpp"
#include "callbacks.hpp"
#include "util.hpp"
#include "logger.hpp"

#include <glfw/glfw3.h>

#include <set>
#include <algorithm>
#include <fstream>

namespace ppr
{
	context::context(const std::string& a_title)
		: m_window(a_title)
        , m_debugger(m_instance)
		, m_swapchain(m_device, m_window, m_instance, m_physical_device)
	{
        log->info("Initializing Vulkan...");
        create_instance();
        m_debugger.init();
        m_swapchain.create_window_surface();
        select_physical_device();
        create_device();
        m_swapchain.init();
        log->info("Vulkan initialized.\n");
    }

    context::~context()
    {
        log->trace("Destroying context objects...");

        m_swapchain.destroy();
        m_device.destroy();
        m_debugger.destroy();
        m_instance.destroy();

        log->info("Context successfully destroyed.");
    }

	void context::run()
	{
		main_loop();
	}

    void context::init()
    {
    }

    void context::main_loop()
    {
        while (!m_window.should_close())
        {
            m_window.poll_events();
            m_swapchain.draw();
        }

        m_device.waitIdle();
    }

    void context::create_instance()
    {
        if (VALIDATION_LAYERS_ENABLED && !m_debugger.supports_validation_layers())
            log->critical("Validation layers requested, but not available.");

        const vk::ApplicationInfo app_info(PROJECT_TITLE, VK_MAKE_VERSION(0, 0, 1), PROJECT_TITLE, VK_MAKE_VERSION(0, 0, 1), VK_API_VERSION_1_1);

        const auto extensions = required_extensions();

        vk::InstanceCreateInfo createinfo({}, &app_info);

        createinfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createinfo.ppEnabledExtensionNames = extensions.data();

        if (VALIDATION_LAYERS_ENABLED)
        {
            createinfo.enabledLayerCount = m_debugger.enabled_layer_count();
            createinfo.ppEnabledLayerNames = m_debugger.enabled_layer_names();
        }
        else
            createinfo.enabledLayerCount = 0;

        const std::vector<vk::ExtensionProperties> extension_properties = vk::enumerateInstanceExtensionProperties();

        m_window.check_extension_compatibility(extensions, extension_properties);

        log->trace("Available extensions:");
        for (const auto& i_property : extension_properties)
        {
            log->trace("\t {} ", i_property.extensionName);
        }
        log->trace("{} extensions found in total.", extension_properties.size());

        log->debug("Extension check successful.");

        m_instance = vk::createInstance(createinfo);

        log->debug("Created Vulkan instance.");
    }

    void context::create_device()
    {
        log->trace("Creating logical Vulkan Device...");

        const queue_families family_indices = m_swapchain.find_queue_families(m_physical_device);

        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
        const std::set<int> unique_families = { family_indices.graphics, family_indices.present };

        const float queue_priority = 1.f;
        for (auto& i_family : unique_families)
        {
            vk::DeviceQueueCreateInfo queue_createinfo({}, i_family, 1, &queue_priority);

            queue_create_infos.push_back(queue_createinfo);
        }

        const vk::PhysicalDeviceFeatures device_features = {};

        vk::DeviceCreateInfo device_createinfo;
        device_createinfo.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_createinfo.pQueueCreateInfos = queue_create_infos.data();
        device_createinfo.pEnabledFeatures = &device_features;

        device_createinfo.enabledExtensionCount = static_cast<uint32_t>(m_device_ext.size());
        device_createinfo.ppEnabledExtensionNames = m_device_ext.data();

        if (VALIDATION_LAYERS_ENABLED)
        {
            device_createinfo.enabledLayerCount = m_debugger.enabled_layer_count();
            device_createinfo.ppEnabledLayerNames = m_debugger.enabled_layer_names();
        }
        else
            device_createinfo.enabledLayerCount = 0;

        m_device = m_physical_device.createDevice(device_createinfo);

        m_swapchain.graphics_queue() = m_device.getQueue(family_indices.graphics, 0);
        m_swapchain.present_queue() = m_device.getQueue(family_indices.present, 0);

        log->debug("Successfully created logical Vulkan Device.");
    }

	void context::select_physical_device()
	{
		log->trace("Searching for viable physical device...");

        const std::vector<vk::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();

		if (devices.empty())
			log->critical("Failed to find any GPU with Vulkan support.");

		for (const auto& i_device : devices)
		{
			if (device_is_suitable(i_device))
			{
				m_physical_device = i_device;
				break;
			}
		}

		if (m_physical_device == vk::PhysicalDevice())
			log->critical("Available GPU(s) have insufficient compatibility with {} features.", PROJECT_TITLE);

		log->debug("Matching Vulkan-compatible GPU(s) successfully found.");
	}

	bool context::device_is_suitable(const vk::PhysicalDevice& a_device) const
	{
		log->trace("Evaluating device suitability...");

        const queue_families family_indices = m_swapchain.find_queue_families(a_device);

        const bool is_ext_supported = check_ext_support(a_device);
		bool is_swapchain_adequate = false;

		if (is_ext_supported)
		{
            const swapchain_support support_details = m_swapchain.query_support(a_device);
			is_swapchain_adequate = !support_details.formats.empty() && !support_details.present_modes.empty();
		}

		return family_indices.is_complete() && is_ext_supported && is_swapchain_adequate;
	}

	std::vector<const char*> context::required_extensions() const
	{
		log->trace("Fetching required extensions...");

		std::vector<const char*> extensions = m_window.required_extensions();

		if (VALIDATION_LAYERS_ENABLED)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	bool context::check_ext_support(const vk::PhysicalDevice& a_device) const
	{
		log->trace("Checking device extension support...");

        const std::vector<vk::ExtensionProperties> available_extensions = a_device.enumerateDeviceExtensionProperties();

		std::set<std::string> required_extensions(m_device_ext.begin(), m_device_ext.end());

		for (const auto& i_extension : available_extensions)
			required_extensions.erase(i_extension.extensionName);

		return required_extensions.empty();
	}
}