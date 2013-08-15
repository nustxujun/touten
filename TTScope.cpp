#include "TTScope.h"

using namespace TT;

Scope::Scope(Scope::Ptr parent):
	mParent(parent)
{

}

Scope::Ptr Scope::getParent()const
{
	return mParent;
}

Symbol* Scope::getSymbol(const String& name)
{
	Symbol* s = mSymbols.getSymbol(name);
	if (s == 0 && !mParent.isNull())
		return mParent->getSymbol(name);
	return s;
}

Symbol* Scope::createSymbol(const String& name, SymbolType st, AccessType at)
{
	return mSymbols.createSymbol(name, st, at);
}

const String ScopeManager::anonymous_scope = L"__anonymous_scope";
const String ScopeManager::global_scope = L"__global_scope";

ScopeManager::ScopeManager()
{
	mGlobal = new Scope(0);
	mScopes[global_scope] = mGlobal;
	mStack.push(mGlobal);
}

ScopeManager::~ScopeManager()
{
	
}

Scope::Ptr ScopeManager::enterScope(const String& name)
{
	Scope::Ptr s;
	if (name == anonymous_scope)
		s = createScope();
	else
	{
		s = getScope(name);
		if (s.isNull())
			s = createScope(name);
	}
	
	mStack.push(s);
	return s;
}

Scope::Ptr ScopeManager::leaveScope()
{
	mStack.pop();
	return mStack.top();
}


Scope::Ptr ScopeManager::getScope(const String& name)
{
	ScopeMap::iterator r = mScopes.find(name);
	if (r == mScopes.end())
	{
		return 0;
	}
	return r->second;
}

Scope::Ptr ScopeManager::getGlobal()
{
	return mGlobal;
}

Scope::Ptr ScopeManager::createScope()
{
	return new Scope(mStack.top());
}

Scope::Ptr ScopeManager::createScope(const String& name)
{
	Scope::Ptr s = createScope();
	std::pair<ScopeMap::iterator, bool> r = 
		mScopes.insert(ScopeMap::value_type(name, s));
	return r.first->second;
}

void ScopeManager::destroyScope(const String& name)
{
	mScopes.erase(name);
}