#pragma once

#include <string>

enum VkResult;

namespace vk
{
	enum class Result;
}

namespace ppr
{
    void       print(const std::string& aText);
	VkResult   print(VkResult aResult);
	vk::Result print(vk::Result aResult);

    std::string to_lower_copy(std::string a_string);
    void to_lower(std::string& a_string);
}

namespace ppr
{
	class common_checks
	{
	public:
		common_checks() 
			: m_destroyed(false)
			, m_initialized(false) 
		{}

		bool is_initialized() const
		{ return m_initialized; }

	protected:
		mutable bool m_destroyed;
		mutable bool m_initialized;
	};
}

namespace ppr
{
    // leaving casing here because this class is going to be deleted
    class Error
    {
    public:
        enum class Code
        {
            // enums iterate upwards, so all these errors will be negative
            NULL_PTR = -32, 
            NULL_PTR_DEREF,
            NO_GPU_SUPPORT,
            NO_PEPPER_SUPPORT,
            REQ_EXT_UNAVAILABLE,
            REQ_VAL_LAYER_UNAVAILABLE,
            DEBUG_MSGR_SETUP_FAIL,
            PIPELINE_CREATION_FAIL,
            SWAPCHAIN_IMAGE_NOT_ACQUIRED,
            SWAPCHAIN_IMAGE_PRESENT_ERROR,
            SHADER_NOT_FOUND,
            BUFFER_MEMORY_TYPE_UNSUITABLE,
        };

        Error(const std::string& aMessage, Code anErrorCode);

        const std::string& What() const;
        void print() const;
        int GetCode() const;

    private:
        const std::string mError;
        const int mCode;
    };
}