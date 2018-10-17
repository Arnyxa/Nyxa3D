namespace ppr
{
	template<typename T>
	void WindowCallbacks::Add(void (T::*aFunction)(), T* anObjPtr, CallType aType)
	{
        if (anObjPtr == nullptr)
            throw Error("Object to call back on was null pointer.", ppr::Error::Code::NULL_PTR);
        if (aFunction == nullptr)
            throw Error("Provided function was null pointer.", ppr::Error::Code::NULL_PTR);

		Callbacks::ptr myCallback(new Call<T>(aFunction, anObjPtr));

		mCallbacks.push_back(std::make_pair(std::move(myCallback), aType));
	}
}