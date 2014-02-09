#ifndef _TTFunction_H_
#define _TTFunction_H_

#include "TTCall.h"
#include "TTFunctor.h"

namespace TT
{

	template<class R, class ... Args>
	class Function : public Functor
	{
	public :
		//去const去volatile去reference
		typedef std::tuple<typename std::remove_cv<typename std::remove_reference<Args>::type>::type...> Tuple;

	public:
		void operator()(const ObjectPtr* paras, int paracount, Object* ret)
		{
			assert(sizeof...(Args) <= paracount);//脚本中的需要抛异常
			Tuple t;
			ArgsConverter<sizeof...(Args)>::Conv<Tuple>::convert<sizeof...(Args)-1>(t, paras);

			Call::call<R, Args...>(this, t, ret);
		}

		virtual R call(Args... args) = 0;
	};
	
	template<class R, class ... Args>
	class NormalFunction : public Function<R, Args...>
	{
		using FuncHandle = R(*)(Args...);
	public :
		NormalFunction(FuncHandle func):
			mFunc(func)
		{}

		R call(Args ... args)
		{
			return mFunc(args...);
		}
	private:
		FuncHandle mFunc;
	};

	template<class O, class R, class ... Args>
	class ClassFunction : public Function<R, Args...>
	{
		using FuncHandle = R(O::*)(Args...);
	public:
		ClassFunction(O* obj, FuncHandle func) :
			mObj(obj), mFunc(func)
		{}

		R call(Args ... args)
		{
			return (mObj->*mFunc)(args...);
		}
	private:
		FuncHandle mFunc;
		O* mObj;
	};
}

#endif//_TTFunction_H_