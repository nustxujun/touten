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
	if (s == 0)
		return mParent->getSymbol(name);
	return s;
}



const String ScopeManager::anonymous_scope = L"__anonymous_scope";
const String ScopeManager::global_scope = L"__global_scope";

ScopeManager::ScopeManager()
{
	mStack.push(createScope(global_scope));
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

Symbol* ScopeManager::getSymbol(const String& name)
{
	return mStack.top()->getSymbol(name);
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