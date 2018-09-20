#include "DbgMsgr.h"

#include <iostream>

namespace ppr
{
	DebugMessenger::DebugMessenger()
		: mDebugOn(false)
		, mDeepOn(false)
	{}

	void DebugMessenger::Init()
	{
	#ifdef PPR_DEBUG
		mDebugOn = true;
	#endif
	#ifdef PPR_VERBOSE
		mDeepOn = true;
	#endif
	}

	DebugMessenger& DebugMessenger::GetInstance()
	{
		static DebugMessenger myDbgMsgr;
		static bool hasInitialized = false;

		if (!hasInitialized)
		{
			myDbgMsgr.Init();
			hasInitialized = true;
		}

		return myDbgMsgr;
	}

	void DebugMessenger::Print(const std::string& aString, DebugLevel aLevel)
	{
		if (mDeepOn && aLevel == DebugLevel::Deep)
			std::cout << aString;
		else if (mDebugOn && aLevel == DebugLevel::Regular)
			std::cout << aString;
	}
}