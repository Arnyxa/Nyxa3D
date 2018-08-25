#pragma once

#include <string>

namespace nx
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

namespace nx
{
#ifndef Debug
#define Debug DebugMessenger::GetInstance()
#endif

#ifndef DbgPrint
#define DbgPrint(msg) DebugMessenger::GetInstance().Print(msg, nx::DebugLevel::Regular)
#endif

#ifndef DeepPrint
#define DeepPrint(msg) DebugMessenger::GetInstance().Print(msg, nx::DebugLevel::Deep)
#endif
}