#pragma once

#include <string>

namespace ppr
{
	enum class DebugLevel
	{
		Regular,
		Deep,
	};

	class DebugMessenger
	{
	public:
		void Print(const std::string& aString, DebugLevel aLevel);

		static DebugMessenger& GetInstance();

	private:
		DebugMessenger();
		void Init();

	private:
		bool mDebugOn;
		bool mDeepOn;
	};
}

namespace ppr
{
#ifndef Debug
#define Debug DebugMessenger::GetInstance()
#endif

#ifndef DbgPrint
#define DbgPrint(msg) DebugMessenger::GetInstance().Print(msg, ppr::DebugLevel::Regular)
#endif

#ifndef VerbosePrint
#define VerbosePrint(msg) DebugMessenger::GetInstance().Print(msg, ppr::DebugLevel::Deep)
#endif
}