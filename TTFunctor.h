#ifndef _TTFunctor_H_
#define _TTFunctor_H_

#include "ToutenCommon.h"

namespace TT
{
	class Functor
	{
	public:
		virtual void operator()(const ObjectPtr* paras, int paracount, Object* ret) = 0;
	};

}

#endif