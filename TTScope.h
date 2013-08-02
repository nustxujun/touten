#ifndef _TTScope_H_
#define _TTScope_H_

#include "ToutenCommon.h"
#include "TTSymbolMap.h"
#include <stack>
#include "SharedPtr.h"
namespace TT
{
	class Scope
	{
	public :
		typedef SharedPtr<Scope> Ptr;
	public :
		Scope(Scope::Ptr parent);
		Scope::Ptr getParent()const;

		Symbol* getSymbol(const String& name);

	private:
		SymbolMap mSymbols;
		Scope::Ptr mParent;
	};

	class ScopeManager
	{
		static const String anonymous_scope;
		static const String global_scope;
	public :
		ScopeManager();
		~ScopeManager();

		Scope::Ptr enterScope(const String& name = anonymous_scope);
		Scope::Ptr leaveScope();

		Scope::Ptr getScope(const String& name);

		Symbol* getSymbol(const String& name);

	private:
		Scope::Ptr createScope();
		Scope::Ptr createScope(const String& name);
		void destroyScope(const String& name);

	private:
		typedef TTMap<String, Scope::Ptr> ScopeMap;
		ScopeMap mScopes;

		typedef std::stack<Scope::Ptr> ScopeStack;
		ScopeStack mStack;
	};
}

#endif