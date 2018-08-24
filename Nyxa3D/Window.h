#pragma once

#include "Structs.h"
#include "Util.h"

#include <string>
#include <vector>
#include <functional>
#include <memory>

struct GLFWwindow;
typedef void(*GLFWwindowsizefun)(GLFWwindow*, int, int);

namespace nx
{	
	class CallbackImpl
	{
	public:
		typedef std::unique_ptr<CallbackImpl> ptr;

		virtual void Execute() = 0;
	};

	template<typename T>
	class Callback : public CallbackImpl
	{
	public:
		Callback(void (T::*aFunction)(), T* anObjPtr) 
			: mFunction(aFunction), mObject(anObjPtr)
		{}

		void Execute() override
		{ (mObject->*mFunction)(); }

	private:
		void(T::*mFunction)();
		T* mObject;
	};

	class Window
	{
	public:
		Window(size_t aDefaultWidth = 800, size_t aDefaultHeight = 600);
		~Window();

		void Destroy();

		void Init();

		bool ShouldClose() const;
		void PollEvents();

		GLFWwindow* GetGlfwWindowPtr();
		void* GetWindowUserPtr();

		void ResetSize();
		Size<int> GetSize() const;
		void SetSize(size_t aWidth, size_t aHeight);

		void SetTitle(const std::string& aTitle);
		std::string GetTitle() const;

		vk::SurfaceKHR CreateSurface(const vk::Instance& anInstance) const;
		std::vector<const char*> GetRequiredExtensions();

		static void OnResize(GLFWwindow* aWindow, int aWidth, int aHeight);

		template<typename T>
		void AddCallback(void (T::*aFunction)(), T* anObjPtr) const
		{
			CallbackImpl::ptr myCallback(new Callback<T>(aFunction, anObjPtr));

			mCallbacks.push_back(std::move(myCallback));
		}

	private:
		void ExecuteResizeCallbacks(); // may add callbacks for other things


	private:
		// GLFW
		GLFWwindow* mWindow;

		size_t mDefaultWidth;
		size_t mDefaultHeight;

		mutable std::vector<CallbackImpl::ptr> mCallbacks;

		const char* mTitle;
	};
}