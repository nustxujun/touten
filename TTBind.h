#ifndef _TTBind_H_
#define _TTBind_H_

#include "Touten.h"
#include "TTFunction.h"
#include "TTException.h"
#include "TTObject.h"

namespace TT
{
	
	class ToutenExport Bind
	{
	public:
		Bind(Touten* tt);
		~Bind();

		template<class R, class ... Args>
		void bind(const String& name, R (*func)(Args...))
		{
			Functor* f = new NormalFunction<R, Args...>(func);
			auto ret = mFactors.insert(
				Factors::value_type(name, f));
			if (!ret.second)
			{
				delete f;
				
				TT_EXCEPT(ET_UNKNOWN, EL_NORMAL, "same function name", 0);
				return;
			}
			mTT->registerOrRetrieveFunction(name, f);
		}

		template<class O, class R, class ... Args>
		void bind(const String& name, O* inst, R(O::*func)(Args...))
		{
			Functor* f = new ClassFunction<O, R, Args...>(inst, func);
			auto ret = mFactors.insert(
				Factors::value_type(name, f));
			if (!ret.second)
			{
				delete f;
				TT_EXCEPT(ET_UNKNOWN, EL_NORMAL, "same function name", 0);
				return;

			}
			mTT->registerOrRetrieveFunction(name, f);
		}

		template<class O, class R, class ... Args>
		void bind(const String& name, O* inst, R(O::*func)(Args...)const)
		{
			bind(name, inst, (R(T::*)(Args...))func)
		}

		template<class O, class R, class ... Args>
		void bind(const String& name, O* inst, R(O::*func)(Args...)volatile)
		{
			bind(name, inst, (R(T::*)(Args...))func)
		}

		void remove(const String& name);

		template<class R, class...Args>
		typename std::enable_if<!std::is_void<R>::value, R>::type call(const String& name, Args... args)
		{
			ObjectPtr paras[sizeof...(Args)] = { Object(args)... };
			Object ret;
			mTT->call(name, sizeof...(Args), paras, &ret);
			return Caster::cast<R>(ret);
		}

		template<class R, class...Args>
		typename std::enable_if<std::is_void<R>::value, R>::type call(const String& name, Args... args)
		{
			ObjectPtr paras[sizeof...(Args)] = { Object(args)... };
			mTT->call(name, sizeof...(Args), paras);
		}

		template<class R>
		typename std::enable_if<!std::is_void<R>::value, R>::type call(const String& name)
		{
			Object ret;
			mTT->call(name, 0, nullptr, &ret);
			return Caster::cast<R>(ret);
		}

		template<class R>
		typename std::enable_if<std::is_void<R>::value, R>::type call(const String& name)
		{
			mTT->call(name);
		}


		Touten* getTouten();
	private:
		Touten* mTT;

		typedef TTMap<String, Functor*> Factors;
		Factors mFactors;
	};
}

#endif//_TTBind_H_