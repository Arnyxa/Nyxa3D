#pragma once

#include <memory>

namespace ppr
{
	class callbacks
	{
	public:
        using ptr = std::unique_ptr<callbacks>;

		virtual void execute() = 0;
	};

	template<typename T>
	class call : public callbacks
	{
	public:
		call(void (T::*a_function)(), T* obj_ptr)
			: m_function(a_function), m_object(obj_ptr)
		{}

		void execute() override
		{ (m_object->*m_function)(); }

	private:
		void(T::*m_function)();
		T* m_object;
	};
}