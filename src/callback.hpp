#pragma once

#include <memory>

namespace ppr
{
	class Callbacks
	{
	public:
		typedef std::unique_ptr<Callbacks> ptr;

		virtual void Execute() = 0;
	};

	template<typename T>
	class Call : public Callbacks
	{
	public:
		Call(void (T::*aFunction)(), T* anObjPtr)
			: mFunction(aFunction), mObject(anObjPtr)
		{}

		void Execute() override
		{ (mObject->*mFunction)(); }

	private:
		void(T::*mFunction)();
		T* mObject;
	};
}