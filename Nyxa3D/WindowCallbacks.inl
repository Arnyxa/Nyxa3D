namespace nx
{
	template<typename T>
	void WindowCallbacks::AddCallback(void (T::*aFunction)(), T* anObjPtr)
	{
		Callbacks::ptr myCallback(new Call<T>(aFunction, anObjPtr));

		mCallbacks.push_back(std::move(myCallback));
	}
}