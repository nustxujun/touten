#include "TTScope.h"
#include "TTTools.h"
using namespace TT;

Scope::Scope(Scope::Ptr parent):
	mParent(parent)
{

}

Scope::Ptr Scope::getParent()const
{
	return mParent;
}

Scope::SymbolObj Scope::getSymbol(const String& name)
{
	SymbolObj obj;
	obj.local = true;
	obj.sym = mSymbols.getSymbol(name);
	if (obj.sym == 0 && !mParent.isNull())
	{
		obj = mParent->getSymbol(name);
		obj.local = false;
	}
	return obj;
}

Symbol* Scope::createSymbol(const String& name, SymbolType st, AccessType at)
{
	return mSymbols.createSymbol(name, st, at);
}

size_t Scope::getOffset()
{
	return mCodes.size();
}


Codes* Scope::getCode()
{
	return &mCodes;
}

size_t Scope::writeCode(const void* buff, size_t size)
{
	size_t head = getOffset();
	const char* cont = (const char*)buff;
	for (size_t i = 0; i < size ;++i)
		mCodes.push_back(*cont++);
	return head;
}

const Char* ScopeManager::anonymous_scope = L"__anonymous_scope";
const Char* ScopeManager::global_scope = L"__global_scope";

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
	{
		static int index = 1;
		
		s = createScope(name + Tools::toString(index++));
	}
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