#ifndef _TTCall_H_
#define _TTCall_H_

namespace TT
{
	template<size_t ArgsNum>
	struct CallImpl;

	template<int ArgsNum>
	struct ArgsConverter
	{
		template< class Tuple>
		struct Conv
		{
			template<int index>
			static void convert(Tuple& t, const ObjectPtr* paras)
			{
				typedef typename std::tuple_element<index, Tuple>::type elemType;
				std::get<index>(t) = Caster::cast<elemType>(**(paras + index));
				convert<index - 1>(t, paras);
			}

			template<>
			static void convert<0>(Tuple& t, const ObjectPtr* paras)
			{
				typedef typename std::tuple_element<0, Tuple>::type elemType;
				std::get<0>(t) = Caster::cast<elemType>(**paras);
			}
		};
	};

	template<>
	struct ArgsConverter<0>
	{
		template< class Tuple>
		struct Conv
		{
			template<int index>
			static void convert(Tuple& t, const ObjectPtr* paras)
			{
			}
		};
	};

	struct Call
	{
		template<class R, class ... Args, class F>
		static void call(F* func, typename F::Tuple& t, Object* ret,
			typename std::enable_if<!std::is_void<R>::value, R>::type* = nullptr)//deduce any type with out void
		{
			*ret = CallImpl<sizeof...(Args)>::call<R>(func, t);
		}

		template<class R, class ... Args, class F>
		static void call(F* func, typename F::Tuple& t, Object* ret,
			typename std::enable_if<std::is_void<R>::value, R>::type* = nullptr)//deduce void 
		{
			CallImpl<sizeof...(Args)>::call<R>(func, t);
		}
	};

	template<>
	struct CallImpl<0>
	{
		template<class R, class F>
		static R call(F* func, typename F::Tuple& para)
		{
			return func->call();
		}
	};

	template<>
	struct CallImpl<1>
	{
		template<class R, class F>
		static R call(F* func, typename F::Tuple& para)
		{
			return func->call(std::get<0>(para));
		}
	};

	template<>
	struct CallImpl<2>
	{
		template<class R, class F>
		static R call(F* func, typename F::Tuple& para)
		{
			return func->call(std::get<0>(para), std::get<1>(para));
		}
	};

	template<>
	struct CallImpl<3>
	{
		template<class R, class F>
		static R call(F* func, typename F::Tuple& para)
		{
			return func->call(std::get<0>(para), std::get<1>(para), std::get<2>(para));
		}
	};

	template<>
	struct CallImpl<4>
	{
		template<class R, class F>
		static R call(F* func, typename F::Tuple& para)
		{
			return func->call(std::get<0>(para), std::get<1>(para), std::get<2>(para), std::get<3>(para));
		}
	};


}

#endif