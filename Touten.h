#ifndef _Touten_H_
#define _Touten_H_

#include "ToutenCommon.h"

#include "TTScope.h"
#include "TTStackBasedInterpreter.h"

namespace TT
{
	class Touten
	{
	public :
		bool loadFile(const String& name);
		void call(const String& name, size_t parasCount = 0, const ObjectPtr* paras = nullptr, Object* ret = nullptr);
		
		void registerFunction(const String& name, Functor* func);
	private:
		ScopeManager mScopemgr;
		StackBasedInterpreter mInterpreter;
		ConstantPool mConstPool;
	};
}

#endif