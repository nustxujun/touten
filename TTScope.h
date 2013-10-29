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
		struct SymbolObj
		{
			Symbol* sym;
			bool	local;
		};
	public :
		Scope(Scope::Ptr parent);
		Scope::Ptr getParent()const;

		SymbolObj getSymbol(const String& name);
		Symbol* createSymbol(const String& name, SymbolType st, AccessType at);
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
		Scope::Ptr getGlobal();
	private:
		Scope::Ptr createScope();
		Scope::Ptr createScope(const String& name);
		void destroyScope(const String& name);

	private:
		typedef TTMap<String, Scope::Ptr> ScopeMap;
		ScopeMap mScopes;

		typedef std::stack<Scope::Ptr> ScopeStack;
		ScopeStack mStack;
		Scope::Ptr mGlobal;
	};
}

#endif