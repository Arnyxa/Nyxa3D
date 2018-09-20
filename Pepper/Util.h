#pragma once

#include <string>

enum VkResult;

namespace vk
{
	enum class Result;
}

namespace ppr
{
	VkResult PrintResult(VkResult aResult);

	void Print(const std::string& aText);
	vk::Result Print(vk::Result aResult);
}

namespace ppr
{
	class CommonChecks
	{
	public:
		CommonChecks() 
			: mDestroyed(false)
			, mInitialised(false) 
		{}

		bool IsInitialized() const
		{ return mInitialised; }

	protected:
		bool mDestroyed;
		bool mInitialised;
	};
}

namespace ppr
{
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
        void Print() const;
        int GetCode() const;

    private:
        const std::string mError;
        const int mCode;
    };
}