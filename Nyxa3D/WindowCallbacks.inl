namespace nx
{
	template<typename T>
	void WindowCallbacks::AddCallback(void (T::*aFunction)(), T* anObjPtr, CallType aType)
	{
		Callbacks::ptr myCallback(new Call<T>(aFunction, anObjPtr));

		mCallbacks.push_back(std::make_pair(std::move(myCallback), aType));
	}
}