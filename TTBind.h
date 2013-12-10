#ifndef _TTBind_H_
#define _TTBind_H_

#include "Touten.h"

namespace TT
{
	
	class Bind
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
				assert(0);
				return;
			}
			mTT->registerFunction(name, f);
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
				assert(0);
				return;

			}
			mTT->registerFunction(name, f);
		}


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

	private:
		Touten* mTT;

		typedef std::hash_map<String, Functor*> Factors;
		Factors mFactors;
	};
}

#endif//_TTBind_H_