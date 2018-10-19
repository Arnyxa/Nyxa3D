namespace ppr
{
	template<typename T>
	void window_callbacks::add(void (T::*a_function)(), T* obj_ptr, call_type a_type)
	{
        if (obj_ptr == nullptr)
            log->critical("{} - Object to call back on was null pointer.", __FUNCTION__);
        if (a_function == nullptr)
            log->critical("{} - Provided function pointer was null pointer", __FUNCTION__);

		callbacks::ptr myCallback(new call<T>(a_function, obj_ptr));

		m_callbacks.push_back(std::make_pair(std::move(myCallback), a_type));
	}
}