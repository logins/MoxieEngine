/*
 Delegate.h

 First DX12 Renderer - https://github.com/logins/FirstDX12Renderer

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Delegate_h__
#define Delegate_h__

#include <list>

namespace Mox {

	// Parameterless, Single Cast Delegate.
	class SingleCastDelegate
	{
	public:
		SingleCastDelegate() : m_ObjPtr(), m_StubPtr(0) { }

		template <class T, void (T::* TMethod)()>
		static SingleCastDelegate FromMethod(T* object_ptr)
		{
			SingleCastDelegate d;
			d.m_ObjPtr = object_ptr;
			d.m_StubPtr = &CreateMethodStub<T, TMethod>;
			return d;
		}

		void Trigger() const
		{
			return (*m_StubPtr)(m_ObjPtr);
		}


	private:
		// typedef is a construct that associates a name to a type, it will just ease the reading of a definition.
		// In this case we are using the syntax to create a name for a function pointer.
		typedef void (*StubType)(void* m_ObjPtr); 
		// By using this, writing: 
		//	StubType m_StubPtr 
		// will compile the same as: 
		//	void (*m_StubPtr)(void* m_ObjPtr)
		// which is a function pointer.
		

		void* m_ObjPtr;
		StubType m_StubPtr;

		template <class T, void (T::* TMethod)()>
		static void CreateMethodStub(void* object_ptr)
		{
			T* p = static_cast<T*>(object_ptr);
			return (p->*TMethod)();
		}

	};

	/** Multicast, Variadic Template Delegate Class
	*	Inspired by: https://www.codeproject.com/Articles/1170503/The-Impossibly-Fast-Cplusplus-Delegates-Fixed
	*	My version can be used only with member functions and does not take return types into consideration
	*	because I do not currently need such additional features.
	*
	*	Example of usage with a parameterless member function:
	*
	*	Mox::MulticastDelegate myDelegate;
	*	myDelegate.Add<Part2, &Part2::Run>(Part2::Get());
	*	myDelegate.Broadcast();
	*/
	template <typename ...PARAMS>
	class MulticastDelegate
	{
	public:

		template <class T, void (T::* InMemberFn)(PARAMS...)>
		void Add(T* InObj)
		{
			m_InvocationList.push_back(typename InvocationElement(InObj, GetStubFromMemberFunction<T, InMemberFn>));
		}

		void Broadcast(PARAMS... InArgs) const 
		{
			for (auto& CurrentInvocElement : m_InvocationList)
			{
				// Call the function, on the corresponding object, with the given arguments
				(*(CurrentInvocElement.Stub))(CurrentInvocElement.ObjPtr, InArgs...); 
			}
		}

	private:

		// Note: A non-static member function always hides an implicit parameter, a pointer to the objects it belongs.
		// That is the famous "this" reference we can use inside C++ methods.
		// Source: https://isocpp.org/wiki/faq/pointers-to-members#addr-of-memfn
		// The following is a free function pointer definition, which can also be used to store non-static member functions (because they are usually different!!). 
		// The trick is stating an object pointer as the first parameter, so it can be used with the usual member function call, like shown in Broadcast( .. ).
		using StubType = void (*)(void* InObjPtr, PARAMS...);

		template <class T, void (T::* InMemberFn)(PARAMS...)>
		static void GetStubFromMemberFunction(void* InObj, PARAMS... InParams)
		{
			T* objCastedT = static_cast<T*>(InObj);
			return (objCastedT->*InMemberFn)(InParams...);
		}
		
		struct InvocationElement
		{
			InvocationElement() = default;
			InvocationElement(void* InObjPtr, StubType InStub) : ObjPtr(InObjPtr), Stub(InStub) { };
			void* ObjPtr = nullptr;
			StubType Stub = nullptr;
		};

		std::list<typename InvocationElement> m_InvocationList;
	};

}
#endif // Delegate_h__
