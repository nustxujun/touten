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
		Touten();
		bool loadFile(const String& name);
		bool call(const String& name, size_t parasCount = 0, const ObjectPtr* paras = nullptr, Object* ret = nullptr);
		
		void registerFunction(const String& name, Functor* func);
		void registerOrRetrieveFunction(const String& name, Functor* func);
		bool isValidFunction(const String& name);// may not be existed or global;
		const Symbol* getSymbol(const String& name);

	private:
		void initInternalFunction();

	private:
		ScopeManager mScopemgr;
		StackBasedInterpreter mInterpreter;
		ConstantPool mConstPool;
	};
}

#endif