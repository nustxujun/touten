#include "TTSymbolMap.h"

using namespace TT;

#define SYMBOLMAP_EXCEPT(t);

SymbolMap::~SymbolMap()
{
	std::for_each(mSymbols.begin(), mSymbols.end(), 
		[](Symbols::value_type& v){delete v.second;});
}

Symbol* SymbolMap::createSymbol(const String& name, SymbolType st, AccessType at)
{
	Symbol* s = createSymbolImpl(st);

	s->symtype = st;
	s->addrOffset = -1;
	s->actype = at;
	s->isdefine = false;

	auto result = mSymbols.insert(Symbols::value_type(name, s));
	if (!result.second)
	{
		delete s;
		SYMBOLMAP_EXCEPT("same name");
		return result.first->second;
	}

	return s;
}

Symbol* SymbolMap::getSymbol(const String& name)
{
	auto result = mSymbols.find(name);
	if (result == mSymbols.end())
		return 0;
	return result->second;
}

Symbol* SymbolMap::createSymbolImpl(SymbolType type)
{
	return new Symbol();
}