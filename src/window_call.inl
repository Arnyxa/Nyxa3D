namespace ppr
{
	template<typename T>
	void window_callbacks::add(void (T::*a_function)(), T* obj_ptr, call_type a_type)
	{
        if (obj_ptr == nullptr)
            throw Error("Object to call back on was null pointer.", ppr::Error::Code::NULL_PTR);
        if (a_function == nullptr)
            throw Error("Provided function was null pointer.", ppr::Error::Code::NULL_PTR);

		callbacks::ptr myCallback(new call<T>(a_function, obj_ptr));

		mCallbacks.push_back(std::make_pair(std::move(myCallback), a_type));
	}
}