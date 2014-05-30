#ifndef _Touten_H_
#define _Touten_H_

#include "ToutenCommon.h"



namespace TT
{
	class ToutenExport Touten
	{
	public :
		Touten();
		~Touten();

		bool loadFile(const String& name);
		bool load(const Char* buffer);
		bool call(const String& name, size_t parasCount = 0, const ObjectPtr* paras = nullptr, Object* ret = nullptr);
		
		void registerFunction(const String& name, Functor* func);
		void registerOrRetrieveFunction(const String& name, Functor* func);
		bool isValidFunction(const String& name);// may not be existed or global;
		const Symbol* getSymbol(const String& name);

	private:
		void initInternalFunction();

	private:
		ScopeManager* mScopemgr;
		StackBasedInterpreter* mInterpreter;
		ConstantPool* mConstPool;
	};
}

#endif